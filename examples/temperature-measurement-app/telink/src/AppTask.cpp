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
#include "SensorManager.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
k_timer sTemperatureMeasurementTimer;
constexpr uint16_t kSensorTimerPeriodMs = 5000; // 5s timer period
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    InitCommonParts();

    // Initialize temperature measurement timer
    k_timer_init(&sTemperatureMeasurementTimer, &AppTask::TemperatureMeasurementTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sTemperatureMeasurementTimer, this);
    k_timer_start(&sTemperatureMeasurementTimer, K_MSEC(kSensorTimerPeriodMs), K_NO_WAIT);

    // Init Temperature Sensor
    CHIP_ERROR err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SensorMgr Init fail");
        return err;
    }

    PlatformMgr().LockChipStack();
    app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(kExampleEndpointId, SensorMgr().GetMinMeasuredValue());
    app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(kExampleEndpointId, SensorMgr().GetMaxMeasuredValue());
    PlatformMgr().UnlockChipStack();

    err = ConnectivityMgr().SetBLEDeviceName("TelinkTerm");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::TemperatureMeasurementTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = TemperatureMeasurementTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::TemperatureMeasurementTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    PlatformMgr().LockChipStack();
    app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(kExampleEndpointId, SensorMgr().GetMeasuredValue());
    PlatformMgr().UnlockChipStack();

    LOG_INF("Current temperature is (%d*0.01)°C", SensorMgr().GetMeasuredValue());

    // Start next timer to handle temp sensor.
    k_timer_start(&sTemperatureMeasurementTimer, K_MSEC(kSensorTimerPeriodMs), K_NO_WAIT);
}
