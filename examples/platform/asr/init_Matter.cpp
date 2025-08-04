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

#include "AppConfig.h"
#include <DeviceInfoProviderImpl.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <init_Matter.h>
#include <mbedtls/platform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#ifdef CONFIG_ENABLE_CHIP_SHELL
#include <shell/launch_shell.h>
#endif
#include <platform/ASR/ASRFactoryDataProvider.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;
using namespace ::chip::System;

namespace {
ASRFactoryDataProvider sFactoryDataProvider;
} // namespace

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

CHIP_ERROR MatterInitializer::Matter_Task_Config(lega_task_config_t * cfg)
{
    lega_rtos_get_chip_task_cfg(cfg);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MatterInitializer::Init_Matter_Stack(const char * appName)
{
    // mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    ASR_LOG("==================================================\r\n");
    ASR_LOG("%s starting\r\n", appName);
    ASR_LOG("==================================================\r\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    if (err != CHIP_NO_ERROR)
    {
        ASR_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        appError(err);
    }

    SetCommissionableDataProvider(&sFactoryDataProvider);
    //==============================================
    // Init Matter Stack
    //==============================================
    ASR_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    err = chip::Platform::MemoryInit();
    ReturnErrorOnFailure(err);

    err = PlatformMgr().InitChipStack();
    ReturnErrorOnFailure(err);

    err = sFactoryDataProvider.Init();
    ReturnErrorOnFailure(err);

    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(appName);

#if CONFIG_NETWORK_LAYER_BLE
    ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR MatterInitializer::Init_Matter_Server(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Init Matter Server and Start Event Loop
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    chip::Server::GetInstance().Init(initParams);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);

    ASR_LOG("Starting Platform Manager Event Loop");
    // // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    ReturnErrorOnFailure(err);

#ifdef CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif

    return CHIP_NO_ERROR;
}
