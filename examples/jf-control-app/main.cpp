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

#include "commands/common/Commands.h"
#include "commands/example/ExampleCredentialIssuerCommands.h"

#include <iostream>
#include <string>
#include <vector>

#include "commands/clusters/SubscriptionsCommands.h"
#include "commands/icd/ICDCommand.h"
#include "commands/interactive/Commands.h"
#include "commands/pairing/Commands.h"

#include "RpcClientProcessor.h"

#include <zap-generated/cluster/Commands.h>

/* RPC params can also be changed through command line arguments
 * see --rpc-server-ip/--rpc-server-port arguments
 */
static std::string rpcServerIp = "127.0.0.1";
static uint16_t rpcServerPort  = 33000;
CHIP_ERROR RpcConnect();

CHIP_ERROR RpcConnect(void)
{
    chip::rpc::client::SetRpcServerAddress(rpcServerIp.c_str());
    chip::rpc::client::SetRpcServerPort(rpcServerPort);
    return chip::rpc::client::StartPacketProcessing();
}

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    // Convert command line arguments to a vector of strings for easier manipulation
    std::vector<std::string> args(argv, argv + argc);
    std::vector<char *> c_args;

    // Check if "interactive" and "start" are not in the arguments
    if (args.size() < 3 || args[1] != "interactive" || args[2] != "start")
    {
        // Insert "interactive" and "start" after the executable name
        args.insert(args.begin() + 1, "interactive");
        args.insert(args.begin() + 2, "start");
    }

    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == "--rpc-server-ip" && ((i + 1) < args.size()))
        {
            rpcServerIp = args[i + 1];
            ++i;
        }
        else if (args[i] == "--rpc-server-port" && ((i + 1) < args.size()))
        {
            rpcServerPort = static_cast<uint16_t>(atoi(args[i + 1].c_str()));
            ++i;
        }
        else
        {
            // do not propagate the RPC details to the interactive engine
            c_args.push_back(const_cast<char *>(args[i].c_str()));
        }
    }

    /* connect to the jf-admin-app RPC server */
    if (RpcConnect() != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "RPC: Unable to connect to the jf-admin-app@%s:%d", rpcServerIp.c_str(), rpcServerPort);
        ChipLogError(JointFabric,
                     "RPC: Try specifying a different IP Address/Port using --rpc-server-ip/rpc-server-port arguments!");
        return -1;
    }

    ExampleCredentialIssuerCommands credIssuerCommands;
    Commands commands;
    registerCommandsICD(commands, &credIssuerCommands);
    registerCommandsInteractive(commands, &credIssuerCommands);
    registerCommandsPairing(commands, &credIssuerCommands);
    registerClusters(commands, &credIssuerCommands);
    registerCommandsSubscriptions(commands, &credIssuerCommands);

    return commands.Run(static_cast<int>(c_args.size()), c_args.data());
}
