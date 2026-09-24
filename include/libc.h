/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

int strlen(char *a);

/* Prints error message for current errno.
 * Desc:  Prints to fd 1 (standard output) the error messaqe corresponding to
 *        the current value of errno (see error_msg).
 */
void perror();

/********************/
/* SYSCALL WRAPPERS */
/********************/

/* Write system call
 * Desc:    Uses a trap to call sys_write with elavated privilages.
 *          Writes to target fd the first size amount of content in buf.
 * Params:  - fd: target file descriptor. Since we are currently working without
 *                a file system this is a false fd with only 1 as a possible
 *                value.
 *          - buf: pointer to the buffer to be written.
 *          - size: amount of characters that will be written.
 * Return:  0 on success, -1 otherwise.
 */
int write(int fd, char *buf, int size);

/* Write system call
 * Desc:    Uses a sysenter to call sys_write with elavated privilages.
 *          Writes to target fd the first size amount of content in buf.
 * Params:  - fd: target file descriptor. Since we are currently working without
 *                a file system this is a false fd with only 1 as a possible
 *                value.
 *          - buf: pointer to the buffer to be written.
 *          - size: amount of characters that will be written.
 * Return:  0 on success, -1 otherwise.
 */
// int write(int fd, char *buf, int size);

#endif /* __LIBC_H__ */
