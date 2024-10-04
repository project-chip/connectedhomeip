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
#include <device_manager/PairingManager.h>
#include <platform/CHIPDeviceLayer.h>

#include <set>

constexpr uint32_t kDefaultSetupPinCode    = 20202021;
constexpr uint16_t kDefaultLocalBridgePort = 5540;
constexpr uint16_t kResponseTimeoutSeconds = 30;

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

    chip::NodeId GetLocalBridgeNodeId() const { return mLocalBridgeNodeId; }

    void UpdateLastUsedNodeId(chip::NodeId nodeId);

    void SetRemoteBridgeNodeId(chip::NodeId nodeId) { mRemoteBridgeNodeId = nodeId; }

    void SetLocalBridgePort(uint16_t port) { mLocalBridgePort = port; }
    void SetLocalBridgeSetupPinCode(uint32_t pinCode) { mLocalBridgeSetupPinCode = pinCode; }
    void SetLocalBridgeNodeId(chip::NodeId nodeId) { mLocalBridgeNodeId = nodeId; }

    bool IsAutoSyncEnabled() const { return mAutoSyncEnabled; }

    bool IsFabricSyncReady() const { return mRemoteBridgeNodeId != chip::kUndefinedNodeId; }

    bool IsLocalBridgeReady() const { return mLocalBridgeNodeId != chip::kUndefinedNodeId; }

    void EnableAutoSync(bool state) { mAutoSyncEnabled = state; }

    void AddSyncedDevice(const Device & device);

    void RemoveSyncedDevice(chip::NodeId nodeId);

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
     * @param nodeId               The ID of the node that should open the commissioning window.
     * @param commissioningTimeoutSec The time in seconds before the commissioning window closes. This value determines
     *                             how long the commissioning window remains open for incoming connections.
     * @param iterations           The number of PBKDF (Password-Based Key Derivation Function) iterations to use
     *                             for deriving the PAKE (Password Authenticated Key Exchange) verifier.
     * @param discriminator        The device-specific discriminator, determined during commissioning, which helps
     *                             to uniquely identify the device among others.
     * @param salt                 The salt used in the cryptographic operations for commissioning.
     * @param verifier             The PAKE verifier used to authenticate the commissioning process.
     *
     */
    void OpenDeviceCommissioningWindow(chip::NodeId nodeId, uint32_t commissioningTimeoutSec, uint32_t iterations,
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

    /**
     * @brief Pair a remote fabric bridge with a given node ID.
     *
     * This function initiates the pairing process for a remote fabric bridge using the specified parameters.

     * @param nodeId            The user-defined ID for the node being commissioned. It doesn’t need to be the same ID,
     *                          as for the first fabric.
     * @param setupPINCode      The setup PIN code used to authenticate the pairing process.
     * @param deviceRemoteIp    The IP address of the remote device that is being paired as part of the fabric bridge.
     * @param deviceRemotePort  The secured device port of the remote device that is being paired as part of the fabric bridge.
     */
    void PairRemoteFabricBridge(chip::NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp, uint16_t deviceRemotePort);

    /**
     * @brief Pair a remote Matter device to the current fabric.
     *
     * This function initiates the pairing process for a remote device using the specified parameters.

     * @param nodeId            The user-defined ID for the node being commissioned. It doesn’t need to be the same ID,
     *                          as for the first fabric.
     * @param payload           The the QR code payload or a manual pairing code generated by the first commissioner
     *                          instance when opened commissioning window.
     */
    void PairRemoteDevice(chip::NodeId nodeId, const char * payload);

    /**
     * @brief Pair a local fabric bridge with a given node ID.
     *
     * This function initiates the pairing process for the local fabric bridge using the specified parameters.

     * @param nodeId            The user-defined ID for the node being commissioned. It doesn’t need to be the same ID,
     *                          as for the first fabric.
     */
    void PairLocalFabricBridge(chip::NodeId nodeId);

    void UnpairRemoteFabricBridge();

    void UnpairLocalFabricBridge();

    void SubscribeRemoteFabricBridge();

    void ReadSupportedDeviceCategories();

    void HandleAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader & data);

    void HandleEventData(const chip::app::EventHeader & header, chip::TLV::TLVReader & data);

    void HandleCommandResponse(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader & data);

    Device * FindDeviceByEndpoint(chip::EndpointId endpointId);
    Device * FindDeviceByNode(chip::NodeId nodeId);

private:
    friend DeviceManager & DeviceMgr();

    void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) override;

    void RequestCommissioningApproval();

    void HandleReadSupportedDeviceCategories(chip::TLV::TLVReader & data);

    void HandleCommissioningRequestResult(chip::TLV::TLVReader & data);

    void HandleAttributePartsListUpdate(chip::TLV::TLVReader & data);

    void SendCommissionNodeRequest(uint64_t requestId, uint16_t responseTimeoutSeconds);

    void HandleReverseOpenCommissioningWindow(chip::TLV::TLVReader & data);

    static DeviceManager sInstance;

    chip::NodeId mLastUsedNodeId = 0;

    // The Node ID of the remote bridge used for Fabric-Sync
    // This represents the bridge on the other ecosystem.
    chip::NodeId mRemoteBridgeNodeId = chip::kUndefinedNodeId;

    uint16_t mLocalBridgePort         = kDefaultLocalBridgePort;
    uint32_t mLocalBridgeSetupPinCode = kDefaultSetupPinCode;
    // The Node ID of the local bridge used for Fabric-Sync
    // This represents the bridge within its own ecosystem.
    chip::NodeId mLocalBridgeNodeId = chip::kUndefinedNodeId;

    std::set<Device> mSyncedDevices;
    bool mAutoSyncEnabled = false;
    bool mInitialized     = false;
    uint64_t mRequestId   = 0;
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
