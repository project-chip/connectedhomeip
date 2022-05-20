/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include "PairingCommandBridge.h"

class PairCode : public PairingCommandBridge
{
public:
    PairCode() : PairingCommandBridge("code", PairingMode::Code, PairingNetworkType::None) {}
};

class PairWithIPAddress : public PairingCommandBridge
{
public:
    PairWithIPAddress() : PairingCommandBridge("ethernet", PairingMode::Ethernet, PairingNetworkType::Ethernet) {}
};

class PairBleWiFi : public PairingCommandBridge
{
public:
    PairBleWiFi() : PairingCommandBridge("ble-wifi", PairingMode::Ble, PairingNetworkType::WiFi) {}
};

class PairBleThread : public PairingCommandBridge
{
public:
    PairBleThread() : PairingCommandBridge("ble-thread", PairingMode::Ble, PairingNetworkType::Thread) {}
};

class Unpair : public PairingCommandBridge
{
public:
    Unpair() : PairingCommandBridge("unpair", PairingMode::None, PairingNetworkType::None) {}
};

void registerCommandsPairing(Commands & commands)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<PairCode>(), make_unique<PairWithIPAddress>(), make_unique<PairBleWiFi>(), make_unique<PairBleThread>(),
        make_unique<Unpair>(),
    };

    commands.Register(clusterName, clusterCommands);
}
