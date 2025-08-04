/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <tracing/metric_keys.h>

namespace chip {
namespace Tracing {
namespace DarwinPlatform {

// BLE Scan duration
constexpr Tracing::MetricKey kMetricBLEScan = "dwnpm_ble_scan";

// Number of peripherals added to the cache during a scan operation
constexpr Tracing::MetricKey kMetricBLEPeripheralAdded = "dwnpm_ble_peripheral_added";

// Number of peripherals removed from the cache during a scan operation
constexpr Tracing::MetricKey kMetricBLEPeripheralRemoved = "dwnpm_ble_peripheral_removed";

// Request to prewarm scan
constexpr Tracing::MetricKey kMetricBLEStartPreWarmScan = "dwnpm_ble_start_prewarm_scan";

// Captures the last known state of the BLE Central manager during a commissioning attempt
constexpr Tracing::MetricKey kMetricBLECentralManagerState = "dwnpm_ble_cbmgr_state";

// Discovered a peripheral after a request to scan
constexpr Tracing::MetricKey kMetricBLEDiscoveredPeripheral = "dwnpm_ble_discovered_peripheral";

// Discovered a peripheral matching scanned from QR Code
constexpr Tracing::MetricKey kMetricBLEDiscoveredMatchingPeripheral = "dwnpm_ble_discovered_matching_peripheral";

// Unexpected length of Matter BLE service data
constexpr Tracing::MetricKey kMetricBLEBadServiceDataLength = "dwnpm_ble_bad_service_data";

// Unexpected OpCode in Matter BLE service data
constexpr Tracing::MetricKey kMetricBLEBadOpCode = "dwnpm_ble_bad_opcode";

// Mismatched discriminator when connecting to a peripheral
constexpr Tracing::MetricKey kMetricBLEMismatchedDiscriminator = "dwnpm_ble_mismatched_discriminator";

// Attempt to connect to discovered peripheral
constexpr Tracing::MetricKey kMetricBLEConnectPeripheral = "dwnpm_ble_connect_peripheral";

// Discover services on connected peripheral
constexpr Tracing::MetricKey kMetricBLEDiscoveredServices = "dwnpm_ble_discovered_svs";

// Discover characteristics on connected peripheral
constexpr Tracing::MetricKey kMetricBLEDiscoveredCharacteristics = "dwnpm_ble_discovered_chrs";

// Failed to write charateristic value
constexpr Tracing::MetricKey kMetricBLEWriteChrValueFailed = "dwnpm_ble_write_chr_val_failed";

// Failed to update notification state change for charateristic
constexpr Tracing::MetricKey kMetricBLEUpdateNotificationStateForChrFailed = "dwnpm_ble_chr_nfy_state_failed";

// Failed to update value for charateristic
constexpr Tracing::MetricKey kMetricBLEUpdateValueForChrFailed = "dwnpm_ble_upd_chr_val_failed";

} // namespace DarwinPlatform
} // namespace Tracing
} // namespace chip
