/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SmokeCoAlarmManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace ::chip::app::Clusters::SmokeCoAlarm;
using namespace ::chip::DeviceLayer;

SmokeCoAlarmManager SmokeCoAlarmManager::sAlarm;

CHIP_ERROR SmokeCoAlarmManager::Init()
{
    mExpressedState = { { ExpressedStateEnum::kEndOfService, false },
                        { ExpressedStateEnum::kSmokeAlarm, false },
                        { ExpressedStateEnum::kCOAlarm, false },
                        { ExpressedStateEnum::kTesting, false },
                        { ExpressedStateEnum::kInterconnectSmoke, false },
                        { ExpressedStateEnum::kInterconnectCO, false },
                        { ExpressedStateEnum::kHardwareFault, false },
                        { ExpressedStateEnum::kBatteryAlert, false },
                        { ExpressedStateEnum::kNormal, true } };

    // read current State on endpoint one
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    ExpressedStateEnum currentExpressedState;
    bool success = SmokeCoAlarmServer::Instance().GetExpressedState(1, currentExpressedState);
    if (success)
    {
        mExpressedState[currentExpressedState] = true;
    }

    AlarmStateEnum currentSmokeState;
    success = SmokeCoAlarmServer::Instance().GetSmokeState(1, currentSmokeState);
    if (success)
    {
        mExpressedState[ExpressedStateEnum::kSmokeAlarm] = !(currentSmokeState == AlarmStateEnum::kNormal);
    }

    AlarmStateEnum currentCOState;
    success = SmokeCoAlarmServer::Instance().GetCOState(1, currentCOState);
    if (success)
    {
        mExpressedState[ExpressedStateEnum::kCOAlarm] = !(currentCOState == AlarmStateEnum::kNormal);
    }

    AlarmStateEnum currentBatteryAlert;
    success = SmokeCoAlarmServer::Instance().GetBatteryAlert(1, currentBatteryAlert);
    if (success)
    {
        mExpressedState[ExpressedStateEnum::kBatteryAlert] = !(currentBatteryAlert == AlarmStateEnum::kNormal);
    }

    bool currentHardwareFaultAlert;
    success = SmokeCoAlarmServer::Instance().GetHardwareFaultAlert(1, currentHardwareFaultAlert);
    if (success)
    {
        mExpressedState[ExpressedStateEnum::kHardwareFault] = currentHardwareFaultAlert;
    }

    EndOfServiceEnum currentEndOfServiceAlert;
    success = SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(1, currentEndOfServiceAlert);
    if (success)
    {
        mExpressedState[ExpressedStateEnum::kEndOfService] = !(currentEndOfServiceAlert == EndOfServiceEnum::kNormal);
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

bool SmokeCoAlarmManager::StartSelfTesting()
{
    bool success = SmokeCoAlarmServer::Instance().SetTestInProgress(1, true);

    if (success)
    {
        SILABS_LOG("Start self-testing!");

        SILABS_LOG("End self-testing!");
    }

    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    SetExpressedState(1, ExpressedStateEnum::kTesting, false);

    return success;
}

bool SmokeCoAlarmManager::SetExpressedState(EndpointId endpointId, ExpressedStateEnum expressedState, bool isSet)
{
    bool success = false;

    mExpressedState[expressedState] = isSet;

    for (auto it : mExpressedState)
    {
        if (it.second)
        {
            success = SmokeCoAlarmServer::Instance().SetExpressedState(endpointId, it.first);
            break;
        }
    }

    return success;
}
