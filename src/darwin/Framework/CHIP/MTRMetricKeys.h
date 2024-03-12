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

#include <tracing/metric_event.h>

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkDeviceCommissioning = "dwnfw_device_commissioning";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkSetupWithPayload = "dwnfw_setup_with_payload";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkPairDevice = "dwnfw_pair_device";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkSetupWithDiscovered = "dwnfw_setup_with_discovered";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkPreWarmCommissioning = "dwnfw_prewarm_commissioning";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkStartBrowseForCommissionables = "dwnfw_start_browse_commissionables";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkStopBrowseForCommissionables = "dwnfw_stop_browse_commissionables";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkCancelCommissioning = "dwnfw_cancel_commissioning";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkContinueCommissioningAfterAttestation =
    "dwnfw_commission_post_attestation";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkCommissionNode = "dwnfw_commission_node";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkDeviceBeingCommissioned = "dwnfw_dev_being_commissioned";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkPASEVerifierForSetupCode = "dwnfw_pase_verifier_for_code";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkOpenPairingWindow = "dwnfw_pase_verifier_for_code";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkDeviceVendorID = "dwnfw_device_vendor_id";

constexpr chip::Tracing::MetricKey kMetricDarwinFrameworkDeviceProductID = "dwnfw_device_product_id";
