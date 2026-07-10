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
 *      This file provides a C wrapper over CHIPMem.h, intended to be used only
 *      by platform or legacy code. See that file for documentation.
 */

#ifndef CHIP_PLATFORM_MEMORY_H
#define CHIP_PLATFORM_MEMORY_H

#include <stdbool.h>
#include <stdlib.h>

#ifndef CHIP_SYSTEM_CONFIG_TYPED_MALLOC
#if defined(__APPLE__) && defined(_MALLOC_TYPE_ENABLED) && _MALLOC_TYPE_ENABLED
#define CHIP_SYSTEM_CONFIG_TYPED_MALLOC 1
#define CHIP_OVERRIDE_MALLOC_TYPED(override, type_param_pos) _MALLOC_TYPED(override, type_param_pos)
#else
#define CHIP_SYSTEM_CONFIG_TYPED_MALLOC 0
#define CHIP_OVERRIDE_MALLOC_TYPED(override, type_param_pos)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if CHIP_SYSTEM_CONFIG_TYPED_MALLOC
extern void * CHIPPlatformMemoryAllocTyped(size_t size, malloc_type_id_t typeId);
extern void * CHIPPlatformMemoryCallocTyped(size_t num, size_t size, malloc_type_id_t typeId);
extern void * CHIPPlatformMemoryReallocTyped(void * p, size_t size, malloc_type_id_t typeId);
#endif

extern int CHIPPlatformMemoryInit(void * buf, size_t bufSize);
extern void CHIPPlatformMemoryShutdown();
extern void * CHIPPlatformMemoryAlloc(size_t size) CHIP_OVERRIDE_MALLOC_TYPED(CHIPPlatformMemoryAllocTyped, 1);
extern void * CHIPPlatformMemoryCalloc(size_t num, size_t size) CHIP_OVERRIDE_MALLOC_TYPED(CHIPPlatformMemoryCallocTyped, 2);
extern void * CHIPPlatformMemoryRealloc(void * p, size_t size) CHIP_OVERRIDE_MALLOC_TYPED(CHIPPlatformMemoryReallocTyped, 2);
extern void CHIPPlatformMemoryFree(void * p);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CHIP_PLATFORM_MEMORY_H
