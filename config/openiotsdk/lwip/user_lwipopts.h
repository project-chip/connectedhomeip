/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#ifndef USER_LWIPOPTS_H
#define USER_LWIPOPTS_H

/**
 * MEM_LIBC_MALLOC: use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator.
 */
#define MEM_LIBC_MALLOC (1)

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 */
#define MEMP_NUM_NETBUF 16

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 *
 * Some unit tests required more pbuf buffers.
 */
#ifdef CHIP_LIB_TESTS
#define PBUF_POOL_SIZE (1001)
#endif // CHIP_LIB_TESTS

/**
 * LWIP_STATS : Turn off statistics gathering
 */
#define LWIP_STATS (0)

/**
 * LWIP_RAW: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW (1)

/**
 * Disable DHCP as the IP6 link local address can be used.
 */
#define LWIP_DHCP 0

#ifdef LWIP_DEBUG

// Debug Options
#define NETIF_DEBUG LWIP_DBG_ON
#define IP_DEBUG LWIP_DBG_ON
#define TCP_DEBUG LWIP_DBG_ON
#define UDP_DEBUG LWIP_DBG_ON
#define IP6_DEBUG LWIP_DBG_ON

#define LWIP_DBG_TYPES_ON LWIP_DBG_ON
#define LWIP_DBG_MIN_LEVEL LWIP_DBG_LEVEL_ALL
#endif

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        DEBUG_PRINT x;                                                                                                             \
        DEBUG_PRINT("\r");                                                                                                         \
    } while (0)
#endif

#endif /* USER_LWIPOPTS_H */
