/*
 * interrupt.c -
 */
#include <hardware.h>
#include <interrupt.h>
#include <io.h>
#include <libc.h>
#include <segment.h>
#include <types.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register idtR;

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
                   '9',  '0',  '\'', '¡',  '\0', '\0', 'q',  'w',  'e',  'r',
                   't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
                   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  'ñ',
                   '\0', 'º',  '\0', 'ç',  'z',  'x',  'c',  'v',  'b',  'n',
                   'm',  ',',  '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                   '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

/*****************************/
/* CUSTOM INTERRUPT HANDLERS */
/*****************************/

/* Overriding page fault handler function.
 * Desc:        Safely calls or_page_fault_routine in system mode.
 *              Gets invoked during when an Page Fault Exception occurs via the
 *              IDT. Occupies the 14th position in the IDT.
 * Implemented: see entry.S.
 */
void or_page_fault_handler();

/* Overriding keyboard handler function.
 * Desc:        Safely calls or_keyboard_handler in system mode.
 *              Gets asyncronously invoked during when the PIC raises the
 *              keyboard interrupt. Occupies the 33th position in the IDT.
 * Implemented: see entry.S.
 */
void or_keyboard_handler();

/* Overriding system call handler handler function.
 * Desc:        Safely calls a system function in system mode.
 *              Gets invoked via trap from user code in order to execute certain
 *              functions with elavated system privilages.
 *              Occupies the 128th (0x80) position in the IDT.
 * Params:      No standard C parameters. However a certain CPU state is
 *              required.
 *              - function_id: the position of the target function within the
 *                             sys_call_table (see sys_call_table.S).
 *              - function_args: the arguments of the target function. See
 *                               entry.S.
 * Return:      Returns the return value of the target function or ENOSYS if
 *              said function does not exist.
 * Implemented: see entry.S.
 */
void sys_call_handler();

/* Overriding system call handler handler function.
 * Desc:        Safely calls a system function in system mode.
 *              Gets invoked via sysenter from user code in order to execute
 *              certain functions with elavated system privilages.
 *              Since it doesn't use the IDT it is not initialized there,
 *              instead MSRs are used to indicate this function's address.
 * Params:      No standard C parameters. However a certain CPU state is
 *              required which indicates the following parameters:
 *              - function_id: the position of the target function within the
 *                             sys_call_table (see sys_call_table.S).
 *              - function_args: the arguments of the target function. See
 *                               entry.S.
 * Return:      Returns the return value of the target function or ENOSYS if
 *              said function does not exist.
 * Implemented: see entry.S.
 */
void fast_sys_call_handler();

/*****************************/
/* CUSTOM INTERRUPT ROUTINES */
/*****************************/

/* Routine for Page Fault Exception.
 * Desc:    Prints to the screen "Page Fault Error (@errno) at %eip = @eip".
 *          This function should only be invoked by or_page_fault_handler.
 * Params:  - errno: The hardware error number, which describes better the type
 *                   of error encountered.
 *          - eip: The value of the %eip register at exception time and
 *                 consequently the memory address of the assembly line that
 *                 caused the error.
 * Return:  Currently doesn't return, instead opting to hang the system (will be
 *          changed).
 * Note:    Could be changed to use more robust printing systems.
 */
void or_page_fault_routine(int errno, int eip) {
  char str[11]; // this variable allocates 11 chars to the stack.
                // % eip (32 bit positive ) can have 10 digits in base 10 + 1
                // for \0.

  printk("\nPage Fault Error (");

  itoa(errno, str);
  printk(str);

  printk(") at %eip = ");

  itoa(eip, str);
  printk(str);

  printk(")\n");

  // Hang the system
  while (1)
    ;
}

/* Routine for Keyboard Interruption.
 * Desc:  Sets the character in the (0, 0) position in the screen buffer to
 *        the character corresponding to the value of the key press (see
 *        char_map).
 */
void or_keyboard_routine() {
  unsigned char key_code = inb(0x60);

  if (!(key_code & 0x80)) {
    printc_xy(0, 0, char_map[key_code]);
  }
}

void setInterruptHandler(int vector, void (*handler)(),
                         int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  // flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void setIdt() {
  /* Program interrups/exception service routines */
  idtR.base = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(14, or_page_fault_handler, 0);
  setInterruptHandler(33, or_keyboard_handler, 0);

  setTrapHandler(0x80, sys_call_handler, 3);

  // To Enable Interrupts see hardware.S

  set_idt_reg(&idtR);

  // Enable Fast System Calls
  writeMSR(0x174, (unsigned long)__KERNEL_CS);
  writeMSR(0x175, INITIAL_ESP);
  writeMSR(0x176, (unsigned long)fast_sys_call_handler);
}
