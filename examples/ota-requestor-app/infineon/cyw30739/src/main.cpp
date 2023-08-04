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

#include <ChipShellCollection.h>
#include <OTAConfig.h>
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
#include <wiced_memory.h>
#include <wiced_platform.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Shell;

static FactoryDataProvider sFactoryDataProvider;

static void InitApp(intptr_t args);

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;

    printf("\nOtaRequestor App starting\n");

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    err = Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR MemoryInit %ld\n", err.AsInteger());
    }

    /* Init. LED Manager. */
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
    if (!ChipError::IsSuccess(ret))
    {
        printf("ERROR Shell Init %d\n", ret);
    }
    Engine::Root().RunMainLoop();

    assert(!wiced_rtos_check_for_stack_overflow());
}

void InitApp(intptr_t args)
{
    ConfigurationMgr().LogDeviceConfig();

    /* Start CHIP datamodel server */
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = [] { ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb              = [] { ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    chip::Server::GetInstance().Init(initParams);

    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);

    OTAConfig::Init();
}
