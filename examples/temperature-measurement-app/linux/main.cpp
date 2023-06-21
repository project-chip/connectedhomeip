/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/temperature.h>
#include <imgui_ui/windows/qrcode.h>
#else
#include "TemperatureOptions.h"
#include "ThermalClassSensor.h"
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sLinuxWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0, &sLinuxWiFiDriver);
} // namespace
#endif

void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    sWiFiNetworkCommissioningInstance.Init();
#endif
}

int main(int argc, char * argv[])
{
#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    if (ChipLinuxAppInit(argc, argv) != 0)
#else
    TemperatureOptions temperature_options;
    if (ChipLinuxAppInit(argc, argv, &temperature_options) != 0)
#endif
    {
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;

    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::Temperature>(chip::EndpointId(1), "Temperature"));

    ChipLinuxAppMainLoop(&ui);
#else
    ThermalClassSensor sensor(chip::EndpointId(1), 
                              temperature_options.getThermalZone(), 
                              temperature_options.getMinTemperature(), 
                              temperature_options.getMaxTemperature());
    ChipLinuxAppMainLoop(&sensor);
#endif
    
    return 0;
}
