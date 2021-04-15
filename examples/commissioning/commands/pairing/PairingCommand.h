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

enum class PairingMode
{
    None,
    Bypass,
    Ble,
    SoftAP,
};

class PairingCommand : public Command, public chip::Controller::DevicePairingDelegate
{
public:
    PairingCommand(const char * commandName, PairingMode mode) :
        Command(commandName), mPairingMode(mode), mRemoteAddr{ IPAddress::Any, INET_NULL_INTERFACEID }
    {
        switch (mode)
        {
        case PairingMode::None:
            break;
        case PairingMode::Bypass:
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        case PairingMode::Ble:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            break;
        case PairingMode::SoftAP:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort);
            break;
        }
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override;

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::RendezvousSessionDelegate::Status status) override;
    void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback) override;
    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           chip::RendezvousDeviceCredentialsDelegate * callback) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR Pair(NodeId remoteId, PeerAddress address);
    CHIP_ERROR PairWithoutSecurity(NodeId remoteId, PeerAddress address);
    CHIP_ERROR Unpair(NodeId remoteId);

    const PairingMode mPairingMode;
    Command::AddressWithInterface mRemoteAddr;
    uint16_t mRemotePort;
    uint16_t mDiscriminator;
    uint32_t mSetupPINCode;
    char * mSSID;
    char * mPassword;

    ChipDeviceCommissioner mCommissioner;
};
