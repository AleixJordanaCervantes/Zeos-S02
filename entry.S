/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

// Restores context from before the interrupt event
#define RESTORE_ALL \
	popl %edx; \
	popl %ecx; \
	popl %ebx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	popl %ds; \
	popl %es; \
	popl %fs; \
	popl %gs


#define EOI         \
  movb $0x20, % al; \
  outb % al, $0x20;

/* Overriding page fault handler function in x86 asm.
 * C Header:  void or_page_handler();
 * Desc:      Safely calls or_page_fault_routine in system mode.
 * Params:    No standard C parameters. However it does receive via hardware an error code.
 * Return:    It doesn't. Hangs the system.
 */
ENTRY(or_page_fault_handler)
	SAVE_ALL
	pushl 0x30(%esp)
	pushl 0x30(%esp)
	call or_page_fault_routine
	RESTORE_ALL
	addl $8, %esp
	iret

/* Overriding keyboard interrupt handler in x86.
 * C Header:  void or_keyboard_handler();
 * Desc:      Safely calls or_keyboard_routine in system mode.
 */
ENTRY(or_keyboard_handler)
  SAVE_ALL
  EOI
  call or_keyboard_routine
  RESTORE_ALL
  iret

/* Overriding sys call interruption (software int).
 * C Header:  void sys_call_handler();
 * Desc:      Calls a target function with system privileges. Said function needs to be assigned an id via the sys_call_table (see sys_call_table.S).
 * Params:    Does not recieve standard C parameters. Instead via CPU registers it receives:
 *              - function_id: Found in %eax, contains the position in the sys_call_table array where the target function is found.
 *              - function_args: Found in %edx, %ecx, %ebx, %esi, %edi, %ebp; from left to right the parameters of the target function.
 *                               Automatically creates target function's parameters with SAVE_ALL macro.
 * Return:    Returns the return value of the target system call or ENOSYS if system call identified by function_id does not exist.
 */
ENTRY(sys_call_handler)
  SAVE_ALL
  cmp $0, %eax                    /* Check %eax validity: %eax [0, MAX_SYSCALL] (max 4 syscalls for now) */
  jl err
  cmp $MAX_SYSCALL, %eax
  jge err
  call *sys_call_table(, %eax, 4) /* call sys_call_table[%eax*4] (mem addresses ocupy 4B) */
  movl %eax, 0x18(%esp)             /* Save %eax (ret value) for use after RESTORE_ALL */
  jmp end
err:
  movl $-38, 0x18(%esp)
end:
  RESTORE_ALL
  iret

/* Handler for fast system calls. Differs from sys_call_handler in that MSRs are used to locate it instead of the IDT.
 * C Header:  void fast_sys_call_handler();
 * Desc:      Calls a target function with system privileges. Said function needs to be assigned an id via the sys_call_table (see sys_call_table.S).
 * Params:    Does not recieve standard C parameters. Instead via CPU registers it receives:
 *              - function_id: Found in %eax, contains the position in the sys_call_table array where the target function is found.
 *              - function_args: Found in %edx, %ecx, %ebx, %esi, %edi, %ebp; from left to right the parameters of the target function.
 *                               Automatically creates target function's parameters with SAVE_ALL macro.
 * Return:    Returns the return value of the target system call or ENOSYS if system call identified by function_id does not exist.
 */
ENTRY(fast_sys_call_handler)
  pushl $__USER_DS                /* Save basic user context */
  pushl %ebp
  pushfl
  pushl $__USER_CS
  pushl 4(%ebp)
  SAVE_ALL
  cmp $0, %eax                    /* Check %eax validity: %eax [0, MAX_SYSCALL] (max 4 syscalls for now) */
  jl fast_err
  cmp $MAX_SYSCALL, %eax
  jge fast_err
  call *sys_call_table(, %eax, 4) /* call sys_call_table[%eax*4] (mem addresses ocupy 4B) */
  movl %eax, 0x18(%esp)           /* Save %eax (ret value) for use after RESTORE_ALL */
  jmp fast_end
fast_err:
  movl $-38, 0x18(%esp)
fast_end:
  RESTORE_ALL
  movl (%esp), %edx
  movl 12(%esp), %ecx
  sti
  sysexit
