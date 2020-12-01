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

#include "PairingCommand.h"

class Unpair : public PairingCommand
{
public:
    Unpair() : PairingCommand("unpair", PairingMode::None) {}
};

class PairBypass : public PairingCommand
{
public:
    PairBypass() : PairingCommand("bypass", PairingMode::Bypass) {}
};

class PairBle : public PairingCommand
{
public:
    PairBle() : PairingCommand("ble", PairingMode::Ble) {}
};

class PairSoftAP : public PairingCommand
{
public:
    PairSoftAP() : PairingCommand("softap", PairingMode::SoftAP) {}
};

void registerCommandsPairing(Commands & commands)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<Unpair>(),
        make_unique<PairBypass>(),
        make_unique<PairBle>(),
        make_unique<PairSoftAP>(),
    };

    commands.Register(clusterName, clusterCommands);
}
