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
#include <air-quality-sensor-manager.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::AirQuality;

AppTask AppTask::sAppTask;

constexpr EndpointId kAirQualityEndpoint = 1;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(AirQualityActionEventHandler);
#endif
    InitCommonParts();

    AirQualitySensorManager::InitInstance(kAirQualityEndpoint);

    return CHIP_NO_ERROR;
}

void AppTask::UpdateClusterState(void)
{
    AirQualitySensorManager * mInstance = AirQualitySensorManager::GetInstance();

    // Update AirQuality value
    mInstance->OnAirQualityChangeHandler(AirQualityEnum::kModerate);

    // Update Carbon Dioxide
    mInstance->OnCarbonDioxideMeasurementChangeHandler(400);

    // Update Temperature value
    mInstance->OnTemperatureMeasurementChangeHandler(18);

    // Update Humidity value
    mInstance->OnHumidityMeasurementChangeHandler(60);
}

void AppTask::AirQualityActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}
