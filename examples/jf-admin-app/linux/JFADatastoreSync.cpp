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
#include <controller/CommissionerDiscoveryController.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::app::Clusters::JointFabricDatastore;

extern DeviceCommissioner * GetDeviceCommissioner();

[[maybe_unused]] constexpr uint8_t kJFAvailableShift = 0;
[[maybe_unused]] constexpr uint8_t kJFAdminShift     = 1;
[[maybe_unused]] constexpr uint8_t kJFAnchorShift    = 2;
[[maybe_unused]] constexpr uint8_t kJFDatastoreShift = 3;

[[maybe_unused]] static constexpr EndpointId kJFDatastoreClusterEndpointId = 1;

JFADatastoreSync JFADatastoreSync::sJFDS;

template <typename T>
class DevicePairedCommand
{
public:
    struct CallbackContext
    {
        NodeId nodeId;
        EndpointId endpointId = kRootEndpointId;
        T objectToWrite;
        std::vector<T> objectsToWrite;
        std::function<void()> onSuccess;
        std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess;

        CallbackContext(chip::NodeId nId, T object, std::function<void()> onSuccessFn) :
            nodeId(nId), objectToWrite(object), onSuccess(onSuccessFn)
        {}

        CallbackContext(chip::NodeId nId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchFn) :
            nodeId(nId), objectToWrite(), onFetchSuccess(onFetchFn)
        {}

        CallbackContext(chip::NodeId nId, EndpointId eId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchFn) :
            nodeId(nId), endpointId(eId), objectToWrite(), onFetchSuccess(onFetchFn)
        {}

        CallbackContext(chip::NodeId nId, std::vector<T> objects, std::function<void()> onSuccessFn) :
            nodeId(nId), objectsToWrite(objects), onSuccess(onSuccessFn)
        {}
    };

    DevicePairedCommand(
        chip::NodeId nodeId, T object, std::function<void()> onSuccess = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext = std::make_shared<CallbackContext>(nodeId, object, onSuccess);
    }

    DevicePairedCommand(
        chip::NodeId nodeId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext   = std::make_shared<CallbackContext>(nodeId, onFetchSuccess);
        mFetchOnly = true;
    }

    DevicePairedCommand(
        chip::NodeId nodeId, EndpointId endpointId,
        std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext   = std::make_shared<CallbackContext>(nodeId, endpointId, onFetchSuccess);
        mFetchOnly = true;
    }

    DevicePairedCommand(
        chip::NodeId nodeId, std::vector<T> objects, std::function<void()> onSuccess = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext         = std::make_shared<CallbackContext>(nodeId, objects, onSuccess);
        mReplaceExisting = true;
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

            if (pairingCommand->mFetchOnly == false)
            {
                if constexpr (std::is_same_v<
                                  T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>)
                {
                    // Invoke Groups:AddGroup on the device's endpoint
                    chip::app::Clusters::Groups::Commands::AddGroup::Type addGroup;
                    addGroup.groupID   = cbContext->objectToWrite.groupID;
                    addGroup.groupName = chip::CharSpan::fromCharString("GroupName");

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, cbContext->objectToWrite.endpointID);

                    err = groupsCluster.InvokeCommand(addGroup, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>)
                {
                    // Invoke GroupKeyManagement::Commands::KeySetWrite on the device
                    chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type keySetWrite;
                    keySetWrite.groupKeySet.groupKeySetID = cbContext->objectToWrite.groupKeySetID;

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = groupsCluster.InvokeCommand(keySetWrite, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
                {
                    if (pairingCommand->mReplaceExisting == false)
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

                        chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle,
                                                                    cbContext->objectToWrite.endpointID);

                        err = groupsCluster.WriteAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                            keySetWrite, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                    }
                    else
                    {
                        // Create a list of targets from the objectsToWrite vector
                        std::vector<chip::app::Clusters::Binding::Structs::TargetStruct::Type> targets;
                        for (const auto & bindingEntry : cbContext->objectsToWrite)
                        {
                            chip::app::Clusters::Binding::Structs::TargetStruct::Type target;
                            target.node     = bindingEntry.binding.node;
                            target.endpoint = bindingEntry.binding.endpoint;
                            target.cluster  = bindingEntry.binding.cluster;
                            target.group    = bindingEntry.binding.group;
                            targets.push_back(target);
                        }

                        chip::app::Clusters::Binding::Attributes::Binding::TypeInfo::Type bindingList =
                            chip::app::DataModel::List<chip::app::Clusters::Binding::Structs::TargetStruct::Type>(
                                targets.data(), static_cast<uint32_t>(targets.size()));

                        chip::Controller::ClusterBase bindingCluster(exchangeMgr, sessionHandle,
                                                                     cbContext->objectsToWrite[0].endpointID);

                        err = bindingCluster.WriteAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                            bindingList, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                    }
                }
                else if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>)
                {
                    if (pairingCommand->mReplaceExisting == false)
                    {
                        // populate keySetWrite from cbContext->objectToWrite
                        chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type target;
                        target.privilege = static_cast<decltype(target.privilege)>(cbContext->objectToWrite.ACLEntry.privilege);
                        target.authMode  = static_cast<decltype(target.authMode)>(cbContext->objectToWrite.ACLEntry.authMode);
                        target.subjects  = cbContext->objectToWrite.ACLEntry.subjects;
                        // Convert each DatastoreAccessControlTargetStruct to AccessControl::AccessControlTargetStruct and add to
                        // list
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
                    else
                    {
                        // Create a list of ACL entries from the objectsToWrite vector
                        std::vector<chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type> aclEntries;
                        for (const auto & aclEntry : cbContext->objectsToWrite)
                        {
                            chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type entry;
                            entry.privilege = static_cast<decltype(entry.privilege)>(aclEntry.ACLEntry.privilege);
                            entry.authMode  = static_cast<decltype(entry.authMode)>(aclEntry.ACLEntry.authMode);
                            entry.subjects  = aclEntry.ACLEntry.subjects;
                            // Convert each DatastoreAccessControlTargetStruct to AccessControl::AccessControlTargetStruct and add
                            // to list
                            std::vector<chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type>
                                convertedTargets;
                            auto targetsIter = aclEntry.ACLEntry.targets.Value().begin();
                            auto targetsEnd  = aclEntry.ACLEntry.targets.Value().end();
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
                            entry.targets = chip::app::DataModel::List<
                                const chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type>(
                                convertedTargets.data(), static_cast<uint32_t>(convertedTargets.size()));

                            aclEntries.push_back(entry);
                        }

                        chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo::Type aclList = chip::app::DataModel::List<
                            const chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type>(
                            aclEntries.data(), static_cast<uint32_t>(aclEntries.size()));

                        chip::Controller::ClusterBase accessControlCluster(exchangeMgr, sessionHandle, kRootEndpointId);
                        err = accessControlCluster.WriteAttribute<chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo>(
                            aclList, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                    }
                }
                else if constexpr (std::is_same_v<T,
                                                  app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>)
                {
                    // Invoke GroupKeyManagement::Commands::KeySetWrite on the device
                    chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type keySetWrite;
                    keySetWrite.groupKeySet.groupKeySetID = cbContext->objectToWrite.groupKeySetID;
                    keySetWrite.groupKeySet.groupKeySecurityPolicy =
                        static_cast<decltype(keySetWrite.groupKeySet.groupKeySecurityPolicy)>(
                            cbContext->objectToWrite.groupKeySecurityPolicy);
                    keySetWrite.groupKeySet.epochKey0       = cbContext->objectToWrite.epochKey0;
                    keySetWrite.groupKeySet.epochStartTime0 = cbContext->objectToWrite.epochStartTime0;
                    keySetWrite.groupKeySet.epochKey1       = cbContext->objectToWrite.epochKey1;
                    keySetWrite.groupKeySet.epochStartTime1 = cbContext->objectToWrite.epochStartTime1;
                    keySetWrite.groupKeySet.epochKey2       = cbContext->objectToWrite.epochKey2;
                    keySetWrite.groupKeySet.epochStartTime2 = cbContext->objectToWrite.epochStartTime2;

                    chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = groupsCluster.InvokeCommand(keySetWrite, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
                else
                {
                    ChipLogError(Controller, "Unknown type for attribute mapping");
                    return;
                }
            }
            else
            {
                if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type>)
                {
                    chip::Controller::ClusterBase descriptorCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = descriptorCluster.ReadAttribute<chip::app::Clusters::Descriptor::Attributes::PartsList::TypeInfo>(
                        pairingCommand,
                        // Success lambda
                        [](void * lambdaContext,
                           const chip::app::Clusters::Descriptor::Attributes::PartsList::TypeInfo::DecodableType & dataResponse) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            std::vector<T> endpointEntries;

                            auto iter = dataResponse.begin();
                            while (iter.Next())
                            {
                                auto & responseEntry = iter.GetValue();

                                T entry;
                                entry.endpointID = responseEntry;
                                entry.nodeID     = lambdaCbContext->nodeId;
                                endpointEntries.push_back(entry);
                            }

                            if (iter.GetStatus() != CHIP_NO_ERROR)
                            {
                                ChipLogError(Controller, "Failed to iterate PartsList: %s", ErrorStr(iter.GetStatus()));
                            }

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(CHIP_NO_ERROR, endpointEntries);
                            }
                        },
                        // Failure lambda
                        [](void * lambdaContext, CHIP_ERROR readError) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(readError, {});
                            }
                        });
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type>)
                {
                    chip::Controller::ClusterBase groupKeyMgmtCluster(exchangeMgr, sessionHandle,
                                                                      pairingCommand->mContext->endpointId);

                    err =
                        groupKeyMgmtCluster
                            .ReadAttribute<chip::app::Clusters::GroupKeyManagement::Attributes::GroupKeyMap::TypeInfo>(
                                pairingCommand,
                                // Success lambda
                                [](void * lambdaContext,
                                   const chip::app::Clusters::GroupKeyManagement::Attributes::GroupKeyMap::TypeInfo::DecodableType &
                                       dataResponse) {
                                    auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                    auto lambdaCbContext = instance->mContext;

                                    std::vector<T> groupInfoEntries;

                                    auto iter = dataResponse.begin();
                                    while (iter.Next())
                                    {
                                        auto & responseEntry = iter.GetValue();

                                        T entry;
                                        entry.groupID       = responseEntry.groupId;
                                        entry.groupKeySetID = responseEntry.groupKeySetID;
                                        groupInfoEntries.push_back(entry);
                                    }

                                    if (iter.GetStatus() != CHIP_NO_ERROR)
                                    {
                                        ChipLogError(Controller, "Failed to iterate GroupKeyMap: %s", ErrorStr(iter.GetStatus()));
                                    }

                                    if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                                    {
                                        lambdaCbContext->onFetchSuccess(CHIP_NO_ERROR, groupInfoEntries);
                                    }
                                },
                                // Failure lambda
                                [](void * lambdaContext, CHIP_ERROR readError) {
                                    auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                    auto lambdaCbContext = instance->mContext;

                                    if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                                    {
                                        lambdaCbContext->onFetchSuccess(readError, {});
                                    }
                                });
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
                {
                    chip::Controller::ClusterBase bindingCluster(exchangeMgr, sessionHandle, pairingCommand->mContext->endpointId);

                    err = bindingCluster.ReadAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                        pairingCommand,
                        // Success lambda
                        [](void * lambdaContext,
                           const chip::app::Clusters::Binding::Attributes::Binding::TypeInfo::DecodableType & dataResponse) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            std::vector<T> bindingEntries;

                            auto iter = dataResponse.begin();
                            while (iter.Next())
                            {
                                auto & responseEntry = iter.GetValue();

                                T entry;
                                entry.binding.node     = responseEntry.node;
                                entry.binding.endpoint = responseEntry.endpoint;
                                entry.binding.cluster  = responseEntry.cluster;
                                entry.binding.group    = responseEntry.group;
                                entry.endpointID       = lambdaCbContext->objectToWrite.endpointID;
                                bindingEntries.push_back(entry);
                            }

                            if (iter.GetStatus() != CHIP_NO_ERROR)
                            {
                                ChipLogError(Controller, "Failed to iterate Binding: %s", ErrorStr(iter.GetStatus()));
                            }

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(CHIP_NO_ERROR, bindingEntries);
                            }
                        },
                        // Failure lambda
                        [](void * lambdaContext, CHIP_ERROR readError) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(readError, {});
                            }
                        });
                }
                else if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>)
                {
                    chip::Controller::ClusterBase aclCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = aclCluster.ReadAttribute<chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo>(
                        pairingCommand,
                        // Success lambda
                        [](void * lambdaContext,
                           const chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo::DecodableType & dataResponse) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            std::vector<T> aclEntries;

                            auto iter = dataResponse.begin();
                            while (iter.Next())
                            {
                                auto & responseEntry = iter.GetValue();

                                T entry;
                                entry.ACLEntry.privilege = static_cast<decltype(entry.ACLEntry.privilege)>(responseEntry.privilege);
                                entry.ACLEntry.authMode  = static_cast<decltype(entry.ACLEntry.authMode)>(responseEntry.authMode);

                                std::vector<uint64_t> subjectsCopy;
                                auto subjectsIter = responseEntry.subjects.Value().begin();
                                while (subjectsIter.Next())
                                {
                                    subjectsCopy.push_back(subjectsIter.GetValue());
                                }
                                entry.ACLEntry.subjects = chip::app::DataModel::List<const uint64_t>(
                                    subjectsCopy.data(), static_cast<uint32_t>(subjectsCopy.size()));

                                std::vector<
                                    chip::app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>
                                    convertedTargets;
                                auto targetsIter = responseEntry.targets.Value().begin();
                                while (targetsIter.Next())
                                {
                                    const auto & srcTarget = targetsIter.GetValue();

                                    chip::app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type
                                        dst{};
                                    dst.cluster    = srcTarget.cluster;
                                    dst.endpoint   = srcTarget.endpoint;
                                    dst.deviceType = srcTarget.deviceType;
                                    convertedTargets.push_back(dst);
                                }
                                entry.ACLEntry.targets =
                                    chip::app::DataModel::List<const chip::app::Clusters::JointFabricDatastore::Structs::
                                                                   DatastoreAccessControlTargetStruct::Type>(
                                        convertedTargets.data(), static_cast<uint32_t>(convertedTargets.size()));

                                aclEntries.push_back(entry);
                            }

                            if (iter.GetStatus() != CHIP_NO_ERROR)
                            {
                                ChipLogError(Controller, "Failed to iterate Acl: %s", ErrorStr(iter.GetStatus()));
                            }

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(CHIP_NO_ERROR, aclEntries);
                            }
                        },
                        // Failure lambda
                        [](void * lambdaContext, CHIP_ERROR readError) {
                            auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                            auto lambdaCbContext = instance->mContext;

                            if (lambdaCbContext && lambdaCbContext->onFetchSuccess)
                            {
                                lambdaCbContext->onFetchSuccess(readError, {});
                            }
                        });
                }
                else
                {
                    ChipLogError(Controller, "Unknown type for attribute mapping");
                    return;
                }
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
    bool mFetchOnly       = false;
    bool mReplaceExisting = false;
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

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

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

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

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

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId,
    std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & bindingEntries,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, bindingEntries, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

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

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & aclEntries,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, aclEntries, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JFADatastoreSync::SyncNode(NodeId nodeId,
                           const app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet,
                           std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>(
                nodeId, groupKeySet, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchEndpointList(
    NodeId nodeId,
    std::function<void(
        CHIP_ERROR,
        const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> & endpointEntryStruct)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type>>(
                nodeId, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchEndpointGroupList(
    NodeId nodeId, EndpointId endpointId,
    std::function<void(
        CHIP_ERROR, const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> &)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Fetching group list for node " ChipLogFormatX64 " endpoint %u", ChipLogValueX64(nodeId),
                    endpointId);

    // TODO: Use endpointId to query GetGroupMembership command on that specific endpoint
    // For now, create the pairing command with the endpoint parameter

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type>>(
            nodeId, endpointId, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchEndpointBindingList(
    NodeId nodeId, EndpointId endpointId,
    std::function<void(
        CHIP_ERROR, const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node " ChipLogFormatX64 " endpoint %u", ChipLogValueX64(nodeId),
                    endpointId);

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, endpointId, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchGroupKeySetList(
    NodeId nodeId,
    std::function<void(CHIP_ERROR,
                       const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> &)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>(
                nodeId, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchACLList(
    NodeId nodeId,
    std::function<void(CHIP_ERROR,
                       const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> &)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, onSuccess);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}
