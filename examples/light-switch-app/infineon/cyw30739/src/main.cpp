/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <LightSwitch.h>
#include <LightingManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAConfig.h>
#endif
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <platform/Infineon/CYW30739/FactoryDataProvider.h>
#include <protocols/secure_channel/PASESession.h>
#include <sparcommon.h>
#include <stdio.h>
#include <wiced_led_manager.h>
#include <wiced_memory.h>
#include <wiced_platform.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;
using namespace ::chip::app;

constexpr chip::EndpointId kLightDimmerSwitchEndpointId = 1;

static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
static FactoryDataProvider sFactoryDataProvider;
static bool sIsThreadProvisioned    = false;
static bool sIsThreadEnabled        = false;
static bool sIsThreadBLEAdvertising = false;
static bool sHaveBLEConnections     = false;

static void InitApp(intptr_t args);
static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent *, intptr_t);

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
        break;
    }
    return;
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnIdentifyTriggerEffect,
};

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;

    printf("\nChip Light-Switch App starting\n");

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
    LightMgr().Init();

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
#else // !CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_DEVICE_CONFIG_ENABLE_SED
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else  /* !CHIP_DEVICE_CONFIG_ENABLE_SED */
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_SED */
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR SetThreadDeviceType %ld\n", err.AsInteger());
    }

    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

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
    static chip::SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static chip::OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(chip::ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = [] { ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb              = [] { ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    chip::Server::GetInstance().Init(initParams);

    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);

    LightSwitch::GetInstance().Init(kLightDimmerSwitchEndpointId);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}

void UpdateStatusLED()
{
    LightMgr().Set(false, PLATFORM_LED_2);
    // Status LED indicates:
    // - blinking 1 s - advertising, ready to commission
    // - blinking 200 ms - commissioning in progress
    // - constant lightning means commissioned with Thread network
    if (sIsThreadBLEAdvertising && !sHaveBLEConnections)
    {
        LightMgr().Blink(PLATFORM_LED_2, 50, 950);
    }
    else if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        LightMgr().Set(true, PLATFORM_LED_2);
    }
    else if (sHaveBLEConnections)
    {
        LightMgr().Blink(PLATFORM_LED_2, 50, 150);
    }
    else
    {
        /* back to default status */
        LightMgr().Set(false, PLATFORM_LED_2);
    }
}

void ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t arg)
{
#if CHIP_DEVICE_CONFIG_ENABLE_LOCAL_LEDSTATUS_DEBUG
    switch (aEvent->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sIsThreadBLEAdvertising = true;
        sHaveBLEConnections     = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        UpdateStatusLED();
        break;
    case DeviceEventType::kThreadConnectivityChange:
        if (aEvent->ThreadConnectivityChange.Result == kConnectivity_Established)
            sHaveBLEConnections = true;
        break;
    default:
        if ((ConnectivityMgr().NumBLEConnections() == 0) && (!sIsThreadProvisioned || !sIsThreadEnabled))
        {
            printf("[Event] Commissioning with a Thread network has not been done. An error occurred\n");
            sIsThreadBLEAdvertising = false;
            sHaveBLEConnections     = false;
            UpdateStatusLED();
        }
        break;
    }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_LOCAL_LEDSTATUS_DEBUG */
}

CHIP_ERROR StartBLEAdvertisingHandler()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    /// Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0)
    {
        printf("Matter service BLE advertising not started - device is already commissioned\n");
        return err;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        printf("BLE advertising is already enabled\n");
        return err;
    }
    else
    {
        printf("Start BLE advertising...\n");
        err = ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (err != CHIP_NO_ERROR)
        {
            printf("Enable BLE advertising failed\n");
            return err;
        }
    }

    printf("Enabling BLE advertising...\n");
    /* Check Commissioning Window*/
    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        printf("OpenBasicCommissioningWindow() failed\n");
        err = CHIP_ERROR_UNEXPECTED_EVENT;
    }

    return err;
}

CHIP_ERROR StopBLEAdvertisingHandler()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    printf("Stop BLE advertising...\n");
    err = ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    if (err != CHIP_NO_ERROR)
    {
        printf("Disable BLE advertising failed\n");
        return err;
    }
    return err;
}
