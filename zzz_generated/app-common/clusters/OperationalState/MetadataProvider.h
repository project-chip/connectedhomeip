// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OperationalState (cluster code: 96/0x60)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OperationalState/Ids.h>
#include <clusters/OperationalState/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OperationalState::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::OperationalState::Attributes;
        switch (attributeId)
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
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OperationalState::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::OperationalState::Commands;
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
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
