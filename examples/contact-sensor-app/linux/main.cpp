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
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/af.h>
#include <platform/CHIPDeviceBuildConfig.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/boolean_state.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>
#endif

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/NetworkCommissioningDriver.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
#if CHIP_DEVICE_LAYER_TARGET_LINUX
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
DeviceLayer::NetworkCommissioning::LinuxThreadDriver sThreadDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DeviceLayer::NetworkCommissioning::LinuxEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
DeviceLayer::NetworkCommissioning::DarwinWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DeviceLayer::NetworkCommissioning::DarwinEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0, &sWiFiDriver);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(0, &sThreadDriver);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(0, &sEthernetDriver);
} // namespace

void ApplicationInit()
{
    const bool kThreadEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        LinuxDeviceOptions::GetInstance().mThread
#else
        false
#endif
    };

    const bool kWiFiEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        LinuxDeviceOptions::GetInstance().mWiFi
#else
        false
#endif
    };

    if (kThreadEnabled && kWiFiEnabled)
    {
        // Just use the Thread one.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
    }
    else if (kThreadEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
    }
    else if (kWiFiEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        sWiFiNetworkCommissioningInstance.Init();
#endif
    }
    else
    {
        sEthernetNetworkCommissioningInstance.Init();
    }
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;

    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::BooleanState>(chip::EndpointId(1), "Contact Sensor"));
    ui.AddWindow(std::make_unique<example::Ui::Windows::OccupancySensing>(chip::EndpointId(1), "Occupancy"));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
