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

#include "PairingManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <platform/CHIPDeviceLayer.h>

class DeviceManager : public PairingDelegate
{
public:
    DeviceManager() = default;

    void Init();

    chip::NodeId GetNextAvailableNodeId();

    chip::NodeId GetRemoteBridgeNodeId() const { return mRemoteBridgeNodeId; }

    void UpdateLastUsedNodeId(chip::NodeId nodeId);

    void SetRemoteBridgeNodeId(chip::NodeId nodeId);

    bool IsFabricSyncReady() const { return mRemoteBridgeNodeId != chip::kUndefinedNodeId; }

    /**
     * @brief Determines whether a given nodeId corresponds to the remote bridge device.
     *
     * @param nodeId            The ID of the node being checked.
     *
     * @return true if the nodeId matches the remote bridge device; otherwise, false.
     */
    bool IsCurrentBridgeDevice(chip::NodeId nodeId) const { return nodeId == mRemoteBridgeNodeId; }

    /**
     * @brief Pair a remote fabric bridge with a given node ID.
     *
     * This function initiates the pairing process for a remote fabric bridge using the specified parameters.

     * @param nodeId            The user-defined ID for the node being commissioned. It doesn’t need to be the same ID,
     *                          as for the first fabric.
     * @param setupPINCode      The setup PIN code used to authenticate the pairing process.
     * @param deviceRemoteIp    The IP address of the remote device that is being paired as part of the fabric bridge.
     * @param deviceRemotePort  The secured device port of the remote device that is being paired as part of the fabric bridge.
     *
     * @return CHIP_ERROR       Returns CHIP_NO_ERROR on success or an appropriate error code on failure.
     */
    CHIP_ERROR PairRemoteFabricBridge(chip::NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                      uint16_t deviceRemotePort);

    CHIP_ERROR UnpairRemoteFabricBridge();

private:
    friend DeviceManager & DeviceMgr();

    void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) override;

    static DeviceManager sInstance;

    chip::NodeId mLastUsedNodeId = 0;

    // The Node ID of the remote bridge used for Fabric-Sync
    // This represents the bridge on the other ecosystem.
    chip::NodeId mRemoteBridgeNodeId = chip::kUndefinedNodeId;

    bool mInitialized = false;
};

/**
 * Returns the public interface of the DeviceManager singleton object.
 *
 * Applications should use this to access features of the DeviceManager
 * object.
 */
inline DeviceManager & DeviceMgr()
{
    if (!DeviceManager::sInstance.mInitialized)
    {
        DeviceManager::sInstance.Init();
    }
    return DeviceManager::sInstance;
}
