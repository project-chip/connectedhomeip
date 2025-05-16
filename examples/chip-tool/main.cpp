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

#include "commands/common/Commands.h"
#include "commands/example/ExampleCredentialIssuerCommands.h"

#include "commands/clusters/SubscriptionsCommands.h"
#include "commands/dcl/Commands.h"
#include "commands/delay/Commands.h"
#include "commands/discover/Commands.h"
#include "commands/group/Commands.h"
#include "commands/icd/ICDCommand.h"
#include "commands/interactive/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"
#include "commands/session-management/Commands.h"
#include "commands/storage/Commands.h"

#include <zap-generated/cluster/Commands.h>

#include "platform/PlatformManager.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
using namespace chip::WiFiPAF;
#endif

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
static void StopSignalHandler(int signum)
{
    WiFiPAFLayer::GetWiFiPAFLayer().Shutdown([](uint32_t id, WiFiPafRole role) {
        switch (role)
        {
        case WiFiPafRole::kWiFiPafRole_Publisher:
            chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelPublish(id);
            break;
        case WiFiPafRole::kWiFiPafRole_Subscriber:
            chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(id);
            break;
        }
    });
}
#endif

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    struct sigaction sa = {};
    sa.sa_handler       = StopSignalHandler;
    sa.sa_flags         = static_cast<int>(SA_RESETHAND);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    ExampleCredentialIssuerCommands credIssuerCommands;
    Commands commands;
    registerCommandsDCL(commands);
    registerCommandsDelay(commands, &credIssuerCommands);
    registerCommandsDiscover(commands, &credIssuerCommands);
    registerCommandsICD(commands, &credIssuerCommands);
    registerCommandsInteractive(commands, &credIssuerCommands);
    registerCommandsPayload(commands);
    registerCommandsPairing(commands, &credIssuerCommands);
    registerCommandsGroup(commands, &credIssuerCommands);
    registerClusters(commands, &credIssuerCommands);
    registerCommandsSubscriptions(commands, &credIssuerCommands);
    registerCommandsStorage(commands);
    registerCommandsSessionManagement(commands, &credIssuerCommands);

    return commands.Run(argc, argv);
}
