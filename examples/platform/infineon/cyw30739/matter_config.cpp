/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "matter_config.h"
#include "AppTask.h"
#include "EventManagementTestEventTriggerHandler.h"
#ifdef BOARD_ENABLE_DISPLAY
#include "GUI.h"
#endif
#include "cy_serial_flash.h"
#include <AppShellCommands.h>
#include <ChipShellCollection.h>
#include <DeviceInfoProviderImpl.h>
#include <LEDWidget.h>
#include <cycfg_pins.h>
#include <matter_config.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAConfig.h>
#endif
#ifdef BOARD_ENABLE_I2C
#include "wiced_hal_i2c.h"
#endif
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <wiced_rtos.h>

#ifdef BOARD_USE_OPTIGA
#include "wiced_optiga.h"
#ifdef USE_PROVISIONED_OPTIGA
#include <platform/Infineon/CYW30739/OptigaFactoryDataProvider.h>
#else /* !USE_PROVISIONED_OPTIGA */
#include <platform/Infineon/CYW30739/UnprovisionedOptigaFactoryDataProvider.h>
#endif /* USE_PROVISIONED_OPTIGA */
#else  /* !BOARD_USE_OPTIGA */
#include <platform/Infineon/CYW30739/FactoryDataProvider.h>
#endif /* BOARD_USE_OPTIGA */

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;
using namespace ::chip::app;

static DeviceInfoProviderImpl sExampleDeviceInfoProvider;
#ifdef BOARD_USE_OPTIGA
#ifdef USE_PROVISIONED_OPTIGA
static OptigaFactoryDataProvider sFactoryDataProvider;
#else  /* !USE_PROVISIONED_OPTIGA */
static UnprovisionedOptigaFactoryDataProvider sFactoryDataProvider;
#endif /* USE_PROVISIONED_OPTIGA */
#else  /* !BOARD_USE_OPTIGA */
static FactoryDataProvider sFactoryDataProvider;
#endif /* BOARD_USE_OPTIGA */

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                         0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                         0xcc, 0xdd, 0xee, 0xff };

void __attribute__((weak)) AppChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * aEvent, intptr_t arg)
{
    (void) aEvent;
    (void) arg;
}

void __attribute__((weak)) RegisterAppShellCommands();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
void LockOpenThreadTask(void)
{
    ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    ThreadStackMgr().UnlockThreadStack();
}

CHIP_ERROR CYW30739MatterConfig::InitOpenThread(void)
{
    CHIP_ERROR err;

    ReturnLogErrorOnFailure(ThreadStackMgr().InitThreadStack());

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else // !CHIP_DEVICE_CONFIG_THREAD_FTD
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else  /* !CHIP_CONFIG_ENABLE_ICD_SERVER */
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER */
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR SetThreadDeviceType %ld\n", err.AsInteger());
    }

    printf("Starting thread task\n");
    ReturnLogErrorOnFailure(ThreadStackMgr().StartThreadTask());

    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

void CYW30739MatterConfig::InitShell(void)
{
    const int ret = Engine::Root().Init();
    if (!chip::ChipError::IsSuccess(ret))
    {
        printf("ERROR Shell Init %d\n", ret);
    }
    RegisterAppShellCommands();
    Engine::Root().RunMainLoop();
}

void CYW30739MatterConfig::InitBoard(void)
{
#ifdef BOARD_ENABLE_I2C
    /* Init. I2C interface. */
    wiced_hal_i2c_select_pads(PLATFORM_I2C_1_SCL, PLATFORM_I2C_1_SDA);
    wiced_hal_i2c_init();
    wiced_hal_i2c_set_speed(I2CM_SPEED_400KHZ);
#endif
#ifdef BOARD_ENABLE_DISPLAY
    GUI_Init();
#endif
#ifdef BOARD_USE_OPTIGA
    wiced_optiga_init();
#endif
}

CHIP_ERROR CYW30739MatterConfig::InitMatter(void)
{
    InitBoard();

#ifdef MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT
    mbedtls_platform_setup(NULL);
#endif
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    // Init Chip memory management before the stack
    ReturnLogErrorOnFailure(chip::Platform::MemoryInit());

    printf("Initializing CHIP\n");
    ReturnLogErrorOnFailure(PlatformMgr().InitChipStack());

    SetCommissionableDataProvider(&sFactoryDataProvider);
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Initializing OpenThread stack\n");
    ReturnLogErrorOnFailure(InitOpenThread());
#endif

    PlatformMgr().ScheduleWork(AppTaskMain, 0);

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(AppChipEventHandler, 0);

    printf("Starting event loop task\n");
    ReturnLogErrorOnFailure(PlatformMgr().StartEventLoopTask());

    InitShell();

    assert(!wiced_rtos_check_for_stack_overflow());

    return CHIP_NO_ERROR;
}

void CYW30739MatterConfig::InitApp(void)
{
    LogAppInit();
    ConfigurationMgr().LogDeviceConfig();

    sFactoryDataProvider.Init();

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    /* Start CHIP datamodel server */
    static chip::SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static chip::OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    static Infineon::CYW30739::EventManagementTestEventTriggerHandler sEventManagementTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(chip::ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sEventManagementTestEventTriggerHandler) == CHIP_NO_ERROR);
    // Create initParams with SDK example defaults here
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider        = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
    sExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    SetDeviceInfoProvider(&sExampleDeviceInfoProvider);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // Set up OpenThread configuration when OpenThread is included
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = [] { ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb              = [] { ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif

    // Init Matter Server
    chip::Server::GetInstance().Init(initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}

void CYW30739MatterConfig::LogAppInit(void)
{
    char productName[ConfigurationManager::kMaxProductNameLength + 1];
    if (CHIP_NO_ERROR == GetDeviceInstanceInfoProvider()->GetProductName(productName, sizeof(productName)))
    {
        printf("Initializing %s\n", productName);
    }
}
