/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#ifndef ARPA_INET_COMPATIBILITY_H
#define ARPA_INET_COMPATIBILITY_H

#include <SystemLayer/SystemConfig.h>

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#include <arpa/inet.h>

#else // !WEAVE_SYSTEM_CONFIG_USE_SOCKETS

// NOTE WELL: when LWIP_VERSION_MAJOR == 1, LWIP_PREFIX_BYTEORDER_FUNCS instead of LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#if WEAVE_SYSTEM_CONFIG_USE_LWIP

#include <lwip/def.h>
#include <lwip/opt.h>

#if (defined(LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS) || defined(LWIP_PREFIX_BYTEORDER_FUNCS))
#define htons(x) lwip_htons(x)
#define ntohs(x) lwip_ntohs(x)
#define htonl(x) lwip_htonl(x)
#define ntohl(x) lwip_ntohl(x)
#endif // (defined(LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS) || defined(LWIP_PREFIX_BYTEORDER_FUNCS))

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
#endif // !WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#endif // !defined(ARPA_INET_COMPATIBILITY_H)
