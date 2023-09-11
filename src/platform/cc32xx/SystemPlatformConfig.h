/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on CC32XX platforms using the Texas Instruments SDK.
 *
 */

#pragma once

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================

//#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
//#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
//#define CHIP_SYSTEM_CONFIG_NO_LOCKING 1
//#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
//#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE int
#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

// #define CHIP_SYSTEM_CONFIG_ERROR_TYPE uint32_t
// #define CHIP_SYSTEM_CONFIG_NO_ERROR (0)

// #define CHIP_SYSTEM_CONFIG_ERROR_MIN 0
// #define CHIP_SYSTEM_CONFIG_ERROR_MAX 999
// #define _CHIP_SYSTEM_CONFIG_ERROR(e) (CHIP_SYSTEM_CONFIG_ERROR_MIN + (e))

// ========== Platform-specific Configuration Overrides =========

/* none yet */
