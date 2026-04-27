// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Groups::Attributes;
        switch (attributeId)
        {
        case NameSupport::Id:
            return NameSupport::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Groups::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
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
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
