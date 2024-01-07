#include <fcntl.h>       // for off_t, fcntl
#include <stdio.h>       // for fprintf, fgetc, stderr, size_t, FILE
#include <stdlib.h>      // for abort
#include <sys/mman.h>    // for mmap
#include <sys/stat.h>    // for fstat, stat
#include <ctype.h>       // for tolower and character type functions
#include <pthread.h>     // for pthread functions
#include <errno.h>       // for errno
#include <string.h>      // for strerror
#include <unistd.h>      // for _SC_PAGESIZE
#include <dlfcn.h>       // for dlsym

#if DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) (void)0
#endif

// Shim for __ctype_b

extern "C" const unsigned short *__ctype_b(void) {
    static unsigned short ctype_b_array[256];
    for (int i = 0; i < 256; ++i) {
        // Directly use the return value of the ctype functions as flags
        ctype_b_array[i] = (isalpha(i)   ? 1 : 0) |
                           (isupper(i)   ? 2 : 0) |
                           (islower(i)   ? 4 : 0) |
                           (isdigit(i)   ? 8 : 0) |
                           (isxdigit(i)  ? 16 : 0) |
                           (isspace(i)   ? 32 : 0) |
                           (ispunct(i)   ? 64 : 0) |
                           (iscntrl(i)   ? 128 : 0) |
                           (isprint(i)   ? 256 : 0) |
                           (isgraph(i)   ? 512 : 0);
    }
    return ctype_b_array;
}

// Shim for __fgetc_unlocked
extern "C" int __fgetc_unlocked(FILE *__stream) {
    return fgetc(__stream);  // Use the standard fgetc as a stand-in
}

// Shim for __ctype_tolower
extern "C" int __ctype_tolower(int c) {
    return tolower(c);
}

// Shim for __pthread_register_cancel and __pthread_unregister_cancel
extern "C" void __pthread_register_cancel(void *buf) {
    fprintf(stderr, "[WARNING] Called __pthread_register_cancel. This is a shim and does nothing.\n");
}

extern "C" void __pthread_unregister_cancel(void *buf) {
    fprintf(stderr, "[WARNING] Called __pthread_unregister_cancel. This is a shim and does nothing.\n");
}

// Shim for __assert
extern "C" void __assert(const char *msg, const char *file, int line) {
    fprintf(stderr, "Assertion failed: %s (%s: %d)\n", msg, file, line);
    abort();
}


void *mmap(void *__addr, size_t __len, int __prot, int __flags, int __fd, off_t __offset) {
    // Function pointer for the system's mmap
    static void *(*sys_mmap)(void *, size_t, int, int, int, off_t) = NULL;

    // Dynamically load the system's mmap function
    if (!sys_mmap) {
        sys_mmap = (void *(*)(void *, size_t, int, int, int, off_t))dlsym(RTLD_NEXT, "mmap");
        if (!sys_mmap) {
            fprintf(stderr, "Error loading system mmap: %s\n", dlerror());
            abort();
        }
    }

    // Logging mmap call details
    DEBUG_PRINT("mmap called with: addr=%p, len=%zu, prot=%d, flags=%d, fd=%d, offset=%lld\n", __addr, __len, __prot, __flags, __fd, __offset);

    // Call the system's mmap
    void *ret_val = sys_mmap(__addr, __len, __prot, __flags, __fd, __offset);

    if (ret_val == (void *)-1) {
        DEBUG_PRINT("System mmap failed with error: %s\n", strerror(errno));
    } else {
        DEBUG_PRINT("System mmap returned: %p\n", ret_val);
    }

    return ret_val;
}
