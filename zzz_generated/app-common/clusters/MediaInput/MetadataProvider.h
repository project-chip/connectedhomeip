// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaInput (cluster code: 1287/0x507)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/MediaInput/Ids.h>
#include <clusters/MediaInput/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::MediaInput::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::MediaInput::Attributes;
        switch (attributeId)
        {
        case InputList::Id:
            return InputList::kMetadataEntry;
        case CurrentInput::Id:
            return CurrentInput::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::MediaInput::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::MediaInput::Commands;
        switch (commandId)
        {
        case SelectInput::Id:
            return SelectInput::kMetadataEntry;
        case ShowInputStatus::Id:
            return ShowInputStatus::kMetadataEntry;
        case HideInputStatus::Id:
            return HideInputStatus::kMetadataEntry;
        case RenameInput::Id:
            return RenameInput::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
