/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief APIs and defines for the Door Lock Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

class DoorLockServer
{
    static DoorLockServer & Instance();

    void InitServer(chip::EndpointId endpointId);

    bool SetLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newLockState);
    bool SetActuatorState(chip::EndpointId endpointId, bool newActuatorState);
    bool SetDoorState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newDoorState);

    bool SetLanguage(chip::EndpointId endpointId, const char * newLanguage);
    bool SetAutoRelockTime(chip::EndpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

private:
    static DoorLockServer instance;
};

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const char * PINCOde);
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const char * PINCode);
