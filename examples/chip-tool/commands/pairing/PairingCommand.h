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

#pragma once

#include "../common/CHIPCommand.h"
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

enum class PairingMode
{
    None,
    QRCode,
    ManualCode,
    Ble,
    SoftAP,
    Ethernet,
    OnNetwork,
};

enum class PairingNetworkType
{
    None,
    WiFi,
    Thread,
    Ethernet,
};

class PairingCommand : public CHIPCommand,
                       public chip::Controller::DevicePairingDelegate,
                       public chip::Controller::DeviceAddressUpdateDelegate,
                       public chip::Controller::DeviceDiscoveryDelegate
{
public:
    PairingCommand(const char * commandName, PairingMode mode, PairingNetworkType networkType,
                   chip::Dnssd::DiscoveryFilterType filterType = chip::Dnssd::DiscoveryFilterType::kNone) :
        CHIPCommand(commandName),
        mPairingMode(mode), mNetworkType(networkType),
        mFilterType(filterType), mRemoteAddr{ IPAddress::Any, chip::Inet::InterfaceId::Null() },
        mOnAddThreadNetworkCallback(OnAddNetworkResponse, this), mOnAddWiFiNetworkCallback(OnAddNetworkResponse, this),
        mOnEnableNetworkCallback(OnEnableNetworkResponse, this), mOnFailureCallback(OnDefaultFailureResponse, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);

        switch (networkType)
        {
        case PairingNetworkType::None:
        case PairingNetworkType::Ethernet:
            break;
        case PairingNetworkType::WiFi:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            break;
        case PairingNetworkType::Thread:
            AddArgument("operationalDataset", &mOperationalDataset);
            break;
        }

        switch (mode)
        {
        case PairingMode::None:
            break;
        case PairingMode::QRCode:
        case PairingMode::ManualCode:
            AddArgument("payload", &mOnboardingPayload);
            break;
        case PairingMode::Ble:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        case PairingMode::OnNetwork:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            break;
        case PairingMode::SoftAP:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        case PairingMode::Ethernet:
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        }

        switch (filterType)
        {
        case chip::Dnssd::DiscoveryFilterType::kNone:
            break;
        case chip::Dnssd::DiscoveryFilterType::kShort:
            AddArgument("discriminator", 0, 15, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kLong:
            AddArgument("discriminator", 0, 4096, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kVendor:
            AddArgument("vendor-id", 0, UINT16_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
            AddArgument("fabric-id", 0, UINT64_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
        case chip::Dnssd::DiscoveryFilterType::kCommissioner:
            break;
        case chip::Dnssd::DiscoveryFilterType::kDeviceType:
            AddArgument("device-type", 0, UINT16_MAX, &mDiscoveryFilterCode);
            break;
        case chip::Dnssd::DiscoveryFilterType::kInstanceName:
            AddArgument("name", &mDiscoveryFilterInstanceName);
            break;
        }
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); }

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override;

    /////////// Network Commissioning Callbacks /////////
    static void OnDefaultFailureResponse(void * context, uint8_t status);
    static void OnAddNetworkResponse(void * context, uint8_t errorCode, chip::CharSpan debugText);
    static void OnEnableNetworkResponse(void * context, uint8_t errorCode, chip::CharSpan debugText);

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR Pair(NodeId remoteId, PeerAddress address);
    CHIP_ERROR PairWithMdns(NodeId remoteId);
    CHIP_ERROR PairWithQRCode(NodeId remoteId);
    CHIP_ERROR PairWithManualCode(NodeId remoteId);
    CHIP_ERROR PairWithCode(NodeId remoteId, chip::SetupPayload payload);
    CHIP_ERROR Unpair(NodeId remoteId);

    CHIP_ERROR SetupNetwork();
    CHIP_ERROR AddNetwork(PairingNetworkType networkType);
    CHIP_ERROR AddThreadNetwork();
    CHIP_ERROR AddWiFiNetwork();
    CHIP_ERROR EnableNetwork();
    CHIP_ERROR UpdateNetworkAddress();

    chip::ByteSpan GetThreadNetworkId();

    const PairingMode mPairingMode;
    const PairingNetworkType mNetworkType;
    const chip::Dnssd::DiscoveryFilterType mFilterType;
    Command::AddressWithInterface mRemoteAddr;
    NodeId mNodeId;
    uint16_t mRemotePort;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    chip::ByteSpan mOperationalDataset;
    uint8_t mExtendedPanId[chip::Thread::kSizeExtendedPanId];
    chip::ByteSpan mSSID;
    chip::ByteSpan mPassword;
    char * mOnboardingPayload;
    uint64_t mDiscoveryFilterCode;
    char * mDiscoveryFilterInstanceName;

    chip::Callback::Callback<NetworkCommissioningClusterAddThreadNetworkResponseCallback> mOnAddThreadNetworkCallback;
    chip::Callback::Callback<NetworkCommissioningClusterAddWiFiNetworkResponseCallback> mOnAddWiFiNetworkCallback;
    chip::Callback::Callback<NetworkCommissioningClusterEnableNetworkResponseCallback> mOnEnableNetworkCallback;
    chip::Callback::Callback<DefaultFailureCallback> mOnFailureCallback;
    chip::CommissioneeDeviceProxy * mDevice;
    chip::Controller::NetworkCommissioningCluster mCluster;
    chip::EndpointId mEndpointId = 0;
};
