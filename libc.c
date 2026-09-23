/*
 * libc.c
 */

#include <libc.h>

#include <types.h>

int errno;

/* Array of error msgs according to errno.
 * For now only contains from 1 to 38 (until ENOSYS which is the max errno
 * currently implemented).
 * Source:
 * https://www.chromium.org/chromium-os/developer-library/reference/linux-constants/errnos/
 */
char *error_msg[39] = {
    "No Error",
    "Operation not permitted",          // EPERM
    "No such file or directory",        // ENOENT
    "No such process",                  // ESRCH
    "Interrupted system call",          // EINTR
    "Input/Output error",               // EIO
    "No such device or address",        // ENXIO
    "Argument list too long",           // E2BIG
    "Exec format error",                // ENOEXEC
    "Bad file descriptor",              // EBADF
    "No child processes",               // ECHILD
    "Resource temporarily unavailable", // EAGAIN | EWOULDBLOCK
    "Cannot allocate memory",           // ENOMEM
    "Permission denied",                // EACCESS
    "Bad address",                      // EFAULT
    "Block device required",            // ENOTBLK
    "Device or resource busy",          // EBUSY
    "File exists",                      // EEXIST
    "Invalid cross-device link",        // EXDEV
    "No such device",                   // ENODEV
    "Not a directory",                  // ENOTDIR
    "Is a directory",                   // EISDIR
    "Invalid Argument",                 // EINVAL
    "Too many open files in system",    // ENFILE
    "Too many open files",              // EMFILE
    "Inappropriate ioctl for device",   // ENOTTY
    "Text file busy",                   // ETXTBSY
    "File too large",                   // EFBIG
    "No space left on device",          // ENOSPC
    "Illegal seek",                     // ESPIPE
    "Read-only file system",            // EROFS
    "Too many links",                   // EMLINK
    "Broken pipe",                      // EPIPE
    "Numerical argument out of domain", // EDOM
    "Numerical result out of range",    // ERANGE
    "Resource deadlock avoided",        // EDEADLK
    "File name too long",               // ENAMETOOLONG
    "No locks available",               // ENOLCK
    "Function not implemented"};        // ENOSYS

/********************/
/* SYSCALL WRAPPERS */
/********************/

int write(int fd, char *buffer, int size);

/* Attempts to write via fd 1 the error msg corresponding to the current errno
 */
void perror() {
  if (errno < 38 && errno >= 0) {
    write(1, error_msg[errno], strlen(error_msg[errno]));
    write(1, "\n", 1);
  }
}

void itoa(int a, char *b) {
  int i, i1;
  char c;

  if (a == 0) {
    b[0] = '0';
    b[1] = 0;
    return;
  }

  i = 0;
  while (a > 0) {
    b[i] = (a % 10) + '0';
    a = a / 10;
    i++;
  }

  for (i1 = 0; i1 < i / 2; i1++) {
    c = b[i1];
    b[i1] = b[i - i1 - 1];
    b[i - i1 - 1] = c;
  }
  b[i] = 0;
}

int strlen(char *a) {
  int i;

  i = 0;

  while (a[i] != 0)
    i++;

  return i;
}
