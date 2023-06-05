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
    ExpressedStateEnum currentExpressedState = ExpressedStateEnum::kNormal;
    // read current ExpressedState on endpoint one
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    SmokeCoAlarmServer::Instance().GetExpressedState(1, currentExpressedState);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mExpressedState = currentExpressedState;

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

    SmokeCoAlarmServer::Instance().SetExpressedState(1, mExpressedState);
    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);

    return success;
}
