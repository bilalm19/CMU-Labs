# Attack Lab
This is an x86-64 attack lab for self-study students.

I also document what I had to do to solve this lab.

## Tools
- GDB
- objdump

## Level 1
This is a simple buffer overflow exploit. You can see the dump of the ctarget binary [here](./dump_ctarget.txt). We need to exploit the vulnerability in [getbuf](./dump_ctarget.txt#L777). The first instruction is the key here. `sub $0x28,%rsp` means we will need to provide 40 bytes before we can modify the return address located in the stack. In the 40th byte, we will give the address of the function [touch1](./dump_ctarget.txt#L787) in little endian format (`c0 17 40 00 00 00 00 00`). Hence, we get the following answer:
```
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 c0 17 40 00 00 00 00 00
```
