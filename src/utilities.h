#ifndef UTILITIES_H
#define UTILITIES_H

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef _WIN32
#define numberof(x) _countof(x)
#define EMPTY 0
#else
#define numberof(x) (sizeof(x) / sizeof(*x))
#define EMPTY
#endif

#define REALLOC_ASSERT(ptr, size)                                                                  \
    do {                                                                                           \
        void* tmp = realloc(ptr, size);                                                            \
        assert(tmp);                                                                               \
        ptr = tmp;                                                                                 \
    } while (0)

static inline void* malloc_assert(size_t size)
{
    void* tmp = malloc(size);
    assert(tmp);
    return tmp;
}



#endif
