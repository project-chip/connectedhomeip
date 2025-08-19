// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FanControl (cluster code: 514/0x202)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/FanControl/Ids.h>
#include <clusters/FanControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::FanControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::FanControl::Attributes;
        switch (attributeId)
        {
        case FanMode::Id:
            return FanMode::kMetadataEntry;
        case FanModeSequence::Id:
            return FanModeSequence::kMetadataEntry;
        case PercentSetting::Id:
            return PercentSetting::kMetadataEntry;
        case PercentCurrent::Id:
            return PercentCurrent::kMetadataEntry;
        case SpeedMax::Id:
            return SpeedMax::kMetadataEntry;
        case SpeedSetting::Id:
            return SpeedSetting::kMetadataEntry;
        case SpeedCurrent::Id:
            return SpeedCurrent::kMetadataEntry;
        case RockSupport::Id:
            return RockSupport::kMetadataEntry;
        case RockSetting::Id:
            return RockSetting::kMetadataEntry;
        case WindSupport::Id:
            return WindSupport::kMetadataEntry;
        case WindSetting::Id:
            return WindSetting::kMetadataEntry;
        case AirflowDirection::Id:
            return AirflowDirection::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::FanControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::FanControl::Commands;
        switch (commandId)
        {
        case Step::Id:
            return Step::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
