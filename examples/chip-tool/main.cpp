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

#include "commands/clusters/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"

#include <transport/SecurePairingSession.h>

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    Commands commands;
    registerCommandsPayload(commands);
    registerCommandsPairing(commands);
    registerClusters(commands);

    return commands.Run(chip::kTestControllerNodeId, chip::kTestDeviceNodeId, argc, argv);
}
