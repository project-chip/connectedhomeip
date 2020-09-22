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

#ifndef __CHIPTOOL_IPCOMMAND_H__
#define __CHIPTOOL_IPCOMMAND_H__

#include "Command.h"

enum NetworkType
{
    UDP,
    BLE,
    ALL
};

class NetworkCommand : public Command
{
public:
    NetworkCommand(const char * commandName, NetworkType type) : Command(commandName), mNetworkType(type)
    {
        if (type == NetworkType::UDP || type == NetworkType::ALL)
        {
            AddArgument("device-remote-ip", &mRemoteAddr);
            AddArgument("device-remote-port", 0, 65536, &mRemotePort);
        }

        if (type == NetworkType::BLE || type == NetworkType::ALL)
        {
            AddArgument("discriminator", 0, 4096, &mDiscriminator);
            AddArgument("setup-pin-code", 0, 134217727, &mSetupPINCode);
        }
    }

    const char * GetName(void) const { return mName; }

    virtual void OnConnect(ChipDeviceController * dc)                        = 0;
    virtual void OnError(ChipDeviceController * dc, CHIP_ERROR err)          = 0;
    virtual void OnMessage(ChipDeviceController * dc, PacketBuffer * buffer) = 0;

    CHIP_ERROR Run(ChipDeviceController * dc, NodeId remoteId) override;

private:
    const NetworkType mNetworkType;
    CHIP_ERROR ConnectBLE(ChipDeviceController * dc, NodeId remoteId);
    CHIP_ERROR ConnectUDP(ChipDeviceController * dc, NodeId remoteId);

    char mName[46];
    Command::AddressWithInterface mRemoteAddr;
    uint32_t mRemotePort;
    uint32_t mDiscriminator;
    uint32_t mSetupPINCode;
};

#endif //__CHIPTOOL_IPCOMMAND_H__
