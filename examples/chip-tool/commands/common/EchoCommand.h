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

#include "NetworkCommand.h"

class EchoCommand : public NetworkCommand
{
public:
    EchoCommand(const char * name, NetworkType type) : NetworkCommand(name, type) {}

    /////////// Command Interface /////////
    CHIP_ERROR Run(ChipDeviceController * dc, NodeId remoteId) override;

    /////////// IPCommand Interface /////////
    void OnConnect(ChipDeviceController * dc) override { mController = dc; }
    void OnMessage(ChipDeviceController * dc, chip::System::PacketBufferHandle buffer) override { ReceiveEcho(std::move(buffer)); }
    void OnError(ChipDeviceController * dc, CHIP_ERROR err) override { mController = nullptr; }

private:
    void SendEcho(void) const;
    void ReceiveEcho(chip::System::PacketBufferHandle buffer) const;

    ChipDeviceController * mController = nullptr;
};
