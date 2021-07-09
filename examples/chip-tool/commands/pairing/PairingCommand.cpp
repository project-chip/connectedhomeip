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

CHIP_ERROR PairingCommand::Run()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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

    switch (mPairingMode)
    {
    case PairingMode::None:
        err = Unpair(remoteId);
        break;
    case PairingMode::Bypass:
        err = PairWithoutSecurity(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::QRCode:
        err = PairAndCommissionWithQRCode(remoteId);
        break;
    case PairingMode::ManualCode:
        err = PairAndCommissionWithManualCode(remoteId);
        break;
    case PairingMode::Ble:
        err = PairAndCommission(remoteId, PeerAddress::BLE());
        break;
    case PairingMode::OnNetwork:
    case PairingMode::SoftAP:
        err = PairAndCommission(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
        break;
    case PairingMode::Ethernet:
        err = PairAndCommission(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort));
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

CHIP_ERROR PairingCommand::PairAndCommissionWithQRCode(NodeId remoteId)
{
    SetupPayload payload;
    ReturnErrorOnFailure(QRCodeSetupPayloadParser(mOnboardingPayload).populatePayload(payload));

    // TODO: Both of these codes should attempt discovery first, like in chip-device-ctrl.
    chip::RendezvousInformationFlags rendezvousInformation = payload.rendezvousInformation;
    ReturnErrorCodeIf(rendezvousInformation != RendezvousInformationFlag::kBLE, CHIP_ERROR_INVALID_ARGUMENT);

    mDiscriminator = payload.discriminator;
    mSetupPINCode  = payload.setUpPINCode;

    return PairAndCommission(remoteId, PeerAddress::BLE());
}

CHIP_ERROR PairingCommand::PairAndCommissionWithManualCode(NodeId remoteId)
{
    SetupPayload payload;
    ReturnErrorOnFailure(ManualSetupPayloadParser(mOnboardingPayload).populatePayload(payload));
    mDiscriminator = payload.discriminator;
    mSetupPINCode  = payload.setUpPINCode;
    return PairAndCommission(remoteId, PeerAddress::BLE());
}

CHIP_ERROR PairingCommand::PairAndCommission(NodeId remoteId, PeerAddress address)
{
    RendezvousParameters rendezvousParams =
        RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    return GetExecContext()->commissioner->PairDevice(remoteId, rendezvousParams);
}

// TODO: This is marked as deprecated and has been for a while. Are we ready to deprecate this?
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
    case DevicePairingDelegate::Status::SecurePairingSuccess: {
        ChipLogProgress(chipTool, "Secure Pairing Success");

        CommissioningParameters commissioningParams;
        if (mPairingMode == PairingMode::Ble)
        {
            commissioningParams.SetWifiCredentials({ mSSID, mPassword });
        }
        GetExecContext()->commissioner->Commission(mRemoteId, commissioningParams);
    }
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
