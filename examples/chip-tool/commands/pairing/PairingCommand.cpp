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
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/PASESession.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

using namespace ::chip;
using namespace ::chip::Controller;
using Commissionee = chip::Commissioner::Commissionee;

CHIP_ERROR PairingCommand::RunCommand()
{
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

CommissioningParameters PairingCommand::GetCommissioningParameters()
{
    switch (mNetworkType)
    {
    case PairingNetworkType::WiFi:
        return CommissioningParameters().SetWiFiCredentials(Controller::WiFiCredentials(mSSID, mPassword));
    case PairingNetworkType::Thread:
        return CommissioningParameters().SetThreadOperationalDataset(mOperationalDataset);
    case PairingNetworkType::Ethernet:
    case PairingNetworkType::None:
        return CommissioningParameters();
    }
    return CommissioningParameters();
}

CHIP_ERROR PairingCommand::PairWithQRCode(NodeId remoteId)
{
    auto stateMachine = Platform::MakeShared<CommissioningStateMachine>();
    VerifyOrReturnError(stateMachine.get() != nullptr, CHIP_ERROR_NO_MEMORY);
    auto onSuccess = [this, stateMachine](Commissionee & commissionee) { OnCommissioningComplete(*stateMachine.get()); };
    auto onFailure = [this, stateMachine](Commissionee & commissionee) { OnCommissioningFailure(*stateMachine.get()); };
    stateMachine.get()->Init(chip::Controller::DeviceControllerFactory::GetInstance().GetSystemState(),
                             mCredIssuerCmds->GetCredentialIssuer(), CurrentController().GetFabricIndex(), mNodeId,
                             mOperationalDataset, mSSID, mPassword);
    CHIP_ERROR err = stateMachine.get()->Commission(mOnboardingPayload, onSuccess, onFailure);
    if (err != CHIP_NO_ERROR)
    {
        stateMachine.get()->Shutdown();
    }
    return err;
}

CHIP_ERROR PairingCommand::PairWithManualCode(NodeId remoteId)
{
    auto stateMachine = Platform::MakeShared<CommissioningStateMachine>();
    VerifyOrReturnError(stateMachine.get() != nullptr, CHIP_ERROR_NO_MEMORY);
    auto onSuccess = [this, stateMachine](Commissionee & commissionee) { OnCommissioningComplete(*stateMachine.get()); };
    auto onFailure = [this, stateMachine](Commissionee & commissionee) { OnCommissioningFailure(*stateMachine.get()); };
    stateMachine.get()->Init(chip::Controller::DeviceControllerFactory::GetInstance().GetSystemState(),
                             mCredIssuerCmds->GetCredentialIssuer(), CurrentController().GetFabricIndex(), mNodeId,
                             mOperationalDataset, mSSID, mPassword);
    CHIP_ERROR err = stateMachine.get()->Commission(mOnboardingPayload, onSuccess, onFailure);
    if (err != CHIP_NO_ERROR)
    {
        stateMachine.get()->Shutdown();
    }
    return err;
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    auto stateMachine = Platform::MakeShared<CommissioningStateMachine>();
    VerifyOrReturnError(stateMachine.get() != nullptr, CHIP_ERROR_NO_MEMORY);
    auto onSuccess = [this, stateMachine](Commissionee & commissionee) { OnCommissioningComplete(*stateMachine.get()); };
    auto onFailure = [this, stateMachine](Commissionee & commissionee) { OnCommissioningFailure(*stateMachine.get()); };
    stateMachine.get()->Init(chip::Controller::DeviceControllerFactory::GetInstance().GetSystemState(),
                             mCredIssuerCmds->GetCredentialIssuer(), CurrentController().GetFabricIndex(), mNodeId,
                             mOperationalDataset, mSSID, mPassword);
    CHIP_ERROR err =
        stateMachine.get()->Commission(chip::RendezvousInformationFlag::kNone, mDiscriminator, mSetupPINCode, onSuccess, onFailure);
    if (err != CHIP_NO_ERROR)
    {
        stateMachine.get()->Shutdown();
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
    case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator: {
        auto stateMachine = Platform::MakeShared<CommissioningStateMachine>();
        VerifyOrReturnError(stateMachine.get() != nullptr, CHIP_ERROR_NO_MEMORY);
        auto onSuccess = [this, stateMachine](Commissionee & commissionee) { OnCommissioningComplete(*stateMachine.get()); };
        auto onFailure = [this, stateMachine](Commissionee & commissionee) { OnCommissioningFailure(*stateMachine.get()); };
        stateMachine.get()->Init(chip::Controller::DeviceControllerFactory::GetInstance().GetSystemState(),
                                 mCredIssuerCmds->GetCredentialIssuer(), CurrentController().GetFabricIndex(), mNodeId,
                                 mOperationalDataset, mSSID, mPassword);
        CHIP_ERROR err =
            stateMachine.get()->Commission(chip::RendezvousInformationFlag::kOnNetwork, static_cast<uint16_t>(mDiscoveryFilterCode),
                                           mSetupPINCode, onSuccess, onFailure);
        return err;
    }
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

    ReturnErrorOnFailure(
        mDnsResolver.Init(chip::Controller::DeviceControllerFactory::GetInstance().GetSystemState()->UDPEndPointManager()));
    mDnsResolver.SetResolverDelegate(this);
    ReturnErrorOnFailure(mDnsResolver.FindCommissionableNodes(filter));
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingCommand::Unpair(NodeId remoteId)
{
    CurrentController().ReleaseOperationalDevice(remoteId);
    CHIP_ERROR err = CHIP_NO_ERROR;
    SetCommandExitStatus(err);
    return err;
}

void PairingCommand::OnCommissioningComplete(CommissioningStateMachine & stateMachine)
{
    CHIP_ERROR err = stateMachine.GrabCommissionee(CurrentController());
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "Device commissioning completed with success");
    }
    else
    {
        ChipLogProgress(chipTool, "Device commissioning Failure: %s", ErrorStr(err));
    }
    stateMachine.Shutdown();
    SetCommandExitStatus(err);
}

void PairingCommand::OnCommissioningFailure(CommissioningStateMachine & stateMachine)
{
    ChipLogProgress(chipTool, "Device commissioning Failure");
    SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

void PairingCommand::OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    const uint16_t port = nodeData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    nodeData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    mDnsResolver.Shutdown();

    Inet::InterfaceId interfaceId = nodeData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.interfaceId[0] : Inet::InterfaceId::Null();
    PeerAddress peerAddress       = PeerAddress::UDP(nodeData.ipAddress[0], port, interfaceId);
    CHIP_ERROR err                = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}

void PairingCommand::OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData) {}

void PairingCommand::OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) {}
