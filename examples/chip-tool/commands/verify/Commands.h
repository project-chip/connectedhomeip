/*
 *   Copyright (c) 2026 Project CHIP Authors
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

#include "commands/common/Commands.h"
#include "commands/verify/VerifyCommand.h"

#include <app/server/Dnssd.h>
#include <commands/common/CredentialIssuerCommands.h>
#include <lib/dnssd/Resolver.h>

class VerifyWithPayload : public VerifyCommand
{
public:
    VerifyWithPayload(CredentialIssuerCommands * credsIssuerConfig) : VerifyCommand("with-payload", credsIssuerConfig) {}
};

void registerCommandsVerify(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Verify";

    commands_list clusterCommands = {
        make_unique<VerifyWithPayload>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for commissioning devices.");
}
