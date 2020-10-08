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

#include "commands/clusters/Basic/Commands.h"
#include "commands/clusters/ColorControl/Commands.h"
#include "commands/clusters/DoorLock/Commands.h"
#include "commands/clusters/Identify/Commands.h"
#include "commands/clusters/OnOff/Commands.h"
#include "commands/clusters/TemperatureMeasurement/Commands.h"
#include "commands/echo/Commands.h"

// NOTE: Remote device ID is in sync with the echo server device id
//       At some point, we may want to add an option to connect to a device without
//       knowing its id, because the ID can be learned on the first response that is received.
constexpr chip::NodeId kLocalDeviceId  = 112233;
constexpr chip::NodeId kRemoteDeviceId = 12344321;

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    Commands commands;

    registerCommandsEcho(commands);
    registerClusterBasic(commands);
    registerClusterColorControl(commands);
    registerClusterDoorLock(commands);
    registerClusterIdentify(commands);
    registerClusterOnOff(commands);
    registerClusterTemperatureMeasurement(commands);

    return commands.Run(kLocalDeviceId, kRemoteDeviceId, argc, argv);
}
