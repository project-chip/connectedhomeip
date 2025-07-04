// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ValveConfigurationAndControl (cluster code: 129/0x81)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ValveConfigurationAndControl/Ids.h>
#include <clusters/ValveConfigurationAndControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ValveConfigurationAndControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ValveConfigurationAndControl::Attributes;
        switch (attributeId)
        {
        case OpenDuration::Id:
            return OpenDuration::kMetadataEntry;
        case DefaultOpenDuration::Id:
            return DefaultOpenDuration::kMetadataEntry;
        case AutoCloseTime::Id:
            return AutoCloseTime::kMetadataEntry;
        case RemainingDuration::Id:
            return RemainingDuration::kMetadataEntry;
        case CurrentState::Id:
            return CurrentState::kMetadataEntry;
        case TargetState::Id:
            return TargetState::kMetadataEntry;
        case CurrentLevel::Id:
            return CurrentLevel::kMetadataEntry;
        case TargetLevel::Id:
            return TargetLevel::kMetadataEntry;
        case DefaultOpenLevel::Id:
            return DefaultOpenLevel::kMetadataEntry;
        case ValveFault::Id:
            return ValveFault::kMetadataEntry;
        case LevelStep::Id:
            return LevelStep::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ValveConfigurationAndControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ValveConfigurationAndControl::Commands;
        switch (commandId)
        {
        case Open::Id:
            return Open::kMetadataEntry;
        case Close::Id:
            return Close::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
