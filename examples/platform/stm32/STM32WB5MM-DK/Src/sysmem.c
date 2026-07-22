/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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


/* Includes */
#include <errno.h>
#include <stdio.h>
#include <sys/errno.h>

/* Variables */
extern int errno;
register char * stack_ptr asm("sp");

/* Functions */

/**
 _sbrk
 Increase program data space. Malloc and related functions depend on this
**/
void * _sbrk(int incr)
{
    extern char end asm("end");
    static char * heap_end;
    char * prev_heap_end;

    if (heap_end == 0)
        heap_end = &end;

    prev_heap_end = heap_end;
    if (heap_end + incr > stack_ptr)
    {
        errno = ENOMEM;
        return (void *) -1;
    }

    heap_end += incr;

    return (void *) prev_heap_end;
}
