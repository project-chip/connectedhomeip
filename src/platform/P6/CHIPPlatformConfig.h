/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          the PSoC6 platform.
 */

#pragma once

#include "cy_result.h"

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID "enc-msg-counter"
// The ESP NVS implementation limits key names to 15 characters.
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 15

#define CHIP_CONFIG_ERROR_TYPE cy_rslt_t
#define CHIP_CONFIG_NO_ERROR CY_RSLT_SUCCESS
#define CHIP_CONFIG_ERROR_MIN 4000000
#define CHIP_CONFIG_ERROR_MAX 4000999
#define CHIP_CONFIG_ERROR_FORMAT PRId32

#define ASN1_CONFIG_ERROR_TYPE cy_rslt_t
#define ASN1_CONFIG_NO_ERROR CY_RSLT_SUCCESS
#define ASN1_CONFIG_ERROR_MIN 5000000
#define ASN1_CONFIG_ERROR_MAX 5000999

#define CHIP_LOG_FILTERING 0

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================
