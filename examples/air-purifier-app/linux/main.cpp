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
#include <AppMain.h>
#include <air-purifier-manager.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/connectivity.h>
#include <imgui_ui/windows/fan_control.h>
#include <imgui_ui/windows/humidity_measurement.h>
#include <imgui_ui/windows/qrcode.h>
#include <imgui_ui/windows/temperature_measurement.h>
#endif

#define AIR_PURIFIER_ENDPOINT 1
#define FAN_CONTROL_ENDPOINT 1
#define AIR_QUALITY_SENSOR_ENDPOINT 2
#define TEMPERATURE_SENSOR_ENDPOINT 3
#define RELATIVE_HUMIDITY_SENSOR_ENDPOINT 4
#define THERMOSTAT_ENDPOINT 5

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// The MatterPostAttributeChangeCallback is offloaded to the main Air Purifier Manager class.
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (AirPurifierManager::GetInstance() != nullptr)
    {
        AirPurifierManager::GetInstance()->PostAttributeChangeCallback(attributePath.mEndpointId, attributePath.mClusterId,
                                                                       attributePath.mAttributeId, type, size, value);
    }
}

// Initialize the Air Purifier Manager and set up the endpoint composition tree.
void ApplicationInit()
{
    AirPurifierManager::InitInstance(EndpointId(AIR_PURIFIER_ENDPOINT), EndpointId(AIR_QUALITY_SENSOR_ENDPOINT),
                                     EndpointId(TEMPERATURE_SENSOR_ENDPOINT), EndpointId(RELATIVE_HUMIDITY_SENSOR_ENDPOINT),
                                     EndpointId(THERMOSTAT_ENDPOINT));

    SuccessOrDie(SetParentEndpointForEndpoint(AIR_QUALITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT));
    SuccessOrDie(SetParentEndpointForEndpoint(TEMPERATURE_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT));
    SuccessOrDie(SetParentEndpointForEndpoint(RELATIVE_HUMIDITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT));
    SuccessOrDie(SetParentEndpointForEndpoint(THERMOSTAT_ENDPOINT, AIR_PURIFIER_ENDPOINT));
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Air Purifier: ApplicationShutdown()");
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;

    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::Connectivity>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::FanControl>(chip::EndpointId(FAN_CONTROL_ENDPOINT)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::HumidityMeasurement>(chip::EndpointId(RELATIVE_HUMIDITY_SENSOR_ENDPOINT)));
    ui.AddWindow(std::make_unique<example::Ui::Windows::TemperatureMeasurement>(chip::EndpointId(TEMPERATURE_SENSOR_ENDPOINT)));
    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
