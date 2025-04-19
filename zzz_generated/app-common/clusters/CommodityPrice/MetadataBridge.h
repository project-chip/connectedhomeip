// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityPrice (cluster code: 149/0x95)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CommodityPrice/Ids.h>
#include <clusters/CommodityPrice/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CommodityPrice::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::CommodityPrice::Attributes;
        switch (commandId)
        {
        case TariffUnit::Id:
            return TariffUnit::kMetadataEntry;
        case Currency::Id:
            return Currency::kMetadataEntry;
        case CurrentPrice::Id:
            return CurrentPrice::kMetadataEntry;
        case PriceForecast::Id:
            return PriceForecast::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommodityPrice::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommodityPrice::Commands;
        switch (commandId)
        {
        case GetDetailedPriceRequest::Id:
            return GetDetailedPriceRequest::kMetadataEntry;
        case GetDetailedForecastRequest::Id:
            return GetDetailedForecastRequest::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
