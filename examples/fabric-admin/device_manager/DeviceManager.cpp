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

constexpr uint16_t kWindowTimeout              = 300;
constexpr uint16_t kIteration                  = 1000;
constexpr uint16_t kSubscribeMinInterval       = 0;
constexpr uint16_t kSubscribeMaxInterval       = 60;
constexpr uint16_t kAggragatorEndpointId       = 1;
constexpr uint16_t kMaxDiscriminatorLength     = 4095;
constexpr uint8_t kEnhancedCommissioningMethod = 1;

} // namespace

// Define the static member
DeviceManager DeviceManager::sInstance;

void DeviceManager::Init()
{
    // TODO: (#34113) Init mLastUsedNodeId from chip config file
    mLastUsedNodeId = 1;
    mInitialized    = true;

    ChipLogProgress(NotSpecified, "DeviceManager initialized: last used nodeId " ChipLogFormatX64,
                    ChipLogValueX64(mLastUsedNodeId));
}

NodeId DeviceManager::GetNextAvailableNodeId()
{
    mLastUsedNodeId++;
    VerifyOrDieWithMsg(mLastUsedNodeId < std::numeric_limits<NodeId>::max(), NotSpecified, "No more available NodeIds.");

    return mLastUsedNodeId;
}

void DeviceManager::UpdateLastUsedNodeId(NodeId nodeId)
{
    if (nodeId > mLastUsedNodeId)
    {
        mLastUsedNodeId = nodeId;
        ChipLogProgress(NotSpecified, "Updating last used NodeId to " ChipLogFormatX64, ChipLogValueX64(mLastUsedNodeId));
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
    ChipLogProgress(NotSpecified, "Open the commissioning window of device with NodeId:" ChipLogFormatX64, ChipLogValueX64(nodeId));

    // Open the commissioning window of a device within its own fabric.
    StringBuilder<kMaxCommandSize> commandBuilder;

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

    // Use random discriminator to have less chance of collision.
    uint16_t discriminator =
        Crypto::GetRandU16() % (kMaxDiscriminatorLength + 1); // Include the upper limit kMaxDiscriminatorLength

    commandBuilder.Add("pairing open-commissioning-window ");
    commandBuilder.AddFormat("%lu %d %d %d %d %d", mRemoteBridgeNodeId, remoteEndpointId, kEnhancedCommissioningMethod,
                             kWindowTimeout, kIteration, discriminator);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::PairRemoteFabricBridge(chip::NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                           uint16_t deviceRemotePort)
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing already-discovered ");
    commandBuilder.AddFormat("%lu %d %s %d", nodeId, setupPINCode, deviceRemoteIp, deviceRemotePort);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::PairRemoteDevice(NodeId nodeId, const char * payload)
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing code ");
    commandBuilder.AddFormat("%lu %s", nodeId, payload);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::PairLocalFabricBridge(NodeId nodeId)
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing already-discovered ");
    commandBuilder.AddFormat("%lu %d ::1 %d", nodeId, mLocalBridgeSetupPinCode, mLocalBridgePort);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::UnpairRemoteFabricBridge()
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing unpair ");
    commandBuilder.AddFormat("%lu", mRemoteBridgeNodeId);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::UnpairLocalFabricBridge()
{
    StringBuilder<kMaxCommandSize> commandBuilder;

    commandBuilder.Add("pairing unpair ");
    commandBuilder.AddFormat("%lu", mLocalBridgeNodeId);

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
    commandBuilder.AddFormat("%d %d %lu %d --is-urgent true --keepSubscriptions true", kSubscribeMinInterval, kSubscribeMaxInterval,
                             mRemoteBridgeNodeId, kRootEndpointId);
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

void DeviceManager::HandleReadSupportedDeviceCategories(chip::TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Attribute SupportedDeviceCategories detected.");

    BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode attribute value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    if (value.Has(CommissionerControl::SupportedDeviceCategoryBitmap::kFabricSynchronization))
    {
        ChipLogProgress(NotSpecified, "Remote Fabric-Bridge supports Fabric Synchronization, start reverse commissioning.");
        RequestCommissioningApproval();
    }
}

void DeviceManager::RequestCommissioningApproval()
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

void DeviceManager::HandleCommissioningRequestResult(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "CommissioningRequestResult event received.");

    CommissionerControl::Events::CommissioningRequestResult::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode event value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    if (value.requestID != mRequestId)
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
    SendCommissionNodeRequest(value.requestID, kResponseTimeoutSeconds);
}

void DeviceManager::HandleAttributePartsListUpdate(chip::TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Attribute PartsList change detected:");

    app::DataModel::DecodableList<EndpointId> value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
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
        // print to console
        fprintf(stderr, "A new device is added on Endpoint: %u\n", endpoint);

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

void DeviceManager::SendCommissionNodeRequest(uint64_t requestId, uint16_t responseTimeoutSeconds)
{
    ChipLogProgress(NotSpecified, "Request the Commissioner Control Server to begin commissioning a previously approved request.");

    StringBuilder<kMaxCommandSize> commandBuilder;
    commandBuilder.Add("commissionercontrol commission-node ");
    commandBuilder.AddFormat("%lu %u %lu %d", requestId, responseTimeoutSeconds, mRemoteBridgeNodeId, kRootEndpointId);

    PushCommand(commandBuilder.c_str());
}

void DeviceManager::HandleReverseOpenCommissioningWindow(TLV::TLVReader & data)
{
    CommissionerControl::Commands::ReverseOpenCommissioningWindow::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Log all fields
    ChipLogProgress(NotSpecified, "DecodableType fields:");
    ChipLogProgress(NotSpecified, "  commissioningTimeout: %u", value.commissioningTimeout);
    ChipLogProgress(NotSpecified, "  discriminator: %u", value.discriminator);
    ChipLogProgress(NotSpecified, "  iterations: %u", value.iterations);

    char verifierHex[Crypto::kSpake2p_VerifierSerialized_Length * 2 + 1];
    Encoding::BytesToHex(value.PAKEPasscodeVerifier.data(), value.PAKEPasscodeVerifier.size(), verifierHex, sizeof(verifierHex),
                         Encoding::HexFlags::kNullTerminate);
    ChipLogProgress(NotSpecified, "  PAKEPasscodeVerifier: %s", verifierHex);

    char saltHex[Crypto::kSpake2p_Max_PBKDF_Salt_Length * 2 + 1];
    Encoding::BytesToHex(value.salt.data(), value.salt.size(), saltHex, sizeof(saltHex), Encoding::HexFlags::kNullTerminate);
    ChipLogProgress(NotSpecified, "  salt: %s", saltHex);

    OpenDeviceCommissioningWindow(mLocalBridgeNodeId, value.commissioningTimeout, value.iterations, value.discriminator, saltHex,
                                  verifierHex);
}

void DeviceManager::HandleAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader & data)
{
    if (path.mClusterId == CommissionerControl::Id &&
        path.mAttributeId == CommissionerControl::Attributes::SupportedDeviceCategories::Id)
    {
        HandleReadSupportedDeviceCategories(data);
        return;
    }

    if (path.mClusterId == Descriptor::Id && path.mAttributeId == Descriptor::Attributes::PartsList::Id)
    {
        HandleAttributePartsListUpdate(data);
        return;
    }
}

void DeviceManager::HandleEventData(const app::EventHeader & header, TLV::TLVReader & data)
{
    if (header.mPath.mClusterId == CommissionerControl::Id &&
        header.mPath.mEventId == CommissionerControl::Events::CommissioningRequestResult::Id)
    {
        HandleCommissioningRequestResult(data);
    }
}

void DeviceManager::HandleCommandResponse(const app::ConcreteCommandPath & path, TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Command Response received.");

    if (path.mClusterId == CommissionerControl::Id &&
        path.mCommandId == CommissionerControl::Commands::ReverseOpenCommissioningWindow::Id)
    {
        HandleReverseOpenCommissioningWindow(data);
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
