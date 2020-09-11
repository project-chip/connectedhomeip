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

#ifndef __CHIPTOOL_COMMAND_ECHO_H__
#define __CHIPTOOL_COMMAND_ECHO_H__

#include "NetworkCommand.h"

class EchoCommand : public NetworkCommand
{
public:
    EchoCommand(const char * name, NetworkType type) : NetworkCommand(name, type) {}

    /////////// IPCommand Interface /////////
    void OnConnect(ChipDeviceController * dc) override { SendEcho(dc); }
    void OnMessage(ChipDeviceController * dc, PacketBuffer * buffer) override { ReceiveEcho(dc, buffer); }
    void OnError(ChipDeviceController * dc, CHIP_ERROR err) override {}

private:
    void SendEcho(ChipDeviceController * dc);
    void ReceiveEcho(ChipDeviceController * dc, PacketBuffer * buffer);
};

#endif // __CHIPTOOL_COMMAND_ECHO_H__
