// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RvcOperationalState (cluster code: 97/0x61)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/RvcOperationalState/Ids.h>
#include <clusters/RvcOperationalState/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::RvcOperationalState::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::RvcOperationalState::Attributes;
        switch (commandId)
        {
        case PhaseList::Id:
            return PhaseList::kMetadataEntry;
        case CurrentPhase::Id:
            return CurrentPhase::kMetadataEntry;
        case CountdownTime::Id:
            return CountdownTime::kMetadataEntry;
        case OperationalStateList::Id:
            return OperationalStateList::kMetadataEntry;
        case OperationalState::Id:
            return OperationalState::kMetadataEntry;
        case OperationalError::Id:
            return OperationalError::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::RvcOperationalState::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::RvcOperationalState::Commands;
        switch (commandId)
        {
        case Pause::Id:
            return Pause::kMetadataEntry;
        case Resume::Id:
            return Resume::kMetadataEntry;
        case GoHome::Id:
            return GoHome::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
