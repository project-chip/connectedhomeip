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
#include "gen/enums.h"
#include <lib/core/CHIPSafeCasts.h>

using namespace ::chip;

constexpr uint16_t kWaitDurationInSeconds     = 120;
constexpr uint64_t kBreadcrumb                = 0;
constexpr uint32_t kTimeoutMs                 = 6000;
constexpr uint8_t kTemporaryThreadNetworkId[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

CHIP_ERROR PairingCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Controller::ControllerInitParams params{
        .storageDelegate              = &storage,
        .mDeviceAddressUpdateDelegate = this,
    };

    err = mCommissioner.Init(localId, params, this);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", ErrorStr(err)));

    err = RunInternal(remoteId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Init Failure! PairDevice: %s", ErrorStr(err)));

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
    return err;
}

CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mRemoteId = remoteId;

    InitCallbacks();
    UpdateWaitForResponse(true);
    switch (mPairingMode)
    {
    case PairingMode::None:
        err = Unpair(remoteId);
        break;
    case PairingMode::Bypass:
        err = PairWithoutSecurity(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::Ble:
        err = Pair(remoteId, PeerAddress::BLE());
        break;
    case PairingMode::SoftAP:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::Ethernet:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    }
    WaitForResponse(kWaitDurationInSeconds);
    ReleaseCallbacks();

    return err;
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    RendezvousParameters params =
        RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    return mCommissioner.PairDevice(remoteId, params);
}

CHIP_ERROR PairingCommand::PairWithoutSecurity(NodeId remoteId, PeerAddress address)
{
    ChipSerializedDevice serializedTestDevice;
    return mCommissioner.PairTestDeviceWithoutSecurity(remoteId, address, serializedTestDevice);
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    SetCommandExitStatus(true);
    return mCommissioner.UnpairDevice(remoteId);
}

void PairingCommand::OnStatusUpdate(RendezvousSessionDelegate::Status status)
{
    switch (status)
    {
    case RendezvousSessionDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        break;
    case RendezvousSessionDelegate::Status::SecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        break;
    case RendezvousSessionDelegate::Status::NetworkProvisioningSuccess:
        ChipLogProgress(chipTool, "Network Provisioning Success");
        break;
    case RendezvousSessionDelegate::Status::NetworkProvisioningFailed:
        ChipLogError(chipTool, "Network Provisioning Failed");
        break;
    }
}

void PairingCommand::OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback)
{
    ChipLogProgress(chipTool, "OnNetworkCredentialsRequested");
    callback->SendNetworkCredentials(mSSID, mPassword);
}

void PairingCommand::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                       RendezvousDeviceCredentialsDelegate * callback)
{
    // TODO Implement this
    ChipLogProgress(chipTool, "OnOperationalCredentialsRequested");
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Success");
        SetupNetwork();
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Failure: %s", ErrorStr(err));
        SetCommandExitStatus(false);
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

    SetCommandExitStatus(err == CHIP_NO_ERROR);
}

CHIP_ERROR PairingCommand::SetupNetwork()
{

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mNetworkType)
    {
    case PairingNetworkType::None:
        // Nothing to do
        SetCommandExitStatus(err == CHIP_NO_ERROR);
        break;
    case PairingNetworkType::WiFi:
    case PairingNetworkType::Ethernet:
    case PairingNetworkType::Thread:
        err = mCommissioner.GetDevice(mRemoteId, &mDevice);
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

void PairingCommand::ReleaseCallbacks()
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
    ByteSpan operationalDataset            = ByteSpan(Uint8::from_char(mOperationalDataset), strlen(mOperationalDataset));

    return mCluster.AddThreadNetwork(successCallback, failureCallback, operationalDataset, kBreadcrumb, kTimeoutMs);
}

CHIP_ERROR PairingCommand::AddWiFiNetwork()
{
    Callback::Cancelable * successCallback = mOnAddWiFiNetworkCallback->Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback->Cancel();
    ByteSpan ssid                          = ByteSpan(Uint8::from_char(mSSID), strlen(mSSID));
    ByteSpan credentials                   = ByteSpan(Uint8::from_char(mPassword), strlen(mPassword));

    return mCluster.AddWiFiNetwork(successCallback, failureCallback, ssid, credentials, kBreadcrumb, kTimeoutMs);
}

CHIP_ERROR PairingCommand::EnableNetwork()
{
    Callback::Cancelable * successCallback = mOnEnableNetworkCallback->Cancel();
    Callback::Cancelable * failureCallback = mOnFailureCallback->Cancel();

    ByteSpan networkId;
    if (mNetworkType == PairingNetworkType::WiFi)
    {
        networkId = ByteSpan(Uint8::from_char(mSSID), strlen(mSSID));
    }
    else
    {
        networkId = ByteSpan(kTemporaryThreadNetworkId, sizeof(kTemporaryThreadNetworkId));
    }

    return mCluster.EnableNetwork(successCallback, failureCallback, networkId, kBreadcrumb, kTimeoutMs);
}

void PairingCommand::OnDefaultFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(chipTool, "Default Failure Response: 0x%02x", status);

    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    command->SetCommandExitStatus(false);
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
    //    command->SetCommandExitStatus(false);
    //    return;
    // }

    CHIP_ERROR err = command->EnableNetwork();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Setup failure. Internal error calling EnableNetwork: %s", ErrorStr(err));
        command->SetCommandExitStatus(false);
        return;
    }

    // When the accessory is configured as a SoftAP and WiFi is configured to an other network
    // there won't be any response since the WiFi network is changing.
    // So returns early, assuming everything goes well
    // and the device address will be updated once a command is issued.
    if (command->mPairingMode == PairingMode::SoftAP && command->mNetworkType == PairingNetworkType::WiFi)
    {
        command->SetCommandExitStatus(true);
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
    //    command->SetCommandExitStatus(false);
    //    return;
    // }

    CHIP_ERROR err = command->UpdateNetworkAddress();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Setup failure. Internal error calling UpdateNetworkAddress: %s", ErrorStr(err));
        command->SetCommandExitStatus(false);
        return;
    }
}

CHIP_ERROR PairingCommand::UpdateNetworkAddress()
{
    ReturnErrorOnFailure(mCommissioner.GetDevice(mRemoteId, &mDevice));
    return mCommissioner.UpdateDevice(mDevice, mFabricId);
}

void PairingCommand::OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR err)
{
    ChipLogProgress(chipTool, "OnAddressUpdateComplete: %s", ErrorStr(err));
    SetCommandExitStatus(CHIP_NO_ERROR == err);
}
