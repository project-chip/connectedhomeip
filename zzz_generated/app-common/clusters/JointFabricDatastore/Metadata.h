// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster JointFabricDatastore (cluster code: 1874/0x752)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/JointFabricDatastore/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace JointFabricDatastore {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace AnchorRootCA {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AnchorRootCA::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AnchorRootCA
namespace AnchorNodeID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AnchorNodeID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AnchorNodeID
namespace AnchorVendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AnchorVendorID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AnchorVendorID
namespace FriendlyName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FriendlyName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace FriendlyName
namespace GroupKeySetList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GroupKeySetList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace GroupKeySetList
namespace GroupList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = GroupList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace GroupList
namespace NodeList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NodeList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace NodeList
namespace AdminList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AdminList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AdminList
namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Status::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace Status
namespace EndpointGroupIDList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndpointGroupIDList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointGroupIDList
namespace EndpointBindingList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndpointBindingList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointBindingList
namespace NodeKeySetList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NodeKeySetList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace NodeKeySetList
namespace NodeACLList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NodeACLList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace NodeACLList
namespace NodeEndpointList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NodeEndpointList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace NodeEndpointList

} // namespace Attributes

namespace Commands {
namespace AddKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddKeySet::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddKeySet
namespace UpdateKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateKeySet::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateKeySet
namespace RemoveKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveKeySet::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveKeySet
namespace AddGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddGroup::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddGroup
namespace UpdateGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateGroup::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateGroup
namespace RemoveGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveGroup::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveGroup
namespace AddAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddAdmin::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddAdmin
namespace UpdateAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateAdmin::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateAdmin
namespace RemoveAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveAdmin::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveAdmin
namespace AddPendingNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddPendingNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddPendingNode
namespace RefreshNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RefreshNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RefreshNode
namespace UpdateNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateNode
namespace RemoveNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveNode
namespace UpdateEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateEndpointForNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateEndpointForNode
namespace AddGroupIDToEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddGroupIDToEndpointForNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddGroupIDToEndpointForNode
namespace RemoveGroupIDFromEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveGroupIDFromEndpointForNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveGroupIDFromEndpointForNode
namespace AddBindingToEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddBindingToEndpointForNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddBindingToEndpointForNode
namespace RemoveBindingFromEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveBindingFromEndpointForNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveBindingFromEndpointForNode
namespace AddACLToNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddACLToNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddACLToNode
namespace RemoveACLFromNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveACLFromNode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveACLFromNode

} // namespace Commands
} // namespace JointFabricDatastore
} // namespace Clusters
} // namespace app
} // namespace chip
