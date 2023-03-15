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
#include <commands/common/DeviceScanner.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/PASESession.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

using namespace ::chip;
using namespace ::chip::Controller;

CHIP_ERROR PairingCommand::RunCommand()
{
    CurrentCommissioner().RegisterPairingDelegate(this);
    // Clear the CATs in OperationalCredentialsIssuer
    mCredIssuerCmds->SetCredentialIssuerCATValues(kUndefinedCATs);

    if (mCASEAuthTags.HasValue() && mCASEAuthTags.Value().size() <= kMaxSubjectCATAttributeCount)
    {
        CATValues cats = kUndefinedCATs;
        for (size_t index = 0; index < mCASEAuthTags.Value().size(); ++index)
        {
            cats.values[index] = mCASEAuthTags.Value()[index];
        }
        if (cats.AreValid())
        {
            mCredIssuerCmds->SetCredentialIssuerCATValues(cats);
        }
    }
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
    case PairingMode::Code:
        err = PairWithCode(remoteId);
        break;
    case PairingMode::CodePaseOnly:
        err = PaseWithCode(remoteId);
        break;
    case PairingMode::Ble:
        err = Pair(remoteId, PeerAddress::BLE());
        break;
    case PairingMode::OnNetwork:
        err = PairWithMdns(remoteId);
        break;
    case PairingMode::SoftAP:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort, mRemoteAddr.interfaceId));
        break;
    case PairingMode::AlreadyDiscovered:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort, mRemoteAddr.interfaceId));
        break;
    case PairingMode::AlreadyDiscoveredByIndex:
        err = PairWithMdnsOrBleByIndex(remoteId, mIndex);
        break;
    }

    return err;
}

CommissioningParameters PairingCommand::GetCommissioningParameters()
{
    auto params = CommissioningParameters();
    params.SetSkipCommissioningComplete(mSkipCommissioningComplete.ValueOr(false));
    if (mBypassAttestationVerifier.ValueOr(false))
    {
        params.SetDeviceAttestationDelegate(this);
    }

    switch (mNetworkType)
    {
    case PairingNetworkType::WiFi:
        params.SetWiFiCredentials(Controller::WiFiCredentials(mSSID, mPassword));
        break;
    case PairingNetworkType::Thread:
        params.SetThreadOperationalDataset(mOperationalDataset);
        break;
    case PairingNetworkType::None:
        break;
    }

    return params;
}

CHIP_ERROR PairingCommand::PaseWithCode(NodeId remoteId)
{
    auto discoveryType = DiscoveryType::kAll;
    if (mUseOnlyOnNetworkDiscovery.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnly;
    }

    if (mDiscoverOnce.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry;
    }

    return CurrentCommissioner().EstablishPASEConnection(remoteId, mOnboardingPayload, discoveryType);
}

CHIP_ERROR PairingCommand::PairWithCode(NodeId remoteId)
{
    CommissioningParameters commissioningParams = GetCommissioningParameters();

    // If no network discovery behavior and no network credentials are provided, assume that the pairing command is trying to pair
    // with an on-network device.
    if (!mUseOnlyOnNetworkDiscovery.HasValue())
    {
        auto threadCredentials = commissioningParams.GetThreadOperationalDataset();
        auto wiFiCredentials   = commissioningParams.GetWiFiCredentials();
        mUseOnlyOnNetworkDiscovery.SetValue(!threadCredentials.HasValue() && !wiFiCredentials.HasValue());
    }

    auto discoveryType = DiscoveryType::kAll;
    if (mUseOnlyOnNetworkDiscovery.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnly;
    }

    if (mDiscoverOnce.ValueOr(false))
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry;
    }

    return CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload, commissioningParams, discoveryType);
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    auto params = RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (mPaseOnly.ValueOr(false))
    {
        err = CurrentCommissioner().EstablishPASEConnection(remoteId, params);
    }
    else
    {
        auto commissioningParams = GetCommissioningParameters();
        err                      = CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);
    }
    return err;
}

CHIP_ERROR PairingCommand::PairWithMdnsOrBleByIndex(NodeId remoteId, uint16_t index)
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    RendezvousParameters params;
    ReturnErrorOnFailure(GetDeviceScanner().Get(index, params));
    params.SetSetupPINCode(mSetupPINCode);

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (mPaseOnly.ValueOr(false))
    {
        err = CurrentCommissioner().EstablishPASEConnection(remoteId, params);
    }
    else
    {
        auto commissioningParams = GetCommissioningParameters();
        err                      = CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);
    }
    return err;
}

CHIP_ERROR PairingCommand::PairWithMdns(NodeId remoteId)
{
    Dnssd::DiscoveryFilter filter(mFilterType);
    switch (mFilterType)
    {
    case chip::Dnssd::DiscoveryFilterType::kNone:
        break;
    case chip::Dnssd::DiscoveryFilterType::kShortDiscriminator:
    case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator:
    case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
    case chip::Dnssd::DiscoveryFilterType::kVendorId:
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
    mCurrentFabricRemover = Platform::MakeUnique<Controller::CurrentFabricRemover>(&CurrentCommissioner());
    return mCurrentFabricRemover->RemoveCurrentFabric(remoteId, &mCurrentFabricRemoveCallback);
}

void PairingCommand::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        ChipLogProgress(chipTool, "CASE establishment successful");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
        break;
    }
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Pairing Success");
        ChipLogProgress(chipTool, "PASE establishment successful");
        if (mPairingMode == PairingMode::CodePaseOnly || mPaseOnly.ValueOr(false))
        {
            SetCommandExitStatus(err);
        }
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

void PairingCommand::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    // Ignore nodes with closed commissioning window
    VerifyOrReturn(nodeData.commissionData.commissioningMode != 0);

    auto & resolutionData = nodeData.resolutionData;

    const uint16_t port = resolutionData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    resolutionData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery.
    auto err = CurrentCommissioner().StopCommissionableDiscovery();

    // Some platforms does not implement a mechanism to stop mdns browse, so
    // we just ignore CHIP_ERROR_NOT_IMPLEMENTED instead of bailing out.
    if (CHIP_NO_ERROR != err && CHIP_ERROR_NOT_IMPLEMENTED != err)
    {
        SetCommandExitStatus(err);
        return;
    }

    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(nullptr);

    auto interfaceId = resolutionData.ipAddress[0].IsIPv6LinkLocal() ? resolutionData.interfaceId : Inet::InterfaceId::Null();
    auto peerAddress = PeerAddress::UDP(resolutionData.ipAddress[0], port, interfaceId);
    err              = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnCurrentFabricRemove(void * context, NodeId nodeId, CHIP_ERROR err)
{
    PairingCommand * command = reinterpret_cast<PairingCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnCurrentFabricRemove: context is null"));

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Device unpair completed with success: " ChipLogFormatX64, ChipLogValueX64(nodeId));
    }
    else
    {
        ChipLogProgress(chipTool, "Device unpair Failure: " ChipLogFormatX64 " %s", ChipLogValueX64(nodeId), ErrorStr(err));
    }

    command->SetCommandExitStatus(err);
}

chip::Optional<uint16_t> PairingCommand::FailSafeExpiryTimeoutSecs() const
{
    // We don't need to set additional failsafe timeout as we don't ask the final user if he wants to continue
    return chip::Optional<uint16_t>();
}

void PairingCommand::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
                                                  chip::DeviceProxy * device,
                                                  const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                                  chip::Credentials::AttestationVerificationResult attestationResult)
{
    // Bypass attestation verification, continue with success
    auto err = deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(
        device, chip::Credentials::AttestationVerificationResult::kSuccess);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}
