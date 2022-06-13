/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Tizen platforms.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 16
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
