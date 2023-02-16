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

#include "LightingManager.h"
#include <AppMain.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include "ui.h"
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
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    sWiFiNetworkCommissioningInstance.Init();
#endif
}

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED

class UiAppMainLoopImplementation : public AppMainLoopImplementation
{
public:
    void RunMainLoop() override;
    void SignalSafeStopMainLoop() override;
};

void UiAppMainLoopImplementation::RunMainLoop()
{
    // Guaranteed to be on the main task (no chip event loop started yet)
    example::Ui::Init();

    // Platform event loop will be on a separate thread,
    // while the event UI loop will be on the main thread.
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    // StopEventLoop will stop the loop below. It is called
    // from within SignalSafeStopMainLoop below and
    // UI knows how to stop itself if windows are closed.
    example::Ui::EventLoop();

    // Stop the chip main loop as well. This is expected to
    // wait for the task to finish.
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

void UiAppMainLoopImplementation::SignalSafeStopMainLoop()
{
    Server::GetInstance().GenerateShutDownEvent();
    example::Ui::StopEventLoop();
}

#endif

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    LightingMgr().Init();

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    UiAppMainLoopImplementation loop;
    ChipLinuxAppMainLoop(&loop);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
