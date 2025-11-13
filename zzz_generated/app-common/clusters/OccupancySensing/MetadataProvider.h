// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OccupancySensing (cluster code: 1030/0x406)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OccupancySensing/Ids.h>
#include <clusters/OccupancySensing/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OccupancySensing::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::OccupancySensing::Attributes;
        switch (attributeId)
        {
        case Occupancy::Id:
            return Occupancy::kMetadataEntry;
        case OccupancySensorType::Id:
            return OccupancySensorType::kMetadataEntry;
        case OccupancySensorTypeBitmap::Id:
            return OccupancySensorTypeBitmap::kMetadataEntry;
        case HoldTime::Id:
            return HoldTime::kMetadataEntry;
        case HoldTimeLimits::Id:
            return HoldTimeLimits::kMetadataEntry;
        case PIROccupiedToUnoccupiedDelay::Id:
            return PIROccupiedToUnoccupiedDelay::kMetadataEntry;
        case PIRUnoccupiedToOccupiedDelay::Id:
            return PIRUnoccupiedToOccupiedDelay::kMetadataEntry;
        case PIRUnoccupiedToOccupiedThreshold::Id:
            return PIRUnoccupiedToOccupiedThreshold::kMetadataEntry;
        case UltrasonicOccupiedToUnoccupiedDelay::Id:
            return UltrasonicOccupiedToUnoccupiedDelay::kMetadataEntry;
        case UltrasonicUnoccupiedToOccupiedDelay::Id:
            return UltrasonicUnoccupiedToOccupiedDelay::kMetadataEntry;
        case UltrasonicUnoccupiedToOccupiedThreshold::Id:
            return UltrasonicUnoccupiedToOccupiedThreshold::kMetadataEntry;
        case PhysicalContactOccupiedToUnoccupiedDelay::Id:
            return PhysicalContactOccupiedToUnoccupiedDelay::kMetadataEntry;
        case PhysicalContactUnoccupiedToOccupiedDelay::Id:
            return PhysicalContactUnoccupiedToOccupiedDelay::kMetadataEntry;
        case PhysicalContactUnoccupiedToOccupiedThreshold::Id:
            return PhysicalContactUnoccupiedToOccupiedThreshold::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OccupancySensing::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::OccupancySensing::Commands;
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
