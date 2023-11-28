/*
 *    Copyright (c) 2023 Project CHIP Authors
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
 *          Stub implementations of the C++ ABI exception handling functions.
 *
 *    These functions replace the standard C++ exception code from the system's
 *    C++ runtime library with stub implementations that simply abort.  This
 *    reduces overall code size as well as eliminating some calls to malloc()
 *    that occur during global initialization (see the code in eh_alloc.cc).
 *    This provides significant memory savings on resource constrained devices
 *    that don't use exceptions.
 *
 */

#include <stdlib.h>

extern "C" {

void * __cxa_allocate_exception(size_t)
{
    abort();
}

void __cxa_free_exception(void *)
{
    abort();
}

void * __cxa_allocate_dependent_exception()
{
    abort();
}

void __cxa_free_dependent_exception(void *)
{
    abort();
}

void __cxa_throw(void *, void *, void (*)(void *))
{
    abort();
}

void __cxa_rethrow()
{
    abort();
}

void * __cxa_begin_catch(void *)
{
    abort();
}

void __cxa_end_catch()
{
    abort();
}

void * __cxa_get_exception_ptr(void *)
{
    abort();
}
}
