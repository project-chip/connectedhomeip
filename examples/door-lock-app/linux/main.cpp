/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/util/af.h>

#include "LockManager.h"

#include "AppMain.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;

// App handles physical aspects of locking but not locking logic. That is it
// should wait for door to be locked on lock command and return success) but
// door lock server should check pin before even calling the lock-door
// callback.
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode)
{
    return LockManager::Instance().Lock(pinCode);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, chip::Optional<chip::ByteSpan> pinCode)
{
    return LockManager::Instance().Unlock(pinCode);
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                       uint16_t size, uint8_t * value)
{
    // TODO: Watch for LockState, DoorState, Mode, etc changes and trigger appropriate action
    if (attributePath.mClusterId == DoorLock::Id)
    {
        emberAfDoorLockClusterPrintln("Door Lock attribute changed");
    }
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    DoorLockServer::Instance().InitServer(endpoint);
}

void ApplicationInit() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
