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
        std::function<void()> onComplete;

        CallbackContext(chip::NodeId nId, T object, std::function<void()> onComp) :
            nodeId(nId), objectToWrite(object), onComplete(onComp)
        {}
    };
    DevicePairedCommand(
        chip::NodeId nodeId, T object, std::function<void()> onComplete = []() {}) :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mContext = std::make_shared<CallbackContext>(nodeId, object, onComplete);
    }

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle)
    {
        auto * pairingCommand = static_cast<DevicePairedCommand *>(context);
        auto cbContext        = pairingCommand->mContext;

        if (pairingCommand)
        {
            ChipLogProgress(DeviceLayer, "OnDeviceConnectedFn - Syncing device with node id: " ChipLogFormatX64,
                            ChipLogValueX64(cbContext->nodeId));

            chip::Controller::ClusterBase cluster(exchangeMgr, sessionHandle, kJFDatastoreClusterEndpointId);

            // Static LUT for mapping type T to attribute ID
            AttributeId attributeId = 0;

            if constexpr (std::is_same_v<T,
                                         app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>)
            {
                attributeId = Attributes::EndpointGroupIDList::Id;
            }
            else if constexpr (std::is_same_v<T,
                                              app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>)
            {
                attributeId = Attributes::NodeKeySetList::Id;
            }
            else if constexpr (std::is_same_v<
                                   T, app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>)
            {
                attributeId = Attributes::EndpointBindingList::Id;
            }
            else if constexpr (std::is_same_v<T, app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>)
            {
                attributeId = Attributes::NodeACLList::Id;
            }
            else
            {
                ChipLogError(Controller, "Unknown type for attribute mapping");
                return;
            }

            CHIP_ERROR err = cluster.WriteAttribute(cbContext->objectToWrite, pairingCommand, Clusters::JointFabricDatastore::Id,
                                                    attributeId, OnWriteSuccessResponse, OnWriteFailureResponse, NullOptional);

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
        if (cbContext && cbContext->onComplete)
        {
            cbContext->onComplete();
        }
    }

    /* Callback when command results in failure */
    static void OnWriteFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnWriteFailureResponse - Failed to write Data: %s", ErrorStr(error));
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
    std::function<void()> onComplete)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>>(
            nodeId, endpointGroupIDEntry, onComplete);

    GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
    std::function<void()> onComplete)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>>(
            nodeId, nodeKeySetEntry, onComplete);

    GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFADatastoreSync::SyncNode(
    NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
    std::function<void()> onComplete)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>
        pairingCommand = std::make_shared<
            DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>>(
            nodeId, bindingEntry, onComplete);

    GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JFADatastoreSync::SyncNode(NodeId nodeId,
                           const app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                           std::function<void()> onComplete)
{
    ChipLogProgress(DeviceLayer, "Creating Pairing Command with node id: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    std::shared_ptr<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>
        pairingCommand =
            std::make_shared<DevicePairedCommand<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>>(
                nodeId, aclEntry, onComplete);

    GetDeviceCommissioner()->GetConnectedDevice(nodeId, &pairingCommand->mOnDeviceConnectedCallback,
                                                &pairingCommand->mOnDeviceConnectionFailureCallback);

    return CHIP_NO_ERROR;
}
