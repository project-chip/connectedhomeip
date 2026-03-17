// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BallastConfiguration (cluster code: 769/0x301)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/BallastConfiguration/Ids.h>
#include <clusters/BallastConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::BallastConfiguration::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::BallastConfiguration::Attributes;
        switch (attributeId)
        {
        case PhysicalMinLevel::Id:
            return PhysicalMinLevel::kMetadataEntry;
        case PhysicalMaxLevel::Id:
            return PhysicalMaxLevel::kMetadataEntry;
        case BallastStatus::Id:
            return BallastStatus::kMetadataEntry;
        case MinLevel::Id:
            return MinLevel::kMetadataEntry;
        case MaxLevel::Id:
            return MaxLevel::kMetadataEntry;
        case IntrinsicBallastFactor::Id:
            return IntrinsicBallastFactor::kMetadataEntry;
        case BallastFactorAdjustment::Id:
            return BallastFactorAdjustment::kMetadataEntry;
        case LampQuantity::Id:
            return LampQuantity::kMetadataEntry;
        case LampType::Id:
            return LampType::kMetadataEntry;
        case LampManufacturer::Id:
            return LampManufacturer::kMetadataEntry;
        case LampRatedHours::Id:
            return LampRatedHours::kMetadataEntry;
        case LampBurnHours::Id:
            return LampBurnHours::kMetadataEntry;
        case LampAlarmMode::Id:
            return LampAlarmMode::kMetadataEntry;
        case LampBurnHoursTripPoint::Id:
            return LampBurnHoursTripPoint::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::BallastConfiguration::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::BallastConfiguration::Commands;
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
