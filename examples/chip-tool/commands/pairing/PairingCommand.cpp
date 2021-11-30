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

CHIP_ERROR PairingCommand::RunCommand()
{
    CurrentCommissioner().RegisterDeviceAddressUpdateDelegate(this);
    CurrentCommissioner().RegisterPairingDelegate(this);
    return RunInternal(mNodeId);
}

CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mPairingMode)
    {
    case PairingMode::None:
        err = Unpair(remoteId);
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
    }

    return err;
}

CHIP_ERROR PairingCommand::PairWithQRCode(NodeId remoteId)
{
    return CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload);
}

CHIP_ERROR PairingCommand::PairWithManualCode(NodeId remoteId)
{
    return CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload);
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    RendezvousParameters params =
        RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    return CurrentCommissioner().PairDevice(remoteId, params);
}

CHIP_ERROR PairingCommand::PairWithMdns(NodeId remoteId)
{
    Dnssd::DiscoveryFilter filter(mFilterType);
    switch (mFilterType)
    {
    case chip::Dnssd::DiscoveryFilterType::kNone:
        break;
    case chip::Dnssd::DiscoveryFilterType::kShort:
    case chip::Dnssd::DiscoveryFilterType::kLong:
    case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
    case chip::Dnssd::DiscoveryFilterType::kVendor:
    case chip::Dnssd::DiscoveryFilterType::kDeviceType:
        filter.code = mDiscoveryFilterCode;
        break;
    case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
        break;
    case chip::Dnssd::DiscoveryFilterType::kCommissioner:
        filter.code = 1;
        break;
    case chip::Dnssd::DiscoveryFilterType::kInstanceName:
        filter.code         = 0;
        filter.instanceName = mDiscoveryFilterInstanceName;
        break;
    }

    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    return CurrentCommissioner().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    CHIP_ERROR err = CurrentCommissioner().UnpairDevice(remoteId);
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
        // Nothing to do here - device address has been resolved as part of the commissioning process.
        break;
    case PairingNetworkType::WiFi:
    case PairingNetworkType::Thread:
        err = CurrentCommissioner().GetDeviceBeingCommissioned(mNodeId, &mDevice);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Setup failure! No pairing for device: %" PRIu64, mNodeId));

        mCluster.Associate(mDevice, mEndpointId);

        err = AddNetwork(mNetworkType);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(chipTool, "Setup failure! Error calling AddWiFiNetwork: %s", ErrorStr(err)));
        break;
    }

exit:
    return err;
}

CHIP_ERROR PairingCommand::AddNetwork(PairingNetworkType networkType)
{
    return (networkType == PairingNetworkType::WiFi) ? AddWiFiNetwork() : AddThreadNetwork();
}

CHIP_ERROR PairingCommand::AddThreadNetwork()
{
    Callback::Cancelable * successCallback = mOnAddThreadNetworkCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback.Cancel();

    return mCluster.AddThreadNetwork(successCallback, failureCallback, mOperationalDataset, kBreadcrumb, kTimeoutMs);
}

CHIP_ERROR PairingCommand::AddWiFiNetwork()
{
    Callback::Cancelable * successCallback = mOnAddWiFiNetworkCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback.Cancel();

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
    Callback::Cancelable * successCallback = mOnEnableNetworkCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback.Cancel();

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

void PairingCommand::OnAddNetworkResponse(void * context, uint8_t errorCode, CharSpan debugText)
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

void PairingCommand::OnEnableNetworkResponse(void * context, uint8_t errorCode, CharSpan debugText)
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
    ChipLogProgress(chipTool, "Mdns: Updating NodeId: %" PRIx64 " Compressed FabricId: %" PRIx64 " ...", mNodeId,
                    CurrentCommissioner().GetCompressedFabricId());
    return CurrentCommissioner().UpdateDevice(mNodeId);
}

void PairingCommand::OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR err)
{
    ChipLogProgress(chipTool, "OnAddressUpdateComplete: %" PRIx64 ": %s", nodeId, ErrorStr(err));
    if (err != CHIP_NO_ERROR)
    {
        // For some devices, it may take more time to appear on the network and become discoverable
        // over DNS-SD, so don't give up on failure and restart the address update. Note that this
        // will not be repeated endlessly as each chip-tool command has a timeout (in the case of
        // the `pairing` command it equals 120s).
        UpdateNetworkAddress();
    }
}

void PairingCommand::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    const uint16_t port = nodeData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    nodeData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery. Is it the right method ?
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(nullptr);

    Inet::InterfaceId interfaceId = nodeData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.interfaceId[0] : Inet::InterfaceId::Null();
    PeerAddress peerAddress       = PeerAddress::UDP(nodeData.ipAddress[0], port, interfaceId);
    CHIP_ERROR err                = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}
