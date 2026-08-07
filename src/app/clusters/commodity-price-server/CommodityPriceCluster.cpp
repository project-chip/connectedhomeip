/*
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

#include "CommodityPriceCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/Metadata.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

CHIP_ERROR CommodityPriceCluster::Startup(ServerClusterContext & context)
{
    return DefaultServerCluster::Startup(context);
}

CHIP_ERROR CommodityPriceCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kForecasting), Attributes::PriceForecast::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR CommodityPriceCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;

    if (SupportsDetailedPrice())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedPriceRequest::kMetadataEntry }));
    }

    if (SupportsDetailedForecast())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedForecastRequest::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;

    // Each response is mandatory whenever its request is supported.
    if (SupportsDetailedPrice())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedPriceResponse::Id }));
    }

    if (SupportsDetailedForecast())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedForecastResponse::Id }));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus CommodityPriceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    default:
        // TODO: TariffUnit, Currency, CurrentPrice and PriceForecast.
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
CommodityPriceCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                     CommandHandler * handler)
{
    // TODO: GetDetailedPriceRequest and GetDetailedForecastRequest.
    return Status::UnsupportedCommand;
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
