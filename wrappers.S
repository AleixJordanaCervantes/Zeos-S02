/*
 * wrappers.S
 * Contains wrapper code for all system calls
 * User Only Code
 */
#include <asm.h>

/* Wrapper for write syscall:
 * C Header: int write(int fd, char * buf, int size)
 * @fd    = 8   (%ebp)
 * @buf   = 12  (%ebp)
 * @size  = 16  (%ebp)
 */
ENTRY(write)
  pushl %ebp            /* Standard stack init */
  movl %esp, %ebp
  pushl %ebx            /* Save Registers ebx, ecx, edx */
  pushl %ecx
  pushl %edx
  movl 8(%ebp), %edx    /* Parameter Passing */
  movl 12(%ebp), %ecx
  movl 16(%ebp), %ebx
  movl $4, %eax         /* syscall routine id */
  int $0x80             /* trap */
  popl %edx             /* Restore Regs */
  popl %ecx
  popl %edx
  popl %ebp
  test %eax, %eax       /* Error Check and Return */
  jns else
  negl %eax
  movl %eax, errno
  movl $-1, %eax
else:
  ret

