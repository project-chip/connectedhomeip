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
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace JointFabricDatastoreCluster = chip::app::Clusters::JointFabricDatastore;

class JointFabricDatastoreAttrAccess : public AttributeAccessInterface
{
public:
    JointFabricDatastoreAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), JointFabricDatastoreCluster::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadAnchorNodeId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAnchorVendorId(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadGroupKeySetList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAdminList(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNodeList(AttributeValueEncoder & aEncoder);
};

JointFabricDatastoreAttrAccess gJointFabricDatastoreAttrAccess;

CHIP_ERROR JointFabricDatastoreAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == JointFabricDatastoreCluster::Id);

    switch (aPath.mAttributeId)
    {
    case JointFabricDatastoreCluster::Attributes::AnchorNodeID::Id: {
        return ReadAnchorNodeId(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::AnchorVendorID::Id: {
        return ReadAnchorVendorId(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::GroupKeySetList::Id: {
        return ReadGroupKeySetList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::AdminList::Id: {
        return ReadAdminList(aEncoder);
    }
    case JointFabricDatastoreCluster::Attributes::NodeList::Id: {
        return ReadNodeList(aEncoder);
    }
    // TODO: Others
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

// TODO
CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAnchorNodeId(AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// TODO
CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAnchorVendorId(AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// TODO
CHIP_ERROR JointFabricDatastoreAttrAccess::ReadGroupKeySetList(AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// TODO
CHIP_ERROR JointFabricDatastoreAttrAccess::ReadAdminList(AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// TODO
CHIP_ERROR JointFabricDatastoreAttrAccess::ReadNodeList(AttributeValueEncoder & aEncoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddAdmin::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddGroup::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddKeySet::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterUpdateNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRefreshNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RefreshNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveAdmin::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveGroup::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterUpdateAdminCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateAdmin::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterUpdateGroupCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateGroup::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddACLToNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddACLToNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveKeySet::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterUpdateKeySetCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateKeySet::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddPendingNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddPendingNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveACLFromNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveACLFromNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterUpdateEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::UpdateEndpointForNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddBindingToEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddBindingToEndpointForNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterAddGroupIDToEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::AddGroupIDToEndpointForNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveBindingFromEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveBindingFromEndpointForNode::DecodableType & commandData)
{
    return true;
}

// TODO
bool emberAfJointFabricDatastoreClusterRemoveGroupIDFromEndpointForNodeCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const JointFabricDatastoreCluster::Commands::RemoveGroupIDFromEndpointForNode::DecodableType & commandData)
{
    return true;
}

void MatterJointFabricDatastorePluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initiating Joint Fabric Datastore cluster.");
    AttributeAccessInterfaceRegistry::Instance().Register(&gJointFabricDatastoreAttrAccess);
}
