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
#include <resource-monitoring-delegates.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>

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

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        // LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
}
void MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                                             const chip::Access::SubjectDescriptor & subjectDescriptor)
{

    ChipLogError(Zcl, "HepaFilterMonitoringDelegate::MatterPostCommandReceivedCallback()");
}


Clusters::ResourceMonitoring::HepaFilterMonitoringDelegate hepaFilterDelegate;
Clusters::ResourceMonitoring::Instance HepafilterInstance(0x1, Clusters::HepaFilterMonitoring::Id, &hepaFilterDelegate);

Clusters::ResourceMonitoring::ActivatedCarbonFilterMonitoringDelegate activatedCarbonFilterDelegate;
Clusters::ResourceMonitoring::Instance ActivatedCarbonFilterInstance(0x1, Clusters::ActivatedCarbonFilterMonitoring::Id, &activatedCarbonFilterDelegate);

void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    sWiFiNetworkCommissioningInstance.Init();
#endif

    HepafilterInstance.Init();
    ActivatedCarbonFilterInstance.Init();
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
