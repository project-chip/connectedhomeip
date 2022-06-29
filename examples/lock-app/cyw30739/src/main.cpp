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
#include <LockManager.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <OTAConfig.h>
#endif
#include <app/clusters/door-lock-server/door-lock-server.h>
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

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlOperationError;
using chip::app::Clusters::DoorLock::DlOperationSource;

using namespace chip;
using namespace ::chip::DeviceLayer::Internal;

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;

wiced_bool_t syncClusterToButtonAction = false;

static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
static void InitApp(intptr_t args);
static void EventHandler(const ChipDeviceEvent * event, intptr_t arg);
static void HandleThreadStateChangeEvent(const ChipDeviceEvent * event);
static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor);
static void ActionCompleted(LockManager::Action_t aAction);
static void WriteClusterState(uint8_t value);

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

static wiced_led_config_t chip_lighting_led_config[2] = {
    {
        .led    = PLATFORM_LED_1,
        .bright = 50,
    },
    {
        .led    = PLATFORM_LED_2,
        .bright = 50,
    },
};

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;
    uint32_t i;

    printf("\nChipLock App starting\n");

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
    for (i = 0; i < _countof(chip_lighting_led_config); i++)
    {
        result = wiced_led_manager_init(&chip_lighting_led_config[i]);
        if (result != WICED_SUCCESS)
            printf("wiced_led_manager_init fail i=%ld, (%d)\n", i, result);
    }

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

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
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

    PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    /* Start CHIP datamodel server */
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = [] { ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb              = [] { ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    chip::Server::GetInstance().Init(initParams);

    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    // Initial lock state
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

    uint8_t maxCredentialsPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, maxCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to 5 "
                     "[endpointId=%d]",
                     endpointId);
        maxCredentialsPerUser = 5;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    CHIP_ERROR err = LockMgr().Init(state, maxCredentialsPerUser);
    if (err != CHIP_NO_ERROR)
    {
        printf("LockMgr().Init() failed\n");
    }

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}

void EventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kThreadStateChange:
        HandleThreadStateChangeEvent(event);
        break;
    default:
        break;
    }
}

void HandleThreadStateChangeEvent(const ChipDeviceEvent * event) {}

void ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LockManager::LOCK_ACTION)
    {
        printf("Lock Action has been initiated\n");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        printf("Unlock Action has been initiated\n");
    }

    if (aActor == LockManager::ACTOR_BUTTON)
    {
        syncClusterToButtonAction = true;
    }

    // Action initiated, update the light led
    if (aAction == LockManager::LOCK_ACTION)
    {
        printf("Lock Action has been initiated");
        wiced_led_manager_disable_led(PLATFORM_LED_2);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        printf("Unlock Action has been initiated");
        wiced_led_manager_enable_led(PLATFORM_LED_2);
    }
}

void UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    DlOperationSource source = DlOperationSource::kUnspecified;

    // write the new lock value
    EmberAfStatus status =
        DoorLockServer::Instance().SetLockState(1, newState, source) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("ERR: updating lock state %x", status);
    }
}

void ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        printf("Lock Action has been completed\n");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        printf("Unlock Action has been completed\n");
    }

    if (syncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        syncClusterToButtonAction = false;
    }
}
