/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

// Include Zephyr-supplied OpenThread configuration

#include "openthread-core-zephyr-config.h"

// Project-specific settings
#include "openthread/platform/logging.h"

#undef OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 64

#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_DEBUG_UART

#undef OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS_MANAGEMENT
#define OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS_MANAGEMENT 1

#undef OPENTHREAD_CONFIG_LOG_LEVEL
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_DEBG

#define OPENTHREAD_CONFIG_ENABLE_DEBUG_UART 1

#undef OPENTHREAD_CONFIG_LOG_API
#define OPENTHREAD_CONFIG_LOG_API 0

#undef OPENTHREAD_CONFIG_LOG_ARP
#define OPENTHREAD_CONFIG_LOG_ARP 0

#undef OPENTHREAD_CONFIG_LOG_BB
#define OPENTHREAD_CONFIG_LOG_BBR 0

#undef OPENTHREAD_CONFIG_LOG_CLI
#define OPENTHREAD_CONFIG_LOG_CLI 0

#undef OPENTHREAD_CONFIG_LOG_COAP
#define OPENTHREAD_CONFIG_LOG_COAP 0

#undef OPENTHREAD_CONFIG_LOG_DUA
#define OPENTHREAD_CONFIG_LOG_DUA 0

#undef OPENTHREAD_CONFIG_LOG_ICMP
#define OPENTHREAD_CONFIG_LOG_ICMP 0

#undef OPENTHREAD_CONFIG_LOG_IP6
#define OPENTHREAD_CONFIG_LOG_IP6 0

#undef OPENTHREAD_CONFIG_LOG_MAC
#define OPENTHREAD_CONFIG_LOG_MAC 0

#undef OPENTHREAD_CONFIG_LOG_MEM
#define OPENTHREAD_CONFIG_LOG_MEM 0

#undef OPENTHREAD_CONFIG_LOG_MESHCOP
#define OPENTHREAD_CONFIG_LOG_MESHCOP 0

#undef OPENTHREAD_CONFIG_LOG_MLE
#define OPENTHREAD_CONFIG_LOG_MLE 0

#undef OPENTHREAD_CONFIG_LOG_MLR
#define OPENTHREAD_CONFIG_LOG_MLR 0

#undef OPENTHREAD_CONFIG_LOG_NETDATA
#define OPENTHREAD_CONFIG_LOG_NETDATA 0

#undef OPENTHREAD_CONFIG_LOG_NETDIAG
#define OPENTHREAD_CONFIG_LOG_NETDIAG 0

#undef OPENTHREAD_CONFIG_LOG_PKT_DUMP
#define OPENTHREAD_CONFIG_LOG_PKT_DUMP 0

#undef OPENTHREAD_CONFIG_LOG_PLATFORM
#define OPENTHREAD_CONFIG_LOG_PLATFORM 0

#undef OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL
#define OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL 0

#undef OPENTHREAD_CONFIG_LOG_PREPEND_REGION
#define OPENTHREAD_CONFIG_LOG_PREPEND_REGION 0

#undef OPENTHREAD_CONFIG_PLAT_LOG_MACRO_NAME
#define OPENTHREAD_CONFIG_PLAT_LOG_MACRO_NAME tlOtPlatLog

void tlOtPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...);
