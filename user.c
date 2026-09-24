#include <libc.h>

char buff[24] = "\nHello World";

int pid;

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  // Uncomment the following line to cause a Page Fault Error with errno 6
  // *((int *)0x00) = 0;

  if (fast_write(1, buff, strlen(buff)) < 0)
    perror();
  while (1) {
  }
}
