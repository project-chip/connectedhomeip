// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioOutput (cluster code: 1291/0x50B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AudioOutput/Ids.h>
#include <clusters/AudioOutput/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AudioOutput::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::AudioOutput::Attributes;
        switch (commandId)
        {
        case OutputList::Id:
            return OutputList::kMetadataEntry;
        case CurrentOutput::Id:
            return CurrentOutput::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AudioOutput::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::AudioOutput::Commands;
        switch (commandId)
        {
        case SelectOutput::Id:
            return SelectOutput::kMetadataEntry;
        case RenameOutput::Id:
            return RenameOutput::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
