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

using namespace ::chip;

constexpr uint16_t kWaitDurationInSeconds = 120;

CHIP_ERROR PairingCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mCommissioner.Init(localId, &storage, this);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    err = RunInternal(remoteId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Init Failure! PairDevice: %s", chip::ErrorStr(err)));

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
    return err;
}

CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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
    }
    WaitForResponse(kWaitDurationInSeconds);

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
    return mCommissioner.PairTestDeviceWithoutSecurity(remoteId, address.GetIPAddress(), serializedTestDevice, address.GetPort());
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    UpdateWaitForResponse(false);
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
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Failure: %s", chip::ErrorStr(err));
    }

    SetCommandExitStatus(err == CHIP_NO_ERROR);
    UpdateWaitForResponse(false);
}

void PairingCommand::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(chipTool, "Pairing Deleted Failure: %s", chip::ErrorStr(err));
    }

    SetCommandExitStatus(err == CHIP_NO_ERROR);
    UpdateWaitForResponse(false);
}
