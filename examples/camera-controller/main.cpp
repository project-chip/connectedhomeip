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

#include <commands/clusters/SubscriptionsCommands.h>
#include <commands/common/Commands.h>
#include <commands/interactive/Commands.h>
#include <commands/pairing/Commands.h>
#include <zap-generated/cluster/Commands.h>

#include <iostream>
#include <string>
#include <vector>

using namespace chip;

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    // Convert command line arguments to a vector of strings for easier manipulation
    std::vector<std::string> args(argv, argv + argc);

    // Check if "interactive" and "start" are not in the arguments
    if (args.size() < 3 || args[1] != "interactive" || args[2] != "start")
    {
        // Insert "interactive" and "start" after the executable name
        args.insert(args.begin() + 1, "interactive");
        args.insert(args.begin() + 2, "start");
    }

    ExampleCredentialIssuerCommands credIssuerCommands;
    Commands & commands = CommandMgr();

    registerCommandsInteractive(commands, &credIssuerCommands);
    registerCommandsPairing(commands, &credIssuerCommands);
    registerClusters(commands, &credIssuerCommands);
    registerCommandsSubscriptions(commands, &credIssuerCommands);

    std::vector<char *> c_args;
    for (auto & arg : args)
    {
        c_args.push_back(const_cast<char *>(arg.c_str()));
    }

    return commands.Run(static_cast<int>(c_args.size()), c_args.data());
}
