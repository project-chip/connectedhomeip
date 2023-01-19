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

#include "../common/CHIPCommand.h"
#include "../common/Commands.h"

#include "WebSocketServer.h"

class Commands;

class InteractiveCommand : public CHIPCommand
{
public:
    InteractiveCommand(const char * name, Commands * commandsHandler, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(name, credsIssuerConfig), mHandler(commandsHandler)
    {}

    /////////// CHIPCommand Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }

    bool ParseCommand(char * command);

private:
    Commands * mHandler = nullptr;
};

class InteractiveStartCommand : public InteractiveCommand
{
public:
    InteractiveStartCommand(Commands * commandsHandler, CredentialIssuerCommands * credsIssuerConfig) :
        InteractiveCommand("start", commandsHandler, credsIssuerConfig)
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
};

class InteractiveServerCommand : public InteractiveCommand, public WebSocketServerDelegate
{
public:
    InteractiveServerCommand(Commands * commandsHandler, CredentialIssuerCommands * credsIssuerConfig) :
        InteractiveCommand("server", commandsHandler, credsIssuerConfig)
    {
        AddArgument("port", 0, UINT16_MAX, &mPort, "Port the websocket will listen to. Defaults to 9002.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    /////////// WebSocketServerDelegate Interface /////////
    bool OnWebSocketMessageReceived(char * msg) override;

private:
    WebSocketServer mWebSocketServer;
    chip::Optional<uint16_t> mPort;
};
