/*
 *   Copyright (c) 2022-2023 Project CHIP Authors
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

#import <Matter/Matter.h>

#include "GetCommissionerNodeIdCommand.h"
#include "OpenCommissioningWindowCommand.h"
#include "PairingCommandBridge.h"
#include "PreWarmCommissioningCommand.h"

class PairCode : public PairingCommandBridge
{
public:
    PairCode() : PairingCommandBridge("code", PairingMode::Code, CommissioningType::WithoutNetwork) {}
};

class PairCodePASEOnly : public PairingCommandBridge
{
public:
    PairCodePASEOnly() : PairingCommandBridge("code-paseonly", PairingMode::Code, CommissioningType::None) {}
};

class PairCodeWifi : public PairingCommandBridge
{
public:
    PairCodeWifi() : PairingCommandBridge("code-wifi", PairingMode::Code, CommissioningType::WithWiFi) {}
};

class PairCodeThread : public PairingCommandBridge
{
public:
    PairCodeThread() : PairingCommandBridge("code-thread", PairingMode::Code, CommissioningType::WithThread) {}
};

class PairBleWiFi : public PairingCommandBridge
{
public:
    PairBleWiFi() : PairingCommandBridge("ble-wifi", PairingMode::Ble, CommissioningType::WithWiFi) {}
};

class PairBleThread : public PairingCommandBridge
{
public:
    PairBleThread() : PairingCommandBridge("ble-thread", PairingMode::Ble, CommissioningType::WithThread) {}
};

class PairAlreadyDiscoveredByIndex : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndex() :
        PairingCommandBridge("by-index", PairingMode::AlreadyDiscoveredByIndex, CommissioningType::WithoutNetwork)
    {}
};

class PairAlreadyDiscoveredByIndexPASEOnly : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndexPASEOnly() :
        PairingCommandBridge("by-index-paseonly", PairingMode::AlreadyDiscoveredByIndex, CommissioningType::None)
    {}
};

class PairAlreadyDiscoveredByIndexWithWiFi : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndexWithWiFi() :
        PairingCommandBridge("by-index-with-wifi", PairingMode::AlreadyDiscoveredByIndex, CommissioningType::WithWiFi)
    {}
};

class PairAlreadyDiscoveredByIndexWithThread : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndexWithThread() :
        PairingCommandBridge("by-index-with-thread", PairingMode::AlreadyDiscoveredByIndex, CommissioningType::WithThread)
    {}
};

class Unpair : public PairingCommandBridge
{
public:
    Unpair() : PairingCommandBridge("unpair", PairingMode::Unpair, CommissioningType::None) {}
};

void registerCommandsPairing(Commands & commands)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<PairCode>(),
        make_unique<PairCodePASEOnly>(),
        make_unique<PairCodeWifi>(),
        make_unique<PairCodeThread>(),
        make_unique<PairBleWiFi>(),
        make_unique<PairBleThread>(),
        make_unique<PairAlreadyDiscoveredByIndex>(),
        make_unique<PairAlreadyDiscoveredByIndexPASEOnly>(),
        make_unique<Unpair>(),
        make_unique<OpenCommissioningWindowCommand>(),
        make_unique<PreWarmCommissioningCommand>(),
        make_unique<GetCommissionerNodeIdCommand>(),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for commissioning devices.");
}
