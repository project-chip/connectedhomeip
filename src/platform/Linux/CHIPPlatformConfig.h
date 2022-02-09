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
 *          Linux platforms.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

// TODO:(#756) Add FabricState support
#define CHIP_CONFIG_ENABLE_FABRIC_STATE 0

using CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE = const char *;
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 16

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY "life-count"

#define CHIP_CONFIG_TIME_ENABLE_CLIENT 1
#define CHIP_CONFIG_TIME_ENABLE_SERVER 0

#define CHIP_CONFIG_ERROR_FORMAT_AS_STRING 1
#define CHIP_CONFIG_ERROR_SOURCE 1

#define CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE 1

// ==================== Security Adaptations ====================

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
#define CHIP_CONFIG_MAX_ACTIVE_CHANNELS 16
#endif // CHIP_CONFIG_MAX_ACTIVE_CHANNELS

#ifndef CHIP_CONFIG_MAX_CHANNEL_HANDLES
#define CHIP_CONFIG_MAX_CHANNEL_HANDLES 32
#endif // CHIP_CONFIG_MAX_CHANNEL_HANDLES

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

// ==================== Security Configuration Overrides ====================

#ifndef CHIP_CONFIG_KVS_PATH
#define CHIP_CONFIG_KVS_PATH "/tmp/chip_kvs"
#endif // CHIP_CONFIG_KVS_PATH
