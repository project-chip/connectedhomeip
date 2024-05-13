/*
 *
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023 NXP
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
 *      Compile-time configuration for LwIP
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define CONFIG_NETWORK_HIGH_PERF 0

#define LWIP_DNS 0
#define LWIP_IPV4 0
#define PBUF_POOL_SIZE 20
#define NETIF_MAX_HWADDR_LEN 8U

#include "lwipopts_common.h"

#endif /* __LWIPOPTS_H__ */
