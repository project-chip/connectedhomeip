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
        using AclEntryType  = app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type;
        using AclTargetType = app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type;

        NodeId nodeId;
        EndpointId endpointId = kRootEndpointId;
        Optional<uint16_t> groupKeySetId;
        Optional<T> objectToWrite;
        Optional<std::vector<T>> objectsToWrite;
        std::vector<uint64_t> aclObjectSubjectsStorage;
        std::vector<AclTargetType> aclObjectTargetsStorage;
        std::vector<std::vector<uint64_t>> aclSubjectsStorage;
        std::vector<std::vector<AclTargetType>> aclTargetsStorage;
        std::function<void()> onSuccess;
        std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess;
        std::function<void(CHIP_ERROR, const T &)> onReadEntrySuccess;
        std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onReadListSuccess;

        CallbackContext(chip::NodeId nId, const T & object, std::function<void()> onSuccessFn) :
            nodeId(nId), objectToWrite(), objectsToWrite(), onSuccess(onSuccessFn)
        {
            if constexpr (std::is_same_v<T, AclEntryType>)
            {
                T copied = object;

                if (object.ACLEntry.subjects.IsNull())
                {
                    copied.ACLEntry.subjects.SetNull();
                }
                else
                {
                    for (const auto & subject : object.ACLEntry.subjects.Value())
                    {
                        aclObjectSubjectsStorage.push_back(subject);
                    }
                    copied.ACLEntry.subjects = chip::app::DataModel::List<const uint64_t>(
                        aclObjectSubjectsStorage.data(), static_cast<uint32_t>(aclObjectSubjectsStorage.size()));
                }

                if (object.ACLEntry.targets.IsNull())
                {
                    copied.ACLEntry.targets.SetNull();
                }
                else
                {
                    for (const auto & target : object.ACLEntry.targets.Value())
                    {
                        aclObjectTargetsStorage.push_back(target);
                    }
                    copied.ACLEntry.targets = chip::app::DataModel::List<const AclTargetType>(
                        aclObjectTargetsStorage.data(), static_cast<uint32_t>(aclObjectTargetsStorage.size()));
                }

                objectToWrite = MakeOptional(copied);
            }
            else
            {
                objectToWrite = MakeOptional(object);
            }
        }

        CallbackContext(chip::NodeId nId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchFn) :
            nodeId(nId), objectToWrite(), objectsToWrite(), onFetchSuccess(onFetchFn)
        {}

        CallbackContext(chip::NodeId nId, std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onReadFn) :
            nodeId(nId), objectToWrite(), objectsToWrite(), onReadListSuccess(onReadFn)
        {}

        CallbackContext(chip::NodeId nId, EndpointId eId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchFn) :
            nodeId(nId), endpointId(eId), objectToWrite(), objectsToWrite(), onFetchSuccess(onFetchFn)
        {}

        CallbackContext(chip::NodeId nId, uint16_t gksId, std::function<void(CHIP_ERROR, const T &)> onReadFn) :
            nodeId(nId), groupKeySetId(MakeOptional(gksId)), objectToWrite(), objectsToWrite(), onReadEntrySuccess(onReadFn)
        {}

        CallbackContext(chip::NodeId nId, const std::vector<T> & objects, std::function<void()> onSuccessFn) :
            nodeId(nId), objectToWrite(), objectsToWrite(), onSuccess(onSuccessFn)
        {
            if constexpr (std::is_same_v<T, AclEntryType>)
            {
                std::vector<T> copiedObjects;
                copiedObjects.reserve(objects.size());
                aclSubjectsStorage.reserve(objects.size());
                aclTargetsStorage.reserve(objects.size());

                for (const auto & object : objects)
                {
                    T copied = object;

                    if (object.ACLEntry.subjects.IsNull())
                    {
                        copied.ACLEntry.subjects.SetNull();
                    }
                    else
                    {
                        auto & subjectsCopy = aclSubjectsStorage.emplace_back();
                        for (const auto & subject : object.ACLEntry.subjects.Value())
                        {
                            subjectsCopy.push_back(subject);
                        }
                        copied.ACLEntry.subjects = chip::app::DataModel::List<const uint64_t>(
                            subjectsCopy.data(), static_cast<uint32_t>(subjectsCopy.size()));
                    }

                    if (object.ACLEntry.targets.IsNull())
                    {
                        copied.ACLEntry.targets.SetNull();
                    }
                    else
                    {
                        auto & targetsCopy = aclTargetsStorage.emplace_back();
                        for (const auto & target : object.ACLEntry.targets.Value())
                        {
                            targetsCopy.push_back(target);
                        }
                        copied.ACLEntry.targets = chip::app::DataModel::List<const AclTargetType>(
                            targetsCopy.data(), static_cast<uint32_t>(targetsCopy.size()));
                    }

                    copiedObjects.push_back(copied);
                }

                objectsToWrite = MakeOptional(copiedObjects);
            }
            else
            {
                objectsToWrite = MakeOptional(objects);
            }
        }
    };

    DevicePairedCommand(chip::NodeId nodeId, const T & object, std::function<void()> onSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
    {
        mContext = std::make_shared<CallbackContext>(nodeId, object, onSuccess);
    }

    DevicePairedCommand(chip::NodeId nodeId, std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
    {
        mContext   = std::make_shared<CallbackContext>(nodeId, onFetchSuccess);
        mFetchOnly = true;
    }

    DevicePairedCommand(chip::NodeId nodeId, std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onReadSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
    {
        mContext  = std::make_shared<CallbackContext>(nodeId, onReadSuccess);
        mReadOnly = true;
    }

    DevicePairedCommand(chip::NodeId nodeId, EndpointId endpointId,
                        std::function<void(CHIP_ERROR, const std::vector<T> &)> onFetchSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
    {
        mContext   = std::make_shared<CallbackContext>(nodeId, endpointId, onFetchSuccess);
        mFetchOnly = true;
    }

    DevicePairedCommand(chip::NodeId nodeId, uint16_t groupKeySetId, std::function<void(CHIP_ERROR, const T &)> onReadSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
    {
        mContext  = std::make_shared<CallbackContext>(nodeId, groupKeySetId, onReadSuccess);
        mReadOnly = true;
    }

    DevicePairedCommand(chip::NodeId nodeId, const std::vector<T> & objects, std::function<void()> onSuccess,
                        std::function<void()> removalCallback) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mRemovalCallback(removalCallback)
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

            if (pairingCommand->mFetchOnly == false && pairingCommand->mReadOnly == false)
            {
                if constexpr (std::is_same_v<
                                  T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>)
                {
                    if (cbContext->objectToWrite.Value().statusEntry.state ==
                        Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                    {
                        chip::app::Clusters::Groups::Commands::RemoveGroup::Type removeGroup;
                        removeGroup.groupID = cbContext->objectToWrite.Value().groupID;

                        chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle,
                                                                    cbContext->objectToWrite.Value().endpointID);
                        err = groupsCluster.InvokeCommand(removeGroup, pairingCommand, OnCommandResponse, OnCommandFailure);
                    }
                    else
                    {
                        // Invoke Groups:AddGroup on the device's endpoint
                        chip::app::Clusters::Groups::Commands::AddGroup::Type addGroup;
                        addGroup.groupID   = cbContext->objectToWrite.Value().groupID;
                        addGroup.groupName = "GroupName"_span;

                        chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle,
                                                                    cbContext->objectToWrite.Value().endpointID);

                        err = groupsCluster.InvokeCommand(addGroup, pairingCommand, OnCommandResponse, OnCommandFailure);
                    }
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>)
                {
                    if (cbContext->objectToWrite.Value().statusEntry.state ==
                        Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                    {
                        // Invoke GroupKeyManagement::Commands::KeySetRemove on the device
                        chip::app::Clusters::GroupKeyManagement::Commands::KeySetRemove::Type keySetRemove;
                        keySetRemove.groupKeySetID = cbContext->objectToWrite.Value().groupKeySetID;

                        chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        err = groupsCluster.InvokeCommand(keySetRemove, pairingCommand, OnCommandResponse, OnCommandFailure);
                    }
                    else
                    {
                        // Invoke GroupKeyManagement::Commands::KeySetWrite on the device
                        const auto groupKeySetId  = cbContext->objectToWrite.Value().groupKeySetID;
                        const auto & groupKeySets = Server::GetInstance().GetJointFabricDatastore().GetGroupKeySetList();

                        const app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type * matchedKeySet =
                            nullptr;
                        for (const auto & groupKeySet : groupKeySets)
                        {
                            if (groupKeySet.groupKeySetID == groupKeySetId)
                            {
                                matchedKeySet = &groupKeySet;
                                break;
                            }
                        }

                        if (matchedKeySet == nullptr)
                        {
                            ChipLogError(Controller, "Unable to sync NodeKeySet entry: GroupKeySetID=%u not found in datastore",
                                         groupKeySetId);
                            return;
                        }

                        chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type keySetWrite;
                        keySetWrite.groupKeySet.groupKeySetID = matchedKeySet->groupKeySetID;
                        keySetWrite.groupKeySet.groupKeySecurityPolicy =
                            static_cast<decltype(keySetWrite.groupKeySet.groupKeySecurityPolicy)>(
                                matchedKeySet->groupKeySecurityPolicy);
                        keySetWrite.groupKeySet.epochKey0       = matchedKeySet->epochKey0;
                        keySetWrite.groupKeySet.epochStartTime0 = matchedKeySet->epochStartTime0;
                        keySetWrite.groupKeySet.epochKey1       = matchedKeySet->epochKey1;
                        keySetWrite.groupKeySet.epochStartTime1 = matchedKeySet->epochStartTime1;
                        keySetWrite.groupKeySet.epochKey2       = matchedKeySet->epochKey2;
                        keySetWrite.groupKeySet.epochStartTime2 = matchedKeySet->epochStartTime2;

                        chip::Controller::ClusterBase groupsCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        err = groupsCluster.InvokeCommand(keySetWrite, pairingCommand, OnCommandResponse, OnCommandFailure);
                    }
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
                {
                    if (pairingCommand->mReplaceExisting == false)
                    {
                        // populate keySetWrite from cbContext->objectToWrite
                        chip::app::Clusters::Binding::Structs::TargetStruct::Type target;
                        target.node     = cbContext->objectToWrite.Value().binding.node;
                        target.endpoint = cbContext->objectToWrite.Value().binding.endpoint;
                        target.cluster  = cbContext->objectToWrite.Value().binding.cluster;
                        target.group    = cbContext->objectToWrite.Value().binding.group;
                        // Create a small array containing the single target and construct a DataModel::List from it
                        chip::app::Clusters::Binding::Structs::TargetStruct::Type targets[] = { target };
                        chip::app::Clusters::Binding::Attributes::Binding::TypeInfo::Type keySetWrite =
                            chip::app::DataModel::List<chip::app::Clusters::Binding::Structs::TargetStruct::Type>(
                                targets, sizeof(targets) / sizeof(targets[0]));

                        chip::Controller::ClusterBase bindingCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        err = bindingCluster.WriteAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                            keySetWrite, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                    }
                    else
                    {
                        // Create a list of targets from the objectsToWrite vector
                        std::vector<chip::app::Clusters::Binding::Structs::TargetStruct::Type> targets;
                        for (const auto & bindingEntry : cbContext->objectsToWrite.Value())
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

                        chip::Controller::ClusterBase bindingCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        err = bindingCluster.WriteAttribute<chip::app::Clusters::Binding::Attributes::Binding::TypeInfo>(
                            bindingList, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                    }
                }
                else if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>)
                {
                    // Create a list of ACL entries from the objectsToWrite vector
                    std::vector<chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type> aclEntries;
                    std::vector<std::vector<chip::app::Clusters::AccessControl::Structs::AccessControlTargetStruct::Type>>
                        aclTargetsStorage;
                    aclEntries.reserve(cbContext->objectsToWrite.Value().size());
                    aclTargetsStorage.reserve(cbContext->objectsToWrite.Value().size());
                    for (const auto & aclEntry : cbContext->objectsToWrite.Value())
                    {
                        chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type entry;
                        entry.privilege = static_cast<decltype(entry.privilege)>(aclEntry.ACLEntry.privilege);
                        entry.authMode  = static_cast<decltype(entry.authMode)>(aclEntry.ACLEntry.authMode);
                        entry.subjects  = aclEntry.ACLEntry.subjects;
                        // Convert each DatastoreAccessControlTargetStruct to AccessControl::AccessControlTargetStruct and add
                        // to list
                        if (aclEntry.ACLEntry.targets.IsNull())
                        {
                            entry.targets.SetNull();
                        }
                        else
                        {
                            auto & convertedTargets = aclTargetsStorage.emplace_back();
                            auto targetsIter        = aclEntry.ACLEntry.targets.Value().begin();
                            auto targetsEnd         = aclEntry.ACLEntry.targets.Value().end();
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
                        }

                        aclEntries.push_back(entry);
                    }

                    chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo::Type aclList = chip::app::DataModel::List<
                        const chip::app::Clusters::AccessControl::Structs::AccessControlEntryStruct::Type>(
                        aclEntries.data(), static_cast<uint32_t>(aclEntries.size()));

                    chip::Controller::ClusterBase accessControlCluster(exchangeMgr, sessionHandle, kRootEndpointId);
                    err = accessControlCluster.WriteAttribute<chip::app::Clusters::AccessControl::Attributes::Acl::TypeInfo>(
                        aclList, pairingCommand, OnWriteSuccessResponse, OnWriteFailureResponse);
                }
                else if constexpr (std::is_same_v<T,
                                                  app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>)
                {
                    // Invoke GroupKeyManagement::Commands::KeySetWrite on the device
                    chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::Type keySetWrite;
                    keySetWrite.groupKeySet.groupKeySetID = cbContext->objectToWrite.Value().groupKeySetID;
                    keySetWrite.groupKeySet.groupKeySecurityPolicy =
                        static_cast<decltype(keySetWrite.groupKeySet.groupKeySecurityPolicy)>(
                            cbContext->objectToWrite.Value().groupKeySecurityPolicy);
                    keySetWrite.groupKeySet.epochKey0       = cbContext->objectToWrite.Value().epochKey0;
                    keySetWrite.groupKeySet.epochStartTime0 = cbContext->objectToWrite.Value().epochStartTime0;
                    keySetWrite.groupKeySet.epochKey1       = cbContext->objectToWrite.Value().epochKey1;
                    keySetWrite.groupKeySet.epochStartTime1 = cbContext->objectToWrite.Value().epochStartTime1;
                    keySetWrite.groupKeySet.epochKey2       = cbContext->objectToWrite.Value().epochKey2;
                    keySetWrite.groupKeySet.epochStartTime2 = cbContext->objectToWrite.Value().epochStartTime2;

                    chip::Controller::ClusterBase groupKeyMgmtCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                    err = groupKeyMgmtCluster.InvokeCommand(keySetWrite, pairingCommand, OnCommandResponse, OnCommandFailure);
                }
                else
                {
                    ChipLogError(Controller, "Unknown type for attribute mapping");
                    return;
                }
            }
            else if (pairingCommand->mFetchOnly == true)
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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
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

                                    // Clean up in-flight command
                                    if (instance && instance->mRemovalCallback)
                                    {
                                        instance->mRemovalCallback();
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

                                    // Clean up in-flight command
                                    if (instance && instance->mRemovalCallback)
                                    {
                                        instance->mRemovalCallback();
                                    }
                                });
                }
                else if constexpr (std::is_same_v<
                                       T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
                {
                    chip::Controller::ClusterBase bindingCluster(exchangeMgr, sessionHandle, kRootEndpointId);

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
                                entry.endpointID       = lambdaCbContext->objectToWrite.Value().endpointID;
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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
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
                            std::vector<std::vector<uint64_t>> subjectsStorage;
                            std::vector<std::vector<
                                chip::app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>>
                                targetsStorage;

                            auto iter = dataResponse.begin();
                            while (iter.Next())
                            {
                                auto & responseEntry = iter.GetValue();

                                T entry;
                                entry.ACLEntry.privilege = static_cast<decltype(entry.ACLEntry.privilege)>(responseEntry.privilege);
                                entry.ACLEntry.authMode  = static_cast<decltype(entry.ACLEntry.authMode)>(responseEntry.authMode);

                                if (responseEntry.subjects.IsNull())
                                {
                                    entry.ACLEntry.subjects.SetNull();
                                }
                                else
                                {
                                    subjectsStorage.emplace_back();
                                    auto & subjectsCopy = subjectsStorage.back();
                                    auto subjectsIter   = responseEntry.subjects.Value().begin();
                                    while (subjectsIter.Next())
                                    {
                                        subjectsCopy.push_back(subjectsIter.GetValue());
                                    }
                                    entry.ACLEntry.subjects = chip::app::DataModel::List<const uint64_t>(
                                        subjectsCopy.data(), static_cast<uint32_t>(subjectsCopy.size()));
                                }

                                if (responseEntry.targets.IsNull())
                                {
                                    entry.ACLEntry.targets.SetNull();
                                }
                                else
                                {
                                    targetsStorage.emplace_back();
                                    auto & convertedTargets = targetsStorage.back();
                                    auto targetsIter        = responseEntry.targets.Value().begin();
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
                                }

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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
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

                            // Clean up in-flight command
                            if (instance && instance->mRemovalCallback)
                            {
                                instance->mRemovalCallback();
                            }
                        });
                }
                else
                {
                    ChipLogError(Controller, "Unknown type for attribute mapping");
                    return;
                }
            }
            else if (pairingCommand->mReadOnly == true)
            {
                if constexpr (std ::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>)
                {
                    if (!cbContext->groupKeySetId.HasValue())
                    {
                        ChipLogProgress(Controller, "Reading all GroupKeySet Indices");

                        chip::Controller::ClusterBase groupKeyMgmtCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        // Use KeySetReadAllIndices command to get the list of all key set IDs
                        chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndices::Type keySetReadAllIndices;

                        err = groupKeyMgmtCluster.InvokeCommand(
                            keySetReadAllIndices, pairingCommand,
                            // Success callback
                            [](void * lambdaContext,
                               const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::
                                   DecodableType & response) {
                                auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                auto lambdaCbContext = instance->mContext;

                                if (!lambdaCbContext)
                                {
                                    if (instance && instance->mRemovalCallback)
                                    {
                                        instance->mRemovalCallback();
                                    }
                                    return;
                                }

                                std::vector<uint16_t> groupKeySetIDs;

                                // Collect all key set IDs from the response
                                auto iter = response.groupKeySetIDs.begin();
                                while (iter.Next())
                                {
                                    groupKeySetIDs.push_back(iter.GetValue());
                                    ChipLogProgress(Controller, "Found key set ID: %u", groupKeySetIDs.back());
                                }

                                ChipLogProgress(Controller, "Retrieved all key set IDs");

                                if (lambdaCbContext->onReadListSuccess)
                                {
                                    lambdaCbContext->onReadListSuccess(CHIP_NO_ERROR, groupKeySetIDs);
                                }

                                // Clean up in-flight command
                                if (instance && instance->mRemovalCallback)
                                {
                                    instance->mRemovalCallback();
                                }
                            },
                            // Failure callback
                            [](void * lambdaContext, CHIP_ERROR error) {
                                auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                auto lambdaCbContext = instance->mContext;

                                ChipLogError(Controller, "KeySetReadAllIndices command failed: %s", ErrorStr(error));

                                if (lambdaCbContext && lambdaCbContext->onReadListSuccess)
                                {
                                    lambdaCbContext->onReadListSuccess(error, {});
                                }

                                // Clean up in-flight command
                                if (instance && instance->mRemovalCallback)
                                {
                                    instance->mRemovalCallback();
                                }
                            });
                    }
                    else
                    {
                        ChipLogProgress(Controller, "Reading GroupKeySet with GroupKeySetId: %u", cbContext->groupKeySetId.Value());

                        chip::Controller::ClusterBase groupKeyMgmtCluster(exchangeMgr, sessionHandle, kRootEndpointId);

                        // Use KeySetRead command for the specific key set ID provided in the context
                        chip::app::Clusters::GroupKeyManagement::Commands::KeySetRead::Type keySetRead;
                        keySetRead.groupKeySetID = cbContext->groupKeySetId.Value();

                        err = groupKeyMgmtCluster.InvokeCommand(
                            keySetRead, pairingCommand,
                            // Success callback
                            [](void * lambdaContext,
                               const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType &
                                   response) {
                                auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                auto lambdaCbContext = instance->mContext;

                                if (!lambdaCbContext)
                                {
                                    if (instance && instance->mRemovalCallback)
                                    {
                                        instance->mRemovalCallback();
                                    }
                                    return;
                                }

                                // Convert the response to the datastore struct type
                                T entry;
                                entry.groupKeySetID          = response.groupKeySet.groupKeySetID;
                                entry.groupKeySecurityPolicy = static_cast<decltype(entry.groupKeySecurityPolicy)>(
                                    response.groupKeySet.groupKeySecurityPolicy);
                                entry.epochKey0       = response.groupKeySet.epochKey0;
                                entry.epochStartTime0 = response.groupKeySet.epochStartTime0;
                                entry.epochKey1       = response.groupKeySet.epochKey1;
                                entry.epochStartTime1 = response.groupKeySet.epochStartTime1;
                                entry.epochKey2       = response.groupKeySet.epochKey2;
                                entry.epochStartTime2 = response.groupKeySet.epochStartTime2;

                                // Call the single-item success callback
                                if (lambdaCbContext->onReadEntrySuccess)
                                {
                                    lambdaCbContext->onReadEntrySuccess(CHIP_NO_ERROR, entry);
                                }

                                // Clean up in-flight command
                                if (instance && instance->mRemovalCallback)
                                {
                                    instance->mRemovalCallback();
                                }
                            },
                            // Failure callback
                            [](void * lambdaContext, CHIP_ERROR error) {
                                auto * instance      = static_cast<DevicePairedCommand *>(lambdaContext);
                                auto lambdaCbContext = instance->mContext;

                                ChipLogError(Controller, "KeySetRead command failed: %s", ErrorStr(error));

                                // Clean up in-flight command
                                if (instance && instance->mRemovalCallback)
                                {
                                    instance->mRemovalCallback();
                                }
                            });
                    }
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

            // Clean up in-flight command
            if (pairingCommand->mRemovalCallback)
            {
                pairingCommand->mRemovalCallback();
            }
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

        // Clean up in-flight command
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
        }
    }

    /* Callback when command results in failure */
    static void OnWriteFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnWriteFailureResponse - Failed to write Data: %s", ErrorStr(error));

        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;
        if (cbContext && cbContext->onSuccess)
        {
            cbContext->onSuccess();
        }

        // Clean up in-flight command
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
        }
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

        // Clean up in-flight command
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
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

        // Clean up in-flight command
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
        }
    }

    /* Callback when command results in success */
    static void OnCommandResponse(void * context,
                                  const chip::app::Clusters::Groups::Commands::RemoveGroup::Type::ResponseType & response)
    {
        ChipLogProgress(Controller, "OnCommandResponse - Command executed Successfully");

        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;
        if (cbContext && cbContext->onSuccess)
        {
            cbContext->onSuccess();
        }

        // Clean up in-flight command
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
        }
    }

    /* Callback when command results in failure */
    static void OnCommandFailure(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnCommandFailure - Failed to execute Command: %s", ErrorStr(error));

        // Clean up in-flight command
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        if (pairingCommand && pairingCommand->mRemovalCallback)
        {
            pairingCommand->mRemovalCallback();
        }
    }

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    std::shared_ptr<CallbackContext> mContext;
    std::function<void()> mRemovalCallback;
    bool mFetchOnly       = false;
    bool mReplaceExisting = false;
    bool mReadOnly        = false;
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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>(
            nodeId, endpointGroupIDEntry, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>(
            nodeId, nodeKeySetEntry, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Appending binding entry for node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type bindingEntryOwned = bindingEntry;
    EndpointId endpointId = bindingEntry.endpointID;

    return FetchEndpointBindingList(
        nodeId, endpointId,
        [this, nodeId, bindingEntryOwned,
         onSuccess](CHIP_ERROR fetchErr,
                    const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &
                        currentBindings) {
            std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> mergedBindings =
                currentBindings;

            if (fetchErr != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to fetch binding list before append: %s", ErrorStr(fetchErr));
                mergedBindings.clear();
            }

            if (bindingEntryOwned.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
                mergedBindings.erase(std::remove_if(mergedBindings.begin(), mergedBindings.end(),
                                                    [&bindingEntryOwned](const auto & entry) {
                                                        return entry.nodeID == bindingEntryOwned.nodeID &&
                                                            entry.endpointID == bindingEntryOwned.endpointID &&
                                                            entry.listID == bindingEntryOwned.listID;
                                                    }),
                                     mergedBindings.end());
            }
            else
            {
                mergedBindings.push_back(bindingEntryOwned);
            }

            CHIP_ERROR writeErr = SyncNode(nodeId, mergedBindings, onSuccess);
            if (writeErr != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to write appended binding list: %s", ErrorStr(writeErr));
                if (onSuccess)
                {
                    onSuccess();
                }
            }
        });
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId,
    std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & bindingEntries,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, bindingEntries, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JFADatastoreSync::SyncNode(NodeId nodeId,
                           const app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                           std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Appending ACL entry for node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type aclEntryOwned = aclEntry;
    auto aclSubjectsStorage = std::make_shared<std::vector<uint64_t>>();
    auto aclTargetsStorage =
        std::make_shared<std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>>();

    if (aclEntry.ACLEntry.subjects.IsNull())
    {
        aclEntryOwned.ACLEntry.subjects.SetNull();
    }
    else
    {
        for (const auto & subject : aclEntry.ACLEntry.subjects.Value())
        {
            aclSubjectsStorage->push_back(subject);
        }

        aclEntryOwned.ACLEntry.subjects = chip::app::DataModel::List<const uint64_t>(
            aclSubjectsStorage->data(), static_cast<uint32_t>(aclSubjectsStorage->size()));
    }

    if (aclEntry.ACLEntry.targets.IsNull())
    {
        aclEntryOwned.ACLEntry.targets.SetNull();
    }
    else
    {
        for (const auto & target : aclEntry.ACLEntry.targets.Value())
        {
            aclTargetsStorage->push_back(target);
        }

        aclEntryOwned.ACLEntry.targets = chip::app::DataModel::List<
            const app::Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
            aclTargetsStorage->data(), static_cast<uint32_t>(aclTargetsStorage->size()));
    }

    return FetchACLList(
        nodeId,
        [this, nodeId, aclEntryOwned, aclSubjectsStorage, aclTargetsStorage,
         onSuccess](CHIP_ERROR fetchErr,
                    const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & currentAcl) {
            static_cast<void>(aclSubjectsStorage);
            static_cast<void>(aclTargetsStorage);

            if (fetchErr != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to fetch ACL list before append: %s", ErrorStr(fetchErr));
                if (onSuccess)
                {
                    onSuccess();
                }
                return;
            }

            std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> mergedAcl = currentAcl;

            if (aclEntryOwned.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
                // If the new entry is marked for deletion, remove any existing entry with the same ID instead of adding it
                mergedAcl.erase(std::remove_if(mergedAcl.begin(), mergedAcl.end(),
                                               [&aclEntryOwned](const auto & entry) {
                                                   return entry.nodeID == aclEntryOwned.nodeID &&
                                                       entry.listID == aclEntryOwned.listID;
                                               }),
                                mergedAcl.end());
            }
            else
            {
                mergedAcl.push_back(aclEntryOwned);
            }

            CHIP_ERROR writeErr = SyncNode(nodeId, mergedAcl, onSuccess);
            if (writeErr != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to write appended ACL list: %s", ErrorStr(writeErr));
                if (onSuccess)
                {
                    onSuccess();
                }
            }
        });
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & aclEntries,
    std::function<void()> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, aclEntries, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>(
                nodeId, groupKeySet, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type>>(
                nodeId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    // TODO: Use endpointId to query GetGroupMembership command on that specific endpoint
    // For now, create the pairing command with the endpoint parameter

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type>>(
            nodeId, endpointId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, endpointId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchGroupKeySetList(NodeId nodeId,
                                                  std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>(
                nodeId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::FetchGroupKeySet(
    NodeId nodeId, uint16_t groupKeySetId,
    std::function<void(CHIP_ERROR, const app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type &)>
        onSuccess)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type>>(
                nodeId, groupKeySetId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

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

    const uint64_t inFlightToken = AllocateInFlightCommandToken();

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, onSuccess, [this, inFlightToken]() { RemoveInFlightCommand(inFlightToken); });

    StoreInFlightCommand(inFlightToken, pairingCommand);

    ReturnErrorOnFailure(GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                                     &pairingCommand->mOnDeviceConnectionFailureCallback));

    return CHIP_NO_ERROR;
}
