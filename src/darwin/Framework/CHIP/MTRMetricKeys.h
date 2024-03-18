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

constexpr Tracing::MetricKey kMetricDeviceCommissioning = "dwnfw_device_commissioning";

constexpr Tracing::MetricKey kMetricSetupWithPayload = "dwnfw_setup_with_payload";

constexpr Tracing::MetricKey kMetricPairDevice = "dwnfw_pair_device";

constexpr Tracing::MetricKey kMetricSetupWithDiscovered = "dwnfw_setup_with_discovered";

constexpr Tracing::MetricKey kMetricPreWarmCommissioning = "dwnfw_prewarm_commissioning";

constexpr Tracing::MetricKey kMetricStartBrowseForCommissionables = "dwnfw_start_browse_commissionables";

constexpr Tracing::MetricKey kMetricStopBrowseForCommissionables = "dwnfw_stop_browse_commissionables";

constexpr Tracing::MetricKey kMetricCancelCommissioning = "dwnfw_cancel_commissioning";

constexpr Tracing::MetricKey kMetricContinueCommissioningAfterAttestation = "dwnfw_commission_post_attestation";

constexpr Tracing::MetricKey kMetricCommissionNode = "dwnfw_commission_node";

constexpr Tracing::MetricKey kMetricDeviceBeingCommissioned = "dwnfw_dev_being_commissioned";

constexpr Tracing::MetricKey kMetricPASEVerifierForSetupCode = "dwnfw_pase_verifier_for_code";

constexpr Tracing::MetricKey kMetricOpenPairingWindow = "dwnfw_pase_verifier_for_code";

constexpr Tracing::MetricKey kMetricDeviceVendorID = "dwnfw_device_vendor_id";

constexpr Tracing::MetricKey kMetricDeviceProductID = "dwnfw_device_product_id";

} // namespace DarwinFramework
} // namespace Tracing
} // namespace chip
