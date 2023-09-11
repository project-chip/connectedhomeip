/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on Darwin platforms.
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

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
// FIXME: these should not be hardcoded here, it is set via build config
// Need to exclude these for now in libev case
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 1
#endif

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 1
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_LOG 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_LOG_INCLUDE <platform/Darwin/Logging.h>

// ========== Platform-specific Configuration Overrides =========
#define CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS 5
