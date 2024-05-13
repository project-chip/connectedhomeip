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

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"
#include "../common/RemoteDataModelLogger.h"
#include "commands/common/Commands.h"

#include "InteractiveCommands.h"

#include <websocket-server/WebSocketServer.h>

class Commands;

class InteractiveCommand : public CHIPCommandBridge
{
public:
    InteractiveCommand(const char * name, Commands * commandsHandler) : CHIPCommandBridge(name), mHandler(commandsHandler)
    {
        AddArgument(
            "additional-prompt", &mAdditionalPrompt,
            "Force printing of an additional prompt that can then be detected by something trying to script interactive mode");
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }

    bool ParseCommand(char * command, int * status);

protected:
    chip::Optional<char *> mAdditionalPrompt;

private:
    Commands * mHandler = nullptr;
};

class InteractiveStartCommand : public InteractiveCommand
{
public:
    InteractiveStartCommand(Commands * commandsHandler) : InteractiveCommand("start", commandsHandler) {}

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
};

class InteractiveServerCommand : public InteractiveCommand, public WebSocketServerDelegate, public RemoteDataModelLoggerDelegate
{
public:
    InteractiveServerCommand(Commands * commandsHandler) : InteractiveCommand("server", commandsHandler)
    {
        AddArgument("port", 0, UINT16_MAX, &mPort, "Port the websocket will listen to. Defaults to 9002.");
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;

    /////////// WebSocketServerDelegate Interface /////////
    bool OnWebSocketMessageReceived(char * msg) override;

    /////////// RemoteDataModelLoggerDelegate interface /////////
    CHIP_ERROR LogJSON(const char * json) override;

private:
    WebSocketServer mWebSocketServer;
    chip::Optional<uint16_t> mPort;
};
