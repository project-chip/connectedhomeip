/*
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

#include <LinuxCommissionableDataProvider.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <cstdio>
#include <string>

using namespace chip;

namespace admin {

namespace {

constexpr EndpointId kAggregatorEndpointId = 1;

} // namespace

LinuxCommissionableDataProvider sCommissionableDataProvider;

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

void DeviceManager::SetRemoteBridgeNodeId(chip::NodeId nodeId)
{
    mRemoteBridgeNodeId = nodeId;

    if (mRemoteBridgeNodeId != kUndefinedNodeId)
    {
        mCommissionerControl.Init(PairingManager::Instance().CurrentCommissioner(), mRemoteBridgeNodeId, kAggregatorEndpointId);
    }
}

SyncedDevice * DeviceManager::FindDeviceByEndpoint(EndpointId endpointId)
{
    for (auto & device : mSyncedDevices)
    {
        if (device.GetEndpointId() == endpointId)
        {
            return const_cast<SyncedDevice *>(&device);
        }
    }
    return nullptr;
}

SyncedDevice * DeviceManager::FindDeviceByNode(NodeId nodeId)
{
    for (auto & device : mSyncedDevices)
    {
        if (device.GetNodeId() == nodeId)
        {
            return const_cast<SyncedDevice *>(&device);
        }
    }
    return nullptr;
}

void DeviceManager::OpenLocalBridgeCommissioningWindow(uint32_t iterations, uint16_t commissioningTimeoutSec,
                                                       uint16_t discriminator, const ByteSpan & salt, const ByteSpan & verifier)
{
    ChipLogProgress(NotSpecified, "Opening commissioning window of the local bridge");

    auto & commissionMgr      = Server::GetInstance().GetCommissioningWindowManager();
    auto commissioningTimeout = System::Clock::Seconds16(commissioningTimeoutSec);

    Optional<std::vector<uint8_t>> spake2pVerifier = verifier.empty()
        ? Optional<std::vector<uint8_t>>::Missing()
        : Optional<std::vector<uint8_t>>(std::vector<uint8_t>(verifier.begin(), verifier.end()));

    Optional<std::vector<uint8_t>> spake2pSalt = salt.empty()
        ? Optional<std::vector<uint8_t>>::Missing()
        : Optional<std::vector<uint8_t>>(std::vector<uint8_t>(salt.begin(), salt.end()));

    CHIP_ERROR err =
        sCommissionableDataProvider.Init(spake2pVerifier, spake2pSalt, iterations, Optional<uint32_t>(), discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to initialize the commissionable data provider of the local bridge: %s", ErrorStr(err));
        return;
    }

    DeviceLayer::SetCommissionableDataProvider(&sCommissionableDataProvider);

    err = commissionMgr.OpenBasicCommissioningWindow(commissioningTimeout);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to open commissioning window of the local bridge: %s", ErrorStr(err));
    }
}

CHIP_ERROR DeviceManager::PairRemoteFabricBridge(NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                                 uint16_t deviceRemotePort)
{
    CHIP_ERROR err = PairingManager::Instance().PairDevice(nodeId, setupPINCode, deviceRemoteIp, deviceRemotePort);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to pair remote fabric bridge: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::PairRemoteDevice(NodeId nodeId, const char * payload)
{
    CHIP_ERROR err = PairingManager::Instance().PairDeviceWithCode(nodeId, payload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::PairRemoteDevice(NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                           uint16_t deviceRemotePort)
{
    CHIP_ERROR err = PairingManager::Instance().PairDevice(nodeId, setupPINCode, deviceRemoteIp, deviceRemotePort);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device: Node ID " ChipLogFormatX64 " with error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::UnpairRemoteFabricBridge()
{
    if (mRemoteBridgeNodeId == kUndefinedNodeId)
    {
        ChipLogError(NotSpecified, "Remote bridge node ID is undefined; cannot unpair device.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = PairingManager::Instance().UnpairDevice(mRemoteBridgeNodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to unpair remote bridge device " ChipLogFormatX64, ChipLogValueX64(mRemoteBridgeNodeId));
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::UnpairRemoteDevice(NodeId nodeId)
{
    CHIP_ERROR err = PairingManager::Instance().UnpairDevice(nodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to unpair remote device " ChipLogFormatX64, ChipLogValueX64(nodeId));
        return err;
    }

    return CHIP_NO_ERROR;
}

void DeviceManager::SubscribeRemoteFabricBridge()
{
    ChipLogProgress(NotSpecified, "Start subscription to the remote bridge.");

    CHIP_ERROR error = mBridgeSubscriber.StartSubscription(PairingManager::Instance().CurrentCommissioner(), mRemoteBridgeNodeId,
                                                           kAggregatorEndpointId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to subscribe to the remote bridge (NodeId: %lu). Error: %" CHIP_ERROR_FORMAT,
                     mRemoteBridgeNodeId, error.Format());
        return;
    }
}

void DeviceManager::ReadSupportedDeviceCategories()
{
    if (!IsFabricSyncReady())
    {
        // print to console
        fprintf(stderr, "Remote Fabric Bridge is not configured yet.\n");
        return;
    }

    ChipLogProgress(NotSpecified, "Read SupportedDeviceCategories from the remote bridge.");

    CHIP_ERROR error = mFabricSyncGetter.GetFabricSynchronizationData(
        [this](TLV::TLVReader & data) { this->HandleReadSupportedDeviceCategories(data); },
        PairingManager::Instance().CurrentCommissioner(), this->GetRemoteBridgeNodeId(), kAggregatorEndpointId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to read SupportedDeviceCategories from the remote bridge (NodeId: %lu). Error: %" CHIP_ERROR_FORMAT,
                     mRemoteBridgeNodeId, error.Format());
    }
}

void DeviceManager::HandleReadSupportedDeviceCategories(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Attribute SupportedDeviceCategories detected.");

    BitMask<app::Clusters::CommissionerControl::SupportedDeviceCategoryBitmap> value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode attribute value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    if (value.Has(app::Clusters::CommissionerControl::SupportedDeviceCategoryBitmap::kFabricSynchronization))
    {
        ChipLogProgress(NotSpecified, "Remote Fabric-Bridge supports Fabric Synchronization, start reverse commissioning.");
        RequestCommissioningApproval();
    }
    else
    {
        ChipLogProgress(NotSpecified, "Remote Fabric-Bridge does not support Fabric Synchronization.");
    }
}

void DeviceManager::RequestCommissioningApproval()
{
    ChipLogProgress(NotSpecified, "Starting reverse commissioning for bridge device: NodeId: " ChipLogFormatX64,
                    ChipLogValueX64(mRemoteBridgeNodeId));

    uint64_t requestId = Crypto::GetRandU64();
    uint16_t vendorId  = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    uint16_t productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);

    CHIP_ERROR error = mCommissionerControl.RequestCommissioningApproval(requestId, vendorId, productId, NullOptional);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to request commissioning-approval to the remote bridge (NodeId: %lu). Error: %" CHIP_ERROR_FORMAT,
                     mRemoteBridgeNodeId, error.Format());
        return;
    }

    mRequestId = requestId;
}

void DeviceManager::HandleCommissioningRequestResult(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "CommissioningRequestResult event received.");

    app::Clusters::CommissionerControl::Events::CommissioningRequestResult::DecodableType value;
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

void DeviceManager::HandleAttributePartsListUpdate(TLV::TLVReader & data)
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
    // because we need to work with the SyncedDevice objects in mSyncedDevices,
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
    }

    // Process removed endpoints
    for (const auto & endpoint : removedEndpoints)
    {
        ChipLogProgress(NotSpecified, "Endpoint removed: %u", endpoint);

        SyncedDevice * device = FindDeviceByEndpoint(endpoint);

        if (device == nullptr)
        {
            ChipLogProgress(NotSpecified, "No device on Endpoint: %u", endpoint);
            continue;
        }
    }
}

void DeviceManager::SendCommissionNodeRequest(uint64_t requestId, uint16_t responseTimeoutSeconds)
{
    ChipLogProgress(NotSpecified, "Request the Commissioner Control Server to begin commissioning a previously approved request.");

    CHIP_ERROR error = mCommissionerControl.CommissionNode(requestId, responseTimeoutSeconds);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to send CommissionNode command to the remote bridge (NodeId: %lu). Error: %" CHIP_ERROR_FORMAT,
                     mRemoteBridgeNodeId, error.Format());
        return;
    }
}

void DeviceManager::HandleReverseOpenCommissioningWindow(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Handle ReverseOpenCommissioningWindow command.");

    app::Clusters::CommissionerControl::Commands::ReverseOpenCommissioningWindow::DecodableType value;
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
    ChipLogProgress(NotSpecified, "  PAKEPasscodeVerifier size: %lu", value.PAKEPasscodeVerifier.size());
    ChipLogProgress(NotSpecified, "  salt size: %lu", value.salt.size());

    OpenLocalBridgeCommissioningWindow(value.iterations, value.commissioningTimeout, value.discriminator,
                                       ByteSpan(value.salt.data(), value.salt.size()),
                                       ByteSpan(value.PAKEPasscodeVerifier.data(), value.PAKEPasscodeVerifier.size()));
}

void DeviceManager::HandleAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader & data)
{
    if (path.mClusterId == app::Clusters::Descriptor::Id &&
        path.mAttributeId == app::Clusters::Descriptor::Attributes::PartsList::Id)
    {
        HandleAttributePartsListUpdate(data);
        return;
    }
}

void DeviceManager::HandleEventData(const app::EventHeader & header, TLV::TLVReader & data)
{
    if (header.mPath.mClusterId == app::Clusters::CommissionerControl::Id &&
        header.mPath.mEventId == app::Clusters::CommissionerControl::Events::CommissioningRequestResult::Id)
    {
        HandleCommissioningRequestResult(data);
    }
}

void DeviceManager::HandleCommandResponse(const app::ConcreteCommandPath & path, TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Command Response received.");

    if (path.mClusterId == app::Clusters::CommissionerControl::Id &&
        path.mCommandId == app::Clusters::CommissionerControl::Commands::ReverseOpenCommissioningWindow::Id)
    {
        VerifyOrDie(path.mEndpointId == kAggregatorEndpointId);
        HandleReverseOpenCommissioningWindow(data);
    }
}

} // namespace admin
