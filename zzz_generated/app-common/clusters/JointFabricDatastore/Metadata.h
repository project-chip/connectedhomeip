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
inline constexpr DataModel::AttributeEntry kMetadataEntry(AnchorRootCA::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AnchorRootCA
namespace AnchorNodeID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AnchorNodeID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AnchorNodeID
namespace AnchorVendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AnchorVendorID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace AnchorVendorID
namespace FriendlyName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FriendlyName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace FriendlyName
namespace GroupKeySetList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(GroupKeySetList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace GroupKeySetList
namespace GroupList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(GroupList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace GroupList
namespace NodeList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NodeList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace NodeList
namespace AdminList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AdminList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace AdminList
namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Status::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace Status
namespace EndpointGroupIDList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EndpointGroupIDList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace EndpointGroupIDList
namespace EndpointBindingList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EndpointBindingList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace EndpointBindingList
namespace NodeKeySetList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NodeKeySetList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace NodeKeySetList
namespace NodeACLList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NodeACLList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace NodeACLList
namespace NodeEndpointList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NodeEndpointList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace NodeEndpointList

} // namespace Attributes

namespace Commands {
namespace AddKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddKeySet::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddKeySet
namespace UpdateKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(UpdateKeySet::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace UpdateKeySet
namespace RemoveKeySet {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveKeySet::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveKeySet
namespace AddGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddGroup::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddGroup
namespace UpdateGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(UpdateGroup::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace UpdateGroup
namespace RemoveGroup {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveGroup::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveGroup
namespace AddAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddAdmin::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddAdmin
namespace UpdateAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(UpdateAdmin::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace UpdateAdmin
namespace RemoveAdmin {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveAdmin::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveAdmin
namespace AddPendingNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddPendingNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddPendingNode
namespace RefreshNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RefreshNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RefreshNode
namespace UpdateNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(UpdateNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace UpdateNode
namespace RemoveNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveNode
namespace UpdateEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdateEndpointForNode::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace UpdateEndpointForNode
namespace AddGroupIDToEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddGroupIDToEndpointForNode::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace AddGroupIDToEndpointForNode
namespace RemoveGroupIDFromEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveGroupIDFromEndpointForNode::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveGroupIDFromEndpointForNode
namespace AddBindingToEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddBindingToEndpointForNode::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace AddBindingToEndpointForNode
namespace RemoveBindingFromEndpointForNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveBindingFromEndpointForNode::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveBindingFromEndpointForNode
namespace AddACLToNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddACLToNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddACLToNode
namespace RemoveACLFromNode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveACLFromNode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveACLFromNode

} // namespace Commands
} // namespace JointFabricDatastore
} // namespace Clusters
} // namespace app
} // namespace chip
