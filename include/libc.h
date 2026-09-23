/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

int strlen(char *a);

int write(int fd, char *buf, int size);

#endif /* __LIBC_H__ */
