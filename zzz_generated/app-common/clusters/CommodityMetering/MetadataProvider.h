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
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CommodityMetering::Attributes;
        switch (attributeId)
        {
        case MeteredQuantity::Id:
            return MeteredQuantity::kMetadataEntry;
        case MeteredQuantityTimestamp::Id:
            return MeteredQuantityTimestamp::kMetadataEntry;
        case MeasurementType::Id:
            return MeasurementType::kMetadataEntry;
        default:
            return AttributeEntry({}, {}, std::nullopt, std::nullopt);
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CommodityMetering::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::CommodityMetering::Commands;
        switch (commandId)
        {

        default:
            return AcceptedCommandEntry{};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
