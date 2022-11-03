/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Matter/Matter.h>

#include "OpenCommissioningWindowCommand.h"
#include "PairingCommandBridge.h"

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
        make_unique<Unpair>(),
        make_unique<OpenCommissioningWindowCommand>(),
    };

    commands.Register(clusterName, clusterCommands);
}
