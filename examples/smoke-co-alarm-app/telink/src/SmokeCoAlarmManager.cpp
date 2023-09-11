/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
