/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "AppConfig.h"
#include "AppEvent.h"
#include "CHIPProjectConfig.h"
#include "CustomerAppTask.h"
#ifdef SL_MATTER_ENABLE_AWS
#include "MatterAws.h"
#endif // SL_MATTER_ENABLE_AWS
#if defined(ENABLE_CHIP_SHELL)
#include "EventHandlerLibShell.h"
#endif // ENABLE_CHIP_SHELL

#include "LEDWidget.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <assert.h>
#include <cstring>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <platform/CHIPDeviceLayer.h>
#define SYSTEM_STATE_LED 0
#define LOCK_STATE_LED 1

#define APP_FUNCTION_BUTTON 0
#define APP_LOCK_SWITCH 1

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
using namespace ::chip::DeviceLayer::Internal;
using namespace SilabsDoorLock;
using namespace SilabsDoorLock::LockInitParams;
using chip::app::DataModel::MakeNullable;

namespace {

using namespace SilabsDoorLock::Storage;
using namespace SilabsDoorLockConfig::ResourceRanges;

static constexpr uint16_t LockUserInfoSize        = sizeof(LockUserInfo);
static constexpr uint16_t LockCredentialInfoSize  = sizeof(LockCredentialInfo);
static constexpr uint16_t CredentialStructSize    = sizeof(CredentialStruct);
static constexpr uint16_t WeekDayScheduleInfoSize = sizeof(WeekDayScheduleInfo);
static constexpr uint16_t YearDayScheduleInfoSize = sizeof(YearDayScheduleInfo);
static constexpr uint16_t HolidayScheduleInfoSize = sizeof(HolidayScheduleInfo);

LEDWidget sLockLED;
TimerHandle_t sUnlatchTimer;

// ---- Index / type validators ------------------------------------------------

bool IsValidUserIndex(uint16_t userIndex)
{
    return (userIndex < kMaxUsers);
}

bool IsValidCredentialIndex(uint16_t credentialIndex, CredentialTypeEnum type)
{
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex); // 0 is required index for Programming PIN
    }
    // Other credential types are valid at all uint16_t indices; the real limit
    // is enforced by available persistent storage.
    return (credentialIndex < UINT16_MAX);
}

bool IsValidCredentialType(CredentialTypeEnum type)
{
    return (to_underlying(type) < kNumCredentialTypes);
}

bool IsValidWeekdayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxWeekdaySchedulesPerUser);
}

bool IsValidYeardayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxYeardaySchedulesPerUser);
}

bool IsValidHolidayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxHolidaySchedules);
}

// ---- Persistent-storage key builders ---------------------------------------

chip::StorageKeyName LockUserEndpoint(chip::EndpointId endpoint, uint16_t userIndex)
{
    return chip::StorageKeyName::Formatted("g/e/%x/lu/%x", endpoint, userIndex);
}

chip::StorageKeyName LockCredentialEndpoint(chip::EndpointId endpoint, CredentialTypeEnum credentialType, uint16_t credentialIndex)
{
    return chip::StorageKeyName::Formatted("g/e/%x/ct/%x/lc/%x", endpoint, chip::to_underlying(credentialType), credentialIndex);
}

chip::StorageKeyName LockUserCredentialMap(uint16_t userIndex)
{
    return chip::StorageKeyName::Formatted("g/lu/%x/lc", userIndex);
}

chip::StorageKeyName LockUserWeekDayScheduleEndpoint(chip::EndpointId endpoint, uint16_t userIndex, uint16_t scheduleIndex)
{
    return chip::StorageKeyName::Formatted("g/e/%x/lu/%x/lw/%x", endpoint, userIndex, scheduleIndex);
}

chip::StorageKeyName LockUserYearDayScheduleEndpoint(chip::EndpointId endpoint, uint16_t userIndex, uint16_t scheduleIndex)
{
    return chip::StorageKeyName::Formatted("g/e/%x/lu/%x/ly/%x", endpoint, userIndex, scheduleIndex);
}

chip::StorageKeyName LockHolidayScheduleEndpoint(chip::EndpointId endpoint, uint16_t scheduleIndex)
{
    return chip::StorageKeyName::Formatted("g/e/%x/lh/%x", endpoint, scheduleIndex);
}

// ---- Misc helpers ----------------------------------------------------------

const char * LockStateToString(chip::app::Clusters::DoorLock::DlLockState lockState)
{
    using chip::app::Clusters::DoorLock::DlLockState;
    switch (lockState)
    {
    case DlLockState::kNotFullyLocked:
        return "Not Fully Locked";
    case DlLockState::kLocked:
        return "Locked";
    case DlLockState::kUnlocked:
        return "Unlocked";
    case DlLockState::kUnlatched:
        return "Unlatched";
    case DlLockState::kUnknownEnumValue:
        break;
    }
    return "Unknown";
}

void UnlatchTimerCallback(TimerHandle_t xTimer)
{
    (void) xTimer;
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(AppTask::UnlockAfterUnlatch,
                                                                           reinterpret_cast<intptr_t>(nullptr));
}

void CancelUnlatchTimer(void)
{
    if (xTimerStop(sUnlatchTimer, pdMS_TO_TICKS(0)) == pdFAIL)
    {
        SILABS_LOG("sUnlatchTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void StartUnlatchTimer(uint32_t timeoutMs)
{
    if (xTimerIsTimerActive(sUnlatchTimer))
    {
        SILABS_LOG("app timer already started!");
        CancelUnlatchTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ms if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerStart(sUnlatchTimer, pdMS_TO_TICKS(timeoutMs)) != pdPASS)
    {
        SILABS_LOG("sUnlatchTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

} // namespace

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

#if defined(ENABLE_CHIP_SHELL)
    err = RegisterLockEvents();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("RegisterLockEvents() failed");
        appError(err);
    }
#endif // ENABLE_CHIP_SHELL

    // Bring up the lock domain via the CRTP hook so derived AppTaskImpl<>
    // classes can override lock-app initialization.
    err = CustomerAppTask::GetAppTask().InitLock();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("InitLock() failed");
        appError(err);
    }

    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));

    ConfigurationMgr().LogDeviceConfig();

    return err;
}

CHIP_ERROR AppTask::InitLock()
{
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

    CHIP_ERROR err = InitLockDomain(state,
                                    ParamBuilder()
                                        .SetNumberOfUsers(numberOfUsers)
                                        .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                                        .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                                        .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                                        .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                                        .GetLockParam(),
                                    &Server::GetInstance().GetPersistentStorage());
    ReturnErrorOnFailure(err);

    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(state.Value() == DlLockState::kUnlocked);

    sUnlatchTimer = xTimerCreate("UnlatchTimer", pdMS_TO_TICKS(UNLATCH_TIME_MS), pdFALSE, (void *) 0, UnlatchTimerCallback);

    // Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(state.Value() != DlLockState::kUnlocked);
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = AppTask::GetAppTask().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    AppTask::GetAppTask().StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            AppTask::GetAppTask().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::LockButtonActionHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "LockButtonActionHandler: unexpected event type %u", aEvent->Type);
        return;
    }

    LockAction action = AppTask::GetAppTask().NextState() ? LockAction::kLock : LockAction::kUnlock;
    if (!AppTask::GetAppTask().InitiateLockAction(AppEvent::kEventType_Button, action))
    {
        SILABS_LOG("Action is already in progress or active.");
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_LOCK_SWITCH && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = &CustomerAppTask::LockButtonActionHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        AppTask::GetAppTask().PostEvent(&button_event);
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    bool unlocked        = AppTask::GetAppTask().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    // write the new lock value
    Protocols::InteractionModel::Status status = DoorLockServer::Instance().SetLockState(1, newState)
        ? Protocols::InteractionModel::Status::Success
        : Protocols::InteractionModel::Status::Failure;

    if (status != Protocols::InteractionModel::Status::Success)
    {
        SILABS_LOG("ERR: updating lock state %x", to_underlying(status));
    }
}

void AppTask::DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    using namespace ::chip::app::Clusters;

    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == DoorLock::Id && attributeId == DoorLock::Attributes::LockState::Id)
    {
        [[maybe_unused]] DoorLock::DlLockState lockState = *(reinterpret_cast<DoorLock::DlLockState *>(value));
        ChipLogProgress(Zcl, "Door lock cluster: " ChipLogFormatMEI " state %d", ChipLogValueMEI(clusterId),
                        to_underlying(lockState));
#ifdef SL_MATTER_ENABLE_AWS
        MatterAwsSendMsg("lock/state", (const char *) (lockState == DoorLock::DlLockState::kLocked ? "lock" : "unlock"));
#endif // SL_MATTER_ENABLE_AWS
    }
}

// AppTask::DM* — default DoorLock cluster data-model behavior. Customers
// override per-callback behavior via `AppTaskImpl<>::DM*Impl()` hooks.

bool AppTask::DMDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                          const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                          OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    bool status = SetLockState(endpointId, fabricIdx, nodeId, DlLockState::kLocked, pinCode, err);
    if (status == true)
    {
        InitiateLockAction(AppEvent::kEventType_Lock, AppTask::LockAction::kLock);
    }
    return status;
}

bool AppTask::DMDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Unlock Command endpoint=%d", endpointId);
    bool status = Unlock(endpointId, fabricIdx, nodeId, pinCode, err);
    if (status == true)
    {
        if (DoorLockServer::Instance().SupportsUnbolt(endpointId))
        {
            InitiateLockAction(AppEvent::kEventType_Lock, AppTask::LockAction::kUnlatch);
        }
        else
        {
            InitiateLockAction(AppEvent::kEventType_Lock, AppTask::LockAction::kUnlock);
        }
    }
    return status;
}

bool AppTask::DMDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Unbolt Command endpoint=%d", endpointId);
    bool status = Unlock(endpointId, fabricIdx, nodeId, pinCode, err);
    if (status == true)
    {
        InitiateLockAction(AppEvent::kEventType_Lock, AppTask::LockAction::kUnlock);
    }
    return status;
}

void AppTask::DMDoorLockOnAutoRelock(chip::EndpointId /*endpointId*/)
{
    InitiateLockAction(AppEvent::kEventType_Lock, AppTask::LockAction::kLock);
}

// emberAfPluginDoorLock* — weak DoorLock plugin callbacks. CRTP forwarders
// into `CustomerAppTask::GetAppTask().DM*` so customers can intercept via
// `AppTaskImpl<>::DM*Impl()`.

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    return CustomerAppTask::GetAppTask().DMDoorLockOnDoorLockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return CustomerAppTask::GetAppTask().DMDoorLockOnDoorUnlockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return CustomerAppTask::GetAppTask().DMDoorLockOnDoorUnboltCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return CustomerAppTask::GetAppTask().DMDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                        chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                        CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
{
    return CustomerAppTask::GetAppTask().DMDoorLockSetCredential(endpointId, credentialIndex, creator, modifier, credentialStatus,
                                                                 credentialType, credentialData);
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return CustomerAppTask::GetAppTask().DMDoorLockGetUser(endpointId, userIndex, user);
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                                  const CredentialStruct * credentials, size_t totalCredentials)
{
    return CustomerAppTask::GetAppTask().DMDoorLockSetUser(endpointId, userIndex, creator, modifier, userName, uniqueId,
                                                           userStatus, usertype, credentialRule, credentials, totalCredentials);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    return CustomerAppTask::GetAppTask().DMDoorLockGetWeekDaySchedule(endpointId, weekdayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return CustomerAppTask::GetAppTask().DMDoorLockGetYearDaySchedule(endpointId, yearDayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule)
{
    return CustomerAppTask::GetAppTask().DMDoorLockGetHolidaySchedule(endpointId, holidayIndex, holidaySchedule);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute)
{
    return CustomerAppTask::GetAppTask().DMDoorLockSetWeekDaySchedule(endpointId, weekdayIndex, userIndex, status, daysMask,
                                                                      startHour, startMinute, endHour, endMinute);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    return CustomerAppTask::GetAppTask().DMDoorLockSetYearDaySchedule(endpointId, yearDayIndex, userIndex, status, localStartTime,
                                                                      localEndTime);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    return CustomerAppTask::GetAppTask().DMDoorLockSetHolidaySchedule(endpointId, holidayIndex, status, localStartTime,
                                                                      localEndTime, operatingMode);
}

void emberAfPluginDoorLockOnAutoRelock(chip::EndpointId endpointId)
{
    CustomerAppTask::GetAppTask().DMDoorLockOnAutoRelock(endpointId);
}

CHIP_ERROR AppTask::InitLockDomain(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam,
                             chip::PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;

    mLockParams = lockParam;

    if (mLockParams.numberOfUsers > kMaxUsers)
    {
        SILABS_LOG("Max number of users is greater than %d, the maximum amount of users currently supported on this platform",
                   kMaxUsers);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (mLockParams.numberOfCredentialsPerUser > kMaxCredentialsPerUser)
    {
        SILABS_LOG("Max number of credentials per user is greater than %d, the maximum amount of users currently supported on this "
                   "platform",
                   kMaxCredentialsPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (mLockParams.numberOfWeekdaySchedulesPerUser > kMaxWeekdaySchedulesPerUser)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxWeekdaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (mLockParams.numberOfYeardaySchedulesPerUser > kMaxYeardaySchedulesPerUser)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxYeardaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (mLockParams.numberOfHolidaySchedules > kMaxHolidaySchedules)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxHolidaySchedules);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    // Migrate legacy configuration, if needed
    MigrateLockConfig(lockParam);

    // Create cmsis os sw timer for lock timer.
    mLockTimer = osTimerNew(TimerEventHandler, // timer callback handler
                            osTimerOnce,       // no timer reload (one-shot timer)
                            (void *) this,     // pass the app task obj context
                            NULL               // No osTimerAttr_t to provide.
    );

    if (mLockTimer == NULL)
    {
        SILABS_LOG("mLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    if (state.Value() == DlLockState::kUnlocked)
        mLockActuatorState = LockActuatorState::kUnlockCompleted;
    else
        mLockActuatorState = LockActuatorState::kLockCompleted;

    return CHIP_NO_ERROR;
}

bool AppTask::NextState()
{
    return (mLockActuatorState == LockActuatorState::kUnlockCompleted);
}

bool AppTask::InitiateLockAction(int32_t aActor, LockAction aAction)
{
    bool action_initiated = false;
    LockActuatorState new_state;

    // Initiate Turn Lock/Unlock Action only when the previous one is complete.
    if ((mLockActuatorState == LockActuatorState::kLockCompleted || mLockActuatorState == LockActuatorState::kUnlatchCompleted) &&
        (aAction == LockAction::kUnlock))
    {
        action_initiated = true;
        new_state        = LockActuatorState::kUnlockInitiated;
    }
    else if ((mLockActuatorState == LockActuatorState::kLockCompleted || mLockActuatorState == LockActuatorState::kUnlockCompleted) &&
             (aAction == LockAction::kUnlatch))
    {
        action_initiated = true;
        new_state        = LockActuatorState::kUnlatchInitiated;
    }
    else if (mLockActuatorState == LockActuatorState::kUnlockCompleted && aAction == LockAction::kLock)
    {
        action_initiated = true;
        new_state        = LockActuatorState::kLockInitiated;
    }

    if (action_initiated)
    {
        if (osTimerStart(mLockTimer, pdMS_TO_TICKS(ACTUATOR_MOVEMENT_PERIOS_MS)) != osOK)
        {
            SILABS_LOG("mLockTimer timer start() failed");
            appError(APP_ERROR_START_TIMER_FAILED);
        }

        mLockActuatorState = new_state;

        // Inlined legacy ActionInitiated callback body: update LED/LCD and flag
        // button-driven actions for later cluster-state sync.
        if (aAction == LockAction::kUnlock || aAction == LockAction::kLock)
        {
            bool locked = (aAction == LockAction::kLock);
            SILABS_LOG("%s Action has been initiated", locked ? "Lock" : "Unlock");
            sLockLED.Set(!locked);

#ifdef DISPLAY_ENABLED
            AppTask::GetAppTask().GetLCD().WriteDemoUI(locked);
#endif // DISPLAY_ENABLED
        }
        else if (aAction == LockAction::kUnlatch)
        {
            SILABS_LOG("Unlatch Action has been initiated");
        }

        if (aActor == AppEvent::kEventType_Button)
        {
            mSyncClusterToButtonAction = true;
        }
    }

    return action_initiated;
}

void AppTask::TimerEventHandler(void * timerCbArg)
{
    // The callback argument is the light obj context assigned at timer creation.
    AppTask * lock = static_cast<AppTask *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once mLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;
    event.Handler            = &AppTask::ActuatorMovementTimerEventHandler;
    AppTask::GetAppTask().PostEvent(&event);
}
void AppTask::UnlockAfterUnlatch(intptr_t /* context */)
{
    AppTask & self = AppTask::GetAppTask();

    // write the new lock value
    bool succes = false;
    if (self.mUnlatchContext.mEndpointId != kInvalidEndpointId)
    {
        Optional<chip::ByteSpan> pin = (self.mUnlatchContext.mPinLength)
            ? MakeOptional(chip::ByteSpan(self.mUnlatchContext.mPinBuffer, self.mUnlatchContext.mPinLength))
            : Optional<chip::ByteSpan>::Missing();
        succes = self.SetLockState(self.mUnlatchContext.mEndpointId, self.mUnlatchContext.mFabricIdx,
                                   self.mUnlatchContext.mNodeId, DlLockState::kUnlocked, pin, self.mUnlatchContext.mErr);
    }

    if (!succes)
    {
        SILABS_LOG("Failed to update the lock state after Unlatch");
    }

    self.InitiateLockAction(AppEvent::kEventType_Lock, LockAction::kUnlock);
}

void AppTask::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    LockAction actionCompleted = LockAction::kInvalid;

    AppTask * lock = static_cast<AppTask *>(aEvent->TimerEvent.Context);

    if (lock->mLockActuatorState == LockActuatorState::kLockInitiated)
    {
        lock->mLockActuatorState = LockActuatorState::kLockCompleted;
        actionCompleted          = LockAction::kLock;
    }
    else if (lock->mLockActuatorState == LockActuatorState::kUnlatchInitiated)
    {
        lock->mLockActuatorState = LockActuatorState::kUnlatchCompleted;
        actionCompleted          = LockAction::kUnlatch;
    }
    else if (lock->mLockActuatorState == LockActuatorState::kUnlockInitiated)
    {
        lock->mLockActuatorState = LockActuatorState::kUnlockCompleted;
        actionCompleted          = LockAction::kUnlock;
    }

    if (actionCompleted != LockAction::kInvalid)
    {
        // Inlined legacy ActionCompleted callback body.
        if (actionCompleted == LockAction::kLock)
        {
            SILABS_LOG("Lock Action has been completed");
        }
        else if (actionCompleted == LockAction::kUnlatch)
        {
            SILABS_LOG("Unlatch Action has been completed");
            StartUnlatchTimer(UNLATCH_TIME_MS);
        }
        else if (actionCompleted == LockAction::kUnlock)
        {
            SILABS_LOG("Unlock Action has been completed");
        }

        if (lock->mSyncClusterToButtonAction)
        {
            TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState,
                                                                                   reinterpret_cast<intptr_t>(nullptr));
            lock->mSyncClusterToButtonAction = false;
        }
    }
}

bool AppTask::Unlock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                         const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    if (DoorLockServer::Instance().SupportsUnbolt(endpointId))
    {
        // TODO: Our current implementation does not support multiple endpoint. This needs to be fixed in the future.
        if (endpointId != mUnlatchContext.mEndpointId)
        {
            // If we get a request to unlock on a different endpoint while the current endpoint is in the middle of an action,
            // we return false for now. This needs to be fixed in the future.
            if (mLockActuatorState != LockActuatorState::kUnlockCompleted && mLockActuatorState != LockActuatorState::kLockCompleted)
            {
                ChipLogError(Zcl, "Cannot unlock while unlatch on another endpoint is in progress on another endpoint");
                return false;
            }
            else
            {
                mUnlatchContext.Update(endpointId, fabricIdx, nodeId, pin, err);
                return SetLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlatched, pin, err);
            }
        }
        else
        {
            return SetLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlatched, pin, err);
        }
    }
    return SetLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlocked, pin, err);
}

bool AppTask::DMDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    ChipLogProgress(Zcl, "Door Lock App: AppTask::DMDoorLockGetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    // Get User struct from nvm3
    chip::StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);

    uint16_t size = LockUserInfoSize;

    LockUserInfo userInStorage;

    error = mStorage->SyncGetKeyValue(userKey.KeyName(), &userInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == LockUserInfoSize, false);

    // If no data is found at user key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        user.userStatus = UserStatusEnum::kAvailable;

        ChipLogDetail(Zcl, "No user data found");
        return true;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        user.userStatus = userInStorage.userStatus;

        if (userInStorage.userStatus == UserStatusEnum::kAvailable)
        {
            ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d]", endpointId);
            return true;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return false;
    }

    VerifyOrReturnValue(userInStorage.currentCredentialCount <= kMaxCredentialsPerUser, false);

    VerifyOrReturnValue(userInStorage.userNameSize <= user.userName.size(), false);

    // Copy username data from storage to the output parameter
    memmove(user.userName.data(), userInStorage.userName, userInStorage.userNameSize);

    // Resize Span to match the actual username size retrieved from storage
    user.userName.reduce_size(userInStorage.userNameSize);

    user.userUniqueId   = userInStorage.userUniqueId;
    user.userType       = userInStorage.userType;
    user.credentialRule = userInStorage.credentialRule;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = userInStorage.createdBy;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = userInStorage.lastModifiedBy;

    // Get credential struct from nvm3
    chip::StorageKeyName credentialKey = LockUserCredentialMap(userIndex);

    uint16_t credentialSize = static_cast<uint16_t>(CredentialStructSize * userInStorage.currentCredentialCount);

    CredentialStruct credentials[kMaxCredentialsPerUser];

    error = mStorage->SyncGetKeyValue(credentialKey.KeyName(), credentials, credentialSize);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(credentialSize == static_cast<uint16_t>(CredentialStructSize * userInStorage.currentCredentialCount),
                        false);

    // If KVS read was successful
    if (error == CHIP_NO_ERROR)
    {

        // Copy credentials attached to user from storage to the output parameter
        memmove(user.credentials.data(), credentials, credentialSize);

        // Resize Span to match the actual size of credentials attached to user
        user.credentials.reduce_size(userInStorage.currentCredentialCount);
    }
    else if (error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "Error reading KVS key");
        return false;
    }

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,name=\"%s\",credentialsCount=%u,uniqueId=%lx,type=%u,credentialRule=%u,"
                  "createdBy=%d,lastModifiedBy=%d]",
                  endpointId, NullTerminated(user.userName).c_str(), user.credentials.size(), user.userUniqueId,
                  to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

    return true;
}

bool AppTask::DMDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                          CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    if (userName.size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ChipLogError(Zcl, "Cannot set user - user name is too long [endpoint=%d,index=%d]", endpointId, userIndex);
        return false;
    }

    if (totalCredentials > mLockParams.numberOfCredentialsPerUser)
    {
        ChipLogError(Zcl, "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,totalCredentials=%u]",
                     endpointId, userIndex, totalCredentials);
        return false;
    }

    LockUserInfo userInStorage;

    memmove(userInStorage.userName, userName.data(), userName.size());
    userInStorage.userNameSize           = userName.size();
    userInStorage.userUniqueId           = uniqueId;
    userInStorage.userStatus             = userStatus;
    userInStorage.userType               = usertype;
    userInStorage.credentialRule         = credentialRule;
    userInStorage.lastModifiedBy         = modifier;
    userInStorage.createdBy              = creator;
    userInStorage.currentCredentialCount = totalCredentials;

    // Save credential struct in nvm3
    chip::StorageKeyName credentialKey = LockUserCredentialMap(userIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(credentialKey.KeyName(), credentials, CredentialStructSize * totalCredentials) ==
                            CHIP_NO_ERROR,
                        false);

    // Save user in nvm3
    chip::StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(userKey.KeyName(), &userInStorage, LockUserInfoSize) == CHIP_NO_ERROR, false);

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool AppTask::DMDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType), false);

    ChipLogProgress(Zcl, "Lock App: AppTask::DMDoorLockGetCredential [credentialType=%u], credentialIndex=%d",
                    to_underlying(credentialType), credentialIndex);

    chip::StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);

    LockCredentialInfo credentialInStorage;

    uint16_t size = LockCredentialInfoSize;

    error = mStorage->SyncGetKeyValue(key.KeyName(), &credentialInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == LockCredentialInfoSize, false);

    // If no data is found at credential key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No credentials found
        credential.status = DlCredentialStatus::kAvailable;

        return true;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        credential.status = credentialInStorage.status;
        ChipLogDetail(Zcl, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);
        if (DlCredentialStatus::kAvailable == credential.status)
        {
            ChipLogDetail(Zcl, "Found unoccupied credential ");
            return true;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading KVS key");
        return false;
    }

    VerifyOrReturnValue(credentialInStorage.credentialDataSize <= credential.credentialData.size(), false);

    // Copy credential data from storage to the output parameter
    memmove(credential.credentialData.data(), credentialInStorage.credentialData, credentialInStorage.credentialDataSize);

    // Resize Span to match the actual size of the credential data retrieved from storage
    credential.credentialData.reduce_size(credentialInStorage.credentialDataSize);

    credential.credentialType = credentialInStorage.credentialType;
    credential.createdBy      = credentialInStorage.createdBy;
    credential.lastModifiedBy = credentialInStorage.lastModifiedBy;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.modificationSource = DlAssetSource::kMatterIM;

    ChipLogDetail(Zcl, "Found occupied credential");

    return true;
}

bool AppTask::DMDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                const chip::ByteSpan & credentialData)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType), false);

    VerifyOrReturnValue(credentialData.size() <= kMaxCredentialSize, false);

    ChipLogProgress(Zcl,
                    "Door Lock App: AppTask::DMDoorLockSetCredential "
                    "[credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%d,modifier=%d]",
                    to_underlying(credentialStatus), to_underlying(credentialType), credentialData.size(), creator, modifier);

    LockCredentialInfo credentialInStorage;

    credentialInStorage.status             = credentialStatus;
    credentialInStorage.credentialType     = credentialType;
    credentialInStorage.createdBy          = creator;
    credentialInStorage.lastModifiedBy     = modifier;
    credentialInStorage.credentialDataSize = credentialData.size();

    // Copy credential data to the storage struct
    memmove(credentialInStorage.credentialData, credentialData.data(), credentialInStorage.credentialDataSize);

    chip::StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);

    error = mStorage->SyncSetKeyValue(key.KeyName(), &credentialInStorage, LockCredentialInfoSize);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return false;
    }

    ChipLogProgress(Zcl, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));

    return true;
}

DlStatus AppTask::DMDoorLockGetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    CHIP_ERROR error;

    WeekDayScheduleInfo weekDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);

    uint16_t size = WeekDayScheduleInfoSize; // Create a non-const variable

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == WeekDayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        if (weekDayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = weekDayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                         uint8_t endHour, uint8_t endMinute)
{

    WeekDayScheduleInfo weekDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    weekDayScheduleInStorage.schedule.daysMask    = daysMask;
    weekDayScheduleInStorage.schedule.startHour   = startHour;
    weekDayScheduleInStorage.schedule.startMinute = startMinute;
    weekDayScheduleInStorage.schedule.endHour     = endHour;
    weekDayScheduleInStorage.schedule.endMinute   = endMinute;
    weekDayScheduleInStorage.status               = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, WeekDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockGetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    CHIP_ERROR error;

    YearDayScheduleInfo yearDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);

    uint16_t size = YearDayScheduleInfoSize;

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == YearDayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {

        if (yearDayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = yearDayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{

    YearDayScheduleInfo yearDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    yearDayScheduleInStorage.schedule.localStartTime = localStartTime;
    yearDayScheduleInStorage.schedule.localEndTime   = localEndTime;
    yearDayScheduleInStorage.status                  = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, YearDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockGetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                         EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    CHIP_ERROR error;

    HolidayScheduleInfo holidayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);

    uint16_t size = HolidayScheduleInfoSize;

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == HolidayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {

        if (holidayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = holidayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                         uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{

    HolidayScheduleInfo holidayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    holidayScheduleInStorage.schedule.localStartTime = localStartTime;
    holidayScheduleInStorage.schedule.localEndTime   = localEndTime;
    holidayScheduleInStorage.schedule.operatingMode  = operatingMode;
    holidayScheduleInStorage.status                  = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, HolidayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

bool AppTask::SetLockState(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                               const Nullable<chip::NodeId> & nodeId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                               OperationErrorEnum & err)
{

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    // Assume pin is required until told otherwise
    bool requirePin = true;
    chip::app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);

    // If a pin code is not given
    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified [endpointId=%d]", endpointId);

        // If a pin code is not required
        if (!requirePin)
        {
            ChipLogDetail(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", LockStateToString(lockState),
                          endpointId);

            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, NullNullable, NullNullable,
                                                    fabricIdx, nodeId);

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);

        return false;
    }

    // Check all pin codes associated to all users to see if this pin code exists
    for (uint32_t userIndex = 1; userIndex <= kMaxUsers; userIndex++)
    {
        EmberAfPluginDoorLockUserInfo user;

        if (!DMDoorLockGetUser(endpointId, userIndex, user))
        {
            ChipLogError(Zcl, "Unable to get the user - internal error [endpointId=%d,userIndex=%lu]", endpointId, userIndex);
            return false;
        }

        if (user.userStatus == UserStatusEnum::kOccupiedEnabled)
        {
            // Loop through each credential attached to the user
            for (auto & userCredential : user.credentials)
            {
                // If the current credential is a pin type, then check it against pin input. Otherwise ignore
                if (userCredential.credentialType == CredentialTypeEnum::kPin)
                {
                    EmberAfPluginDoorLockCredentialInfo credential;

                    if (!DMDoorLockGetCredential(endpointId, userCredential.credentialIndex, userCredential.credentialType, credential))
                    {
                        ChipLogError(Zcl,
                                     "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                                     endpointId, to_underlying(userCredential.credentialType), userCredential.credentialIndex);
                        return false;
                    }

                    if (credential.status == DlCredentialStatus::kOccupied)
                    {

                        // See if credential retrieved from storage matches the provided PIN
                        if (credential.credentialData.data_equal(pin.Value()))
                        {
                            ChipLogDetail(Zcl,
                                          "Lock App: specified PIN code was found in the database, setting lock state to "
                                          "\"%s\" [endpointId=%d]",
                                          LockStateToString(lockState), endpointId);

                            LockOpCredentials userCred[] = { { CredentialTypeEnum::kPin, userCredential.credentialIndex } };
                            auto userCredentials         = MakeNullable<List<const LockOpCredentials>>(userCred);

                            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, userIndex,
                                                                    userCredentials, fabricIdx, nodeId);

                            return true;
                        }
                    }
                }
            }
        }
    }

    ChipLogDetail(Zcl,
                  "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                  "[endpointId=%d]",
                  LockStateToString(lockState), endpointId);

    err = OperationErrorEnum::kInvalidCredential;
    return false;
}
