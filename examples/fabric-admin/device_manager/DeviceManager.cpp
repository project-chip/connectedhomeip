/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DeviceManager.h"

#include <commands/interactive/InteractiveCommands.h>

#include <cstdio>
#include <string>

using namespace chip;
using namespace chip::app::Clusters;

// Define the static member
DeviceManager DeviceManager::sInstance;

void DeviceManager::Init()
{
    // TODO: (#34113) Init mLastUsedNodeId from chip config file
    mLastUsedNodeId = 1;
}

NodeId DeviceManager::GetNextAvailableNodeId()
{
    mLastUsedNodeId++;
    VerifyOrDieWithMsg(mLastUsedNodeId < std::numeric_limits<chip::NodeId>::max(), NotSpecified, "No more available NodeIds.");

    return mLastUsedNodeId;
}

void DeviceManager::UpdateLastUsedNodeId(chip::NodeId nodeId)
{
    if (nodeId > mLastUsedNodeId)
    {
        ChipLogProgress(NotSpecified, "Updating last used NodeId to " ChipLogFormatX64, ChipLogValueX64(nodeId));
        mLastUsedNodeId = nodeId;
    }
}

void DeviceManager::AddSyncedDevice(const Device & device)
{
    mSyncedDevices.insert(device);
    ChipLogProgress(NotSpecified, "Added synced device: NodeId:" ChipLogFormatX64 ", EndpointId %u",
                    ChipLogValueX64(device.GetNodeId()), device.GetEndpointId());
}

Device * DeviceManager::FindDeviceByEndpoint(EndpointId endpointId)
{
    for (auto & device : mSyncedDevices)
    {
        if (device.GetEndpointId() == endpointId)
        {
            return const_cast<Device *>(&device);
        }
    }
    return nullptr;
}

Device * DeviceManager::FindDeviceByNode(NodeId nodeId)
{
    for (auto & device : mSyncedDevices)
    {
        if (device.GetNodeId() == nodeId)
        {
            return const_cast<Device *>(&device);
        }
    }
    return nullptr;
}

void DeviceManager::RemoveSyncedDevice(NodeId nodeId)
{
    Device * device = FindDeviceByNode(nodeId);
    if (device == nullptr)
    {
        ChipLogProgress(NotSpecified, "No device found with NodeId:" ChipLogFormatX64, ChipLogValueX64(nodeId));
        return;
    }

    mSyncedDevices.erase(*device);
    ChipLogProgress(NotSpecified, "Removed synced device: NodeId:" ChipLogFormatX64 ", EndpointId %u",
                    ChipLogValueX64(device->GetNodeId()), device->GetEndpointId());
}

void DeviceManager::HandleAttributeChange(const app::ConcreteDataAttributePath & path, TLV::TLVReader * data)
{
    if (path.mClusterId != Descriptor::Id || path.mAttributeId != Descriptor::Attributes::PartsList::Id)
    {
        return;
    }

    ChipLogProgress(NotSpecified, "Attribute change detected:");
    ChipLogProgress(
        NotSpecified, "Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI ", DataVersion: %" PRIu32,
        path.mEndpointId, ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId), path.mDataVersion.ValueOr(0));

    app::DataModel::DecodableList<EndpointId> value;
    CHIP_ERROR error = app::DataModel::Decode(*data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode attribute value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    std::set<EndpointId> newEndpoints;

    // Populate the newEndpoints set from the decoded value using an iterator
    auto iter = value.begin();
    while (iter.Next())
    {
        newEndpoints.insert(iter.GetValue());
    }

    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to iterate over decoded attribute value.");
        return;
    }

    // Compare newEndpoints with mSyncedDevices to determine added and removed endpoints
    std::vector<EndpointId> addedEndpoints;
    std::vector<EndpointId> removedEndpoints;

    // Note: We're using vectors and manual searches instead of set operations
    // because we need to work with the Device objects in mSyncedDevices,
    // not just their EndpointIds. This approach allows us to access the full
    // Device information when processing changes.

    // Find added endpoints
    for (const auto & endpoint : newEndpoints)
    {
        if (FindDeviceByEndpoint(endpoint) == nullptr)
        {
            addedEndpoints.push_back(endpoint);
        }
    }

    // Find removed endpoints
    for (auto & device : mSyncedDevices)
    {
        EndpointId endpointId = device.GetEndpointId();
        if (newEndpoints.find(endpointId) == newEndpoints.end())
        {
            removedEndpoints.push_back(endpointId);
        }
    }

    // Process added endpoints
    for (const auto & endpoint : addedEndpoints)
    {
        ChipLogProgress(NotSpecified, "Endpoint added: %u", endpoint);

        if (mAutoSyncEnabled)
        {
            char command[64];
            snprintf(command, sizeof(command), "fabricsync sync-device %d", endpoint);
            PushCommand(command);
        }
    }

    // Process removed endpoints
    for (const auto & endpoint : removedEndpoints)
    {
        ChipLogProgress(NotSpecified, "Endpoint removed: %u", endpoint);

        Device * device = FindDeviceByEndpoint(endpoint);

        if (device == nullptr)
        {
            ChipLogProgress(NotSpecified, "No device on Endpoint: %u", endpoint);
            continue;
        }

        if (mAutoSyncEnabled)
        {
            char command[64];
            snprintf(command, sizeof(command), "pairing unpair %ld", device->GetNodeId());

            PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "unpair"));

            if (pairingCommand == nullptr)
            {
                ChipLogError(NotSpecified, "Pairing code command is not available");
                return;
            }

            pairingCommand->RegisterPairingDelegate(this);
            PushCommand(command);
        }
    }
}

void DeviceManager::OnDeviceRemoved(NodeId deviceId, CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to remove synced device:(" ChipLogFormatX64 ") with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(deviceId), err.Format());
        return;
    }

    RemoveSyncedDevice(deviceId);
    ChipLogProgress(NotSpecified, "Synced device with NodeId:" ChipLogFormatX64 " has been removed.", ChipLogValueX64(deviceId));
}
