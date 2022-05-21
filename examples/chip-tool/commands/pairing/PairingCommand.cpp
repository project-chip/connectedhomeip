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
using namespace ::chip::Controller;

CHIP_ERROR PairingCommand::RunCommand()
{
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
    case PairingMode::Ethernet:
        err = Pair(remoteId, PeerAddress::UDP(mRemoteAddr.address, mRemotePort, mRemoteAddr.interfaceId));
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

CHIP_ERROR PairingCommand::PaseWithCode(NodeId remoteId)
{
    return CurrentCommissioner().EstablishPASEConnection(remoteId, mOnboardingPayload);
}

CHIP_ERROR PairingCommand::PairWithCode(NodeId remoteId)
{
    CommissioningParameters commissioningParams = GetCommissioningParameters();
    return CurrentCommissioner().PairDevice(remoteId, mOnboardingPayload, commissioningParams);
}

CHIP_ERROR PairingCommand::Pair(NodeId remoteId, PeerAddress address)
{
    RendezvousParameters params =
        RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);
    CommissioningParameters commissioningParams = GetCommissioningParameters();
    return CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);
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
        if (mPairingMode == PairingMode::CodePaseOnly)
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
    // Ignore nodes with closed comissioning window
    VerifyOrReturn(nodeData.commissionData.commissioningMode != 0);

    const uint16_t port = nodeData.resolutionData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    nodeData.resolutionData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery. Is it the right method ?
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(nullptr);

    Inet::InterfaceId interfaceId =
        nodeData.resolutionData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.resolutionData.interfaceId : Inet::InterfaceId::Null();
    PeerAddress peerAddress = PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], port, interfaceId);
    CHIP_ERROR err          = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        SetCommandExitStatus(err);
    }
}
