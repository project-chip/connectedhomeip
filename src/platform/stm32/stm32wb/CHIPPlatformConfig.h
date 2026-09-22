/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          the STM32 platform.
 */

#pragma once
#include "CHIPDevicePlatformConfig.h"
// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_CONFIG_WRMP_TIMER_DEFAULT_PERIOD
#define CHIP_CONFIG_WRMP_TIMER_DEFAULT_PERIOD 50
#endif // CHIP_CONFIG_WRMP_TIMER_DEFAULT_PERIOD

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_MAX_PEER_NODES
#define CHIP_CONFIG_MAX_PEER_NODES 16
#endif // CHIP_CONFIG_MAX_PEER_NODES

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif // CHIP_CONFIG_MAX_FABRICS

#ifndef CHIP_CONFIG_MAX_ACTIVE_CHANNELS
#define CHIP_CONFIG_MAX_ACTIVE_CHANNELS 8
#endif // CHIP_CONFIG_MAX_ACTIVE_CHANNELS

#ifndef CHIP_CONFIG_MAX_CHANNEL_HANDLES
#define CHIP_CONFIG_MAX_CHANNEL_HANDLES 16
#endif // CHIP_CONFIG_MAX_CHANNEL_HANDLES

#ifndef CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT 6
#endif // CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
// ==================== Security Configuration Overrides ====================

#ifndef CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
#define CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE 1
#endif

#ifndef CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
#define CHIP_CONFIG_FREERTOS_USE_STATIC_TASK 1
#endif

#ifndef CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE
#define CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE 1
#endif
