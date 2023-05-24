/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <system/SystemConfig.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <arpa/inet.h>

#else // !CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/def.h>
#include <lwip/opt.h>

#if defined(LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS)
#ifndef htons
#define htons(x) lwip_htons(x)
#endif
#ifndef ntohs
#define ntohs(x) lwip_ntohs(x)
#endif
#ifndef htonl
#define htonl(x) lwip_htonl(x)
#endif
#ifndef ntohl
#define ntohl(x) lwip_ntohl(x)
#endif
#endif // defined(LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS)

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

#ifndef __BYTE_ORDER__
#error Endianness is not defined
#endif // BYTE_ORDER

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#ifndef htons
#define htons(x) (x)
#endif
#ifndef ntohs
#define ntohs(x) (x)
#endif
#ifndef htonl
#define htonl(x) (x)
#endif
#ifndef ntohl
#define ntohl(x) (x)
#endif

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef htons
#define htons(x) ((u16_t)((((x) & (u16_t) 0x00ffU) << 8) | (((x) & (u16_t) 0xff00U) >> 8)))
#endif
#ifndef ntohs
#define ntohs(x) htons(x)
#endif
#ifndef htonl
#define htonl(x)                                                                                                                   \
    ((((x) & (uint32_t) 0x000000ffUL) << 24) | (((x) & (uint32_t) 0x0000ff00UL) << 8) | (((x) & (uint32_t) 0x00ff0000UL) >> 8) |   \
     (((x) & (uint32_t) 0xff000000UL) >> 24))
#endif
#ifndef ntohl
#define ntohl(x) htonl(x)
#endif

#else
#error __BYTE_ORDER__ value not recognized
#endif // __BYTE_ORDER__ ==

#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

#endif // !CHIP_SYSTEM_CONFIG_USE_SOCKETS
