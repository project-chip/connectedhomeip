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
#include "thermostat-delegate-impl.h"
#include "thermostat-hold-delegate-impl.h"
#include "thermostat-sensors-delegate-impl.h"
#include "thermostat-setpoints-delegate-impl.h"
#include <app/InteractionModelEngine.h>
#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <app/util/attribute-storage.h>

using namespace chip;

void ThermostatApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP thermostat Demo App");
}

// Thermostat codegen delegates for endpoint 1
static chip::app::Clusters::Thermostat::ThermostatDelegate gThermostatDelegate(1);
static chip::app::Clusters::Thermostat::ThermostatSetpointsDelegate gSetpointsDelegate(1);
static chip::app::Clusters::Thermostat::ThermostatHoldDelegate gHoldDelegate(1);
static chip::app::Clusters::Thermostat::ThermostatSensorsDelegate gSensorsDelegate(1);
void ThermostatApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

// This returns an instance of this class.
ThermostatApp::AppTask & ThermostatApp::AppTask::GetDefaultInstance()
{
    static ThermostatApp::AppTask sAppTask;
    return sAppTask;
}

void MatterThermostatClusterInitCallback(chip::EndpointId endpointId)
{
    using namespace chip::app::Clusters::Thermostat;
    // Register only heating and cooling interfaces; the concrete delegate also implements AutoMode.
    ServerInit(endpointId, gThermostatDelegate, static_cast<ThermostatHeatingSetpoints::Delegate &>(gSetpointsDelegate),
               static_cast<ThermostatCoolingSetpoints::Delegate &>(gSetpointsDelegate), gHoldDelegate, gSensorsDelegate);
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return ThermostatApp::AppTask::GetDefaultInstance();
}
