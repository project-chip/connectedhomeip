// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LevelControl (cluster code: 8/0x8)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LevelControl/Ids.h>
#include <clusters/LevelControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LevelControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LevelControl::Attributes;
        switch (attributeId)
        {
        case CurrentLevel::Id:
            return CurrentLevel::kMetadataEntry;
        case RemainingTime::Id:
            return RemainingTime::kMetadataEntry;
        case MinLevel::Id:
            return MinLevel::kMetadataEntry;
        case MaxLevel::Id:
            return MaxLevel::kMetadataEntry;
        case CurrentFrequency::Id:
            return CurrentFrequency::kMetadataEntry;
        case MinFrequency::Id:
            return MinFrequency::kMetadataEntry;
        case MaxFrequency::Id:
            return MaxFrequency::kMetadataEntry;
        case Options::Id:
            return Options::kMetadataEntry;
        case OnOffTransitionTime::Id:
            return OnOffTransitionTime::kMetadataEntry;
        case OnLevel::Id:
            return OnLevel::kMetadataEntry;
        case OnTransitionTime::Id:
            return OnTransitionTime::kMetadataEntry;
        case OffTransitionTime::Id:
            return OffTransitionTime::kMetadataEntry;
        case DefaultMoveRate::Id:
            return DefaultMoveRate::kMetadataEntry;
        case StartUpCurrentLevel::Id:
            return StartUpCurrentLevel::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LevelControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LevelControl::Commands;
        switch (commandId)
        {
        case MoveToLevel::Id:
            return MoveToLevel::kMetadataEntry;
        case Move::Id:
            return Move::kMetadataEntry;
        case Step::Id:
            return Step::kMetadataEntry;
        case Stop::Id:
            return Stop::kMetadataEntry;
        case MoveToLevelWithOnOff::Id:
            return MoveToLevelWithOnOff::kMetadataEntry;
        case MoveWithOnOff::Id:
            return MoveWithOnOff::kMetadataEntry;
        case StepWithOnOff::Id:
            return StepWithOnOff::kMetadataEntry;
        case StopWithOnOff::Id:
            return StopWithOnOff::kMetadataEntry;
        case MoveToClosestFrequency::Id:
            return MoveToClosestFrequency::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
