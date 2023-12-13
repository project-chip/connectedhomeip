/*
 *   Copyright (c) 2023 Project CHIP Authors
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
#include "commands/common/Commands.h"

#include <lib/support/Span.h>

class ICDCommand : public CHIPCommand
{
public:
    ICDCommand(const char * commandName, CredentialIssuerCommands * credIssuerCmds, const char * description) :
        CHIPCommand(commandName, credIssuerCmds, description)
    {}

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }
};

class ICDListCommand : public ICDCommand
{
public:
    ICDListCommand(CredentialIssuerCommands * credIssuerCmds) :
        ICDCommand("list", credIssuerCmds, "List ICDs registed by this controller.")
    {}
    CHIP_ERROR RunCommand() override;
};

void registerCommandsICD(Commands & commands, CredentialIssuerCommands * credsIssuerConfig);
