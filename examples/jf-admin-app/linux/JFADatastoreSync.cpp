/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include "JFADatastoreSync.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>

#include <controller/CHIPCluster.h>
#include <lib/support/logging/CHIPLogging.h>

#include <controller/CHIPDeviceController.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::app::Clusters::JointFabricDatastore;

extern DeviceCommissioner * GetDeviceCommissioner();

constexpr uint8_t kJFAvailableShift = 0;
constexpr uint8_t kJFAdminShift     = 1;
constexpr uint8_t kJFAnchorShift    = 2;
constexpr uint8_t kJFDatastoreShift = 3;

static constexpr EndpointId kJFDatastoreClusterEndpointId = 1;

JFADatastoreSync JFADatastoreSync::sJFDS;

template <typename T>
class DevicePairedCommand
{
public:
    struct CallbackContext
    {
        chip::NodeId nodeId;
        T objectToWrite;
        std::function<void()> onSuccess;

        CallbackContext(chip::NodeId nId, T object, std::function<void()> onSuccessFn) :
            nodeId(nId), objectToWrite(object), onSuccess(onSuccessFn)
        {}
    };
    DevicePairedCommand(
        chip::NodeId nodeId, T object, std::function<void()> onSuccess = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext = std::make_shared<CallbackContext>(nodeId, object, onSuccess);
    }

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle)
    {
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;

        CHIP_ERROR err = CHIP_NO_ERROR;

        if (pairingCommand)
        {
            ChipLogProgress(DeviceLayer, "OnDeviceConnectedFn - Syncing device with node id: " ChipLogFormatX64,
                            ChipLogValueX64(cbContext->nodeId));

            // Static LUT for mapping type T to attribute ID
            AttributeId attributeId = 0;

            if constexpr (std::is_same_v<T,
                                         app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>)
            {
                attributeId = Attributes::EndpointGroupIDList::Id;

                {
                    // Invoke Groups:AddGroup on the device's endpoint
                    chip::app::Clusters::Groups::Commands::AddGroup::Type addGroup;
                    addGroup.groupID   = cbContext->objectToWrite.groupID;
                    addGroup.groupName = chip::CharSpan::fromCharString("GroupName");

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, cbContext->objectToWrite.endpointID);

                    err = groupsCluster.InvokeCommand(addGroup, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
            }
            else if constexpr (std::is_same_v<T,
                                              app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>)
            {
                attributeId = Attributes::NodeKeySetList::Id;

                {
                    // Invoke GroupKeyManagement::Commands::KeySetWrite on the device
                    chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type keySetWrite;
                    keySetWrite.groupKeySet.groupKeySetID = cbContext->objectToWrite.groupKeySetID;

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = groupsCluster.InvokeCommand(keySetWrite, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
            }
            else if constexpr (std::is_same_v<
                                   T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
            {
                attributeId = Attributes::EndpointBindingList::Id;

                {
                    // populate keySetWrite from cbContext->objectToWrite
                    chip::app::Clusters::Binding::Structs::TargetStruct::Type target;
                    target.node     = cbContext->objectToWrite.binding.node;
                    target.endpoint = cbContext->objectToWrite.binding.endpoint;
                    target.cluster  = cbContext->objectToWrite.binding.cluster;
                    target.group    = cbContext->objectToWrite.binding.group;
                    // Create a small array containing the single target and construct a DataModel::List from it
                    chip::app::Clusters::Binding::Structs::TargetStruct::Type targets[] = { target };
                    chip::app::Clusters::Binding::Attributes::Binding::TypeInfo::Type keySetWrite =
                        chip::app::DataModel::List<chip::app::Clusters::Binding::Structs::TargetStruct::Type>(
                            targets, sizeof(targets) / sizeof(targets[0]));

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, cbContext->objectToWrite.endpointID);

                    err = groupsCluster.WriteAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                        keySetWrite, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                }
            }
            else if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>)
            {
                attributeId = Attributes::NodeACLList::Id;

                {
                    // populate keySetWrite from cbContext->objectToWrite
                    chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type target;
                    target.privilege = static_cast<decltype(target.privilege)>(cbContext->objectToWrite.ACLEntry.privilege);
                    target.authMode  = static_cast<decltype(target.authMode)>(cbContext->objectToWrite.ACLEntry.authMode);
                    target.subjects  = cbContext->objectToWrite.ACLEntry.subjects;
                    // Convert each DatastoreAccessControlTargetStruct to AccessControl::AccessControlTargetStruct and add to list
                    std::vector<chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type> convertedTargets;
                    auto targetsIter = cbContext->objectToWrite.ACLEntry.targets.Value().begin();
                    auto targetsEnd  = cbContext->objectToWrite.ACLEntry.targets.Value().end();
                    while (targetsIter != targetsEnd)
                    {
                        const auto & srcTarget = *targetsIter;
                        targetsIter++;
                        chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type dst{};
                        dst.cluster    = srcTarget.cluster;
                        dst.endpoint   = srcTarget.endpoint;
                        dst.deviceType = srcTarget.deviceType;
                        convertedTargets.push_back(dst);
                    }

                    target.targets = chip::app::DataModel::List<
                        const chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type>(
                        convertedTargets.data(), static_cast<uint32_t>(convertedTargets.size()));
                    // Create a small array containing the single target and construct a DataModel::List from it
                    chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type targets[] = { target };
                    chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo::Type aclList = chip::app::DataModel::List<
                        const chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type>(
                        targets, sizeof(targets) / sizeof(targets[0]));

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = groupsCluster.WriteAttribute<chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo>(
                        aclList, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                }
            }
            else
            {
                ChipLogError(Controller, "Unknown type for attribute mapping");
                return;
            }

            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(Controller, "Failed in cluster.WriteAttribute: %s", ErrorStr(err));
            }
        }
    }

    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
    {
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;

        if (pairingCommand)
        {
            ChipLogProgress(DeviceLayer, "OnDeviceConnectionFailureFn - Not syncing device with node id: " ChipLogFormatX64,
                            ChipLogValueX64(cbContext->nodeId));
        }
    }

    /* Callback when command results in success */
    static void OnWriteSuccessResponse(void * context)
    {
        ChipLogProgress(Controller, "OnWriteSuccessResponse - Data written Successfully");

        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;
        if (cbContext && cbContext->onSuccess)
        {
            cbContext->onSuccess();
        }
    }

    /* Callback when command results in failure */
    static void OnWriteFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnWriteFailureResponse - Failed to write Data: %s", ErrorStr(error));
    }

    /* Callback when command results in success */
    static void OnCommandResponse(void * context,
                                  const chip::app::Clusters::Groups::Commands::AddGroup::Type::ResponseType & response)
    {
        ChipLogProgress(Controller, "OnCommandResponse - Command executed Successfully");

        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;
        if (cbContext && cbContext->onSuccess)
        {
            cbContext->onSuccess();
        }
    }

    /* Callback when command results in success */
    static void
    OnCommandResponse(void * context,
                      const chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type::ResponseType & response)
    {
        ChipLogProgress(Controller, "OnCommandResponse - Command executed Successfully");

        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;
        if (cbContext && cbContext->onSuccess)
        {
            cbContext->onSuccess();
        }
    }

    /* Callback when command results in failure */
    static void OnCommandFailure(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnCommandFailure - Failed to execute Command: %s", ErrorStr(error));
    }

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    std::shared_ptr<CallbackContext> mContext;
};

CHIP_ERROR JFADatastoreSync::Init(Server & server)
{
    mServer = &server;

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId,
    const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & endpointGroupIDEntry,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>(
            nodeId, endpointGroupIDEntry, onSuccess);

    TEMPORARY_RETURN_IGNORED GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                         &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>(
            nodeId, nodeKeySetEntry, onSuccess);

    TEMPORARY_RETURN_IGNORED GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                         &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, bindingEntry, onSuccess);

    TEMPORARY_RETURN_IGNORED GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                         &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JFADatastoreSync::SyncNode(NodeId nodeId,
                           const app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                           std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, aclEntry, onSuccess);

    TEMPORARY_RETURN_IGNORED GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                         &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}
