/*
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/persistence/String.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/ClusterId.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Events.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
#include <lib/core/DataModelTypes.h>

// Include the default policy for backward compatibility
#include <app/clusters/basic-information/DeviceLayerBasicInformationPolicy.h>

namespace chip {
namespace app {
namespace Clusters {

/// This class provides a code-driven implementation for the Basic Information cluster,
/// centralizing its logic and state.
///
/// It uses a Policy-based design to decouple from DeviceLayer and generic platform logic.
template <typename Policy>
class BasicInformationClusterImpl : public DefaultServerCluster, public Policy::DelegateBase
{
public:
    using Context = typename Policy::Context;

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

    BasicInformationClusterImpl(OptionalAttributesSet optionalAttributeSet, Context ctx) :
        DefaultServerCluster({ kRootEndpointId, BasicInformation::Id }), mEnabledOptionalAttributes(optionalAttributeSet),
        mPolicy(ctx)
    {
        mEnabledOptionalAttributes
            .template Set<BasicInformation::Attributes::UniqueID::Id>(); // Unless told otherwise, unique id is mandatory
    }

    OptionalAttributesSet & OptionalAttributes() { return mEnabledOptionalAttributes; }

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Policy::DelegateBase implementation (PlatformManagerDelegate for DeviceLayer)
    /**
     * @brief Initialize the cluster
     *
     * This method attempts to register the cluster as the DeviceLayer::PlatformManagerDelegate
     * to receive system shutdown events (OnShutDown).
     * * NOTE: Registration is conditional. It will ONLY register this cluster as the delegate
     * if the PlatformManager does not currently have a delegate set. If the application
     * has already registered a delegate, this cluster will respect that configuration
     * and will NOT overwrite it.
     */
    void OnStartUp(uint32_t softwareVersion) override;

    void OnShutDown() override;

    // ConfigurationVersionDelegate, however NOT overridable to save
    // some flash in case this feature is never used. This means applications that may
    // change configurations at runtime pay a bit more flash, however those are probably more
    // dynamic (i.e. larger) systems like bridges or more complex systems.
    CHIP_ERROR IncreaseConfigurationVersion();

private:
    // Implementation of just the write, without doing a attribute change notification.
    //
    // Will return ActionReturnStatus::FixedStatus::kWriteSuccessNoop if the attribute write
    // is not changing the underlying data.
    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);

    // Reads a single device info string and handles error sanitization (clearing the buffer)
    // if the attribute is optional and not implemented.
    template <typename EncodeFunction>
    CHIP_ERROR ReadConfigurationString(EncodeFunction && getter, AttributeValueEncoder & encoder);

    OptionalAttributesSet mEnabledOptionalAttributes;

    Storage::String<32> mNodeLabel;
    Policy mPolicy;
};

// Externally available alias for backward compatibility
using BasicInformationCluster = BasicInformationClusterImpl<DeviceLayerBasicInformationPolicy>;

} // namespace Clusters
} // namespace app
} // namespace chip

// Include the implementation
#include <app/clusters/basic-information/BasicInformationCluster.ipp>

namespace chip {
namespace app {
namespace Clusters {

extern template class BasicInformationClusterImpl<DeviceLayerBasicInformationPolicy>;

} // namespace Clusters
} // namespace app
} // namespace chip
