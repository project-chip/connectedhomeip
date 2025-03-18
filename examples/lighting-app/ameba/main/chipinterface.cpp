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
#include "Server.h"
#include <DeviceInfoProviderImpl.h>

#include "chip_porting.h"
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <setup_payload/OnboardingCodesUtil.h>
#if CONFIG_ENABLE_AMEBA_CRYPTO
#include <platform/Ameba/crypto/AmebaPersistentStorageOperationalKeystore.h>
#endif
#include <lwip_netconf.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

using namespace ::chip;
using namespace ::chip::app;
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

#ifdef CONFIG_PLATFORM_8721D
#define STATUS_LED_GPIO_NUM PB_5
#elif defined(CONFIG_PLATFORM_8710C)
#define STATUS_LED_GPIO_NUM PA_20
#else
#define STATUS_LED_GPIO_NUM NC
#endif

static DeviceCallbacks EchoCallbacks;
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

void OnIdentifyStart(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
#if CONFIG_ENABLE_AMEBA_CRYPTO
    ChipLogProgress(DeviceLayer, "platform crypto enabled!");
    static chip::AmebaPersistentStorageOperationalKeystore sAmebaPersistentStorageOpKeystore;
    VerifyOrDie((sAmebaPersistentStorageOpKeystore.Init(initParams.persistentStorageDelegate)) == CHIP_NO_ERROR);
    initParams.operationalKeystore = &sAmebaPersistentStorageOpKeystore;
#endif
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
    ChipLogProgress(DeviceLayer, "Lighting App Demo!");
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    initPref();

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

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, 0);

    statusLED1.Init(STATUS_LED_GPIO_NUM);
}

bool lowPowerClusterSleep()
{
    return true;
}
