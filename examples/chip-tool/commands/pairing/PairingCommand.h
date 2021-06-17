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

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"
#include "gen/CHIPClientCallbacks.h"
#include "gen/CHIPClusters.h"

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <support/Span.h>

enum class PairingMode
{
    None,
    Bypass,
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

class PairingCommand : public Command,
                       public chip::Controller::DevicePairingDelegate,
                       public chip::Controller::DeviceAddressUpdateDelegate
{
public:
    PairingCommand(const char * commandName, PairingMode mode, PairingNetworkType networkType) :
        Command(commandName), mPairingMode(mode), mNetworkType(networkType), mRemoteAddr{ IPAddress::Any, INET_NULL_INTERFACEID }
    {
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
        case PairingMode::Bypass:
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        case PairingMode::Ble:
            AddArgument("fabric-id", 0, UINT64_MAX, &mFabricId);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        case PairingMode::OnNetwork:
        case PairingMode::SoftAP:
            AddArgument("fabric-id", 0, UINT64_MAX, &mFabricId);
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
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run() override;
    uint16_t GetWaitDurationInSeconds() const override { return 120; }
    void Shutdown() override;

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override;

    /////////// Network Commissioning Callbacks /////////
    static void OnDefaultFailureResponse(void * context, uint8_t status);
    static void OnAddNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText);
    static void OnEnableNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText);

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR Pair(NodeId remoteId, PeerAddress address);
    CHIP_ERROR PairWithoutSecurity(NodeId remoteId, PeerAddress address);
    CHIP_ERROR Unpair(NodeId remoteId);

    void InitCallbacks();
    CHIP_ERROR SetupNetwork();
    CHIP_ERROR AddNetwork(PairingNetworkType networkType);
    CHIP_ERROR AddThreadNetwork();
    CHIP_ERROR AddWiFiNetwork();
    CHIP_ERROR EnableNetwork();
    CHIP_ERROR UpdateNetworkAddress();

    const PairingMode mPairingMode;
    const PairingNetworkType mNetworkType;
    Command::AddressWithInterface mRemoteAddr;
    NodeId mRemoteId;
    uint16_t mRemotePort;
    uint64_t mFabricId;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    chip::ByteSpan mOperationalDataset;
    chip::ByteSpan mSSID;
    chip::ByteSpan mPassword;

    chip::Callback::Callback<NetworkCommissioningClusterAddThreadNetworkResponseCallback> * mOnAddThreadNetworkCallback = nullptr;
    chip::Callback::Callback<NetworkCommissioningClusterAddWiFiNetworkResponseCallback> * mOnAddWiFiNetworkCallback     = nullptr;
    chip::Callback::Callback<NetworkCommissioningClusterEnableNetworkResponseCallback> * mOnEnableNetworkCallback       = nullptr;
    chip::Callback::Callback<DefaultFailureCallback> * mOnFailureCallback                                               = nullptr;
    ChipDevice * mDevice;
    chip::Controller::NetworkCommissioningCluster mCluster;
    chip::EndpointId mEndpointId = 0;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
};
