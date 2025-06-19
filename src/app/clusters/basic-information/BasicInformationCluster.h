/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/basic-information/BasicInformationLogic.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BasicInformation/ClusterId.h>
#include <lib/core/DataModelTypes.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {
namespace Clusters {

enum class OptionalBasicInformationAttributes : uint8_t
{
    kManufacturingDate   = 1 << 0,
    kPartNumber          = 1 << 1,
    kProductURL          = 1 << 2,
    kProductLabel        = 1 << 3,
    kSerialNumber        = 1 << 4,
    kLocalConfigDisabled = 1 << 5,
    kReachable           = 1 << 6,
    kProductAppearance   = 1 << 7,
};

/// The BasicInformationCluster is a SINGLETON that only applies to
/// the root endpoint id.
///
/// If registered, it auto-registers itself as a PlatforMmanagerDelegate
/// and will emit startup/shtdown events
class BasicInformationCluster : public DefaultServerCluster, public DeviceLayer::PlatformManagerDelegate
{
public:
    BasicInformationCluster(BitFlags<OptionalBasicInformationAttributes> enabledOptional) :
        DefaultServerCluster({ kRootEndpointId, BasicInformation::Id }), mEnabledOptionalAttributes(enabledOptional)
    {}

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // PlatforMmanagerDelegate
    void OnStartUp(uint32_t softwareVersion) override;
    void OnShutDown() override;

private:
    const BitFlags<OptionalBasicInformationAttributes> mEnabledOptionalAttributes;
};

// TODO: define the cluster

} // namespace Clusters
} // namespace app
} // namespace chip
