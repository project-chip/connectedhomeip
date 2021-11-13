/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Credentials;

bool lowPowerClusterSleep() { return true; }


static chip::Shell::Engine sShellServerSubcommands;

CHIP_ERROR CmdAppServerHelp(int argc, char ** argv)
{
    sShellServerSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerStart(int argc, char ** argv)
{
    uint16_t securePort   = CHIP_PORT;
    uint16_t unsecurePort = CHIP_UDC_PORT;
/*
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    // use a different service port to make testing possible with other sample devices running on same host
    securePort   = LinuxDeviceOptions::GetInstance().securedDevicePort;
    unsecurePort = LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
*/
    // Init ZCL Data Model and CHIP App Server
    chip::Server::GetInstance().Init(nullptr, securePort, unsecurePort);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CmdAppServerStop(int argc, char ** argv)
{
    chip::Server::GetInstance().Shutdown();
    return CHIP_NO_ERROR;
}


static CHIP_ERROR CmdAppServer(int argc, char ** argv)
{
    switch (argc)
    {
    case 0:
        return CmdAppServerHelp(argc, argv);
    case 1:
        if ((strcmp(argv[0], "help") == 0) || 
            (strcmp(argv[0], "-h") == 0))
        {
            return CmdAppServerHelp(argc, argv);
        }
    }
    return sShellServerSubcommands.ExecCommand(argc, argv);
}

void cmd_app_server_init()
{

    static const shell_command_t sServerComand = { &CmdAppServer, "server",
                                                   "Manage the ZCL application server. Usage: server [help|start|stop]"};

    static const shell_command_t sServerSubCommands[] = {
        { &CmdAppServerHelp, "help", "Usage: server <subcommand>" },
        { &CmdAppServerStart, "start", "Start the ZCL application server." },
        { &CmdAppServerStop, "stop", "Stop the ZCL application server." },
    };

    // Register `config` subcommands with the local shell dispatcher.
    sShellServerSubcommands.RegisterCommands(sServerSubCommands, ArraySize(sServerSubCommands));

    // Register the root `config` command with the top-level shell.
    Engine::Root().RegisterCommands(&sServerComand, 1);
    return;
}
