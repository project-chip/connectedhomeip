/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ChipDeviceController-ScriptPairingDeviceDiscoveryDelegate.h"

namespace chip {
namespace Controller {

void ScriptPairingDeviceDiscoveryDelegate::OnDiscoveredDevice(const Dnssd::DiscoveredNodeData & nodeData)
{
    // Ignore nodes with closed comissioning window
    VerifyOrReturn(nodeData.commissionData.commissioningMode != 0);
    VerifyOrReturn(mActiveDeviceCommissioner != nullptr);

    const uint16_t port = nodeData.resolutionData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    nodeData.resolutionData.ipAddress[0].ToString(buf);
    ChipLogProgress(chipTool, "Discovered Device: %s:%u", buf, port);

    // Cancel discovery timer.
    chip::DeviceLayer::SystemLayer().CancelTimer(OnDiscoveredTimeout, this);

    // Stop Mdns discovery.
    mActiveDeviceCommissioner->RegisterDeviceDiscoveryDelegate(nullptr);

    Inet::InterfaceId interfaceId =
        nodeData.resolutionData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.resolutionData.interfaceId : Inet::InterfaceId::Null();
    PeerAddress peerAddress = PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], port, interfaceId);

    RendezvousParameters keyExchangeParams = RendezvousParameters().SetSetupPINCode(mSetupPasscode).SetPeerAddress(peerAddress);

    CHIP_ERROR err = mActiveDeviceCommissioner->PairDevice(mNodeId, keyExchangeParams, mParams);
    if (err != CHIP_NO_ERROR)
    {
        VerifyOrReturn(mPairingDelegate != nullptr);
        mPairingDelegate->OnCommissioningComplete(mNodeId, err);
    }
}
} // namespace Controller
} // namespace chip
