/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on the Ameba platform.
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
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

#define CHIP_SYSTEM_CONFIG_ERROR_TYPE uint32_t
#define CHIP_SYSTEM_CONFIG_NO_ERROR 0
#define CHIP_SYSTEM_CONFIG_ERROR_MIN 7000000
#define CHIP_SYSTEM_CONFIG_ERROR_MAX 7000999
#define _CHIP_SYSTEM_CONFIG_ERROR(e) (CHIP_SYSTEM_CONFIG_ERROR_MIN + (e))
#define CHIP_SYSTEM_LWIP_ERROR_MIN 3000000
#define CHIP_SYSTEM_LWIP_ERROR_MAX 3000128

// ==================== General Configuration Overrides ====================

// NOTE: Values that are mapped to CONFIG_ #defines are settable via the Kconfig mechanism.

#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF 0
#define CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS 0
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS00
#define CHIP_SYSTEM_CONFIG_USE_LWIP 1
#define CHIP_SYSTEM_CONFIG_USE_SOCKETS 0
#define CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK 0
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
