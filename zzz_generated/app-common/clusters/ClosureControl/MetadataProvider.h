// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ClosureControl (cluster code: 260/0x104)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ClosureControl/Ids.h>
#include <clusters/ClosureControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ClosureControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ClosureControl::Attributes;
        switch (attributeId)
        {
        case CountdownTime::Id:
            return CountdownTime::kMetadataEntry;
        case MainState::Id:
            return MainState::kMetadataEntry;
        case CurrentErrorList::Id:
            return CurrentErrorList::kMetadataEntry;
        case OverallCurrentState::Id:
            return OverallCurrentState::kMetadataEntry;
        case OverallTargetState::Id:
            return OverallTargetState::kMetadataEntry;
        case LatchControlModes::Id:
            return LatchControlModes::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ClosureControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ClosureControl::Commands;
        switch (commandId)
        {
        case Stop::Id:
            return Stop::kMetadataEntry;
        case MoveTo::Id:
            return MoveTo::kMetadataEntry;
        case Calibrate::Id:
            return Calibrate::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
