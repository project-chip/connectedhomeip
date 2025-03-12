/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include <commands/clusters/DataModelLogger.h>
#include <commands/common/CHIPCommand.h>
#include <commands/common/Commands.h>

#include <string>

class Commands;

class InteractiveCommand : public CHIPCommand
{
public:
    InteractiveCommand(const char * name, Commands * commandsHandler, const char * helpText,
                       CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(name, credsIssuerConfig, helpText),
        mHandler(commandsHandler)
    {
        AddArgument("advertise-operational", 0, 1, &mAdvertiseOperational,
                    "Advertise operational node over DNS-SD and accept incoming CASE sessions.");
    }

    /////////// CHIPCommand Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }
    bool NeedsOperationalAdvertising() override;

    bool ParseCommand(char * command, int * status);

private:
    Commands * mHandler = nullptr;
    chip::Optional<bool> mAdvertiseOperational;
};

class InteractiveStartCommand : public InteractiveCommand
{
public:
    InteractiveStartCommand(Commands * commandsHandler, CredentialIssuerCommands * credsIssuerConfig) :
        InteractiveCommand("start", commandsHandler, "Start an interactive shell that can then run other commands.",
                           credsIssuerConfig)
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    char * GetCommand(char * command);
    std::string GetHistoryFilePath() const;
};

void PushCommand(const std::string & command);
