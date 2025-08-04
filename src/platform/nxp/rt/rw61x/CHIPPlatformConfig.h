/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
 *    Copyright 2023 NXP
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
 *          NXP RW61x platform.
 */

#pragma once

#include <stdint.h>

// ==================== General Platform Adaptations ====================

/* Default NXP platform adaptations are used */

/* In src/crypto/CHIPCryptoPALmbedTLS.cpp we verify kMAX_Hash_SHA256_Context_Size >= sizeof(mbedtls_sha256_context)
 * sizeof(mbedtls_sha256_context) is 392 bytes with els_pkc mbedtls port
 */
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE (sizeof(unsigned int) * 120)

// ==================== Security Adaptations ====================

/* Default NXP Platform security adaptations are used */

// ==================== General Configuration Overrides ====================

/* Default NXP Platform general configuration overrides are used */

/* For now on RW610, due to the current file system implementation only 5 fabrics are supported */
#define CHIP_CONFIG_MAX_FABRICS 5

// ==================== Security Configuration Overrides ====================

/* Default NXP Platform security configuration overrides are used */

// ==================== WDM Configuration Overrides ====================

/* Default NXP Platform WDM Configuration overrides are used */

// Include default nxp platform configurations
#include "platform/nxp/common/CHIPNXPPlatformDefaultConfig.h"
