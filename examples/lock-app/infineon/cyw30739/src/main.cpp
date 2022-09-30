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

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::DlOperationError;
using chip::app::Clusters::DoorLock::DlOperationSource;

using namespace chip;
using namespace ::chip::DeviceLayer::Internal;

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;
using namespace CYW30739DoorLock::LockInitParams;

wiced_bool_t syncClusterToButtonAction = false;

static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
static void InitApp(intptr_t args);
static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor);
static void ActionCompleted(LockManager::Action_t aAction);
static void WriteClusterState(uint8_t value);
static void UpdateClusterState(intptr_t context);

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                         0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                         0xcc, 0xdd, 0xee, 0xff };

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

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;
    uint32_t i;

    ChipLogProgress(Zcl, "ChipLock App starting");

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR MemoryInit %ld", err.AsInteger());
    }

    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        ChipLogError(Zcl, "ERROR app_button_init %d", result);
    }

    /* Init. LED Manager. */
    for (i = 0; i < _countof(chip_lighting_led_config); i++)
    {
        result = wiced_led_manager_init(&chip_lighting_led_config[i]);
        if (result != WICED_SUCCESS)
            ChipLogError(Zcl, "wiced_led_manager_init fail i=%ld, (%d)", i, result);
    }

    ChipLogProgress(Zcl, "Initializing CHIP");
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR InitChipStack %ld", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ChipLogProgress(Zcl, "Initializing OpenThread stack");
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR InitThreadStack %ld", err.AsInteger());
    }
#endif

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR SetThreadDeviceType %ld", err.AsInteger());
    }

    ChipLogProgress(Zcl, "Starting event loop task");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR StartEventLoopTask %ld", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ChipLogProgress(Zcl, "Starting thread task");
    err = ThreadStackMgr().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ERROR StartThreadTask %ld", err.AsInteger());
    }
#endif

    PlatformMgr().ScheduleWork(InitApp, 0);

    const int ret = Engine::Root().Init();
    if (!chip::ChipError::IsSuccess(ret))
    {
        ChipLogError(Zcl, "ERROR Shell Init %d", ret);
    }
    RegisterAppShellCommands();
    Engine::Root().RunMainLoop();

    assert(!wiced_rtos_check_for_stack_overflow());
}

void InitApp(intptr_t args)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    // Initial lock state
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

    uint8_t numberOfCredentialsPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, numberOfCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to 5 "
                     "[endpointId=%d]",
                     endpointId);
        numberOfCredentialsPerUser = 5;
    }

    uint16_t numberOfUsers = 0;
    if (!DoorLockServer::Instance().GetNumberOfUserSupported(endpointId, numberOfUsers))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported users when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
                     endpointId);
        numberOfUsers = 10;
    }

    uint8_t numberOfWeekdaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekdaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported weekday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfWeekdaySchedulesPerUser = 10;
    }

    uint8_t numberOfYeardaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYeardaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported yearday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfYeardaySchedulesPerUser = 10;
    }

    uint8_t numberOfHolidaySchedules = 0;
    if (!DoorLockServer::Instance().GetNumberOfHolidaySchedulesSupported(endpointId, numberOfHolidaySchedules))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported holiday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfHolidaySchedules = 10;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    err = LockMgr().Init(state,
                         ParamBuilder()
                             .SetNumberOfUsers(numberOfUsers)
                             .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                             .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                             .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                             .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                             .GetLockParam());

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAConfig::Init();
#endif
}

void ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been initiated");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        syncClusterToButtonAction = true;
    }

    // Action initiated, update the light led
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been initiated");
        wiced_led_manager_disable_led(PLATFORM_LED_2);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been initiated");
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
        ChipLogError(Zcl, "ERR: updating lock state %x", status);
    }
}

void ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been completed");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been completed");
    }

    if (syncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        syncClusterToButtonAction = false;
    }
}
