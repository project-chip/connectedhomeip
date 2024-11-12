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
#include <device_manager/BridgeSubscription.h>
#include <device_manager/CommissionerControl.h>
#include <device_manager/FabricSyncGetter.h>
#include <device_manager/PairingManager.h>
#include <platform/CHIPDeviceLayer.h>
#include <set>

namespace admin {

class SyncedDevice
{
public:
    SyncedDevice(chip::NodeId nodeId, chip::EndpointId endpointId) : mNodeId(nodeId), mEndpointId(endpointId) {}

    chip::NodeId GetNodeId() const { return mNodeId; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

    bool operator<(const SyncedDevice & other) const
    {
        return mNodeId < other.mNodeId || (mNodeId == other.mNodeId && mEndpointId < other.mEndpointId);
    }

private:
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
};

class DeviceManager
{
public:
    DeviceManager() = default;

    static DeviceManager & Instance()
    {
        static DeviceManager instance;
        return instance;
    }

    void Init();

    chip::NodeId GetNextAvailableNodeId();

    chip::NodeId GetRemoteBridgeNodeId() const { return mRemoteBridgeNodeId; }

    chip::NodeId GetLocalBridgeNodeId() const { return mLocalBridgeNodeId; }

    void UpdateLastUsedNodeId(chip::NodeId nodeId);

    void SetRemoteBridgeNodeId(chip::NodeId nodeId);

    void SetLocalBridgeNodeId(chip::NodeId nodeId) { mLocalBridgeNodeId = nodeId; }

    bool IsFabricSyncReady() const { return mRemoteBridgeNodeId != chip::kUndefinedNodeId; }

    bool IsLocalBridgeReady() const { return mLocalBridgeNodeId != chip::kUndefinedNodeId; }

    void AddSyncedDevice(const SyncedDevice & device);

    void RemoveSyncedDevice(chip::ScopedNodeId scopedNodeId);

    /**
     * @brief Initializes the CommissionerControl for fabric sync setup process.
     */
    void InitCommissionerControl();

    /**
     * @brief Determines whether a given nodeId corresponds to the "current bridge device," either local or remote.
     *
     * @param nodeId            The ID of the node being checked.
     *
     * @return true if the nodeId matches either the local or remote bridge device; otherwise, false.
     */
    bool IsCurrentBridgeDevice(chip::NodeId nodeId) const { return nodeId == mLocalBridgeNodeId || nodeId == mRemoteBridgeNodeId; }

    /**
     * @brief Open the commissioning window for a specific device within its own fabric.
     *
     * This function initiates the process to open the commissioning window for a device identified by the given node ID.
     *
     * @param scopedNodeId         The scoped node ID of the device that should open the commissioning window.
     * @param iterations           The number of PBKDF (Password-Based Key Derivation Function) iterations to use
     *                             for deriving the PAKE (Password Authenticated Key Exchange) verifier.
     * @param commissioningTimeoutSec The time in seconds before the commissioning window closes. This value determines
     *                             how long the commissioning window remains open for incoming connections.
     * @param discriminator        The device-specific discriminator, determined during commissioning, which helps
     *                             to uniquely identify the device among others.
     * @param salt                 The salt used in the cryptographic operations for commissioning.
     * @param verifier             The PAKE verifier used to authenticate the commissioning process.
     *
     */
    void OpenDeviceCommissioningWindow(chip::ScopedNodeId scopedNodeId, uint32_t iterations, uint16_t commissioningTimeoutSec,
                                       uint16_t discriminator, const chip::ByteSpan & salt, const chip::ByteSpan & verifier);

    /**
     * @brief Open the commissioning window of a device from another fabric via its fabric bridge.
     *
     * This function initiates the process to open the commissioning window for a device that belongs to another
     * fabric, accessed through a fabric bridge.
     *
     * @param remoteEndpointId The endpoint ID of the remote device that should open the commissioning window.
     *                         This endpoint is associated with the device in the other fabric, accessed via the
     *                         fabric bridge.
     *
     * @note This function is used when the device to be commissioned is part of a different fabric and must be
     *       accessed through an intermediary fabric bridge.
     */
    void OpenRemoteDeviceCommissioningWindow(chip::EndpointId remoteEndpointId);

    void SubscribeRemoteFabricBridge();

    void ReadSupportedDeviceCategories();

    void HandleAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader & data);

    void HandleEventData(const chip::app::EventHeader & header, chip::TLV::TLVReader & data);

    void HandleCommandResponse(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader & data);

    SyncedDevice * FindDeviceByEndpoint(chip::EndpointId endpointId);
    SyncedDevice * FindDeviceByNode(chip::NodeId nodeId);

private:
    void RequestCommissioningApproval();

    void HandleReadSupportedDeviceCategories(chip::TLV::TLVReader & data);

    void HandleCommissioningRequestResult(chip::TLV::TLVReader & data);

    void HandleAttributePartsListUpdate(chip::TLV::TLVReader & data);

    void SendCommissionNodeRequest(uint64_t requestId, uint16_t responseTimeoutSeconds);

    void HandleReverseOpenCommissioningWindow(chip::TLV::TLVReader & data);

    chip::NodeId mLastUsedNodeId = 0;

    // The Node ID of the remote bridge used for Fabric-Sync
    // This represents the bridge on the other ecosystem.
    chip::NodeId mRemoteBridgeNodeId = chip::kUndefinedNodeId;

    // The Node ID of the local bridge used for Fabric-Sync
    // This represents the bridge within its own ecosystem.
    chip::NodeId mLocalBridgeNodeId = chip::kUndefinedNodeId;

    std::set<SyncedDevice> mSyncedDevices;
    bool mInitialized   = false;
    uint64_t mRequestId = 0;

    BridgeSubscription mBridgeSubscriber;
    CommissionerControl mCommissionerControl;
    FabricSyncGetter mFabricSyncGetter;
};

} // namespace admin
