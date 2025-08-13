// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Chime::Attributes;
        switch (attributeId)
        {
        case InstalledChimeSounds::Id:
            return InstalledChimeSounds::kMetadataEntry;
        case SelectedChime::Id:
            return SelectedChime::kMetadataEntry;
        case Enabled::Id:
            return Enabled::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Chime::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Chime::Commands;
        switch (commandId)
        {
        case PlayChimeSound::Id:
            return PlayChimeSound::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
