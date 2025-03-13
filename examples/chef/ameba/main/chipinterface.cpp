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

#include <platform_stdlib.h>

#include "AmebaObserver.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "chip_porting.h"
#include <DeviceInfoProviderImpl.h>
#include <lwip_netconf.h>

#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/FactoryDataProvider.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CHIPMem.h>

#if CONFIG_ENABLE_PW_RPC
#include <Rpc.h>
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <shell/launch_shell.h>
#endif

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

namespace { // Network Commissioning
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::AmebaWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
};

#ifdef CONFIG_PLATFORM_8721D
#define STATUS_LED_GPIO_NUM PB_5
#elif defined(CONFIG_PLATFORM_8710C)
#define STATUS_LED_GPIO_NUM PA_20
#else
#define STATUS_LED_GPIO_NUM NC
#endif

static DeviceCallbacks EchoCallbacks;
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

    static AmebaObserver sAmebaObserver;
    initParams.appDelegate = &sAmebaObserver;

    chip::Server::GetInstance().Init(initParams);

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    NetWorkCommissioningInstInit();

    if (RTW_SUCCESS != wifi_is_connected_to_ap())
    {
        // QR code will be used with CHIP Tool
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sAmebaObserver);
}

extern "C" void ChipTest(void)
{
    ChipLogProgress(DeviceLayer, "Chef App!");
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    initPref();

    mFactoryDataProvider.Init();
    SetCommissionableDataProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK\r\n");
    }

    // Set DeviceInstanceInfoProvider after CHIPDeviceManager init
    // CHIPDeviceManager init will set GenericDeviceInsanceInfoProvider first
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, 0);

    statusLED1.Init(STATUS_LED_GPIO_NUM);

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif
}

bool lowPowerClusterSleep()
{
    return true;
}
