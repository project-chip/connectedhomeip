/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Alternate implementation of _sbrk() for use with gcc on nRF5 platforms.
 *
 */

#include <stdlib.h>
#include <errno.h>

extern void OnSBRKFail(char * heapBase, char * heapLimit, char * heapBreak, ptrdiff_t inc);

extern char __HeapBase;
extern char __HeapLimit;

char * __HeapBreak = NULL;

/**
 *  Alternate implementation of _sbrk() for use with gcc on nRF5 platforms.
 *
 *  The default implementation of _sbrk() that comes with the GCC ARM toolchain
 *  (based on the newlib C library) is ill suited for use on RTOS-based embedded
 *  systems. In particular, the heap size limit check makes assumptions about
 *  the relative ordering in memory of the stack and heap that don't always hold
 *  true when _sbrk() is called from an RTOS task.
 *
 *  This implementation uses the __HeapBase and __HeapLimit definitions declared
 *  in the Nordic GCC startup code (e.g. see gcc_startup_nrf52840.S) to properly
 *  limit the max heap size to the configured value.
 */
void * _sbrk(ptrdiff_t inc)
{
    void * res;

    if (__HeapBreak == NULL)
    {
        __HeapBreak = &__HeapBase;
    }

    if (inc < 0
        ? (inc < (&__HeapBase  - __HeapBreak))
        : (inc > (&__HeapLimit - __HeapBreak)))
    {
        OnSBRKFail(&__HeapBase, &__HeapLimit, __HeapBreak, inc);

        errno = ENOMEM;
        return (void *)-1;
    }

    res = __HeapBreak;

    __HeapBreak += inc;

    return res;
}

size_t GetHeapTotalSize(void)
{
    return (size_t)(&__HeapLimit - &__HeapBase);
}

size_t GetHeapFreeSize(void)
{
    return (size_t)(&__HeapLimit - __HeapBreak);
}

void __attribute__((weak)) OnSBRKFail(char * heapBase, char * heapLimit, char * heapBreak, ptrdiff_t inc)
{
}
