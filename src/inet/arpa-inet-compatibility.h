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
#endif // !CHIP_SYSTEM_CONFIG_USE_SOCKETS
