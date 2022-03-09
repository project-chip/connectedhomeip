/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          nRF Connect SDK platform.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY "rc"
#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER "mc"

#define CHIP_CONFIG_TIME_ENABLE_CLIENT 1
#define CHIP_CONFIG_TIME_ENABLE_SERVER 0

// ==================== Security Adaptations ====================

// Size of the statically allocated context for SHA256 operations in CryptoPAL
// determined empirically.
#ifdef CONFIG_CC3XX_BACKEND
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE 240
#else
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE 208
#endif

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_PEER_NODES
#define CHIP_CONFIG_MAX_PEER_NODES 16
#endif // CHIP_CONFIG_MAX_PEER_NODES

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_CONFIG_MAX_ACTIVE_CHANNELS
#define CHIP_CONFIG_MAX_ACTIVE_CHANNELS 8
#endif // CHIP_CONFIG_MAX_ACTIVE_CHANNELS

#ifndef CHIP_CONFIG_MAX_CHANNEL_HANDLES
#define CHIP_CONFIG_MAX_CHANNEL_HANDLES 16
#endif // CHIP_CONFIG_MAX_CHANNEL_HANDLES

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif
