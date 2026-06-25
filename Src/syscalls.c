/**
  ******************************************************************************
  * @file    syscalls.c
  * @brief   Minimal newlib syscall stubs for bare-metal STM32
  ******************************************************************************
  */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int _close(int fd) { (void)fd; errno = EBADF; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; (void)st; errno = EBADF; return -1; }
int _getpid(void) { return 1; }
int _isatty(int fd) { (void)fd; errno = ENOTTY; return 0; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _lseek(int fd, int ptr, int dir) { (void)fd; (void)ptr; (void)dir; errno = EBADF; return -1; }
int _read(int fd, char *ptr, int len) { (void)fd; (void)ptr; (void)len; errno = EBADF; return -1; }
int _write(int fd, char *ptr, int len) { (void)fd; (void)ptr; (void)len; return len; }
