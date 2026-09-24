/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions) {
  if (fd != 1)
    return -9; /*EBADF*/
  if (permissions != ESCRIPTURA)
    return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall() { return -38; /*ENOSYS*/ }

/* System Call Implementation for write.
 * Desc:    If parameters are valid the contents up to size of the buffer are
 *          written to the target fd. Currently only fd 1 if valid and it prints
 *          to the screen.
 * Params:  - fd: Target file descriptor. Only fd 1 is valid and it represents
 *                the screen device.
 *          - buffer: Pointer to the contents to be written.
 *          - size: Amount of characters that will be written.
 * Returns: 0 on success, error code otherwise.
 * Note:    Might be good practise copying first the buffer using
 *          copy_from_user, however it would also be expensive on stack memory
 *          size.
 */
int sys_write(int fd, char *buffer, int size) {
  int ret;

  if ((ret = check_fd(fd, ESCRIPTURA)) < 0)
    return ret;
  if (buffer == NULL)
    return -14; /* EFAULT */
  if (size < 0)
    return -27; /* EFBIG (Since real linux write uses unsigned) */

  char ker_buff[size];
  if ((ret = copy_from_user(buffer, ker_buff, size)) < 0)
    return ret;

  sys_write_console(ker_buff, size);

  return ret;
}
