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
#include <tracing/metric_event.h>

namespace chip {
namespace Tracing {
namespace DarwinFramework {

// Tracks overall commissioning via one of the setup APIs
constexpr Tracing::MetricKey kMetricDeviceCommissioning = "dwnfw_device_commissioning";

// Tracks portion related to starting setup with a payload
constexpr Tracing::MetricKey kMetricSetupWithPayload = "dwnfw_setup_with_payload";

// Tracks portion related to legacy setup APIs
constexpr Tracing::MetricKey kMetricPairDevice = "dwnfw_pair_device";

// Tracks portion related to starting setup with a discovered device
constexpr Tracing::MetricKey kMetricSetupWithDiscovered = "dwnfw_setup_with_discovered";

// Tracks PASE session during setup of a device
constexpr Tracing::MetricKey kMetricSetupPASESession = "dwnfw_setup_pase_session";

// Marks the request to prewarm for commissioning
constexpr Tracing::MetricKey kMetricPreWarmCommissioning = "dwnfw_prewarm_commissioning";

// Marks the request to start browsing for commissionable devices
constexpr Tracing::MetricKey kMetricStartBrowseForCommissionables = "dwnfw_start_browse_commissionables";

// Marks the request to stop browsing for commissionable devices
constexpr Tracing::MetricKey kMetricStopBrowseForCommissionables = "dwnfw_stop_browse_commissionables";

// Marks the request to cancel commissioning a device
constexpr Tracing::MetricKey kMetricCancelCommissioning = "dwnfw_cancel_commissioning";

// Marks the request to continue or cancel after attestation failure
constexpr Tracing::MetricKey kMetricContinueCommissioningAfterAttestation = "dwnfw_commission_post_attestation";

// Tracks request to commission device with a node ID
constexpr Tracing::MetricKey kMetricCommissionNode = "dwnfw_commission_node";

// Marks the request to get device being commissioned
constexpr Tracing::MetricKey kMetricDeviceBeingCommissioned = "dwnfw_dev_being_commissioned";

// Tracks the request to generate PASE verifier for a given setup code
constexpr Tracing::MetricKey kMetricPASEVerifierForSetupCode = "dwnfw_pase_verifier_for_code";

// Marks the request to open pairing window
constexpr Tracing::MetricKey kMetricOpenPairingWindow = "dwnfw_pase_verifier_for_code";

// Device Vendor ID
constexpr Tracing::MetricKey kMetricDeviceVendorID = "dwnfw_device_vendor_id";

// Device Product ID
constexpr Tracing::MetricKey kMetricDeviceProductID = "dwnfw_device_product_id";

// Counter of number of devices discovered on the network during setup
constexpr Tracing::MetricKey kMetricOnNetworkDevicesAdded = "dwnfw_onnet_devices_added";

// Counter of number of devices removed from the network during setup
constexpr Tracing::MetricKey kMetricOnNetworkDevicesRemoved = "dwnfw_onnet_devices_removed";

// Counter of number of BLE devices discovered during setup
constexpr Tracing::MetricKey kMetricBLEDevicesAdded = "dwnfw_ble_devices_added";

// Counter of number of BLE devices removed during setup
constexpr Tracing::MetricKey kMetricBLEDevicesRemoved = "dwnfw_ble_devices_removed";

} // namespace DarwinFramework
} // namespace Tracing
} // namespace chip
