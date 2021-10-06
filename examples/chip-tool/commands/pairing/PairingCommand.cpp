/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "PairingCommand.h"
#include "platform/PlatformManager.h"
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/PASESession.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

using namespace ::chip;

constexpr uint64_t kBreadcrumb = 0;
constexpr uint32_t kTimeoutMs  = 6000;

CHIP_ERROR PairingCommand::Run()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    GetExecContext()->commissioner->RegisterDeviceAddressUpdateDelegate(this);
    GetExecContext()->commissioner->RegisterPairingDelegate(this);

    if (mPairingMode != PairingMode::OpenCommissioningWindow)
    {
#if CONFIG_PAIR_WITH_RANDOM_ID
        // Generate a random remote id so we don't end up reusing the same node id
        // for different nodes.
        //
        // TODO: Ideally we'd just ask for an operational cert for the commissionnee
        // and get the node from that, but the APIs are not set up that way yet.
        NodeId randomId;
        ReturnErrorOnFailure(Controller::ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(&randomId));

        ChipLogProgress(Controller, "Generated random node id: 0x" ChipLogFormatX64, ChipLogValueX64(randomId));

        ReturnErrorOnFailure(GetExecContext()->storage->SetRemoteNodeId(randomId));
        GetExecContext()->remoteId = randomId;
#else  // CONFIG_PAIR_WITH_RANDOM_ID
       // Use the default id, not whatever happens to be in our storage, since this
       // is a new pairing.
        GetExecContext()->remoteId = kTestDeviceNodeId;
#endif // CONFIG_PAIR_WITH_RANDOM_ID
    }

    err = RunInternal(GetExecContext()->remoteId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Init Failure! PairDevice: %s", ErrorStr(err)));

exit:
    return err;
}

CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mRemoteId = remoteId;

    InitCallbacks();

    switch (mPairingMode)
    {
    case PairingMode::None:
        err = Unpair(remoteId);
        break;
    case PairingMode::Bypass:
        err = PairWithoutSecurity(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::QRCode:
        err = PairWithQRCode(remoteId);
        break;
    case PairingMode::ManualCode:
        err = PairWithManualCode(remoteId);
        break;
    case PairingMode::Ble:
        err = Pair(remoteId, PeerAddress::BLE());
        break;
    case PairingMode::OnNetwork:
        err = PairWithMdns(remoteId);
        break;
    case PairingMode::SoftAP:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::Ethernet:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::OpenCommissioningWindow:
        err = GetExecContext()->commissioner->GetConnectedDevice(GetExecContext()->remoteId, &mOnDeviceConnectedCallback,
                                                                 &mOnDeviceConnectionFailureCallback);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Failed in initiating connection to the device: %" PRIu64 ", error %" CHIP_ERROR_FORMAT,
                         GetExecContext()->remoteId, err.Format());
        }

        break;
    }

    return err;
}

void PairingCommand::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    command->OpenCommissioningWindow();
}
void PairingCommand::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    ChipLogError(chipTool, "Failed in connecting to the device %" PRIu64 ". Error %" CHIP_ERROR_FORMAT, deviceId, error.Format());
    command->SetCommandExitStatus(error);
}

CHIP_ERROR PairingCommand::PairWithQRCode(NodeId remoteId)
{
    return GetExecContext()->commissioner->PairDevice(remoteId, mOnboardingPayload);
}

CHIP_ERROR PairingCommand::PairWithManualCode(NodeId remoteId)
{
    return GetExecContext()->commissioner->PairDevice(remoteId, mOnboardingPayload);
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    RendezvousParameters params =
        RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    return GetExecContext()->commissioner->PairDevice(remoteId, params);
}

CHIP_ERROR PairingCommand::PairWithMdns(NodeId remoteId)
{
    Mdns::DiscoveryFilter filter(mFilterType);
    switch (mFilterType)
    {
    case chip::Mdns::DiscoveryFilterType::kNone:
        break;
    case chip::Mdns::DiscoveryFilterType::kShort:
    case chip::Mdns::DiscoveryFilterType::kLong:
    case chip::Mdns::DiscoveryFilterType::kCompressedFabricId:
    case chip::Mdns::DiscoveryFilterType::kVendor:
    case chip::Mdns::DiscoveryFilterType::kDeviceType:
        filter.code = mDiscoveryFilterCode;
        break;
    case chip::Mdns::DiscoveryFilterType::kCommissioningMode:
        break;
    case chip::Mdns::DiscoveryFilterType::kCommissioner:
        filter.code = 1;
        break;
    case chip::Mdns::DiscoveryFilterType::kInstanceName:
        filter.code         = 0;
        filter.instanceName = mDiscoveryFilterInstanceName;
        break;
    }

    GetExecContext()->commissioner->RegisterDeviceDiscoveryDelegate(this);
    return GetExecContext()->commissioner->DiscoverCommissionableNodes(filter);
}

CHIP_ERROR PairingCommand::PairWithoutSecurity(NodeId remoteId, PeerAddress address)
{
    ChipSerializedDevice serializedTestDevice;
    return GetExecContext()->commissioner->PairTestDeviceWithoutSecurity(remoteId, address, serializedTestDevice);
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    CHIP_ERROR err = GetExecContext()->commissioner->UnpairDevice(remoteId);
    SetCommandExitStatus(err);
    return err;
}

CHIP_ERROR PairingCommand::OpenCommissioningWindow()
{
    CHIP_ERROR err = GetExecContext()->commissioner->OpenCommissioningWindow(GetExecContext()->remoteId, mTimeout, mIteration,
                                                                             mDiscriminator, mCommissioningWindowOption);
    SetCommandExitStatus(err);
    return err;
}

void PairingCommand::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        break;
    }
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Success");
        err = SetupNetwork();
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Failure: %s", ErrorStr(err));
    }

    if (err != CHIP_NO_ERROR)
    {
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Deleted Failure: %s", ErrorStr(err));
    }

    SetCommandExitStatus(err);
}

void PairingCommand::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Device commissioning completed with success");
    }
    else
    {
        ChipLogProgress(chipTool, "Device commissioning Failure: %s", ErrorStr(err));
    }

    SetCommandExitStatus(err);
}

CHIP_ERROR PairingCommand::SetupNetwork()
{

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mNetworkType)
    {
    case PairingNetworkType::None:
    case PairingNetworkType::Ethernet:
        // Nothing to do other than to resolve the device's operational address.
        err = UpdateNetworkAddress();
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(chipTool, "Setup failure! Error calling UpdateNetworkAddress: %s", ErrorStr(err)));
        break;
    case PairingNetworkType::WiFi:
    case PairingNetworkType::Thread:
        err = GetExecContext()->commissioner->GetDevice(mRemoteId, &mDevice);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Setup failure! No pairing for device: %" PRIu64, mRemoteId));

        mCluster.Associate(mDevice, mEndpointId);

        err = AddNetwork(mNetworkType);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(chipTool, "Setup failure! Error calling AddWiFiNetwork: %s", ErrorStr(err)));
        break;
    }

exit:
    return err;
}

void PairingCommand::InitCallbacks()
{
    mOnAddThreadNetworkCallback =
        new Callback::Callback<NetworkCommissioningClusterAddThreadNetworkResponseCallback>(OnAddNetworkResponse, this);
    mOnAddWiFiNetworkCallback =
        new Callback::Callback<NetworkCommissioningClusterAddWiFiNetworkResponseCallback>(OnAddNetworkResponse, this);
    mOnEnableNetworkCallback =
        new Callback::Callback<NetworkCommissioningClusterEnableNetworkResponseCallback>(OnEnableNetworkResponse, this);
    mOnFailureCallback = new Callback::Callback<DefaultFailureCallback>(OnDefaultFailureResponse, this);
}

void PairingCommand::Shutdown()
{
    delete mOnAddThreadNetworkCallback;
    delete mOnAddWiFiNetworkCallback;
    delete mOnEnableNetworkCallback;
    delete mOnFailureCallback;
}

CHIP_ERROR PairingCommand::AddNetwork(PairingNetworkType networkType)
{
    return (networkType == PairingNetworkType::WiFi) ? AddWiFiNetwork() : AddThreadNetwork();
}

CHIP_ERROR PairingCommand::AddThreadNetwork()
{
    Callback::Cancelable * successCallback = mOnAddThreadNetworkCallback->Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback->Cancel();

    return mCluster.AddThreadNetwork(successCallback, failureCallback, mOperationalDataset, kBreadcrumb, kTimeoutMs);
}

CHIP_ERROR PairingCommand::AddWiFiNetwork()
{
    Callback::Cancelable * successCallback = mOnAddWiFiNetworkCallback->Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback->Cancel();

    return mCluster.AddWiFiNetwork(successCallback, failureCallback, mSSID, mPassword, kBreadcrumb, kTimeoutMs);
}

chip::ByteSpan PairingCommand::GetThreadNetworkId()
{
    // For Thread devices the networkId is the extendedPanId and it is
    // part of the dataset defined by OpenThread

    Thread::OperationalDataset dataset;

    if (dataset.Init(mOperationalDataset) != CHIP_NO_ERROR)
    {
        return ByteSpan();
    }

    if (dataset.GetExtendedPanId(mExtendedPanId) != CHIP_NO_ERROR)
    {
        return ByteSpan();
    }

    return ByteSpan(mExtendedPanId);
}

CHIP_ERROR PairingCommand::EnableNetwork()
{
    Callback::Cancelable * successCallback = mOnEnableNetworkCallback->Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback->Cancel();

    ByteSpan networkId;
    if (mNetworkType == PairingNetworkType::WiFi)
    {
        networkId = mSSID;
    }
    else
    {
        networkId = GetThreadNetworkId();
    }

    if (networkId.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return mCluster.EnableNetwork(successCallback, failureCallback, networkId, kBreadcrumb, kTimeoutMs);
}

void PairingCommand::OnDefaultFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(chipTool, "Default Failure Response: 0x%02x", status);

    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    command->SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

void PairingCommand::OnAddNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText)
{
    ChipLogProgress(chipTool, "AddNetworkResponse");

    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);

    // Normally, the errorCode should be checked, but the current codebase send a default response
    // instead of the command specific response. So errorCode is not set correctly.
    // if (EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS != errorCode)
    // {
    //    ChipLogError(chipTool, "Setup failure. Error calling EnableNetwork: %d", errorCode);
    //    command->SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    //    return;
    // }

    CHIP_ERROR err = command->EnableNetwork();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Setup failure. Internal error calling EnableNetwork: %s", ErrorStr(err));
        command->SetCommandExitStatus(err);
        return;
    }

    // When the accessory is configured as a SoftAP and WiFi is configured to an other network
    // there won't be any response since the WiFi network is changing.
    // So returns early, assuming everything goes well
    // and the device address will be updated once a command is issued.
    if (command->mPairingMode == PairingMode::SoftAP && command->mNetworkType == PairingNetworkType::WiFi)
    {
        command->SetCommandExitStatus(CHIP_NO_ERROR);
    }
}

void PairingCommand::OnEnableNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText)
{
    ChipLogProgress(chipTool, "EnableNetworkResponse");

    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);

    // Normally, the errorCode should be checked, but the current codebase send a default response
    // instead of the command specific response. So errorCode is not set correctly.
    // if (EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS != errorCode)
    // {
    //    ChipLogError(chipTool, "Setup failure. Error calling EnableNetwork: %d", errorCode);
    //    command->SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    //    return;
    // }

    CHIP_ERROR err = command->UpdateNetworkAddress();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Setup failure. Internal error calling UpdateNetworkAddress: %s", ErrorStr(err));
        command->SetCommandExitStatus(err);
        return;
    }
}

CHIP_ERROR PairingCommand::UpdateNetworkAddress()
{
    ChipLogProgress(chipTool, "Mdns: Updating NodeId: %" PRIx64 " Compressed FabricId: %" PRIx64 " ...", mRemoteId,
                    GetExecContext()->commissioner->GetCompressedFabricId());
    return GetExecContext()->commissioner->UpdateDevice(mRemoteId);
}

void PairingCommand::OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR err)
{
    ChipLogProgress(chipTool, "OnAddressUpdateComplete: %" PRIx64 ": %s", nodeId, ErrorStr(err));
    if (err != CHIP_NO_ERROR && nodeId == mRemoteId)
    {
        // Set exit status only if the address update failed.
        // Otherwise wait for OnCommissioningComplete() callback.
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnDiscoveredDevice(const chip::Mdns::DiscoveredNodeData & nodeData)
{
    const uint16_t port = nodeData.port;
    char buf[chip::Inet::kMaxIPAddressStringLength];
    nodeData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery. Is it the right method ?
    GetExecContext()->commissioner->RegisterDeviceDiscoveryDelegate(nullptr);

    Inet::InterfaceId interfaceId = nodeData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.interfaceId[0] : INET_NULL_INTERFACEID;
    PeerAddress peerAddress       = PeerAddress::UDP(nodeData.ipAddress[0], port, interfaceId);
    CHIP_ERROR err                = Pair(mRemoteId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}
