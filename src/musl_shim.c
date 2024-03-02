#include <fcntl.h>       // for off_t, fcntl
#include <stdarg.h> // For va_start, va_end
#include <stdio.h>       // for fprintf, fgetc, stderr, size_t, FILE
#include <stdlib.h>      // for abort
//#include <sys/mman.h>    // for mmap
#include <sys/stat.h>    // for fstat, stat
//#include "bits/fcntl.h"  // for F_GETFL
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>

/**
 * Shim to create missing function calls in the ingenic libimp library
 */

#define DEBUG 0  // Set this to 1 to enable debug output or 0 to disable

#if DEBUG
#define DEBUG_PRINT(...) fprintf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) (void)0
#endif

void __pthread_register_cancel(void *buf) {
    DEBUG_PRINT(stderr, "[WARNING] Called __pthread_register_cancel. This is a shim and does nothing.\n");
}

void __pthread_unregister_cancel(void *buf) {
    DEBUG_PRINT(stderr, "[WARNING] Called __pthread_unregister_cancel. This is a shim and does nothing.\n");
}

void __assert(const char *msg, const char *file, int line) {
    DEBUG_PRINT(stderr, "Assertion failed: %s (%s: %d)\n", msg, file, line);
    abort();
}

int __fgetc_unlocked(FILE *stream) {
    DEBUG_PRINT(stderr, "[WARNING] Called __fgetc_unlocked. This is a shim and does nothing.\n");
    return fgetc(stream);
}

/* Required for Xburst2 libraries */
int __fputc_unlocked(int c, FILE *stream) {
    DEBUG_PRINT(stderr, "[WARNING] Called __fputc_unlocked. This is a shim and does nothing.\n");
    return fputc(c, stream);
}

FILE* fopen64(const char *path, const char *mode) {
    DEBUG_PRINT(stderr, "[WARNING] Called fopen64. This is a shim and does nothing.\n");
    return fopen(path, mode);
}

int open64(const char *path, int flags, ...) {
    mode_t mode = 0;

    // Mode is only provided if O_CREAT is passed in flags
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    DEBUG_PRINT(stderr, "[WARNING] Called open64. This is a shim and does nothing.\n");
    if (flags & O_CREAT) {
        return open(path, flags, mode);
    } else {
        return open(path, flags);
    }
}

int fseeko64(FILE *stream, off_t offset, int whence) {
    DEBUG_PRINT(stderr, "[WARNING] Called fseeko64. This is a shim and does nothing.\n");
    return fseeko(stream, offset, whence);
}
/* Required for Xburst2 libraries */

void __ctype_b(void) {
    DEBUG_PRINT(stderr, "[WARNING] Called __ctype_b. This is a shim and does nothing.\n");
}
void __ctype_tolower(void) {
    DEBUG_PRINT(stderr, "[WARNING] Called __ctype_tolower. This is a shim and does nothing.\n");
}

// mmap begin

void* mmap(void *start, size_t len, int prot, int flags, int fd, uint32_t off) {
  return (void *)syscall(SYS_mmap2, start, len, prot, flags, fd, off >> 12);
}

/* Required for Xburst2 libraries */
void* mmap64(void *start, size_t len, int prot, int flags, int fd, uint32_t off) {
  return (void *)syscall(SYS_mmap2, start, len, prot, flags, fd, off >> 12);
}
/* Required for Xburst2 libraries */
