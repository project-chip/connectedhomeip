/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/shell/Engine.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

using namespace chip;
using namespace chip::Shell;

int main()
{
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    // Network connectivity
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    chip::DeviceLayer::ConnectivityManagerImpl().StartWiFiManagement();
#endif
#if CHIP_ENABLE_OPENTHREAD
    chip::DeviceLayer::ThreadStackMgr().InitThreadStack();
#ifdef CONFIG_OPENTHREAD_MTD_SED
    chip::DeviceLayer::ConnectivityMgr().SetThreadDeviceType(
        chip::DeviceLayer::ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
    chip::DeviceLayer::ConnectivityMgr().SetThreadDeviceType(
        chip::DeviceLayer::ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif /* CONFIG_OPENTHREAD_MTD_SED */
#endif /* CHIP_ENABLE_OPENTHREAD */

    // Start IM server
    chip::Server::GetInstance().Init();

    // Device Attestation & Onboarding codes
    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

    // When SoftAP support becomes available, it should be added here.
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    // Starts commissioning window automatically. Starts BLE advertising when BLE enabled
    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "OpenBasicCommissioningWindow() failed");
    }

    const int rc = Engine::Root().Init();

    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return rc;
    }

#if CONFIG_ENABLE_CHIP_SHELL
    cmd_misc_init();
    cmd_otcli_init();
    cmd_ping_init();
    cmd_send_init();
#endif

#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif

#if CONFIG_ENABLE_CHIP_SHELL
    Engine::Root().RunMainLoop();
#endif

    return 0;
}
