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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <commands/pairing/PairingCommand.h>
#include <platform/CHIPDeviceLayer.h>

#include <set>

class Device
{
public:
    Device(chip::NodeId nodeId, chip::EndpointId endpointId) : mNodeId(nodeId), mEndpointId(endpointId) {}

    chip::NodeId GetNodeId() const { return mNodeId; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

    bool operator<(const Device & other) const
    {
        return mNodeId < other.mNodeId || (mNodeId == other.mNodeId && mEndpointId < other.mEndpointId);
    }

private:
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
};

class DeviceManager : public PairingDelegate
{
public:
    DeviceManager() = default;

    void Init();

    chip::NodeId GetNextAvailableNodeId();

    chip::NodeId GetRemoteBridgeNodeId() const { return mRemoteBridgeNodeId; }

    void UpdateLastUsedNodeId(chip::NodeId nodeId);

    void SetRemoteBridgeNodeId(chip::NodeId remoteBridgeNodeId) { mRemoteBridgeNodeId = remoteBridgeNodeId; }

    bool IsAutoSyncEnabled() const { return mAutoSyncEnabled; }

    bool IsFabricSyncReady() const { return mRemoteBridgeNodeId != chip::kUndefinedNodeId; }

    void EnableAutoSync(bool state) { mAutoSyncEnabled = state; }

    void AddSyncedDevice(const Device & device);

    void RemoveSyncedDevice(chip::NodeId nodeId);

    void HanldeAttributeChange(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data);

    void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) override;

private:
    friend DeviceManager & DeviceMgr();

    static DeviceManager sInstance;

    chip::NodeId mLastUsedNodeId     = 0;
    chip::NodeId mRemoteBridgeNodeId = chip::kUndefinedNodeId;
    std::set<Device> mSyncedDevices;
    bool mAutoSyncEnabled = false;

    Device * FindDeviceByEndpoint(chip::EndpointId endpointId);
    Device * FindDeviceByNode(chip::NodeId nodeId);
};

/**
 * Returns the public interface of the DeviceManager singleton object.
 *
 * Applications should use this to access features of the DeviceManager
 * object.
 */
inline DeviceManager & DeviceMgr()
{
    return DeviceManager::sInstance;
}
