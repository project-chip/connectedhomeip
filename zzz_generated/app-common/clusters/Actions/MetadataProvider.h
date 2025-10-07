// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Actions (cluster code: 37/0x25)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Actions/Ids.h>
#include <clusters/Actions/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Actions::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Actions::Attributes;
        switch (attributeId)
        {
        case ActionList::Id:
            return ActionList::kMetadataEntry;
        case EndpointLists::Id:
            return EndpointLists::kMetadataEntry;
        case SetupURL::Id:
            return SetupURL::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Actions::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Actions::Commands;
        switch (commandId)
        {
        case InstantAction::Id:
            return InstantAction::kMetadataEntry;
        case InstantActionWithTransition::Id:
            return InstantActionWithTransition::kMetadataEntry;
        case StartAction::Id:
            return StartAction::kMetadataEntry;
        case StartActionWithDuration::Id:
            return StartActionWithDuration::kMetadataEntry;
        case StopAction::Id:
            return StopAction::kMetadataEntry;
        case PauseAction::Id:
            return PauseAction::kMetadataEntry;
        case PauseActionWithDuration::Id:
            return PauseActionWithDuration::kMetadataEntry;
        case ResumeAction::Id:
            return ResumeAction::kMetadataEntry;
        case EnableAction::Id:
            return EnableAction::kMetadataEntry;
        case EnableActionWithDuration::Id:
            return EnableActionWithDuration::kMetadataEntry;
        case DisableAction::Id:
            return DisableAction::kMetadataEntry;
        case DisableActionWithDuration::Id:
            return DisableActionWithDuration::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
