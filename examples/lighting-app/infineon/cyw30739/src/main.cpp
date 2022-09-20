/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#include <platform/CHIPDeviceLayer.h>

#include <AppShellCommands.h>
#include <ButtonHandler.h>
#include <ChipShellCollection.h>
#include <DeviceInfoProviderImpl.h>
#include <LightingManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAConfig.h>
#endif
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <protocols/secure_channel/PASESession.h>
#include <sparcommon.h>
#include <stdio.h>
#include <wiced_led_manager.h>
#include <wiced_memory.h>
#include <wiced_platform.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;

static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
static void InitApp(intptr_t args);
static void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t value);

static wiced_led_config_t chip_lighting_led_config = {
    .led    = PLATFORM_LED_1,
    .bright = 50,
};

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                         0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                         0xcc, 0xdd, 0xee, 0xff };

/**********************************************************
 * Identify Callbacks
 *********************************************************/

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
    return;
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_NONE,
    OnIdentifyTriggerEffect,
};

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;

    printf("\nChipLighting App starting\n");

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR MemoryInit %ld\n", err.AsInteger());
    }

    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        printf("ERROR app_button_init %d\n", result);
    }

    /* Init. LED Manager. */
    result = wiced_led_manager_init(&chip_lighting_led_config);
    if (result != WICED_SUCCESS)
        printf("wiced_led_manager_init fail (%d)\n", result);

    printf("Initializing CHIP\n");
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR InitChipStack %ld\n", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Initializing OpenThread stack\n");
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR InitThreadStack %ld\n", err.AsInteger());
    }
#endif

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else  // !CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR SetThreadDeviceType %ld\n", err.AsInteger());
    }

    printf("Starting event loop task\n");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR StartEventLoopTask %ld\n", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Starting thread task\n");
    err = ThreadStackMgr().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR StartThreadTask %ld\n", err.AsInteger());
    }
#endif

    PlatformMgr().ScheduleWork(InitApp, 0);

    const int ret = Engine::Root().Init();
    if (!chip::ChipError::IsSuccess(ret))
    {
        printf("ERROR Shell Init %d\n", ret);
    }
    RegisterAppShellCommands();
    Engine::Root().RunMainLoop();

    assert(!wiced_rtos_check_for_stack_overflow());
}

void InitApp(intptr_t args)
{
    ConfigurationMgr().LogDeviceConfig();

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    /* Start CHIP datamodel server */
    static chip::OTATestEventTriggerDelegate testEventTriggerDelegate{ chip::ByteSpan(sTestEventTriggerEnableKey) };
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = [] { ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb              = [] { ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    chip::Server::GetInstance().Init(initParams);

    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    LightMgr().Init();
    LightMgr().SetCallbacks(LightManagerCallback, nullptr);
    LightMgr().WriteClusterLevel(254);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}

void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t level)
{
    if (action == LightingManager::ON_ACTION)
    {
        printf("Turning light ON\n");
        wiced_led_manager_enable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        printf("Turning light OFF\n");
        wiced_led_manager_disable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::LEVEL_ACTION)
    {
        printf("Set light level = %d\n", level);
        chip_lighting_led_config.bright = (uint16_t) level * 100 / 0xfe;
        wiced_led_manager_reconfig_led(&chip_lighting_led_config);
    }
}
