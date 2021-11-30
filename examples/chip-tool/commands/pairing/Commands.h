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
#include <lib/dnssd/Resolver.h>

class Unpair : public PairingCommand
{
public:
    Unpair() : PairingCommand("unpair", PairingMode::None, PairingNetworkType::None) {}
};

class PairQRCode : public PairingCommand
{
public:
    PairQRCode() : PairingCommand("qrcode", PairingMode::QRCode, PairingNetworkType::None) {}
};

class PairManualCode : public PairingCommand
{
public:
    PairManualCode() : PairingCommand("manualcode", PairingMode::ManualCode, PairingNetworkType::None) {}
};

class PairOnNetwork : public PairingCommand
{
public:
    PairOnNetwork() : PairingCommand("onnetwork", PairingMode::OnNetwork, PairingNetworkType::None) {}
};

class PairOnNetworkShort : public PairingCommand
{
public:
    PairOnNetworkShort() :
        PairingCommand("onnetwork-short", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kShort)
    {}
};

class PairOnNetworkLong : public PairingCommand
{
public:
    PairOnNetworkLong() :
        PairingCommand("onnetwork-long", PairingMode::OnNetwork, PairingNetworkType::None, chip::Dnssd::DiscoveryFilterType::kLong)
    {}
};

class PairOnNetworkVendor : public PairingCommand
{
public:
    PairOnNetworkVendor() :
        PairingCommand("onnetwork-vendor", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kVendor)
    {}
};

class PairOnNetworkFabric : public PairingCommand
{
public:
    PairOnNetworkFabric() :
        PairingCommand("onnetwork-fabric", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kCompressedFabricId)
    {}
};

class PairOnNetworkCommissioningMode : public PairingCommand
{
public:
    PairOnNetworkCommissioningMode() :
        PairingCommand("onnetwork-commissioning-mode", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kCommissioningMode)
    {}
};

class PairOnNetworkCommissioner : public PairingCommand
{
public:
    PairOnNetworkCommissioner() :
        PairingCommand("onnetwork-commissioner", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kCommissioner)
    {}
};

class PairOnNetworkDeviceType : public PairingCommand
{
public:
    PairOnNetworkDeviceType() :
        PairingCommand("onnetwork-device-type", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kDeviceType)
    {}
};

class PairOnNetworkInstanceName : public PairingCommand
{
public:
    PairOnNetworkInstanceName() :
        PairingCommand("onnetwork-instance-name", PairingMode::OnNetwork, PairingNetworkType::None,
                       chip::Dnssd::DiscoveryFilterType::kInstanceName)
    {}
};

class PairBleWiFi : public PairingCommand
{
public:
    PairBleWiFi() : PairingCommand("ble-wifi", PairingMode::Ble, PairingNetworkType::WiFi) {}
};

class PairBleThread : public PairingCommand
{
public:
    PairBleThread() : PairingCommand("ble-thread", PairingMode::Ble, PairingNetworkType::Thread) {}
};

class PairSoftAP : public PairingCommand
{
public:
    PairSoftAP() : PairingCommand("softap", PairingMode::SoftAP, PairingNetworkType::WiFi) {}
};

class Ethernet : public PairingCommand
{
public:
    Ethernet() : PairingCommand("ethernet", PairingMode::Ethernet, PairingNetworkType::Ethernet) {}
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

void registerCommandsPairing(Commands & commands)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<Unpair>(),
        make_unique<PairQRCode>(),
        make_unique<PairManualCode>(),
        make_unique<PairBleWiFi>(),
        make_unique<PairBleThread>(),
        make_unique<PairSoftAP>(),
        make_unique<Ethernet>(),
        make_unique<PairOnNetwork>(),
        make_unique<PairOnNetworkShort>(),
        make_unique<PairOnNetworkLong>(),
        make_unique<PairOnNetworkVendor>(),
        make_unique<PairOnNetworkCommissioningMode>(),
        make_unique<PairOnNetworkCommissioner>(),
        make_unique<PairOnNetworkDeviceType>(),
        make_unique<PairOnNetworkDeviceType>(),
        make_unique<PairOnNetworkInstanceName>(),
        // TODO - enable CommissionedListCommand once DNS Cache is implemented
        //        make_unique<CommissionedListCommand>(),
        make_unique<StartUdcServerCommand>(),
        make_unique<OpenCommissioningWindowCommand>(),
    };

    commands.Register(clusterName, clusterCommands);
}
