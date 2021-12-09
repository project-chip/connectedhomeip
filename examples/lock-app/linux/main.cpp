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

#include <app/Command.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/util/af.h>

#include "LockManager.h"

#include "AppMain.h"

/* Current proposed DoorLockServer API:
class DoorLockServer
{
    static DoorLockServer & Instance();

    void InitServer();

   bool SetLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newLockState);
    bool SetActuatorState(chip::EndpointId endpointId, bool actuatorState);
    bool SetDoorState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState doorState);

    // Also needs SetOperatingMode just to be sure
    bool SetLanguage(chip::EndpointId endpointId, const char * newLanguage);
    bool SetAutoRelockTime(chip::EndpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

    private:
    static DoorLockServer instance;
};

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const char * PINCode);
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const char * PINCode);

bool emberAfPluginDoorLockGetUsers(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockClearUser(chip::EndpointId endpointId, ...);

bool emberAfPluginDoorLockGetCredentials(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockClearCredential(chip::EndpointId endpointId, ...);
*/

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const char * PINCode)
{
    if(LockMgr().CheckPin(PINCode))
    {
        return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked);
    }

    return false;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const char * PINCode)
{
    if(LockMgr().CheckPin(PINCode))
    {
        return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked);
    }

    return false;
}

/*
bool emberAfPluginDoorLockGetUsers(chip::EndpointId endpointId, ...)
{
    // TODO: Get (how to get? send out msg somehow?) 
    return true;
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, ...)
{
    // TODO: 
    return true;
}

bool emberAfPluginDoorLockClearUser(chip::EndpointId endpointId, ...)
{
    // TODO: 
    return true;
}


bool emberAfPluginDoorLockGetCredentials(chip::EndpointId endpointId, ...)
{
    // TODO: 
    return true;
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, ...)
{
    // TODO: 
    return true;
}

bool emberAfPluginDoorLockClearCredential(chip::EndpointId endpointId, ...)
{
    // TODO: 
    return true;
}
*/

/* TODO: Don't think we need this
bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
*/

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type, uint16_t size, uint8_t * value)
{
    // TODO: Watch for LockState, DoorState, Mode, etc changes and trigger appropriate action

    if (attributePath.mClusterId == DoorLock::Id)
    {
        emberAfDoorLockClusterPrintln("Door Lock attribute changed");
    }
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    // TODO: Implement if needed
}


int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    LockMgr().Init();
    ChipLinuxAppMainLoop();
    return 0;
}
