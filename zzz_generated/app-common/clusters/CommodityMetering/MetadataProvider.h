// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityMetering (cluster code: 2823/0xB07)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CommodityMetering/Ids.h>
#include <clusters/CommodityMetering/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CommodityMetering::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CommodityMetering::Attributes;
        switch (attributeId)
        {
        case MeteredQuantity::Id:
            return MeteredQuantity::kMetadataEntry;
        case MeteredQuantityTimestamp::Id:
            return MeteredQuantityTimestamp::kMetadataEntry;
        case TariffUnit::Id:
            return TariffUnit::kMetadataEntry;
        case MaximumMeteredQuantities::Id:
            return MaximumMeteredQuantities::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommodityMetering::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommodityMetering::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
