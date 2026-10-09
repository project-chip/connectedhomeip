/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on webOS platforms.
 *
 */

#pragma once

namespace chip {
namespace DeviceLayer {
namespace Internal {
struct WbsConnection;
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================
#define BLE_CONNECTION_OBJECT chip::DeviceLayer::Internal::WbsConnection *
#define BLE_CONNECTION_UNINITIALIZED nullptr

// ========== Platform-specific Configuration Overrides =========

/* none so far */

// ================ webOS (wbs) connection tuning ===============

/**
 * Delay between stopping the scan and connecting. WBS drops the scanned (unpaired) device from its device list
 * shortly after the scan is stopped, and gatt/connect then fails with errorCode 106 "Device with supplied address
 * is not available" (a 1 s delay already hits this), so connect right away by default.
 */
#ifndef WEBOS_BLE_CONNECT_START_DELAY_MS
#define WEBOS_BLE_CONNECT_START_DELAY_MS 0
#endif

/**
 * The wbs layer issues gatt/connect with a fixed 10 s timeout. When WBS needs longer, the call times out but WBS still
 * completes the connection, leaving an orphaned WBS client: the peer stays connected (and stops advertising), so the
 * retries and the following commissioning attempts fail. Connect first with a longer timeout: the wbs gatt/connect
 * then returns immediately for the already connected peer. 0 disables this.
 */
#ifndef WEBOS_BLE_PRECONNECT_TIMEOUT_SEC
#define WEBOS_BLE_PRECONNECT_TIMEOUT_SEC 30
#endif
