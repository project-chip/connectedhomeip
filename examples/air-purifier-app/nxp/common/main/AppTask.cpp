/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
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
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

#ifdef AIR_PURIFIER
#include "air-purifier-manager.h"

#define AIR_PURIFIER_ENDPOINT 1
#define AIR_QUALITY_SENSOR_ENDPOINT 2
#define TEMPERATURE_SENSOR_ENDPOINT 3
#define RELATIVE_HUMIDITY_SENSOR_ENDPOINT 4
#define THERMOSTAT_ENDPOINT 5

using namespace ::chip::app;
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void AirPurifierApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Air Purifier Demo App");
}

void AirPurifierApp::AppTask::PostInitMatterStack(){}

void AirPurifierApp::AppTask::PostInitMatterServerInstance()
{
#ifdef AIR_PURIFIER
	AirPurifierManager::InitInstance(EndpointId(AIR_PURIFIER_ENDPOINT), EndpointId(AIR_QUALITY_SENSOR_ENDPOINT),
                                     EndpointId(TEMPERATURE_SENSOR_ENDPOINT), EndpointId(RELATIVE_HUMIDITY_SENSOR_ENDPOINT),
                                     EndpointId(THERMOSTAT_ENDPOINT));
 
    SetParentEndpointForEndpoint(AIR_QUALITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);
    SetParentEndpointForEndpoint(TEMPERATURE_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);
    SetParentEndpointForEndpoint(RELATIVE_HUMIDITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);
    SetParentEndpointForEndpoint(THERMOSTAT_ENDPOINT, AIR_PURIFIER_ENDPOINT);
#endif
}

// This returns an instance of this class.
AirPurifierApp::AppTask & AirPurifierApp::AppTask::GetDefaultInstance()
{
    static AirPurifierApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return AirPurifierApp::AppTask::GetDefaultInstance();
}
