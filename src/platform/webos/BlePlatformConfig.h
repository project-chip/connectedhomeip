/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on webOS platforms.
 *
 */

#pragma once

namespace chip {
namespace DeviceLayer {
namespace Internal {
struct BluezConnection;
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================
#define BLE_CONNECTION_UNINITIALIZED nullptr
// ========== Platform-specific Configuration Overrides =========

/* none so far */
