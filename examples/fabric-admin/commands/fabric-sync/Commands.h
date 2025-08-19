/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include <commands/common/Commands.h>
#include <commands/fabric-sync/FabricSyncCommand.h>

void registerCommandsFabricSync(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "FabricSync";

    commands_list clusterCommands = {
        make_unique<admin::FabricSyncAddBridgeCommand>(credsIssuerConfig),
        make_unique<admin::FabricSyncRemoveBridgeCommand>(credsIssuerConfig),
        make_unique<admin::FabricSyncAddLocalBridgeCommand>(credsIssuerConfig),
        make_unique<admin::FabricSyncRemoveLocalBridgeCommand>(credsIssuerConfig),
        make_unique<admin::FabricSyncDeviceCommand>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for fabric synchronization.");
}
