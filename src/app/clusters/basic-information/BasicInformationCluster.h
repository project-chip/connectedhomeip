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

#include <app/persistence/String.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/BasicInformation/AttributeIds.h>
#include <clusters/BasicInformation/ClusterId.h>
#include <lib/core/DataModelTypes.h>
#include <platform/PlatformManager.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace app {
namespace Clusters {

/// This class provides a code-driven implementation for the Basic Information cluster,
/// centralizing its logic and state. It is designed as a singleton because the cluster
/// is defined to exist only once per node, specifically on the root endpoint (Endpoint 0).
///
/// As a PlatformManagerDelegate, it automatically hooks into the node's lifecycle to
/// emit the mandatory StartUp and optional ShutDown events, ensuring spec compliance.
///
/// Note on the implementation of the singleton pattern:
/// The constructor is public to allow for a global variable instantiation. This approach
/// can save flash memory compared to a function-static instance, which often requires
/// additional thread-safety mechanisms. The intended usage is via the static
/// `Instance()` method, which returns a reference to the global instance.
class BasicInformationCluster : public DefaultServerCluster, public DeviceLayer::PlatformManagerDelegate
{
public:
    BasicInformationCluster() : DefaultServerCluster({ kRootEndpointId, BasicInformation::Id })
    {
        // Unless told otherwise, unique id is mandatory
        mEnabledOptionalAttributes.Set<BasicInformation::Attributes::UniqueID::Id>();
    }

    using OptionalAttributesSet = chip::app::OptionalAttributeSet< //
        BasicInformation::Attributes::ManufacturingDate::Id,       //
        BasicInformation::Attributes::PartNumber::Id,              //
        BasicInformation::Attributes::ProductURL::Id,              //
        BasicInformation::Attributes::ProductLabel::Id,            //
        BasicInformation::Attributes::SerialNumber::Id,            //
        BasicInformation::Attributes::LocalConfigDisabled::Id,     //
        BasicInformation::Attributes::Reachable::Id,               //
        BasicInformation::Attributes::ProductAppearance::Id,       //
        // Old specification versions had UniqueID as optional, so this
        // appears here even though MANDATORY in the latest spec. We
        // default it enabled (to decrease chances of error)
        BasicInformation::Attributes::UniqueID::Id //
        >;

    static BasicInformationCluster & Instance();

    OptionalAttributesSet & OptionalAttributes() { return mEnabledOptionalAttributes; }

    bool GetLocalConfigDisabled() { return mLocalConfigDisabled; }

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // PlatformManagerDelegate
    void OnStartUp(uint32_t softwareVersion) override;
    void OnShutDown() override;

private:
    // write without notification
    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);

    OptionalAttributesSet mEnabledOptionalAttributes;

    Storage::String<32> mNodeLabel;
    bool mLocalConfigDisabled = false;
};

} // namespace Clusters
} // namespace app
} // namespace chip
