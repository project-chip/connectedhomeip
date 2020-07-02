/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements heap memory allocation APIs for CHIP. These functions are platform
 *      specific and might be C Standard Library heap functions re-direction in most of cases.
 *
 */

#include <support/CHIPMem.h>

#include <string.h>

namespace chip {

void *MemoryAlloc(size_t size)
{
    return malloc(size);
}

void *MemoryCalloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void *MemoryRealloc(void *p, size_t size)
{
    return realloc(p, size);
}

void MemoryFree(void *p)
{
    free(p);
}

} // namespace chip
