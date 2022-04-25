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
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

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

} // anonymous namespace

void ApplicationInit()
{
    InitNetworkCommissioning();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
