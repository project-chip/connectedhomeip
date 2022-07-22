/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          Ameba platforms.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ERROR_TYPE uint32_t
#define CHIP_CONFIG_ERROR_FORMAT PRId32
#define CHIP_CONFIG_NO_ERROR 0
#define CHIP_CONFIG_ERROR_MIN 4000000
#define CHIP_CONFIG_ERROR_MAX 4000999

#define ASN1_CONFIG_ERROR_TYPE uint32_t
#define ASN1_CONFIG_NO_ERROR 0
#define ASN1_CONFIG_ERROR_MIN 5000000
#define ASN1_CONFIG_ERROR_MAX 5000999

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif // CHIP_CONFIG_MAX_FABRICS

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 16
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

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
