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
// src/platform/silabs/Logging.cpp).
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP

// Turn on a moderate level of logging in OpenThread
// Enable use of external heap allocator (calloc/free) for OpenThread.
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1

#ifdef SL_ICD_ENABLED
#define OPENTHREAD_CONFIG_PARENT_SEARCH_ENABLE 0

// In seconds
#define SL_MLE_TIMEOUT_s (SL_OT_IDLE_INTERVAL / 1000)

// Timeout after 2 missed checkin or 4 mins if sleep interval is too short.
#define OPENTHREAD_CONFIG_MLE_CHILD_TIMEOUT_DEFAULT ((SL_MLE_TIMEOUT_s < 120) ? 240 : ((SL_MLE_TIMEOUT_s * 2) + 1))

#if defined(SL_CSL_ENABLE) && SL_CSL_ENABLE

#define OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE 1
#define OPENTHREAD_CONFIG_MAC_CSL_AUTO_SYNC_ENABLE 1
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE 1
#define OPENTHREAD_CONFIG_CHILD_SUPERVISION_CHECK_TIMEOUT 0
#define OPENTHREAD_CONFIG_CSL_RECEIVE_TIME_AHEAD 750
#define OPENTHREAD_CONFIG_CSL_TIMEOUT SL_CSL_TIMEOUT
#define SL_OPENTHREAD_CSL_TX_UNCERTAINTY 200

#endif // SL_CSL_ENABLE

#endif // SL_ICD_ENABLED

/****Uncomment below section for OpenThread Debug logs*/
// #define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_DEBG

// #ifndef OPENTHREAD_CONFIG_LOG_CLI
// #define OPENTHREAD_CONFIG_LOG_CLI 1
// #endif

// #ifndef OPENTHREAD_CONFIG_LOG_PKT_DUMP
// #define OPENTHREAD_CONFIG_LOG_PKT_DUMP 1
// #endif

// #ifndef OPENTHREAD_CONFIG_LOG_PLATFORM
// #define OPENTHREAD_CONFIG_LOG_PLATFORM 1
// #endif

// #ifndef OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL
// #define OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL 1
// #endif

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
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE 0
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 0
#define OPENTHREAD_CONFIG_TCP_ENABLE 0

// Support udp multicast by enabling Multicast Listener Registration (MLR)
#define OPENTHREAD_CONFIG_MLR_ENABLE 1

// Define as 1 to stay awake between fragments while transmitting a large packet,
// and to stay awake after receiving a packet with frame pending set to true.
#define OPENTHREAD_CONFIG_MAC_STAY_AWAKE_BETWEEN_FRAGMENTS 1

#define OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS 0

#define OPENTHREAD_CONFIG_DETERMINISTIC_ECDSA_ENABLE 0

// Use the SiLabs-supplied default platform configuration for remainder
// of OpenThread config options.
//
// NB: This file gets included during the build of OpenThread.  Hence
// it cannot use "openthread" in the path to the included file.
//
#include "openthread-core-efr32-config.h"
