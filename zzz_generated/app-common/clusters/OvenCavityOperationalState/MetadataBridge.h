// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OvenCavityOperationalState (cluster code: 72/0x48)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OvenCavityOperationalState/Ids.h>
#include <clusters/OvenCavityOperationalState/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OvenCavityOperationalState::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::OvenCavityOperationalState::Attributes;
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
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OvenCavityOperationalState::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::OvenCavityOperationalState::Commands;
        switch (commandId)
        {
        case Pause::Id:
            return Pause::kMetadataEntry;
        case Stop::Id:
            return Stop::kMetadataEntry;
        case Start::Id:
            return Start::kMetadataEntry;
        case Resume::Id:
            return Resume::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
