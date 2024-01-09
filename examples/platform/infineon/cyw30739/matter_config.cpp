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
#include "AppTask.h"
#include <AppShellCommands.h>
#include <ChipShellCollection.h>
#include <DeviceInfoProviderImpl.h>
#include <LEDWidget.h>
#include <matter_config.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAConfig.h>
#endif
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <platform/Infineon/CYW30739/FactoryDataProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/PASESession.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;
using namespace ::chip::app;

static DeviceInfoProviderImpl sExampleDeviceInfoProvider;
static FactoryDataProvider sFactoryDataProvider;

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                         0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                         0xcc, 0xdd, 0xee, 0xff };

void __attribute__((weak)) AppChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * aEvent, intptr_t arg)
{
    (void) aEvent;
    (void) arg;
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
void LockOpenThreadTask(void)
{
    ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    ThreadStackMgr().UnlockThreadStack();
}

namespace {
CHIP_ERROR InitOpenThread(void)
{
    CHIP_ERROR err;

    ReturnLogErrorOnFailure(ThreadStackMgr().InitThreadStack());

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

    printf("Starting thread task\n");
    ReturnLogErrorOnFailure(ThreadStackMgr().StartThreadTask());

    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

void InitShell(void)
{
    const int ret = Engine::Root().Init();
    if (!chip::ChipError::IsSuccess(ret))
    {
        printf("ERROR Shell Init %d\n", ret);
    }
    RegisterAppShellCommands();
    Engine::Root().RunMainLoop();
}
void InitApp(intptr_t args)
{
    ConfigurationMgr().LogDeviceConfig();

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    /* Start CHIP datamodel server */
    static chip::OTATestEventTriggerDelegate testEventTriggerDelegate{ chip::ByteSpan(sTestEventTriggerEnableKey) };
    // Create initParams with SDK example defaults here
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
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

    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);

    InitAppTask();

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}
} // anonymous namespace

CHIP_ERROR InitMatter(const char * appName)
{
    wiced_result_t result;

    printf("\n%s App starting\n", appName);

#ifdef MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT
    mbedtls_platform_setup(NULL);
#endif
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    // Init Chip memory management before the stack
    ReturnLogErrorOnFailure(chip::Platform::MemoryInit());

    printf("Initializing CHIP\n");
    ReturnLogErrorOnFailure(PlatformMgr().InitChipStack());

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Initializing OpenThread stack\n");
    ReturnLogErrorOnFailure(InitOpenThread());
#endif

    PlatformMgr().ScheduleWork(InitApp, 0);

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
