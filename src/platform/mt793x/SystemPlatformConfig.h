/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on MediaTek Genio Platforms.
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

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_SYSTEM_CONFIG_NUM_TIMERS
#define CHIP_SYSTEM_CONFIG_NUM_TIMERS 16
#endif // CHIP_SYSTEM_CONFIG_NUM_TIMERS

#define CHIP_CONFIG_MDNS_CACHE_SIZE 4

/*
 * Overrides CHIP_SYSTEM_HEADER_RESERVE_SIZE in SystemConfig.h
 *
 * Note: hard to include header files needed for NIC_TX_HEAD_ROOM (62)
 */
#define CHIP_SYSTEM_HEADER_RESERVE_SIZE (62 + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE)
