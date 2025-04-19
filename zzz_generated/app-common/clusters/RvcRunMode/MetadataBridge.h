// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RvcRunMode (cluster code: 84/0x54)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/RvcRunMode/Ids.h>
#include <clusters/RvcRunMode/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::RvcRunMode::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::RvcRunMode::Attributes;
        switch (commandId)
        {
        case SupportedModes::Id:
            return SupportedModes::kMetadataEntry;
        case CurrentMode::Id:
            return CurrentMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::RvcRunMode::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::RvcRunMode::Commands;
        switch (commandId)
        {
        case ChangeToMode::Id:
            return ChangeToMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
