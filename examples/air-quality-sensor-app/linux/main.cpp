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

#include "AirQualitySensorAppAttrUpdateDelegate.h"
#include <AppMain.h>
#include <air-quality-sensor-manager.h>

#include <app/util/af.h>
#include <platform/CHIPDeviceConfig.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/qrcode.h>
#endif

#define AIR_QUALITY_SENSOR_ENDPOINT 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
constexpr char kChipEventFifoPathPrefix[] = "/tmp/chip_air_quality_fifo_";
NamedPipeCommands sChipNamedPipeCommands;
AirQualitySensorAppAttrUpdateDelegate sAirQualitySensorAppCommandDelegate;
} // namespace

void ApplicationInit()
{
    AirQualitySensorManager::InitInstance(EndpointId(AIR_QUALITY_SENSOR_ENDPOINT));
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sAirQualitySensorAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
