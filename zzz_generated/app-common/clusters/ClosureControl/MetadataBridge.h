// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ClosureControl (cluster code: 260/0x104)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

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
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ClosureControl::Attributes;
        switch (commandId)
        {
        case CountdownTime::Id:
            return CountdownTime::kMetadataEntry;
        case MainState::Id:
            return MainState::kMetadataEntry;
        case CurrentErrorList::Id:
            return CurrentErrorList::kMetadataEntry;
        case OverallState::Id:
            return OverallState::kMetadataEntry;
        case OverallTarget::Id:
            return OverallTarget::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ClosureControl::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
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
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
