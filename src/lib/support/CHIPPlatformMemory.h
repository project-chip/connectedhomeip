/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

#ifdef __cplusplus
extern "C" {
#endif

extern int CHIPPlatformMemoryInit(void * buf, size_t bufSize);
extern void CHIPPlatformMemoryShutdown();
extern void * CHIPPlatformMemoryAlloc(size_t size);
extern void * CHIPPlatformMemoryCalloc(size_t num, size_t size);
extern void * CHIPPlatformMemoryRealloc(void * p, size_t size);
extern void CHIPPlatformMemoryFree(void * p);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CHIP_PLATFORM_MEMORY_H
