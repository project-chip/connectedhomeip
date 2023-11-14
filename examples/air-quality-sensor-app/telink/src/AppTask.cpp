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
#include <air-quality-sensor-manager.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::AirQuality;

namespace {
volatile bool mSensorBanForNextMeasur = false;
k_timer sSensorMeasurTimer;
k_timer sSensorBanForNextMeasurTimer;
constexpr uint16_t kSensorMeasurTimerPeriodMs = 10000; // 10s timer period
constexpr uint16_t kSensorBanForNextMeasurTimeoutMs = 1000; // 1s timeout
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    CHIP_ERROR err;

#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(AirQualitySensorMeasurActionEventHandler);
#endif
    InitCommonParts();

    err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Init of the Sensor Manager failed");
        return err;
    }

    // Initialize sensor measurement timer
    k_timer_init(&sSensorMeasurTimer, &AppTask::AirQualitySensorMeasurTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sSensorMeasurTimer, this);
    k_timer_start(&sSensorMeasurTimer, K_MSEC(kSensorMeasurTimerPeriodMs), K_NO_WAIT);

    // Initialise the timer to ban sensor measurement
    k_timer_init(&sSensorBanForNextMeasurTimer, &AppTask::AirQualitySensorBanForNextMeasurTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sSensorBanForNextMeasurTimer, this);

    AirQualitySensorManager::InitInstance(kExampleEndpointId);

    return CHIP_NO_ERROR;
}

void AppTask::UpdateClusterState(void)
{
    CHIP_ERROR ret;
    AirQualitySensorManager * mInstance = AirQualitySensorManager::GetInstance();
    static int16_t temperature = 0;
    static uint16_t humidity = 0;

    if (!mSensorBanForNextMeasur)
    {
        ret = SensorMgr().GetTempAndHumMeasurValue(&temperature, &humidity);
        if (ret != CHIP_NO_ERROR)
        {
            LOG_ERR("Update of the Air Quality clusters failed");
            return;
        }

        mSensorBanForNextMeasur = true;

        // Start next timer to measurement the air quality sensor
        k_timer_start(&sSensorBanForNextMeasurTimer, K_MSEC(kSensorBanForNextMeasurTimeoutMs), K_NO_WAIT);
    }

    // Update AirQuality value
    mInstance->OnAirQualityChangeHandler(AirQualityEnum::kUnknown);

    // Update Carbon Dioxide
    mInstance->OnCarbonDioxideMeasurementChangeHandler(0);

    // Update Temperature value
    mInstance->OnTemperatureMeasurementChangeHandler(temperature);

    // Update Humidity value
    mInstance->OnHumidityMeasurementChangeHandler(humidity);

    LOG_INF("Update Air Quality: temperature is %d*0.01°C, humidity is %d", temperature, humidity);
}

void AppTask::AirQualitySensorMeasurTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = AirQualitySensorMeasurActionEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::AirQualitySensorBanForNextMeasurTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    mSensorBanForNextMeasur = false;
}

void AppTask::AirQualitySensorMeasurActionEventHandler(AppEvent * aEvent)
{
    if ((aEvent->Type == AppEvent::kEventType_Button) || (aEvent->Type == AppEvent::kEventType_Timer))
    {
        sAppTask.UpdateClusterState();
    }

    if (aEvent->Type == AppEvent::kEventType_Timer)
    {
        // Start next timer to measurement the air quality sensor
        k_timer_start(&sSensorMeasurTimer, K_MSEC(kSensorMeasurTimerPeriodMs), K_NO_WAIT);
    }
}
