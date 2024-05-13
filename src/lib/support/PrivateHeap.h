/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2019 Google Inc. All Rights Reserved.
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <cstddef>
#include <type_traits>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Initializes the heap (set start and end blocks)
void PrivateHeapInit(void * heap, size_t size);

// Allocates a new block on the specified heap
void * PrivateHeapAlloc(void * heap, size_t size);

// Marks the specified block as free
void PrivateHeapFree(void * ptr);

// Reallocate a block to a new size
// Special arguments:
//    - null ptr value represents an Alloc
//    - zero size represents a Free
void * PrivateHeapRealloc(void * heap, void * ptr, size_t size);

void PrivateHeapDump(void * ptr);

#ifdef __cplusplus
} // extern "C"

namespace internal {

// Heap structure, exposed for tests
//
//  +---------+---------+-----+                      +---------+-----------+
//  | prev: 0 | next: n | ... |  ....<n_bytes> ....  | prev: n | next: ... |
//  +---------+---------+-----+                      +---------+-----------+
//
struct PrivateHeapBlockHeader
{
    uint32_t prevBytes;
    uint32_t nextBytes;
    uint32_t state;
    uint32_t checksum; // super-basic attempt to detect errors
};

} // namespace internal

inline constexpr size_t kPrivateHeapAllocationAlignment = std::alignment_of<max_align_t>::value;

#endif // ifdef __cplusplus
