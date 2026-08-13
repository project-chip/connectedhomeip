// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster JointFabricDatastore (cluster code: 1874/0x752)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/JointFabricDatastore/Ids.h>
#include <clusters/JointFabricDatastore/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::JointFabricDatastore::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::JointFabricDatastore::Attributes;
        switch (attributeId)
        {
        case AnchorRootCA::Id:
            return AnchorRootCA::kMetadataEntry;
        case AnchorNodeID::Id:
            return AnchorNodeID::kMetadataEntry;
        case AnchorVendorID::Id:
            return AnchorVendorID::kMetadataEntry;
        case FriendlyName::Id:
            return FriendlyName::kMetadataEntry;
        case GroupKeySetList::Id:
            return GroupKeySetList::kMetadataEntry;
        case GroupList::Id:
            return GroupList::kMetadataEntry;
        case NodeList::Id:
            return NodeList::kMetadataEntry;
        case AdminList::Id:
            return AdminList::kMetadataEntry;
        case Status::Id:
            return Status::kMetadataEntry;
        case EndpointGroupIDList::Id:
            return EndpointGroupIDList::kMetadataEntry;
        case EndpointBindingList::Id:
            return EndpointBindingList::kMetadataEntry;
        case NodeKeySetList::Id:
            return NodeKeySetList::kMetadataEntry;
        case NodeACLList::Id:
            return NodeACLList::kMetadataEntry;
        case NodeEndpointList::Id:
            return NodeEndpointList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::JointFabricDatastore::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::JointFabricDatastore::Commands;
        switch (commandId)
        {
        case AddKeySet::Id:
            return AddKeySet::kMetadataEntry;
        case UpdateKeySet::Id:
            return UpdateKeySet::kMetadataEntry;
        case RemoveKeySet::Id:
            return RemoveKeySet::kMetadataEntry;
        case AddGroup::Id:
            return AddGroup::kMetadataEntry;
        case UpdateGroup::Id:
            return UpdateGroup::kMetadataEntry;
        case RemoveGroup::Id:
            return RemoveGroup::kMetadataEntry;
        case AddAdmin::Id:
            return AddAdmin::kMetadataEntry;
        case UpdateAdmin::Id:
            return UpdateAdmin::kMetadataEntry;
        case RemoveAdmin::Id:
            return RemoveAdmin::kMetadataEntry;
        case AddPendingNode::Id:
            return AddPendingNode::kMetadataEntry;
        case RefreshNode::Id:
            return RefreshNode::kMetadataEntry;
        case UpdateNode::Id:
            return UpdateNode::kMetadataEntry;
        case RemoveNode::Id:
            return RemoveNode::kMetadataEntry;
        case UpdateEndpointForNode::Id:
            return UpdateEndpointForNode::kMetadataEntry;
        case AddGroupIDToEndpointForNode::Id:
            return AddGroupIDToEndpointForNode::kMetadataEntry;
        case RemoveGroupIDFromEndpointForNode::Id:
            return RemoveGroupIDFromEndpointForNode::kMetadataEntry;
        case AddBindingToEndpointForNode::Id:
            return AddBindingToEndpointForNode::kMetadataEntry;
        case RemoveBindingFromEndpointForNode::Id:
            return RemoveBindingFromEndpointForNode::kMetadataEntry;
        case AddACLToNode::Id:
            return AddACLToNode::kMetadataEntry;
        case RemoveACLFromNode::Id:
            return RemoveACLFromNode::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
