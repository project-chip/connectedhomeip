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

#include "CommissionedListCommand.h"
#include "OpenCommissioningWindowCommand.h"
#include "PairingCommand.h"

#include <app/server/Dnssd.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <lib/dnssd/Resolver.h>

class Unpair : public PairingCommand
{
public:
    Unpair(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("unpair", PairingMode::None, PairingNetworkType::None, credIssuerCommands)
    {}
};

class PairQRCode : public PairingCommand
{
public:
    PairQRCode(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("qrcode", PairingMode::QRCode, PairingNetworkType::None, credIssuerCommands)
    {}
};

class PairManualCode : public PairingCommand
{
public:
    PairManualCode(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("manualcode", PairingMode::ManualCode, PairingNetworkType::None, credIssuerCommands)
    {}
};

class PairOnNetwork : public PairingCommand
{
public:
    PairOnNetwork(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands)
    {}
};

class PairOnNetworkShort : public PairingCommand
{
public:
    PairOnNetworkShort(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-short", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kShort)
    {}
};

class PairOnNetworkLong : public PairingCommand
{
public:
    PairOnNetworkLong(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-long", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kLong)
    {}
};

class PairOnNetworkVendor : public PairingCommand
{
public:
    PairOnNetworkVendor(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-vendor", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kVendor)
    {}
};

class PairOnNetworkFabric : public PairingCommand
{
public:
    PairOnNetworkFabric(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-fabric", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kCompressedFabricId)
    {}
};

class PairOnNetworkCommissioningMode : public PairingCommand
{
public:
    PairOnNetworkCommissioningMode(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-commissioning-mode", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kCommissioningMode)
    {}
};

class PairOnNetworkCommissioner : public PairingCommand
{
public:
    PairOnNetworkCommissioner(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-commissioner", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kCommissioner)
    {}
};

class PairOnNetworkDeviceType : public PairingCommand
{
public:
    PairOnNetworkDeviceType(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-device-type", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kDeviceType)
    {}
};

class PairOnNetworkInstanceName : public PairingCommand
{
public:
    PairOnNetworkInstanceName(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("onnetwork-instance-name", PairingMode::OnNetwork, PairingNetworkType::None, credIssuerCommands,
                       chip::Dnssd::DiscoveryFilterType::kInstanceName)
    {}
};

class PairBleWiFi : public PairingCommand
{
public:
    PairBleWiFi(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("ble-wifi", PairingMode::Ble, PairingNetworkType::WiFi, credIssuerCommands)
    {}
};

class PairBleThread : public PairingCommand
{
public:
    PairBleThread(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("ble-thread", PairingMode::Ble, PairingNetworkType::Thread, credIssuerCommands)
    {}
};

class PairSoftAP : public PairingCommand
{
public:
    PairSoftAP(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("softap", PairingMode::SoftAP, PairingNetworkType::WiFi, credIssuerCommands)
    {}
};

class Ethernet : public PairingCommand
{
public:
    Ethernet(CredentialIssuerCommands * credIssuerCommands) :
        PairingCommand("ethernet", PairingMode::Ethernet, PairingNetworkType::Ethernet, credIssuerCommands)
    {}
};

class StartUdcServerCommand : public CHIPCommand
{
public:
    StartUdcServerCommand() : CHIPCommand("start-udc-server") {}
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(300); }

    CHIP_ERROR RunCommand() override
    {
        chip::app::DnssdServer::Instance().StartServer(chip::Dnssd::CommissioningMode::kDisabled);
        return CHIP_NO_ERROR;
    }
};

void registerCommandsPairing(Commands & commands, CredentialIssuerCommands * credIssuerCommands)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<Unpair>(credIssuerCommands),
        make_unique<PairQRCode>(credIssuerCommands),
        make_unique<PairManualCode>(credIssuerCommands),
        make_unique<PairBleWiFi>(credIssuerCommands),
        make_unique<PairBleThread>(credIssuerCommands),
        make_unique<PairSoftAP>(credIssuerCommands),
        make_unique<Ethernet>(credIssuerCommands),
        make_unique<PairOnNetwork>(credIssuerCommands),
        make_unique<PairOnNetworkShort>(credIssuerCommands),
        make_unique<PairOnNetworkLong>(credIssuerCommands),
        make_unique<PairOnNetworkVendor>(credIssuerCommands),
        make_unique<PairOnNetworkCommissioningMode>(credIssuerCommands),
        make_unique<PairOnNetworkCommissioner>(credIssuerCommands),
        make_unique<PairOnNetworkDeviceType>(credIssuerCommands),
        make_unique<PairOnNetworkDeviceType>(credIssuerCommands),
        make_unique<PairOnNetworkInstanceName>(credIssuerCommands),
        // TODO - enable CommissionedListCommand once DNS Cache is implemented
        //        make_unique<CommissionedListCommand>(),
        make_unique<StartUdcServerCommand>(),
        make_unique<OpenCommissioningWindowCommand>(credIssuerCommands),
    };

    commands.Register(clusterName, clusterCommands);
}
