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

/*
 * System Call Implementation for write.
 * @fd:     Target file descriptor (1 always for now (console)).
 * @buffer: char buffer containing string to be printed.
 * @size:   size of the buffer.
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
  if (size <= 0)
    return -27; /* EFBIG (Since real linux write uses unsigned) */

  char ker_buff[size];
  if ((ret = copy_from_user(buffer, ker_buff, size)) < 0)
    return ret;

  sys_write_console(ker_buff, size);

  return ret;
}
