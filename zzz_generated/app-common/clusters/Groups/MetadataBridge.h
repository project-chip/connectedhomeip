// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Groups/Ids.h>
#include <clusters/Groups/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Groups::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Groups::Attributes;
        switch (commandId)
        {
        case NameSupport::Id:
            return NameSupport::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Groups::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Groups::Commands;
        switch (commandId)
        {
        case AddGroup::Id:
            return AddGroup::kMetadataEntry;
        case ViewGroup::Id:
            return ViewGroup::kMetadataEntry;
        case GetGroupMembership::Id:
            return GetGroupMembership::kMetadataEntry;
        case RemoveGroup::Id:
            return RemoveGroup::kMetadataEntry;
        case RemoveAllGroups::Id:
            return RemoveAllGroups::kMetadataEntry;
        case AddGroupIfIdentifying::Id:
            return AddGroupIfIdentifying::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
