// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterHeaterManagement (cluster code: 148/0x94)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WaterHeaterManagement/Ids.h>
#include <clusters/WaterHeaterManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WaterHeaterManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WaterHeaterManagement::Attributes;
        switch (attributeId)
        {
        case HeaterTypes::Id:
            return HeaterTypes::kMetadataEntry;
        case HeatDemand::Id:
            return HeatDemand::kMetadataEntry;
        case TankVolume::Id:
            return TankVolume::kMetadataEntry;
        case EstimatedHeatRequired::Id:
            return EstimatedHeatRequired::kMetadataEntry;
        case TankPercentage::Id:
            return TankPercentage::kMetadataEntry;
        case BoostState::Id:
            return BoostState::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WaterHeaterManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::WaterHeaterManagement::Commands;
        switch (commandId)
        {
        case Boost::Id:
            return Boost::kMetadataEntry;
        case CancelBoost::Id:
            return CancelBoost::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
