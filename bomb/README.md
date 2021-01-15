# Bomb Lab
This is an x86-64 bomb for self-study students.

I also document what I had to do to solve this lab.

## Tools
- GDB
- objdump
- strings

## Phase 1
By disassembling the `bomb` binary file using gdb and/or objdump, the following asm code can be seen for function `phase_1`:
```asm
1:  0000000000400ee0 <phase_1>:
2:    400ee0:	48 83 ec 08          	sub    $0x8,%rsp
3:    400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
4:    400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>
5:    400eee:	85 c0                	test   %eax,%eax
6:    400ef0:	74 05                	je     400ef7 <phase_1+0x17>
7:    400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
8:    400ef7:	48 83 c4 08          	add    $0x8,%rsp
9:    400efb:	c3                   	retq
```
In line 4, the function `strings_not_equal` is being called. Before that, in line 3, the register `%esi` is being populated with some value that is stored in address `0x402400`. By printing the value using the gdb command `x/s` (print in string), we get the string we need to solve phase 1. The string is:
```bash
Border relations with Canada have never been better.
```
You can get this string by using the `strings` tool as well.

## Phase 2
The asm code for the function `phase_2` is given below:
```asm
1:  0000000000400efc <phase_2>:
2:    400efc:	55                   	push   %rbp
3:    400efd:	53                   	push   %rbx
4:    400efe:	48 83 ec 28          	sub    $0x28,%rsp
5:    400f02:	48 89 e6             	mov    %rsp,%rsi
6:    400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>
7:    400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
8:    400f0e:	74 20                	je     400f30 <phase_2+0x34>
9:    400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
10:   400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
11:   400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
12:   400f1a:	01 c0                	add    %eax,%eax
13:   400f1c:	39 03                	cmp    %eax,(%rbx)
14:   400f1e:	74 05                	je     400f25 <phase_2+0x29>
15:   400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
16:   400f25:	48 83 c3 04          	add    $0x4,%rbx
17:   400f29:	48 39 eb             	cmp    %rbp,%rbx
18:   400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
19:   400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
20:   400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
21:   400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
22:   400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
23:   400f3c:	48 83 c4 28          	add    $0x28,%rsp
24:   400f40:	5b                   	pop    %rbx
25:   400f41:	5d                   	pop    %rbp
26:   400f42:	c3                   	retq
```
Notice in line 6, there's a function `read_six_numbers` being called. Its asm is as follows:
```asm
1:  000000000040145c <read_six_numbers>:
2:    40145c:	48 83 ec 18          	sub    $0x18,%rsp
3:    401460:	48 89 f2             	mov    %rsi,%rdx
4:    401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx
5:    401467:	48 8d 46 14          	lea    0x14(%rsi),%rax
6:    40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp)
7:    401470:	48 8d 46 10          	lea    0x10(%rsi),%rax
8:    401474:	48 89 04 24          	mov    %rax,(%rsp)
9:    401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9
10:   40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8
11:   401480:	be c3 25 40 00       	mov    $0x4025c3,%esi
12:   401485:	b8 00 00 00 00       	mov    $0x0,%eax
13:   40148a:	e8 61 f7 ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
14:   40148f:	83 f8 05             	cmp    $0x5,%eax
15:   401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
16:   401494:	e8 a1 ff ff ff       	callq  40143a <explode_bomb>
17:   401499:	48 83 c4 18          	add    $0x18,%rsp
18:   40149d:	c3                   	retq
```
The function `phase_2` uses `read_six_numbers` to check if the input passed contains at least 6 integers. The integers need to be separated by a space. This can be seen in line 11-14, where the value in address `0x4025c3` is `%d %d %d %d %d %d` and is being passed as the second argument to `scanf` using register `%esi`. If the returned value of `scanf` (stored in register %eax) is greater than 5, it will not explode the bomb. **Note that we only need to pass 6 integers. Phase 2 will not check anything beyond that.**

When the program returns back to `phase_2`, it'll be at line 7: `cmpl $0x1,(%rsp)`. This is comparing the value in register `%rsp` with hex value `0x1`. What the comparison is, is given in the next line: `je 400f30 <phase_2+0x34>`. This line says if the value in `%rsp == 0x1` then move to line with address `400f30`, i.e. line 20 in function `phase_2`. This comparison is significant for us as this is the first integer we needed. So we have:
```bash
1 %d %d %d %d %d
```
We now need to know the rest of the 5 integers to solve this phase.

We continue to proceed from line 20. Line 20-22 basically states to load the resulting address of operation `0x4(%rsp)` and `0x18(%rsp)` to registers `%rbx` and `%rbp` respectively, and the jump to line 11: `mov -0x4(%rbx),%eax`. Notice that the hex value `0x18` is equal to `24` in decimal. An integer is `0x4` bytes and storing 6 integers means a total of `6 * 0x4 = 0x18` bytes. From this, we get that the addresses `0x4(%rsp)` and `0x18(%rsp)` are the addresses of the second and last indexes of the array of numbers we had passed as input to this phase. Hence `0x4(%rsp) == &array_of_numbers[1]` and `0x18(%rsp) == &array_of_numbers[5]`. So the register `%rbx` stores the value of the address of `array_of_numbers[1]` that points to the second number we had entered in the input.

Let's look at line 11-19. These lines can be summarized as:
```asm
; i = 1, 2, 3, 4, 5

11: mov    -0x4(%rbx),%eax           ; move array_of_numbers[i-1] to register %eax
12: add    %eax,%eax                 ; 2 * value in %eax and store it in %eax
13: cmp    %eax,(%rbx)               ; compare %eax and array_of_numbers[i]
14: je     400f25 <phase_2+0x29>     ; jump to line 16 if value in %eax is equal to array_of_numbers[i]
15: callq  40143a <explode_bomb>     ; else explode bomb
16: add    $0x4,%rbx                 ; move to next index of array_of_numbers or array_of_numbers[i+1]
17: cmp    %rbp,%rbx                 ; compare values in %rbp and %rbx
18: jne    400f17 <phase_2+0x1b>     ; jump to line 11 of phase_2 if values in %rbp and %rbx are not equal
19: jmp    400f3c <phase_2+0x40>     ; else jump to line 23 of phase_2
```
This is essentially a loop, iterating over an array. The loop ends at the last element of array. Here, the lines 12-13 are important because they tell us what the solution is. From the index 1-5, the numbers need to be twice the number in the previous index. So the solution is:
```asm
1 2 4 8 16 32
```

## Phase 3
This is a really simple phase. Below is the asm of `phase_3`:
```asm
1:  0000000000400f43 <phase_3>:
2:    400f43:	48 83 ec 18          	sub    $0x18,%rsp
3:    400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
4:    400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
5:    400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
6:    400f56:	b8 00 00 00 00       	mov    $0x0,%eax
7:    400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
8:    400f60:	83 f8 01             	cmp    $0x1,%eax
9:    400f63:	7f 05                	jg     400f6a <phase_3+0x27>
10:   400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
11:   400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
12:   400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
13:   400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
14:   400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)
15:   400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
16:   400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
17:   400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
18:   400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
19:   400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
20:   400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
21:   400f91:	b8 85 01 00 00       	mov    $0x185,%eax
22:   400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
23:   400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
24:   400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
25:   400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
26:   400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
27:   400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
28:   400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
29:   400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
30:   400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
31:   400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
32:   400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
33:   400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
34:   400fc2:	74 05                	je     400fc9 <phase_3+0x86>
35:   400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
36:   400fc9:	48 83 c4 18          	add    $0x18,%rsp
37:   400fcd:	c3                   	retq
```
This phase has several solutions but they follow a certain rule. In line 7: `callq 400bf0 <__isoc99_sscanf@plt>`, the `scanf` function is being called. From [phase_2](#Phase-2), we know that this function was used to check if the input follow an expected format and also to return the number of input arguments. The expected format can be gotten by printing the string (gdb command `x/s`) in address `0x4025cf` (see line 5), which is `%d %d`. So the input needs to have two integers values. Note that the input is stored in an array, which is pushed to the stack. The values are in `0x8(%rsp)` and `0xc(%rsp)` (see lines 3-4).

Now we examine lines 11-12. The lines state if the first value is greater than 7, explode the bomb. This means our first integer needs to be less than 8. It might seem trivial to choose any number less than 8, however, this is not the case. We shall soon see why.

Lines 13-14 state to store the first integer input into register `%eax` and jump to line with address stored in address `%rax*8+*0x402470` (address in address). This address comes from a **jump table**, which is given below for the address 0x402470:
```bash
(gdb) x/8gx 0x402470
0x402470:       0x0000000000400f7c    0x0000000000400fb9
0x402480:       0x0000000000400f83    0x0000000000400f8a
0x402490:       0x0000000000400f91    0x0000000000400f98
0x4024a0:       0x0000000000400f9f    0x0000000000400fa6
```
If we look at all these lines with addresses shown in the above jump table (15, 32, 17, 19, 21, 23, 25, 27), we get a list of possible second integers. This second integer is directly influenced by what we chose as the first integer. Hence the list of possible solutions are:
```bash
0 207
1 311
2 707
3 256
4 389
5 206
6 682
7 327
```

## Phase 4
This phase has some similarities to phase 4. We look at two functions here; `phase_4` and `func4`, whose asm is given below:
```asm
1:  0000000000400fce <func4>:
2:    400fce:	48 83 ec 08          	sub    $0x8,%rsp
3:    400fd2:	89 d0                	mov    %edx,%eax
4:    400fd4:	29 f0                	sub    %esi,%eax
5:    400fd6:	89 c1                	mov    %eax,%ecx
6:    400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
7:    400fdb:	01 c8                	add    %ecx,%eax
8:    400fdd:	d1 f8                	sar    %eax
9:    400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
10:   400fe2:	39 f9                	cmp    %edi,%ecx
11:   400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
12:   400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
13:   400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
14:   400fee:	01 c0                	add    %eax,%eax
15:   400ff0:	eb 15                	jmp    401007 <func4+0x39>
16:   400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
17:   400ff7:	39 f9                	cmp    %edi,%ecx
18:   400ff9:	7d 0c                	jge    401007 <func4+0x39>
19:   400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
20:   400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
21:   401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
22:   401007:	48 83 c4 08          	add    $0x8,%rsp
23:   40100b:	c3                   	retq   

1:  000000000040100c <phase_4>:
2:    40100c:	48 83 ec 18          	sub    $0x18,%rsp
3:    401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
4:    401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
5:    40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
6:    40101f:	b8 00 00 00 00       	mov    $0x0,%eax
7:    401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
8:    401029:	83 f8 02             	cmp    $0x2,%eax
9:    40102c:	75 07                	jne    401035 <phase_4+0x29>
10:   40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
11:   401033:	76 05                	jbe    40103a <phase_4+0x2e>
12:   401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
13:   40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
14:   40103f:	be 00 00 00 00       	mov    $0x0,%esi
15:   401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
16:   401048:	e8 81 ff ff ff       	callq  400fce <func4>
17:   40104d:	85 c0                	test   %eax,%eax
18:   40104f:	75 07                	jne    401058 <phase_4+0x4c>
19:   401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
20:   401056:	74 05                	je     40105d <phase_4+0x51>
21:   401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
22:   40105d:	48 83 c4 18          	add    $0x18,%rsp
23:   401061:	c3                   	retq
```
From lines 3-4, we gather that our input needs to be two arguments long and are stored in addresses `0x8(%rsp)` and `0x8(%rsp)` in the stack. By printing out the string stored in the address `0x4025cf` (line 5), we gather that the required format of our input needs to be `%d %d`, i.e. two integers.

Lines 10-11 shows us that our first integer argument needs to be less than or equal to `14`. But the first integer isn't so obvious here because it is used in `func4` and the return value from this function needs to be equal to `0` (lines 17-18).

`func4` is a recursive function that has 3 function parameters. The first parameter is our first integer input; the second is `0x0` and the third is `0xe`. If we look at lines 17-18, the function `func4` needs to return `0`. This means the first integer input we provide has to make `func4` return `0`. From tedious experimentation, I found that `0`, `1`, `3` and `7` satisfy this.

Lines 19-20 tells us our second integer argument needs to be equal to `0`.

The list of possible solutions are:
```bash
0 0
1 0
3 0
7 0
```

## Phase 5
The assembly for `phase_5` function is given below:
```asm
1:  0x0000000000401062 <phase_5>:
2:    0x0000000000401062 <+0>:     push   %rbx
3:    0x0000000000401063 <+1>:     sub    $0x20,%rsp
4:    0x0000000000401067 <+5>:     mov    %rdi,%rbx
5:    0x000000000040106a <+8>:     mov    %fs:0x28,%rax
6:    0x0000000000401073 <+17>:    mov    %rax,0x18(%rsp)
7:    0x0000000000401078 <+22>:    xor    %eax,%eax
8:    0x000000000040107a <+24>:    callq  0x40131b <string_length>
9:    0x000000000040107f <+29>:    cmp    $0x6,%eax
10:   0x0000000000401082 <+32>:    je     0x4010d2 <phase_5+112>
11:   0x0000000000401084 <+34>:    callq  0x40143a <explode_bomb>
12:   0x0000000000401089 <+39>:    jmp    0x4010d2 <phase_5+112>
13:   0x000000000040108b <+41>:    movzbl (%rbx,%rax,1),%ecx
14:   0x000000000040108f <+45>:    mov    %cl,(%rsp)
15:   0x0000000000401092 <+48>:    mov    (%rsp),%rdx
16:   0x0000000000401096 <+52>:    and    $0xf,%edx
17:   0x0000000000401099 <+55>:    movzbl 0x4024b0(%rdx),%edx
18:   0x00000000004010a0 <+62>:    mov    %dl,0x10(%rsp,%rax,1)
20:   0x00000000004010a4 <+66>:    add    $0x1,%rax
21:   0x00000000004010a8 <+70>:    cmp    $0x6,%rax
22:   0x00000000004010ac <+74>:    jne    0x40108b <phase_5+41>
23:   0x00000000004010ae <+76>:    movb   $0x0,0x16(%rsp)
24:   0x00000000004010b3 <+81>:    mov    $0x40245e,%esi
26:   0x00000000004010b8 <+86>:    lea    0x10(%rsp),%rdi
27:   0x00000000004010bd <+91>:    callq  0x401338 <strings_not_equal>
28:   0x00000000004010c2 <+96>:    test   %eax,%eax
29:   0x00000000004010c4 <+98>:    je     0x4010d9 <phase_5+119>
30:   0x00000000004010c6 <+100>:   callq  0x40143a <explode_bomb>
31:   0x00000000004010cb <+105>:   nopl   0x0(%rax,%rax,1)
32:   0x00000000004010d0 <+110>:   jmp    0x4010d9 <phase_5+119>
33:   0x00000000004010d2 <+112>:   mov    $0x0,%eax
34:   0x00000000004010d7 <+117>:   jmp    0x40108b <phase_5+41>
35:   0x00000000004010d9 <+119>:   mov    0x18(%rsp),%rax
36:   0x00000000004010de <+124>:   xor    %fs:0x28,%rax
37:   0x00000000004010e7 <+133>:   je     0x4010ee <phase_5+140>
38:   0x00000000004010e9 <+135>:   callq  0x400b30 <__stack_chk_fail@plt>
39:   0x00000000004010ee <+140>:   add    $0x20,%rsp
40:   0x00000000004010f2 <+144>:   pop    %rbx
41:   0x00000000004010f3 <+145>:   retq
```
First requirement of the input we pass is that it needs to be a string of length 6. This can be seen at lines 8-11; if string input string length is equal to 6, do not explode bomb.

Our input string will be iterated through in a loop (see lines 13-21). The ascii value of each of the characters in the string will be anded with `0xf`. The resulting value will then be used as an index to the string stored in address `0x4024b0`; the value stored in this array is `maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?`. The character on that index will then be stored in a new string, and then this string will be passed to the function `strings_not_equal` (line 25). The string to which this will be compared is stored in address `0x0x40245e` (line 23), and its value is `flyers`.

Based on this information, we need 6 characters whose ascii values' last 4 bits should be:
```bash
1001  0x9
1111  0xf
1110  0xe
0101  0x5
0110  0x6
0111  0x7
```
From the ascii table, we can determine that the characters we need are:
```bash
ionefg
```

## Phase 6
Below is the assembly for phase_6:
```asm
1:  0x00000000004010f4 <phase_6>:
2:    0x00000000004010f4 <+0>:     push   %r14
3:    0x00000000004010f6 <+2>:     push   %r13
4:    0x00000000004010f8 <+4>:     push   %r12
5:    0x00000000004010fa <+6>:     push   %rbp
6:    0x00000000004010fb <+7>:     push   %rbx
7:    0x00000000004010fc <+8>:     sub    $0x50,%rsp
8:    0x0000000000401100 <+12>:    mov    %rsp,%r13
9:    0x0000000000401103 <+15>:    mov    %rsp,%rsi
10:   0x0000000000401106 <+18>:    callq  0x40145c <read_six_numbers>
11:   0x000000000040110b <+23>:    mov    %rsp,%r14
12:   0x000000000040110e <+26>:    mov    $0x0,%r12d
13:   0x0000000000401114 <+32>:    mov    %r13,%rbp
14:   0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
15:   0x000000000040111b <+39>:    sub    $0x1,%eax
16:   0x000000000040111e <+42>:    cmp    $0x5,%eax
17:   0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
18:   0x0000000000401123 <+47>:    callq  0x40143a <explode_bomb>
19:   0x0000000000401128 <+52>:    add    $0x1,%r12d
20:   0x000000000040112c <+56>:    cmp    $0x6,%r12d
21:   0x0000000000401130 <+60>:    je     0x401153 <phase_6+95>
22:   0x0000000000401132 <+62>:    mov    %r12d,%ebx
23:   0x0000000000401135 <+65>:    movslq %ebx,%rax
24:   0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax
25:   0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
26:   0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
27:   0x0000000000401140 <+76>:    callq  0x40143a <explode_bomb>
28:   0x0000000000401145 <+81>:    add    $0x1,%ebx
29:   0x0000000000401148 <+84>:    cmp    $0x5,%ebx
30:   0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
31:   0x000000000040114d <+89>:    add    $0x4,%r13
32:   0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
33:   0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi
34:   0x0000000000401158 <+100>:   mov    %r14,%rax
35:   0x000000000040115b <+103>:   mov    $0x7,%ecx
36:   0x0000000000401160 <+108>:   mov    %ecx,%edx
37:   0x0000000000401162 <+110>:   sub    (%rax),%edx
38:   0x0000000000401164 <+112>:   mov    %edx,(%rax)
39:   0x0000000000401166 <+114>:   add    $0x4,%rax
40:   0x000000000040116a <+118>:   cmp    %rsi,%rax
41:   0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
42:   0x000000000040116f <+123>:   mov    $0x0,%esi
43:   0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>
44:   0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx
45:   0x000000000040117a <+134>:   add    $0x1,%eax
46:   0x000000000040117d <+137>:   cmp    %ecx,%eax
47:   0x000000000040117f <+139>:   jne    0x401176 <phase_6+130>
48:   0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148>
49:   0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
50:   0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)
51:   0x000000000040118d <+153>:   add    $0x4,%rsi
52:   0x0000000000401191 <+157>:   cmp    $0x18,%rsi
53:   0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
54:   0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx
55:   0x000000000040119a <+166>:   cmp    $0x1,%ecx
56:   0x000000000040119d <+169>:   jle    0x401183 <phase_6+143>
57:   0x000000000040119f <+171>:   mov    $0x1,%eax
58:   0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
59:   0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
60:   0x00000000004011ab <+183>:   mov    0x20(%rsp),%rbx
61:   0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax
62:   0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi
63:   0x00000000004011ba <+198>:   mov    %rbx,%rcx
64:   0x00000000004011bd <+201>:   mov    (%rax),%rdx
65:   0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)
66:   0x00000000004011c4 <+208>:   add    $0x8,%rax
67:   0x00000000004011c8 <+212>:   cmp    %rsi,%rax
68:   0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
69:   0x00000000004011cd <+217>:   mov    %rdx,%rcx
70:   0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
71:   0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
72:   0x00000000004011da <+230>:   mov    $0x5,%ebp
73:   0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
74:   0x00000000004011e3 <+239>:   mov    (%rax),%eax
75:   0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
76:   0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250>
77:   0x00000000004011e9 <+245>:   callq  0x40143a <explode_bomb>
78:   0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
79:   0x00000000004011f2 <+254>:   sub    $0x1,%ebp
80:   0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235>
81:   0x00000000004011f7 <+259>:   add    $0x50,%rsp
82:   0x00000000004011fb <+263>:   pop    %rbx
83:   0x00000000004011fc <+264>:   pop    %rbp
84:   0x00000000004011fd <+265>:   pop    %r12
85:   0x00000000004011ff <+267>:   pop    %r13
86:   0x0000000000401201 <+269>:   pop    %r14
87:   0x0000000000401203 <+271>:   retq
```
Observing line 6, we get that our input needs to be 6 numbers that are separated by spaces.

Lines 13-32 signify two nested loops. Lines 14-18 places a condition on each of our numbers; the numbers, when subtracted by `1` each, should not be greater than 5 (otherwise it will explode bomb). So all our numbers need to be less than or equal to 6.

The Lines 23-30 show a loop which is inspects if any of our numbers are identical. If identical, it will explode bomb. Hence, our numbers should not be equal.

The lines 36-41 is another loop. What this is doing is in each iteration, it is subtracting each of our input from 7 and storing all the results to another array.

Lines 44-59 is iterating over a the new array created in the loop shown in lines 36-41. Notice the special address `0x6032d0` (lines 49 and 58). This signifies a linked list. We can print its jump table and see its contents:
```asm
x/12x 0x6032d0
0x6032d0 <node1>:       0x000000010000014c      0x00000000006032e0
0x6032e0 <node2>:       0x00000002000000a8      0x00000000006032f0
0x6032f0 <node3>:       0x000000030000039c      0x0000000000603300
0x603300 <node4>:       0x00000004000002b3      0x0000000000603310
0x603310 <node5>:       0x00000005000001dd      0x0000000000603320
0x603320 <node6>:       0x00000006000001bb      0x0000000000000000
```
The second column above represents the numbers stored in each node while the third column represents the pointer to the next node. Hence, the node struct should look like this:
```C
struct node {
    int numb;
    struct node* next;
};
```
Please note that we will look at the numbers as 32 bits rather than the full 64 bits, so the numbers, shown in the second column in the jump table of `0x6032d0`, are actually:
```asm
0x14c    322
0xa8     168
0x39c    924
0x2b3    691
0x1dd    477
0x1bb    443
```
The lines 44-59 is essentially rearranging the order of the linkedlist (and probably storing it in an array of nodes), and the order is effected by the new array. Therefore, our input effects this order.

Lines 73-80 roughly translates to the following C code:
```c
node node_arry[6]; // array of node that stores the values of all the nodes that are in the linked list.
for (int i = 0; i < 5; i++) {
    if (node_arry[i] < node_arry [i+1])
      explode_bomb();
}
```
This means the array of nodes, `node_arry`, needs to be in descending order:
```asm
node3: 924
node4: 691
node5: 477
node6: 443
node1: 322
node2: 168
```

From this, we gather that our input will be used to rearrange the values in the linked list, and the arrangement needs to be in descending order. So the flow looks like this:
- Read the input and check if it contains 6 numbers.
- Check if all the numbers are less than 7.
- Subtract each input from 7 and store the result in an array. Lets call this array `sub7array`.
- `sub7array` will be used to order the linked list in descending order. Each element in this array signifies the position of the node.

Using this logic we produce the following input:
```bash
4 3 2 1 6 5
```
From above input, we will get following `sub7array`:
```
3 4 5 6 1 2
```
This is the order of nodes in descending order. The lines 44-70 are essentially rearranging the linkedlist into the order specified in the `sub7array` array.

## Secret Phase
