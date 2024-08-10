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
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <cstdio>
#include <string>

using namespace chip;
using namespace chip::app::Clusters;

namespace {

// Constants
constexpr uint32_t kSetupPinCode               = 20202021;
constexpr uint16_t kRemoteBridgePort           = 5540;
constexpr uint16_t kDiscriminator              = 3840;
constexpr uint16_t kWindowTimeout              = 300;
constexpr uint16_t kIteration                  = 1000;
constexpr uint16_t kSubscribeMinInterval       = 0;
constexpr uint16_t kSubscribeMaxInterval       = 60;
constexpr uint16_t kAggragatorEndpointId       = 1;
constexpr uint8_t kEnhancedCommissioningMethod = 1;

} // namespace

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

void DeviceManager::OpenDeviceCommissioningWindow(NodeId nodeId, uint32_t commissioningTimeout, uint32_t iterations,
                                                  uint32_t discriminator, const char * saltHex, const char * verifierHex)
{
    // Open the commissioning window of a device within its own fabric.
    StringBuilder<512> commandBuilder;

    commandBuilder.Add("pairing open-commissioning-window ");
    commandBuilder.AddFormat("%lu %d %d %d %d %d --salt hex:%s --verifier hex:%s", nodeId, kRootEndpointId,
                             kEnhancedCommissioningMethod, commissioningTimeout, iterations, discriminator, saltHex, verifierHex);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::OpenRemoteDeviceCommissioningWindow(EndpointId remoteEndpointId)
{
    // Open the commissioning window of a device from another fabric via its fabric bridge.
    // This method constructs and sends a command to open the commissioning window for a device
    // that is part of a different fabric, accessed through a fabric bridge.
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing open-commissioning-window ");
    commandBuilder.AddFormat("%lu %d %d %d %d %d", mRemoteBridgeNodeId, remoteEndpointId, kEnhancedCommissioningMethod,
                             kWindowTimeout, kIteration, kDiscriminator);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::PairRemoteFabricBridge(NodeId nodeId, const char * deviceRemoteIp)
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing already-discovered ");
    commandBuilder.AddFormat("%lu %d %s %d", nodeId, kSetupPinCode, deviceRemoteIp, kRemoteBridgePort);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::PairRemoteDevice(chip::NodeId nodeId, const char * payload)
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing code ");
    commandBuilder.AddFormat("%lu %s", nodeId, payload);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::UnpairRemoteFabricBridge()
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing unpair ");
    commandBuilder.AddFormat("%lu", mRemoteBridgeNodeId);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::SubscribeRemoteFabricBridge()
{
    // Listen to the state changes of the remote fabric bridge.
    StringBuilder<kMaxCommandSize> commandBuilder;

    // Prepare and push the descriptor subscribe command
    commandBuilder.Add("descriptor subscribe parts-list ");
    commandBuilder.AddFormat("%d %d %lu %d", kSubscribeMinInterval, kSubscribeMaxInterval, mRemoteBridgeNodeId,
                             kAggragatorEndpointId);
    PushCommand(commandBuilder.c_str());

    // Clear the builder for the next command
    commandBuilder.Reset();

    // Prepare and push the commissioner control subscribe command
    commandBuilder.Add("commissionercontrol subscribe-event commissioning-request-result ");
    commandBuilder.AddFormat("%d %d %lu %d --is-urgent true", kSubscribeMinInterval, kSubscribeMaxInterval, mRemoteBridgeNodeId,
                             kRootEndpointId);
    PushCommand(commandBuilder.c_str());
}

void DeviceManager::ReadSupportedDeviceCategories()
{
    if (!IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet.\n");
        return;
    }

    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("commissionercontrol read supported-device-categories ");
    commandBuilder.AddFormat("%ld ", mRemoteBridgeNodeId);
    commandBuilder.AddFormat("%d", kRootEndpointId);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::StartReverseCommissioning()
{
    ChipLogProgress(NotSpecified, "Starting reverse commissioning for bridge device: NodeId: " ChipLogFormatX64,
                    ChipLogValueX64(mRemoteBridgeNodeId));

    uint64_t requestId = Crypto::GetRandU64();
    uint16_t vendorId  = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    uint16_t productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);

    StringBuilder<kMaxCommandSize> commandBuilder;
    commandBuilder.Add("commissionercontrol request-commissioning-approval ");
    commandBuilder.AddFormat("%lu %u %u %lu %d", requestId, vendorId, productId, mRemoteBridgeNodeId, kRootEndpointId);

    mRequestId = requestId;
    PushCommand(commandBuilder.c_str());
}

void DeviceManager::CommissionApprovedRequest(uint64_t requestId, uint16_t responseTimeoutSeconds)
{
    ChipLogProgress(NotSpecified, "Request the Commissioner Control Server to begin commissioning a previously approved request.");

    StringBuilder<kMaxCommandSize> commandBuilder;
    commandBuilder.Add("commissionercontrol commission-node ");
    commandBuilder.AddFormat("%lu %u %lu %d", requestId, responseTimeoutSeconds, mRemoteBridgeNodeId, kRootEndpointId);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::HandleAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader * data)
{
    if (path.mClusterId == CommissionerControl::Id &&
        path.mAttributeId == CommissionerControl::Attributes::SupportedDeviceCategories::Id)
    {
        ChipLogProgress(NotSpecified, "Attribute SupportedDeviceCategories detected.");

        BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> value;
        CHIP_ERROR error = app::DataModel::Decode(*data, value);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to decode attribute value. Error: %" CHIP_ERROR_FORMAT, error.Format());
            return;
        }

        if (value.Has(CommissionerControl::SupportedDeviceCategoryBitmap::kFabricSynchronization))
        {
            ChipLogProgress(NotSpecified, "Remote Fabric-Bridge supports Fabric Synchronization, start reverse commissioning.");
            StartReverseCommissioning();
        }

        return;
    }

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
            StringBuilder<kMaxCommandSize> commandBuilder;
            commandBuilder.Add("fabricsync sync-device ");
            commandBuilder.AddFormat("%d", endpoint);
            PushCommand(commandBuilder.c_str());
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
            StringBuilder<kMaxCommandSize> commandBuilder;
            commandBuilder.Add("pairing unpair ");
            commandBuilder.AddFormat("%lu", device->GetNodeId());

            PairingCommand * pairingCommand = static_cast<PairingCommand *>(CommandMgr().GetCommandByName("pairing", "unpair"));

            if (pairingCommand == nullptr)
            {
                ChipLogError(NotSpecified, "Pairing code command is not available");
                return;
            }

            pairingCommand->RegisterPairingDelegate(this);
            PushCommand(commandBuilder.c_str());
        }
    }
}

void DeviceManager::HandleEventData(const chip::app::EventHeader & header, chip::TLV::TLVReader * data)
{
    if (header.mPath.mClusterId != CommissionerControl::Id ||
        header.mPath.mEventId != CommissionerControl::Events::CommissioningRequestResult::Id)
    {
        return;
    }

    ChipLogProgress(NotSpecified, "CommissioningRequestResult event received.");

    CommissionerControl::Events::CommissioningRequestResult::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(*data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode event value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    if (value.requestId != mRequestId)
    {
        ChipLogError(NotSpecified, "The RequestId does not match the RequestId provided to RequestCommissioningApproval");
        return;
    }

    if (value.statusCode != static_cast<uint8_t>(Protocols::InteractionModel::Status::Success))
    {
        ChipLogError(NotSpecified, "The server is not ready to begin commissioning the requested device");
        return;
    }

    // The server is ready to begin commissioning the requested device, request the Commissioner Control Server to begin
    // commissioning a previously approved request.
    CommissionApprovedRequest(value.requestId, kResponseTimeoutSeconds);
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
