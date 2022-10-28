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

#include <app/server/Dnssd.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <zephyr/logging/log.h>

#if CONFIG_ENABLE_CHIP_SHELL || CONFIG_CHIP_LIB_SHELL
#include <ChipShellCollection.h>
#endif

#ifdef CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

LOG_MODULE_REGISTER(app, CONFIG_MATTER_LOG_LEVEL);

using namespace chip;
using namespace chip::Shell;
using namespace chip::DeviceLayer;

namespace {
constexpr int kExtDiscoveryTimeoutSecs = 20;
}

int main()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef CONFIG_ENABLE_PW_RPC
    rpc::Init();
#endif

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Platform::MemoryInit() failed");
        return 1;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PlatformMgr().InitChipStack() failed");
        return 1;
    }

    // Network connectivity
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    ConnectivityManagerImpl().StartWiFiManagement();
#endif
#if CHIP_ENABLE_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ThreadStackMgr().InitThreadStack() failed");
        return 1;
    }

#ifdef CONFIG_OPENTHREAD_MTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#else
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#endif
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ConnectivityMgr().SetThreadDeviceType() failed");
        return 1;
    }
#endif /* CHIP_ENABLE_OPENTHREAD */

    // Device Attestation & Onboarding codes
    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(kExtDiscoveryTimeoutSecs);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY */

    // Start IM server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    err = chip::Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        return 1;
    }

    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

    err = chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PlatformMgr().StartEventLoopTask() failed");
    }

    // When SoftAP support becomes available, it should be added here.
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    // Starts commissioning window automatically. Starts BLE advertising when BLE enabled
    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OpenBasicCommissioningWindow() failed");
    }

#if CONFIG_ENABLE_CHIP_SHELL || CONFIG_CHIP_LIB_SHELL
    int rc = Engine::Root().Init();
    if (rc != 0)
    {
        ChipLogError(AppServer, "Streamer initialization failed: %d", rc);
        return 1;
    }

    cmd_misc_init();
    cmd_otcli_init();
#endif

#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif

#if CONFIG_ENABLE_CHIP_SHELL || CONFIG_CHIP_LIB_SHELL
    Engine::Root().RunMainLoop();
#endif

    return 0;
}
