/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AppTask.h"
#include <app/server/Server.h>

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::app::Clusters::AirQuality;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(AirQualityActionEventHandler);
#endif
    InitCommonParts();

   CHIP_ERROR err = ConnectivityMgr().SetBLEDeviceName("TelinkAirQuality");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::UpdateClusterState(void)
{
    EndpointId endpoint  = 1;
    AirQualityState = AirQualityEnum::kGood;
    EmberAfStatus status = Clusters::AirQuality::Attributes::AirQuality::Set(endpoint, AirQualityState);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set AirQuality attribute"));
}

void AppTask::SelfTestHandler(AppEvent * aEvent)
{
    sAppTask.UpdateClusterState();
}

void AppTask::AirQualityActionEventHandler(AppEvent * aEvent)
{
    AppEvent event;
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        event.ButtonEvent.Action = kButtonPushEvent;
        event.Handler            = SelfTestHandler;
        GetAppTask().PostEvent(&event);
    }
}
