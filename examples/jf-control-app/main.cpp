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
#include "commands/pairing/OpenJointCommissioningWindowCommand.h"

#include "RpcClientProcessor.h"
#include "RpcConnection.h"

#include <zap-generated/cluster/Commands.h>

namespace {

/* RPC connection parameters.
 *
 * The RPC server is hosted by the jf-admin-app, which is the Anchor
 * Administrator this application commissions. The jf-admin-app may run on a
 * separate host from this control application, so its address is not assumed.
 * Rather than hardcoding the server IPv6 address, it is discovered via mDNS
 * after the Anchor Administrator has been commissioned (the node id is then
 * known).
 *
 * The address may still be provided manually through the --rpc-server-ip
 * argument, in which case the RPC connection is established at startup and the
 * post-commissioning discovery is skipped. The RPC port is a fixed pigweed
 * listen port and is not discoverable over Matter, so it keeps a default value
 * and can only be overridden through --rpc-server-port.
 */
std::string gRpcServerIp;
uint16_t gRpcServerPort   = 33000;
bool gRpcServerIpProvided = false;

} // namespace

bool RpcServerAddressProvidedManually()
{
    return gRpcServerIpProvided;
}

uint16_t RpcServerPort()
{
    return gRpcServerPort;
}

CHIP_ERROR RpcConnect(const char * serverIp, uint16_t serverPort)
{
    chip::rpc::client::SetRpcServerAddress(serverIp);
    chip::rpc::client::SetRpcServerPort(serverPort);
    return chip::rpc::client::StartPacketProcessing();
}

void registerCommandsJCM(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Pairing";

    commands_list clusterCommands = {
        make_unique<OpenJointCommissioningWindowCommand>(credsIssuerConfig),
    };

    commands.UpdateCommandSet(clusterName, clusterCommands);
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
            gRpcServerIp         = args[i + 1];
            gRpcServerIpProvided = true;
            ++i;
        }
        else if (args[i] == "--rpc-server-port" && ((i + 1) < args.size()))
        {
            gRpcServerPort = static_cast<uint16_t>(atoi(args[i + 1].c_str()));
            ++i;
        }
        else
        {
            // do not propagate the RPC details to the interactive engine
            c_args.push_back(const_cast<char *>(args[i].c_str()));
        }
    }

    /* If the RPC server IP was provided manually, connect to the jf-admin-app RPC server now.
     * Otherwise, the connection is deferred: the address is discovered via mDNS once the
     * Anchor Administrator has been commissioned and its node id is known.
     */
    if (gRpcServerIpProvided)
    {
        if (RpcConnect(gRpcServerIp.c_str(), gRpcServerPort) != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "RPC: Unable to connect to the jf-admin-app@%s:%d", gRpcServerIp.c_str(), gRpcServerPort);
            ChipLogError(JointFabric,
                         "RPC: Try specifying a different IP Address/Port using --rpc-server-ip/rpc-server-port arguments!");
            return -1;
        }
    }

    ExampleCredentialIssuerCommands credIssuerCommands;
    Commands commands;
    registerCommandsICD(commands, &credIssuerCommands);
    registerCommandsInteractive(commands, &credIssuerCommands);
    registerCommandsPairing(commands, &credIssuerCommands);
    registerClusters(commands, &credIssuerCommands);
    registerCommandsSubscriptions(commands, &credIssuerCommands);
    registerCommandsJCM(commands, &credIssuerCommands);

    return commands.Run(static_cast<int>(c_args.size()), c_args.data());
}
