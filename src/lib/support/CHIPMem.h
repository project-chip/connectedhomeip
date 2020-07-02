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
 *      This file defines heap memory allocation APIs for CHIP.
 *
 */

#ifndef CHIP_MEM_H
#define CHIP_MEM_H

#include <stdlib.h>

namespace chip {

/**
 * This function is called by the CHIP layer to allocate a block of memory of "size" bytes.
 *
 * @param[in]  size             Specifies requested memory size in bytes.
 *
 * @retval  Pointer to a memory block in case of success.
 * @retval  NULL-pointer if memory allocation fails.
 *
 */
extern void *MemoryAlloc(size_t size);

/**
 * This function is called by the CHIP layer to allocate a block of memory for an array of num
 * elements, each of them size bytes long, and initializes all its bits to zero.
 * The effective result is the allocation of a zero-initialized memory block of (num*size) bytes.
 *
 * @param[in]  num              Specifies number of elements to allocate.
 * @param[in]  size             Specifies size of each element in bytes.
 *
 * @retval  Pointer to a memory block in case of success.
 * @retval  NULL-pointer if memory allocation fails.
 *
 */
extern void *MemoryCalloc(size_t num, size_t size);

/**
 * This function is called by the Chip layer to change the size of the memory block pointed to by p.
 * The function may move the memory block to a new location (whose address is returned by the function).
 * The content of the memory block is preserved up to the lesser of the new and old sizes, even if the
 * block is moved to a new location. If the new size is larger, the value of the newly allocated portion
 * is indeterminate.

 * In case that p is a null pointer, the function behaves like malloc, assigning a new block of size bytes
 * and returning a pointer to its beginning.
 *
 * @param[in]  p                Pointer to a memory block previously allocated with MemoryAlloc, MemoryCalloc
 *                              or MemoryRealloc.
 * @param[in]  size             Specifies new size for the memory block, in bytes..
 *
 * @retval  Pointer to a memory block in case of success.
 * @retval  NULL-pointer if memory allocation fails.
 *
 */
extern void *MemoryRealloc(void *p, size_t size);

/**
 * This function is called by the Chip layer to release a memory block allocated by
 * the MemeoryAlloc(), MemoryCalloc or MemoryRealloc.
 * @param[in]  p                Pointer to a memory block that should be released.
 *
 */
extern void MemoryFree(void *p);

} // namespace chip

#endif // CHIP_MEM_H
