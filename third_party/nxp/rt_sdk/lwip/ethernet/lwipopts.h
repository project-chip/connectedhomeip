/*
 *
 *    Copyright (c) 2020 Nest Labs, Inc.
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

#define LWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS 1
#define PBUF_POOL_SIZE 15
#define TCP_MSS (1500 - 40)   /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
#define TCP_WND (8 * TCP_MSS) /* ENET_RXBD_NUM minus spare buffer */
#define ENET_RXBUFF_NUM 14
#define ENET_RXBD_NUM 9

#define LWIP_ETHERNET 1
#define LWIP_IPV4 0

#include "lwipopts_common.h"

#endif /* __LWIPOPTS_H__ */
