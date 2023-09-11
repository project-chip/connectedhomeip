/*
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    PairCode() : PairingCommandBridge("code", PairingMode::Code, PairingNetworkType::None) {}
};

class PairCodeWifi : public PairingCommandBridge
{
public:
    PairCodeWifi() : PairingCommandBridge("code-wifi", PairingMode::Code, PairingNetworkType::WiFi) {}
};

class PairCodeThread : public PairingCommandBridge
{
public:
    PairCodeThread() : PairingCommandBridge("code-thread", PairingMode::Code, PairingNetworkType::Thread) {}
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

class PairAlreadyDiscoveredByIndex : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndex() :
        PairingCommandBridge("by-index", PairingMode::AlreadyDiscoveredByIndex, PairingNetworkType::None)
    {}
};

class PairAlreadyDiscoveredByIndexWithWiFi : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndexWithWiFi() :
        PairingCommandBridge("by-index-with-wifi", PairingMode::AlreadyDiscoveredByIndex, PairingNetworkType::WiFi)
    {}
};

class PairAlreadyDiscoveredByIndexWithThread : public PairingCommandBridge
{
public:
    PairAlreadyDiscoveredByIndexWithThread() :
        PairingCommandBridge("by-index-with-thread", PairingMode::AlreadyDiscoveredByIndex, PairingNetworkType::Thread)
    {}
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
        make_unique<PairCode>(),
        make_unique<PairCodeWifi>(),
        make_unique<PairCodeThread>(),
        make_unique<PairBleWiFi>(),
        make_unique<PairBleThread>(),
        make_unique<PairAlreadyDiscoveredByIndex>(),
        make_unique<Unpair>(),
        make_unique<OpenCommissioningWindowCommand>(),
        make_unique<PreWarmCommissioningCommand>(),
        make_unique<GetCommissionerNodeIdCommand>(),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for commissioning devices.");
}
