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

#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <resource-monitoring-instances.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>

#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sLinuxWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0, &sLinuxWiFiDriver);
} // namespace
#endif

// clang-format off
HepaFilterMonitoringInstance            HepafilterInstance              (0x1, HepaFilterMonitoring::Id,             1, DegradationDirectionEnum::kDown, true);
ActivatedCarbonFilterMonitoringInstance ActivatedCarbonFilterInstance   (0x1, ActivatedCarbonFilterMonitoring::Id,  1, DegradationDirectionEnum::kDown, true);
CeramicFilterMonitoringInstance         CeramicFilterInstance           (0x1, CeramicFilterMonitoring::Id,          1, DegradationDirectionEnum::kDown, true);
ElectrostaticFilterMonitoringInstance   ElectrostaticFilterInstance     (0x1, ElectrostaticFilterMonitoring::Id,    1, DegradationDirectionEnum::kDown, true);
FuelTankMonitoringInstance              FuelTankInstance                (0x1, FuelTankMonitoring::Id,               1, DegradationDirectionEnum::kDown, true);
InkCartridgeMonitoringInstance          InkCartridgeInstance            (0x1, InkCartridgeMonitoring::Id,           1, DegradationDirectionEnum::kDown, true);
IonizingFilterMonitoringInstance        IonizingFilterInstance          (0x1, IonizingFilterMonitoring::Id,         1, DegradationDirectionEnum::kDown, true);
OzoneFilterMonitoringInstance           OzoneFilterInstance             (0x1, OzoneFilterMonitoring::Id,            1, DegradationDirectionEnum::kDown, true);
UvFilterMonitoringInstance              UvFilterInstance                (0x1, UvFilterMonitoring::Id,               1, DegradationDirectionEnum::kDown, true);
WaterTankMonitoringInstance             WaterTankInstance               (0x1, WaterTankMonitoring::Id,              1, DegradationDirectionEnum::kDown, true);
ZeoliteFilterMonitoringInstance         ZeoliteFilterInstance           (0x1, ZeoliteFilterMonitoring::Id,          1, DegradationDirectionEnum::kDown, true);
TonerCartridgeMonitoringInstance        TonerCartridgeInstance          (0x1, TonerCartridgeMonitoring::Id,         1, DegradationDirectionEnum::kDown, true);
// clang-format on
void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    sWiFiNetworkCommissioningInstance.Init();
#endif

    HepafilterInstance.Init();
    ActivatedCarbonFilterInstance.Init();
    CeramicFilterInstance.Init();
    ElectrostaticFilterInstance.Init();
    FuelTankInstance.Init();
    InkCartridgeInstance.Init();
    IonizingFilterInstance.Init();
    OzoneFilterInstance.Init();
    UvFilterInstance.Init();
    WaterTankInstance.Init();
    ZeoliteFilterInstance.Init();
    TonerCartridgeInstance.Init();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
