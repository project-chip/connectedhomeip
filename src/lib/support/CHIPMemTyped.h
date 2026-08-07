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

/**
 *    @file
 *      This file defines some macros for type-aware memory allocation for CHIP.
 *
 */

#pragma once

// If type-aware malloc is available and enabled, then we'll try to use it.
// We define CHIP_SYSTEM_CONFIG_TYPED_MALLOC to 1 in that case (0 if disabled).
//
// We also provide a CHIP_OVERRIDE_MALLOC_TYPED macro, which can be added to
// malloc-like wrappers, and allows the compiler to automatically replace the
// wrapper with the corresponding type-aware variant. The
// CHIP_OVERRIDE_MALLOC_TYPED macro has two arguments, the first one points to
// the relevant type-aware replacement, and the second is a 1-based index that
// points to the argument that can be used to perform type inference over.
//
// For example, if we have a calloc wrapper CallocWrapper, with a corresponding
// typed variant CallocTypedWrapper, the CallocWrapper declaration would be
// annotated as follows:
//
//   void * CallocTypedWrapper(size_t num, size_t size,
//     malloc_type_id_t typeId);
//   void * CallocWrapper(size_t num, size_t size)
//     CHIP_OVERRIDE_MALLOC_TYPED(CallocTypedWrapper, 2);
//
// This signals to the compiler that the typed variant is CallocTypedWrapper,
// and the second argument of CallocWrapper should be used to do the type
// inference used to replace it with the typed variant.
//
// (See https://discourse.llvm.org/t/rfc-typed-allocator-support/79720 for
// information on how clang/llvm uses this).
#ifndef CHIP_SYSTEM_CONFIG_TYPED_MALLOC
#if defined(__APPLE__) && defined(_MALLOC_TYPE_ENABLED) && _MALLOC_TYPE_ENABLED
#define CHIP_SYSTEM_CONFIG_TYPED_MALLOC 1
#define CHIP_OVERRIDE_MALLOC_TYPED(override, type_param_pos) _MALLOC_TYPED(override, type_param_pos)
#else
#define CHIP_SYSTEM_CONFIG_TYPED_MALLOC 0
#define CHIP_OVERRIDE_MALLOC_TYPED(override, type_param_pos)
#endif
#endif

// Macros to turn off warnings for allocator wrappers. We use this to turn off
// the warnings for our own type-aware wrappers when they are enabled. For
// example, to turn off the warning for MemoryAllocTyped because it is a
// type-aware wrapper:
//
// CHIP_MALLOC_WRAPPER_BEGIN
// T * MemoryAllocTyped(size_t num)
// {
//     …
// }
// CHIP_MALLOC_WRAPPER_END
//
#if defined(__APPLE__) && CHIP_SYSTEM_CONFIG_TYPED_MALLOC
#define CHIP_MALLOC_WRAPPER_BEGIN _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wallocator-wrappers\"")
#define CHIP_MALLOC_WRAPPER_END _Pragma("clang diagnostic pop")
#else
#define CHIP_MALLOC_WRAPPER_BEGIN
#define CHIP_MALLOC_WRAPPER_END
#endif
