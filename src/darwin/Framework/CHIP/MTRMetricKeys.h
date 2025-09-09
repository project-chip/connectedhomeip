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
#include <tracing/metric_keys.h>

namespace chip {
namespace Tracing {
namespace DarwinFramework {

// When metrics were originally added, they were used for logging events during the commissioning of devices. The data was sent at
// the end of commissioning (in OnCommissioningComplete). This means that if we want to add metrics that are not related to
// commissioning, that need to be sent outside of the commissioning of the device, we need a way to separate out metrics by a
// category. The CHIP metrics infrastructure is fairly simple, with just a key and a numeric value. To add categories without
// changing existing keys we've opted to use a special prefix with double underscores (dwnfw__CATEGORY__) for the keys that are in a
// category. Any keys that do not contain this special prefix are now assumed to be used for commissioning events.

// Note that these are not used for commissioning metrics, because those predate the encoding of a category.
#define METRICS_KEY_PREFIX "dwnfw__"
#define METRICS_KEY(category, key) METRICS_KEY_PREFIX #category "__" #key

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

// Tracks the request to get attestation challenge for a device
constexpr Tracing::MetricKey kMetricAttestationChallengeForDevice = "dwnfw_attestation_challenge_for_device";

// Marks the request to open pairing window
constexpr Tracing::MetricKey kMetricOpenPairingWindow = "dwnfw_open_pairing_window";

// Device Vendor ID
constexpr Tracing::MetricKey kMetricDeviceVendorID = "dwnfw_device_vendor_id";

// Device Product ID
constexpr Tracing::MetricKey kMetricDeviceProductID = "dwnfw_device_product_id";

// Device Uses Thread
constexpr Tracing::MetricKey kMetricDeviceUsesThread = "dwnfw_device_uses_thread_bool";

// Counter of number of devices discovered on the network during setup
constexpr Tracing::MetricKey kMetricOnNetworkDevicesAdded = "dwnfw_onnet_devices_added";

// Counter of number of devices removed from the network during setup
constexpr Tracing::MetricKey kMetricOnNetworkDevicesRemoved = "dwnfw_onnet_devices_removed";

// Counter of number of BLE devices discovered during setup
constexpr Tracing::MetricKey kMetricBLEDevicesAdded = "dwnfw_ble_devices_added";

// Counter of number of BLE devices removed during setup
constexpr Tracing::MetricKey kMetricBLEDevicesRemoved = "dwnfw_ble_devices_removed";

// Unexpected C quality attribute update outside of priming
constexpr Tracing::MetricKey kMetricUnexpectedCQualityUpdate = "dwnpm_bad_c_attr_update";

// Setup from darwin MTRDevice for initial subscription to a device
constexpr Tracing::MetricKey kMetricMTRDeviceInitialSubscriptionSetup = "dwnpm_dev_initial_subscription_setup";

constexpr Tracing::MetricKey kMetricOTATransfer = METRICS_KEY(ota, transfer);

// Device Vendor ID
constexpr Tracing::MetricKey kMetricOTADeviceVendorID = METRICS_KEY(ota, device_vendor_id);

// Device Product ID
constexpr Tracing::MetricKey kMetricOTADeviceProductID = METRICS_KEY(ota, device_product_id);

// Device Uses Thread
constexpr Tracing::MetricKey kMetricOTADeviceUsesThread = METRICS_KEY(ota, device_uses_thread_bool);

constexpr Tracing::MetricKey kMetricOTATransferLength = METRICS_KEY(ota, transfer_length);

constexpr Tracing::MetricKey kMetricOTATransferOffset = METRICS_KEY(ota, transfer_offset);

constexpr Tracing::MetricKey kMetricOTATNumBytesProcessed = METRICS_KEY(ota, num_bytes_processed);

} // namespace DarwinFramework
} // namespace Tracing
} // namespace chip
