

#include <FreeRTOS.h>

#include <assert.h>
#include <stdlib.h>

void * __wrap__malloc_r(void * REENT, size_t size)
{
    void * p = pvPortMalloc(size);
    while (!p)
        ;
    return p;
}

void __wrap__free_r(void * REENT, void * ptr)
{
    return vPortFree(ptr);
}

void * __wrap__realloc_r(void * REENT, void * ptr, size_t size)
{
    void * p = pvPortRealloc(ptr, size);
    while (!p)
        ;
    return p;
}
