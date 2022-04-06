/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      Preprocessor definitions related to packet buffer memory configuration.
 *      These are not part of the public PacketBuffer interface.
 */

#pragma once

#include <lib/core/CHIPConfig.h>
#include <system/SystemConfig.h>

/**
 * CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
 *
 * True if packet buffers are allocated in the SDK using Platform::MemoryAlloc.
 */
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && (CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE == 0)
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL
 *
 * True if packet buffers are allocated in the SDK using an internal pool.
 */
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && (CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE > 0)
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL
 *
 * True if packet buffers are allocated from an LwIP pool (either standard or custom).
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP && (CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_TYPE == PBUF_POOL)
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL
 *
 * True if packet buffers are allocated from an LwIP custom pool.
 */
#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL && !LWIP_PBUF_FROM_CUSTOM_POOLS
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL
 *
 * True if packet buffers are allocated from an LwIP custom pool.
 */
#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL && LWIP_PBUF_FROM_CUSTOM_POOLS
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE
 *
 * True if RightSize() has a nontrivial implementation.
 */
#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL || CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
#define CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE 0
#endif

/**
 * CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK
 *
 * True if Check() has a nontrivial implementation.
 */
#if CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP && CHIP_CONFIG_MEMORY_DEBUG_CHECKS
#define CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK 1
#else
#define CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK 0
#endif

// Sanity checks

#if (CHIP_SYSTEM_CONFIG_USE_LWIP + CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP + CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL) != 1
#error "Inconsistent PacketBuffer allocation configuration"
#endif

#if (CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL + CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL) !=                         \
    CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_POOL
#error "Inconsistent PacketBuffer LwIP pool configuration"
#endif
