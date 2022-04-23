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
#ifdef HAVE_NETWORK_COMMISSIONING_DRIVER
#undef HAVE_NETWORK_COMMISSIONING_DRIVER
#endif // HAVE_NETWORK_COMMISSIONING_DRIVER

// We support either thread or wi-fi commissioning, not both.  Prefer thread if
// both are available.
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD

#define HAVE_NETWORK_COMMISSIONING_DRIVER 1
DeviceLayer::NetworkCommissioning::LinuxThreadDriver sNetworkCommissioningDriver;

#else // CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

#define HAVE_NETWORK_COMMISSIONING_DRIVER 1
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sNetworkCommissioningDriver;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#endif // CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if HAVE_NETWORK_COMMISSIONING_DRIVER
Clusters::NetworkCommissioning::Instance sNetworkCommissioningInstance(0, &sNetworkCommissioningDriver);
#endif // HAVE_NETWORK_COMMISSIONING_DRIVER

} // anonymous namespace

void ApplicationInit()
{
#if HAVE_NETWORK_COMMISSIONING_DRIVER
    sNetworkCommissioningInstance.Init();
#endif // HAVE_NETWORK_COMMISSIONING_DRIVER
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    ChipLinuxAppMainLoop();
    return 0;
}
