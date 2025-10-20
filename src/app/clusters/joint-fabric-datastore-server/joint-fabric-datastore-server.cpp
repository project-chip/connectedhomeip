/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Joint Fabric Datastore Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace JointFabricDatastoreCluster = chip::app::Clusters::JointFabricDatastore;

class JointFabricDatastoreAttrAccess : public AttributeAccessInterface, public app::JointFabricDatastore::Listener
{
public:
    JointFabricDatastoreAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), JointFabricDatastoreCluster::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    void MarkNodeListChanged() override;

private:
    CHIP_ERROR ReadAnchorRootCA(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAnchorNodeId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAnchorVendorId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFriendlyName(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadGroupKeySetList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadGroupList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNodeList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAdminList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadStatus(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadEndpointGroupIDList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadEndpointBindingList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNodeKeySetList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNodeACLList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNodeEndpointList(AttributeValueEncoder & aEncoder);
};

JointFabricDatastoreAttrAccess gJointFabricDatastoreAttrAccess;

CHIP_ERROR JointFabricDatastoreAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == JointFabricDatastoreCluster::Id);

    switch (aPath.mAttributeId)
    {
    case JointFabricDatastoreCluster::Attributes::AnchorRootCA::Id: {
        return ReadAnchorRootCA(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::AnchorNodeID::Id: {
        return ReadAnchorNodeId(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::AnchorVendorID::Id: {
        return ReadAnchorVendorId(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::FriendlyName::Id: {
        return ReadFriendlyName(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::GroupKeySetList::Id: {
        return ReadGroupKeySetList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::GroupList::Id: {
        return ReadGroupList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::NodeList::Id: {
        return ReadNodeList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::AdminList::Id: {
        return ReadAdminList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::Status::Id: {
        return ReadStatus(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::EndpointGroupIDList::Id: {
        return ReadEndpointGroupIDList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::EndpointBindingList::Id: {
        return ReadEndpointBindingList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::NodeKeySetList::Id: {
        return ReadNodeKeySetList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::NodeACLList::Id: {
        return ReadNodeACLList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::NodeEndpointList::Id: {
        return ReadNodeEndpointList(aEncoder);
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAnchorRootCA(AttributeValueEncoder & aEncoder)
{
    ByteSpan anchorRootCA = Server::GetInstance().GetJointFabricDatastore().GetAnchorRootCA();
    ReturnErrorOnFailure(aEncoder.Encode(anchorRootCA));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAnchorNodeId(AttributeValueEncoder & aEncoder)
{
    NodeId anchorNodeId = Server::GetInstance().GetJointFabricDatastore().GetAnchorNodeId();
    ReturnErrorOnFailure(aEncoder.Encode(anchorNodeId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAnchorVendorId(AttributeValueEncoder & aEncoder)
{
    VendorId anchorVendorId = Server::GetInstance().GetJointFabricDatastore().GetAnchorVendorId();
    ReturnErrorOnFailure(aEncoder.Encode(anchorVendorId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadFriendlyName(AttributeValueEncoder & aEncoder)
{
    CharSpan friendlyName = Server::GetInstance().GetJointFabricDatastore().GetFriendlyName();
    ReturnErrorOnFailure(aEncoder.Encode(friendlyName));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadGroupKeySetList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetGroupKeySetList();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadGroupList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetGroupEntries();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadNodeList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetNodeInformationEntries();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAdminList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetAdminEntries();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadStatus(AttributeValueEncoder & aEncoder)
{
    auto status = Server::GetInstance().GetJointFabricDatastore().GetStatus();
    return aEncoder.Encode(status);
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadEndpointGroupIDList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetEndpointGroupIDList();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadEndpointBindingList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetEndpointBindingList();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadNodeKeySetList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetNodeKeySetList();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadNodeACLList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetNodeACLList();

        for (auto & entry : entries)
        {
            Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToEncode;
            entryToEncode.nodeID             = entry.nodeID;
            entryToEncode.listID             = entry.listID;
            entryToEncode.ACLEntry.privilege = entry.ACLEntry.privilege;
            entryToEncode.ACLEntry.authMode  = entry.ACLEntry.authMode;
            entryToEncode.ACLEntry.subjects =
                DataModel::List<const uint64_t>(entry.ACLEntry.subjects.data(), entry.ACLEntry.subjects.size());
            entryToEncode.ACLEntry.targets =
                DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
                    entry.ACLEntry.targets.data(), entry.ACLEntry.targets.size());
            entryToEncode.statusEntry = entry.statusEntry;
            ReturnErrorOnFailure(encoder.Encode(entryToEncode));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR JointFabricDatastoreAttrAccess::ReadNodeEndpointList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        auto entries = Server::GetInstance().GetJointFabricDatastore().GetNodeEndpointList();

        for (auto & entry : entries)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

void JointFabricDatastoreAttrAccess::MarkNodeListChanged()
{
    MatterReportingAttributeChangeCallback(kRootEndpointId, JointFabricDatastoreCluster::Id,
                                           JointFabricDatastoreCluster::Attributes::NodeList::Id);
}

bool emberAfJointFabricDatastoreClusterAddKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddKeySet::DecodableType & commandData)
{
    CHIP_ERROR err                                                                              = CHIP_NO_ERROR;
    JointFabricDatastoreCluster::Structs::DatastoreGroupKeySetStruct::DecodableType groupKeySet = commandData.groupKeySet;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    VerifyOrExit(jointFabricDatastore.IsGroupKeySetEntryPresent(groupKeySet.groupKeySetID) == false,
                 err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SuccessOrExit(err = jointFabricDatastore.AddGroupKeySetEntry(groupKeySet));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterUpdateKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateKeySet::DecodableType & commandData)
{
    CHIP_ERROR err                                                                              = CHIP_NO_ERROR;
    JointFabricDatastoreCluster::Structs::DatastoreGroupKeySetStruct::DecodableType groupKeySet = commandData.groupKeySet;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    VerifyOrExit(jointFabricDatastore.IsGroupKeySetEntryPresent(groupKeySet.groupKeySetID), err = CHIP_ERROR_NOT_FOUND);
    SuccessOrExit(err = jointFabricDatastore.UpdateGroupKeySetEntry(groupKeySet));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveKeySet::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    uint16_t groupKeySetId                           = commandData.groupKeySetID;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    VerifyOrExit(jointFabricDatastore.IsGroupKeySetEntryPresent(groupKeySetId), err = CHIP_ERROR_NOT_FOUND);
    SuccessOrExit(err = jointFabricDatastore.RemoveGroupKeySetEntry(groupKeySetId));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddGroup::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    SuccessOrExit(err = jointFabricDatastore.AddGroup(commandData));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterUpdateGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateGroup::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    SuccessOrExit(err = jointFabricDatastore.UpdateGroup(commandData));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveGroup::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    SuccessOrExit(err = jointFabricDatastore.RemoveGroup(commandData));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddAdmin::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    auto nodeID       = commandData.nodeID;
    auto friendlyName = commandData.friendlyName;
    auto vendorID     = commandData.vendorID;
    auto icac         = commandData.icac;

    JointFabricDatastoreCluster::Structs::DatastoreAdministratorInformationEntryStruct::DecodableType adminInformationEntry = {
        .nodeID = nodeID, .friendlyName = friendlyName, .vendorID = vendorID, .icac = icac
    };

    SuccessOrExit(err = jointFabricDatastore.AddAdmin(adminInformationEntry));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterUpdateAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateAdmin::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    auto nodeId                                      = commandData.nodeID.Value();
    auto friendlyName                                = commandData.friendlyName.Value();
    auto icac                                        = commandData.icac.Value();
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    SuccessOrExit(err = jointFabricDatastore.UpdateAdmin(nodeId, friendlyName, icac));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveAdmin::DecodableType & commandData)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    auto nodeId                                      = commandData.nodeID;
    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    SuccessOrExit(err = jointFabricDatastore.RemoveAdmin(nodeId));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddPendingNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddPendingNode::DecodableType & commandData)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    NodeId nodeId                 = commandData.nodeID;
    const CharSpan & friendlyName = commandData.friendlyName;

    SuccessOrExit(err = Server::GetInstance().GetJointFabricDatastore().AddPendingNode(nodeId, friendlyName));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRefreshNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RefreshNode::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    NodeId nodeId  = commandData.nodeID;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsList;
    // TODO: Get Endpoints List from connected device with <nodeId>

    SuccessOrExit(err = jointFabricDatastore.RefreshNode(nodeId, endpointsList.TakeBuffer()));

exit:
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterUpdateNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateNode::DecodableType & commandData)
{
    NodeId nodeId                 = commandData.nodeID;
    const CharSpan & friendlyName = commandData.friendlyName;

    CHIP_ERROR err = Server::GetInstance().GetJointFabricDatastore().UpdateNode(nodeId, friendlyName);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveNode::DecodableType & commandData)
{
    NodeId nodeId = commandData.nodeID;

    CHIP_ERROR err = Server::GetInstance().GetJointFabricDatastore().RemoveNode(nodeId);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterUpdateEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateEndpointForNode::DecodableType & commandData)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    NodeId nodeId                 = commandData.nodeID;
    EndpointId endpointId         = commandData.endpointID;
    const CharSpan & friendlyName = commandData.friendlyName;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.UpdateEndpointForNode(nodeId, endpointId, friendlyName);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddGroupIDToEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddGroupIDToEndpointForNode::DecodableType & commandData)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    NodeId nodeId         = commandData.nodeID;
    EndpointId endpointId = commandData.endpointID;
    GroupId groupID       = commandData.groupID;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.AddGroupIDToEndpointForNode(nodeId, endpointId, groupID);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveGroupIDFromEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveGroupIDFromEndpointForNode::DecodableType & commandData)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    NodeId nodeId         = commandData.nodeID;
    EndpointId endpointId = commandData.endpointID;
    GroupId groupID       = commandData.groupID;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.RemoveGroupIDFromEndpointForNode(nodeId, endpointId, groupID);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddBindingToEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddBindingToEndpointForNode::DecodableType & commandData)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    NodeId nodeId         = commandData.nodeID;
    EndpointId endpointId = commandData.endpointID;
    auto & binding        = commandData.binding;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.AddBindingToEndpointForNode(nodeId, endpointId, binding);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveBindingFromEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveBindingFromEndpointForNode::DecodableType & commandData)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    uint16_t listId       = commandData.listID;
    NodeId nodeId         = commandData.nodeID;
    EndpointId endpointId = commandData.endpointID;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.RemoveBindingFromEndpointForNode(listId, nodeId, endpointId);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterAddACLToNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddACLToNode::DecodableType & commandData)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    NodeId nodeId   = commandData.nodeID;
    auto & aclEntry = commandData.ACLEntry;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.AddACLToNode(nodeId, aclEntry);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

bool emberAfJointFabricDatastoreClusterRemoveACLFromNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveACLFromNode::DecodableType & commandData)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint16_t listId = commandData.listID;
    NodeId nodeId   = commandData.nodeID;

    app::JointFabricDatastore & jointFabricDatastore = Server::GetInstance().GetJointFabricDatastore();

    err = jointFabricDatastore.RemoveACLFromNode(listId, nodeId);

    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        ChipLogError(DataManagement, "JointFabricDatastoreCluster: failed with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::ClusterStatusCode(err));
    }

    return true;
}

void MatterJointFabricDatastorePluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initiating Joint Fabric Datastore cluster.");
    AttributeAccessInterfaceRegistry::Instance().Register(&gJointFabricDatastoreAttrAccess);

    Server::GetInstance().GetJointFabricDatastore().AddListener(gJointFabricDatastoreAttrAccess);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
