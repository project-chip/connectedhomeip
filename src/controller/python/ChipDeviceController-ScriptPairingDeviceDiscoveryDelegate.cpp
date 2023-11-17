/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
