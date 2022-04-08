/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *      Overrides to default OpenThread configuration.
 *
 */

#pragma once

#include <em_device.h>

// Disable the SiLabs-supplied OpenThread logging facilities and use
// the facilities provided by the Device Layer (see
// src/platform/EFR32/Logging.cpp).
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP

// Turn on a moderate level of logging in OpenThread
// Enable use of external heap allocator (calloc/free) for OpenThread.
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1

// EFR32MG21A020F1024IM32 has 96k of RAM. Reduce the number of buffers to
// conserve RAM for this Series 2 part.
#if defined(EFR32MG21)
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 22
#define OPENTHREAD_CONFIG_CLI_UART_TX_BUFFER_SIZE 512
#define OPENTHREAD_CONFIG_CLI_UART_RX_BUFFER_SIZE 512
#endif

#define OPENTHREAD_CONFIG_NCP_HDLC_ENABLE 1
#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1

#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_ECDSA_ENABLE 1

// disable unused features
#define OPENTHREAD_CONFIG_COAP_API_ENABLE 0
#define OPENTHREAD_CONFIG_JOINER_ENABLE 0
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 0
#define OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE 0
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE 0
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE 0
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 0
#define OPENTHREAD_CONFIG_TCP_ENABLE 0

// Support udp multicast by enabling Multicast Listener Registration (MLR)
#define OPENTHREAD_CONFIG_MLR_ENABLE 1

// Define as 1 to stay awake between fragments while transmitting a large packet,
// and to stay awake after receiving a packet with frame pending set to true.
#define OPENTHREAD_CONFIG_MAC_STAY_AWAKE_BETWEEN_FRAGMENTS 1

// Use the SiLabs-supplied default platform configuration for remainder
// of OpenThread config options.
//
// NB: This file gets included during the build of OpenThread.  Hence
// it cannot use "openthread" in the path to the included file.
//
#include "openthread-core-efr32-config.h"
