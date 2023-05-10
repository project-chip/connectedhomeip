/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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
 *          the Texas Instruments CC1352 platform.
 *
 * NOTE: currently a bare-bones implementation to allow for building.
 */

#pragma once

#include <stdint.h>

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE uint16_t
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY 0x01

// ==================== Security Adaptations ====================

#define CHIP_CONFIG_SHA256_CONTEXT_SIZE (sizeof(unsigned int) * 76)

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_MAX_INTERFACES
#define CHIP_CONFIG_MAX_INTERFACES 4
#endif // CHIP_CONFIG_MAX_INTERFACES

#ifndef CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS
#define CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS 4
#endif // CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS

// Limit the number of device admins to ensure enough ressources for handling them
#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif
