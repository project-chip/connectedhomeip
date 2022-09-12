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

#include "AppMain.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include "LockAppCommandDelegate.h"
#include "LockManager.h"

using namespace chip;
using namespace chip::app;

namespace {
// We support either thread or wi-fi commissioning, not both.  Prefer thread if
// both are available.
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD
DeviceLayer::NetworkCommissioning::LinuxThreadDriver sNetworkCommissioningDriver;
Clusters::NetworkCommissioning::Instance sNetworkCommissioningInstance(0, &sNetworkCommissioningDriver);

void InitNetworkCommissioning()
{
    sNetworkCommissioningInstance.Init();
}

#elif CHIP_DEVICE_CONFIG_ENABLE_WPA

DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sNetworkCommissioningDriver;
Clusters::NetworkCommissioning::Instance sNetworkCommissioningInstance(0, &sNetworkCommissioningDriver);

void InitNetworkCommissioning()
{
    sNetworkCommissioningInstance.Init();
}

#else // !CHIP_DEVICE_CONFIG_ENABLE_WPA && !(CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD)

void InitNetworkCommissioning() {}

#endif // (CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD) || CHIP_DEVICE_CONFIG_ENABLE_WPA

// Variables for handling named pipe commands
constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_lock_app_fifo-";
NamedPipeCommands sChipNamedPipeCommands;
LockAppCommandDelegate sLockAppCommandDelegate;

} // anonymous namespace

void ApplicationInit()
{
    InitNetworkCommissioning();

    auto path = kChipEventFifoPathPrefix + std::to_string(getpid());
    if (sChipNamedPipeCommands.Start(path, &sLockAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // TODO: Watch for LockState, DoorState, Mode, etc changes and trigger appropriate action
    if (attributePath.mClusterId == Clusters::DoorLock::Id)
    {
        emberAfDoorLockClusterPrintln("Door Lock attribute changed");
    }
}
