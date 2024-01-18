/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "SensorManagerCommon.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
k_timer sTemperatureMeasurementUpdateTimer;
constexpr uint16_t kTemperatureMeasurementUpdateTimerPeriodMs = 5000; // 5s timer period
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    CHIP_ERROR err;

    InitCommonParts();

    err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Init of the Sensor Manager failed");
        return err;
    }

    // Initialize temperature measurement update timer
    k_timer_init(&sTemperatureMeasurementUpdateTimer, &AppTask::TemperatureMeasurementUpdateTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sTemperatureMeasurementUpdateTimer, this);
    k_timer_start(&sTemperatureMeasurementUpdateTimer, K_MSEC(kTemperatureMeasurementUpdateTimerPeriodMs), K_NO_WAIT);

    PlatformMgr().LockChipStack();
    app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(kExampleEndpointId,
                                                                             SensorMgr().GetMinMeasuredTempValue());
    app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(kExampleEndpointId,
                                                                             SensorMgr().GetMaxMeasuredTempValue());
    PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

void AppTask::TemperatureMeasurementUpdateTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = TemperatureMeasurementUpdateTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::TemperatureMeasurementUpdateTimerEventHandler(AppEvent * aEvent)
{
    CHIP_ERROR ret;
    int16_t temperature;

    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    ret = SensorMgr().GetTempAndHumMeasurValue(&temperature, NULL);
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("Update of the Temperature clusters failed");
        return;
    }

    LOG_INF("Current temperature is (%d*0.01)°C", temperature);

    PlatformMgr().LockChipStack();
    app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(kExampleEndpointId, temperature);
    PlatformMgr().UnlockChipStack();

    // Start next timer to handle temp sensor.
    k_timer_start(&sTemperatureMeasurementUpdateTimer, K_MSEC(kTemperatureMeasurementUpdateTimerPeriodMs), K_NO_WAIT);
}
