// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Chime/Ids.h>
#include <clusters/Chime/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Chime::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Chime::Attributes;
        switch (commandId)
        {
        case InstalledChimeSounds::Id:
            return InstalledChimeSounds::kMetadataEntry;
        case SelectedChime::Id:
            return SelectedChime::kMetadataEntry;
        case Enabled::Id:
            return Enabled::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Chime::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Chime::Commands;
        switch (commandId)
        {
        case PlayChimeSound::Id:
            return PlayChimeSound::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
