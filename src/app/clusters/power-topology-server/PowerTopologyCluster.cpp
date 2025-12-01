/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PowerTopology/Metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

CHIP_ERROR PowerTopologyCluster::GetAvailableEndpoints(AttributeValueEncoder & aEncoder) const
{
    VerifyOrReturnError(mFeatureFlags.Has(Feature::kSetTopology), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Power Topology: can not get AvailableEndpoints, feature is not supported"));
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetAvailableEndpointAtIndex(i, endpointId);

            VerifyOrReturnError(!(err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED), CHIP_NO_ERROR); // End of list, safe to exit normally

            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

CHIP_ERROR PowerTopologyCluster::GetActiveEndpoints(AttributeValueEncoder & aEncoder) const
{
    VerifyOrReturnError(mFeatureFlags.Has(Feature::kDynamicPowerFlow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Power Topology: can not get ActiveEndpoints, feature is not supported"));

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetActiveEndpointAtIndex(i, endpointId);

            VerifyOrReturnError(!(err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED), CHIP_NO_ERROR); // End of list, safe to exit normally

            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

CHIP_ERROR PowerTopologyCluster::Startup(ServerClusterContext & context)
{

    VerifyOrReturnError(
        !(mFeatureFlags.Has(Feature::kDynamicPowerFlow) && !mFeatureFlags.Has(Feature::kSetTopology)), CHIP_ERROR_INCORRECT_STATE,
        ChipLogError(Zcl, "Power Topology Cluster: DynamicPowerFlow feature requires SetTopology feature to be enabled"));
    return DefaultServerCluster::Startup(context);
}

DataModel::ActionReturnStatus PowerTopologyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case ClusterRevision::Id:
        return encoder.Encode(kRevision);

    case AvailableEndpoints::Id:
        return GetAvailableEndpoints(encoder);

    case ActiveEndpoints::Id:
        return GetActiveEndpoints(encoder);

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR PowerTopologyCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        AvailableEndpoints::kMetadataEntry, //
        ActiveEndpoints::kMetadataEntry,    //
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
