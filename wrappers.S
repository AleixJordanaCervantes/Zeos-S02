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
 * Note: May be innecessary to save register content for ecx and edx since they are not protected registers that need to be saved in functions.
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

/* Wrapper for write syscall using fast system calls:
 * C Header: int fast_write(int fd, char * buf, int size)
 * @fd    = 8   (%ebp)
 * @buf   = 12  (%ebp)
 * @size  = 16  (%ebp)
 * Note: May be innecessary to save register content for ecx and edx since they are not protected registers that need to be saved in functions.
 */
ENTRY(fast_write)
  pushl %ebp
  movl %esp, %ebp
  pushl %ebx            /* Save Registers ebx, ecx, edx */
  pushl %ecx
  pushl %edx
  movl 8(%ebp), %edx    /* Parameter Passing */
  movl 12(%ebp), %ecx
  movl 16(%ebp), %ebx
  movl $4, %eax         /* syscall routine id */
  pushl $ret             /* Save @RET */
  pushl %ebp
  movl %esp, %ebp
  sysenter
ret:
  movl %ebp, %esp
  popl %ebp
  add $4, %esp
  popl %edx             /* Restore Regs */
  popl %ecx
  popl %edx
  popl %ebp
  test %eax, %eax       /* Error Check and Return */
  jns else_1
  negl %eax
  movl %eax, errno
  movl $-1, %eax
else_1:
  ret
