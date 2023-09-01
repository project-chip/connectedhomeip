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
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace ::chip::app::Clusters::SmokeCoAlarm;
using namespace ::chip::DeviceLayer;

LOG_MODULE_DECLARE(COsensor, CONFIG_CHIP_APP_LOG_LEVEL);

SmokeCoAlarmManager SmokeCoAlarmManager::sAlarm;

static std::array<ExpressedStateEnum, SmokeCoAlarmServer::kPriorityOrderLength> sPriorityOrder = {
    ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kCOAlarm,
    ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kHardwareFault,     ExpressedStateEnum::kTesting,
    ExpressedStateEnum::kEndOfService,   ExpressedStateEnum::kBatteryAlert
};

CHIP_ERROR SmokeCoAlarmManager::Init()
{
    return CHIP_NO_ERROR;
}

void SmokeCoAlarmManager::StartSelfTesting()
{
    LOG_INF("Start self-testing!");

    // It will take some time here

    SmokeCoAlarmServer::Instance().SetTestInProgress(1, false);
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(1, sPriorityOrder);
    LOG_INF("End self-testing!");
}
