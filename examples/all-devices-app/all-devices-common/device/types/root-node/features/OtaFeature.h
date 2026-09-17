/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <array>

#include <clusters/OtaSoftwareUpdateProvider/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/root-node/RootNode.h>
#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/// Feature policy that declares OTA Requestor support on the root endpoint.
///
/// The requestor state machine is provided by the platform stack; from the
/// data-model side this feature only advertises:
///   - the `OtaRequestor` device type on the root endpoint, and
///   - the `OtaSoftwareUpdateProvider` client cluster (for outgoing bindings).
///
/// It owns no server clusters and does no work at register/unregister time.
class OtaFeature
{
public:
    struct Context
    {};

    OtaFeature() = default;
    explicit OtaFeature(const Context &) {}

    static constexpr std::array<DataModel::DeviceTypeEntry, 1> kExtraDeviceTypes{ Device::Type::kOtaRequestor };
    static constexpr std::array<ClusterId, 1> kExtraClientClusters{ Clusters::OtaSoftwareUpdateProvider::Id };

    CHIP_ERROR RegisterFeatureClusters(EndpointId, CodeDrivenDataModelProvider &, RootNode::Context &,
                                       Clusters::GeneralCommissioningCluster &)
    {
        return CHIP_NO_ERROR;
    }

    void UnregisterFeatureClusters(CodeDrivenDataModelProvider &) {}
};

} // namespace app
} // namespace chip
