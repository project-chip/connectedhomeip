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
using chip::app::DataModel::MakeNullable;
using chip::Protocols::InteractionModel::Status;
using namespace chip::TLV;

namespace {

using namespace SilabsDoorLockConfig::ResourceRanges;

CustomerAppTask & appInstance()
{
    return CustomerAppTask::GetAppTask();
}

LEDWidget sLockLED;
osTimerId_t sUnlatchTimer;

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

StorageKeyName LockUserEndpoint(EndpointId endpoint, uint16_t userIndex)
{
    return StorageKeyName::Formatted("g/e/%x/lu/%x", endpoint, userIndex);
}

StorageKeyName LockCredentialEndpoint(EndpointId endpoint, CredentialTypeEnum credentialType, uint16_t credentialIndex)
{
    return StorageKeyName::Formatted("g/e/%x/ct/%x/lc/%x", endpoint, to_underlying(credentialType), credentialIndex);
}

StorageKeyName LockUserCredentialMap(uint16_t userIndex)
{
    return StorageKeyName::Formatted("g/lu/%x/lc", userIndex);
}

StorageKeyName LockUserWeekDayScheduleEndpoint(EndpointId endpoint, uint16_t userIndex, uint16_t scheduleIndex)
{
    return StorageKeyName::Formatted("g/e/%x/lu/%x/lw/%x", endpoint, userIndex, scheduleIndex);
}

StorageKeyName LockUserYearDayScheduleEndpoint(EndpointId endpoint, uint16_t userIndex, uint16_t scheduleIndex)
{
    return StorageKeyName::Formatted("g/e/%x/lu/%x/ly/%x", endpoint, userIndex, scheduleIndex);
}

StorageKeyName LockHolidayScheduleEndpoint(EndpointId endpoint, uint16_t scheduleIndex)
{
    return StorageKeyName::Formatted("g/e/%x/lh/%x", endpoint, scheduleIndex);
}

// ---- Misc helpers ----------------------------------------------------------

void CancelUnlatchTimer(void)
{
    if (osTimerStop(sUnlatchTimer) != osOK)
    {
        ChipLogError(NotSpecified, "sUnlatchTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void StartUnlatchTimer(uint32_t timeoutMs)
{
    if (osTimerIsRunning(sUnlatchTimer))
    {
        ChipLogDetail(NotSpecified, "app timer already started!");
        CancelUnlatchTimer();
    }

    if (osTimerStart(sUnlatchTimer, pdMS_TO_TICKS(timeoutMs)) != osOK)
    {
        ChipLogError(NotSpecified, "sUnlatchTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

} // namespace

AppTask::LockRequest AppTask::sStagedLockRequest{};
bool AppTask::sStagedLockRequestValid      = false;
osMutexId_t AppTask::sLockSharedStateMutex = nullptr;

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

#if defined(ENABLE_CHIP_SHELL)
    err = RegisterLockEvents();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "RegisterLockEvents() failed");
        appError(err);
    }
#endif // ENABLE_CHIP_SHELL

    // Bring up the lock domain via the CRTP hook so derived AppTaskImpl<>
    // classes can override lock-app initialization.
    err = appInstance().InitLock();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "InitLock() failed");
        appError(err);
    }

    DlLockState bootState =
        (appInstance().GetActuatorState() == LockActuatorState::kUnlockCompleted) ? DlLockState::kUnlocked : DlLockState::kLocked;
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState,
                                                                     static_cast<intptr_t>(to_underlying(bootState)));

    ConfigurationMgr().LogDeviceConfig();

    return err;
}

CHIP_ERROR AppTask::InitLock()
{
    app::DataModel::Nullable<DlLockState> state;
    EndpointId endpointId = EndpointId(LOCK_ENDPOINT);
    DeviceLayer::PlatformMgr().LockChipStack();
    app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

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

    DeviceLayer::PlatformMgr().UnlockChipStack();

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

    osTimerAttr_t unlatchTimerAttr = { .name = "UnlatchTimer" };
    sUnlatchTimer                  = osTimerNew(&CustomerAppTask::UnlatchCallback, osTimerOnce, nullptr, &unlatchTimerAttr);

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

    CHIP_ERROR err = appInstance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    appInstance().StartStatusLEDTimer();
#endif

    ChipLogProgress(NotSpecified, "App Task started");

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            appInstance().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::LockButtonActionHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_Button,
                   ChipLogError(NotSpecified, "LockButtonActionHandler: unexpected event type %u", aEvent->Type));

    LockRequest req;
    req.endpointId = LOCK_ENDPOINT;
    req.action =
        (appInstance().GetActuatorState() == LockActuatorState::kUnlockCompleted) ? LockAction::kLock : LockAction::kUnlock;
    req.isButtonAction = true;

    appInstance().HandleLockRequestOnAppTask(req);
}

void AppTask::PostLockActionEvent(int32_t actor, LockAction action)
{
    AppEvent event         = {};
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = actor;
    event.LockEvent.Action = static_cast<uint8_t>(action);
    event.Handler          = &CustomerAppTask::LockActionEventHandler;
    appInstance().PostEvent(&event);
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_Lock,
                   ChipLogError(NotSpecified, "LockActionEventHandler: unexpected event type %u", aEvent->Type));

    const LockAction action = static_cast<LockAction>(aEvent->LockEvent.Action);
    if (!appInstance().InitiateLockAction(action, aEvent->LockEvent.Actor == AppEvent::kEventType_Button))
    {
        ChipLogDetail(NotSpecified, "Action is already in progress or active.");
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
        appInstance().PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        appInstance().PostEvent(&button_event);
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    using DlLockStateUnderlying = std::underlying_type_t<DlLockState>;
    DlLockState newState        = static_cast<DlLockState>(static_cast<DlLockStateUnderlying>(context));

    Status status = DoorLockServer::Instance().SetLockState(LOCK_ENDPOINT, newState, OperationSourceEnum::kManual)
        ? Status::Success
        : Status::Failure;
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "ERR: updating lock state %x", to_underlying(status));
    }
}

void AppTask::DMPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    using namespace ::chip::app::Clusters;

    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
    if (clusterId == DoorLock::Id && attributeId == DoorLock::Attributes::LockState::Id)
    {
        [[maybe_unused]] DlLockState lockState = *(reinterpret_cast<DlLockState *>(value));
        ChipLogDetail(Zcl, "Door lock cluster: " ChipLogFormatMEI " state %d", ChipLogValueMEI(clusterId),
                      to_underlying(lockState));
#ifdef SL_MATTER_ENABLE_AWS
        MatterAwsSendMsg("lock/state", (const char *) (lockState == DlLockState::kLocked ? "lock" : "unlock"));
#endif // SL_MATTER_ENABLE_AWS
    }
}

bool AppTask::DMDoorLockOnDoorLockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                          const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                          OperationErrorEnum & err)
{
    ChipLogDetail(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    VerifyOrReturnValue(endpointId == LOCK_ENDPOINT, false,
                        ChipLogError(Zcl, "Door Lock App: rejecting command on unsupported endpoint %d (only %d supported)",
                                     endpointId, LOCK_ENDPOINT);
                        err = OperationErrorEnum::kUnspecified);

    Nullable<uint16_t> userIndex;
    LockOpCredentials cred{};
    bool hasCred = false;
    VerifyOrReturnValue(appInstance().ValidatePin(endpointId, pinCode, userIndex, cred, hasCred, err), false);

    LockRequest req;
    req.endpointId         = endpointId;
    req.action             = LockAction::kLock;
    req.targetClusterState = DlLockState::kLocked;
    req.fabricIdx          = fabricIdx;
    req.nodeId             = nodeId;
    req.userIndex          = userIndex;
    req.credential         = cred;
    req.hasCredential      = hasCred;
    req.isUnboltUnlatch    = false;
    req.isButtonAction     = false;
    EnqueueLockRequest(req);
    return true;
}

bool AppTask::DMDoorLockOnDoorUnlockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                            const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    ChipLogDetail(Zcl, "Door Lock App: Unlock Command endpoint=%d", endpointId);
    VerifyOrReturnValue(endpointId == LOCK_ENDPOINT, false,
                        ChipLogError(Zcl, "Door Lock App: rejecting command on unsupported endpoint %d (only %d supported)",
                                     endpointId, LOCK_ENDPOINT);
                        err = OperationErrorEnum::kUnspecified);
    const bool supportsUnbolt = DoorLockServer::Instance().SupportsUnbolt(endpointId);
    Nullable<uint16_t> userIndex;
    LockOpCredentials cred{};
    bool hasCred = false;
    VerifyOrReturnValue(appInstance().ValidatePin(endpointId, pinCode, userIndex, cred, hasCred, err), false);

    LockRequest req;
    req.endpointId         = endpointId;
    req.action             = supportsUnbolt ? LockAction::kUnlatch : LockAction::kUnlock;
    req.targetClusterState = supportsUnbolt ? DlLockState::kUnlatched : DlLockState::kUnlocked;
    req.fabricIdx          = fabricIdx;
    req.nodeId             = nodeId;
    req.userIndex          = userIndex;
    req.credential         = cred;
    req.hasCredential      = hasCred;
    req.isUnboltUnlatch    = supportsUnbolt;
    req.isButtonAction     = false;
    EnqueueLockRequest(req);
    return true;
}

bool AppTask::DMDoorLockOnDoorUnboltCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                            const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    ChipLogDetail(Zcl, "Door Lock App: Unbolt Command endpoint=%d", endpointId);
    VerifyOrReturnValue(endpointId == LOCK_ENDPOINT, false,
                        ChipLogError(Zcl, "Door Lock App: rejecting command on unsupported endpoint %d (only %d supported)",
                                     endpointId, LOCK_ENDPOINT);
                        err = OperationErrorEnum::kUnspecified);

    Nullable<uint16_t> userIndex;
    LockOpCredentials cred{};
    bool hasCred = false;
    VerifyOrReturnValue(appInstance().ValidatePin(endpointId, pinCode, userIndex, cred, hasCred, err), false);

    LockRequest req;
    req.endpointId         = endpointId;
    req.action             = LockAction::kUnlock;
    req.targetClusterState = DlLockState::kUnlocked;
    req.fabricIdx          = fabricIdx;
    req.nodeId             = nodeId;
    req.userIndex          = userIndex;
    req.credential         = cred;
    req.hasCredential      = hasCred;
    req.isUnboltUnlatch    = false;
    req.isButtonAction     = false;
    EnqueueLockRequest(req);
    return true;
}

void AppTask::DMDoorLockOnAutoRelock(EndpointId /*endpointId*/)
{
    PostLockActionEvent(AppEvent::kEventType_Lock, AppTask::LockAction::kLock);
}

// emberAfPluginDoorLock* — weak DoorLock plugin callbacks. CRTP forwarders
// into `appInstance().DM*` so customers can intercept via
// `AppTaskImpl<>::DM*Impl()`.

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                            const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    return appInstance().DMDoorLockOnDoorLockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                              const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return appInstance().DMDoorLockOnDoorUnlockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockOnDoorUnboltCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                              const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return appInstance().DMDoorLockOnDoorUnboltCommand(endpointId, fabricIdx, nodeId, pinCode, err);
}

bool emberAfPluginDoorLockGetCredential(EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return appInstance().DMDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(EndpointId endpointId, uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                        DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                        const ByteSpan & credentialData)
{
    return appInstance().DMDoorLockSetCredential(endpointId, credentialIndex, creator, modifier, credentialStatus, credentialType,
                                                 credentialData);
}

bool emberAfPluginDoorLockGetUser(EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return appInstance().DMDoorLockGetUser(endpointId, userIndex, user);
}

bool emberAfPluginDoorLockSetUser(EndpointId endpointId, uint16_t userIndex, FabricIndex creator, FabricIndex modifier,
                                  const CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                                  CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    return appInstance().DMDoorLockSetUser(endpointId, userIndex, creator, modifier, userName, uniqueId, userStatus, usertype,
                                           credentialRule, credentials, totalCredentials);
}

DlStatus emberAfPluginDoorLockGetSchedule(EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    return appInstance().DMDoorLockGetWeekDaySchedule(endpointId, weekdayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockGetSchedule(EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return appInstance().DMDoorLockGetYearDaySchedule(endpointId, yearDayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockGetSchedule(EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule)
{
    return appInstance().DMDoorLockGetHolidaySchedule(endpointId, holidayIndex, holidaySchedule);
}

DlStatus emberAfPluginDoorLockSetSchedule(EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
                                          DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                          uint8_t endMinute)
{
    return appInstance().DMDoorLockSetWeekDaySchedule(endpointId, weekdayIndex, userIndex, status, daysMask, startHour, startMinute,
                                                      endHour, endMinute);
}

DlStatus emberAfPluginDoorLockSetSchedule(EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime)
{
    return appInstance().DMDoorLockSetYearDaySchedule(endpointId, yearDayIndex, userIndex, status, localStartTime, localEndTime);
}

DlStatus emberAfPluginDoorLockSetSchedule(EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    return appInstance().DMDoorLockSetHolidaySchedule(endpointId, holidayIndex, status, localStartTime, localEndTime,
                                                      operatingMode);
}

void emberAfPluginDoorLockOnAutoRelock(EndpointId endpointId)
{
    appInstance().DMDoorLockOnAutoRelock(endpointId);
}

CHIP_ERROR AppTask::InitLockDomain(app::DataModel::Nullable<DlLockState> state, LockParam lockParam,
                                   PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;

    mLockParams = lockParam;
    VerifyOrReturnError(
        mLockParams.numberOfUsers <= kMaxUsers, APP_ERROR_ALLOCATION_FAILED,
        ChipLogError(Zcl,
                     "Max number of users is greater than %d, the maximum amount of users currently supported on this platform",
                     kMaxUsers));
    VerifyOrReturnError(
        mLockParams.numberOfCredentialsPerUser <= kMaxCredentialsPerUser, APP_ERROR_ALLOCATION_FAILED,
        ChipLogError(Zcl,
                     "Max number of credentials per user is greater than %d, the maximum amount of users currently supported on "
                     "this platform",
                     kMaxCredentialsPerUser));
    VerifyOrReturnError(
        mLockParams.numberOfWeekdaySchedulesPerUser <= kMaxWeekdaySchedulesPerUser, APP_ERROR_ALLOCATION_FAILED,
        ChipLogError(Zcl,
                     "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this "
                     "platform",
                     kMaxWeekdaySchedulesPerUser));
    VerifyOrReturnError(
        mLockParams.numberOfYeardaySchedulesPerUser <= kMaxYeardaySchedulesPerUser, APP_ERROR_ALLOCATION_FAILED,
        ChipLogError(Zcl,
                     "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this "
                     "platform",
                     kMaxYeardaySchedulesPerUser));
    VerifyOrReturnError(
        mLockParams.numberOfHolidaySchedules <= kMaxHolidaySchedules, APP_ERROR_ALLOCATION_FAILED,
        ChipLogError(Zcl,
                     "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this "
                     "platform",
                     kMaxHolidaySchedules));

    // Migrate legacy configuration, if needed
    MigrateLockConfig(lockParam);

    // Create cmsis os sw timer for lock timer.
    mLockTimer = osTimerNew(TimerEventHandler, // timer callback handler
                            osTimerOnce,       // no timer reload (one-shot timer)
                            (void *) this,     // pass the app task obj context
                            NULL               // No osTimerAttr_t to provide.
    );
    VerifyOrReturnError(mLockTimer != NULL, APP_ERROR_CREATE_TIMER_FAILED,
                        ChipLogError(NotSpecified, "mLockTimer timer create failed"));
    if (sLockSharedStateMutex == nullptr)
    {
        sLockSharedStateMutex = osMutexNew(nullptr);
        VerifyOrReturnError(sLockSharedStateMutex != nullptr, APP_ERROR_ALLOCATION_FAILED,
                            ChipLogError(NotSpecified, "sLockSharedStateMutex create failed"));
    }
    if (!state.IsNull() && state.Value() == DlLockState::kUnlocked)
    {
        mLockActuatorState = LockActuatorState::kUnlockCompleted;
    }
    else
    {
        mLockActuatorState = LockActuatorState::kLockCompleted;
    }
    return CHIP_NO_ERROR;
}

bool AppTask::IsActuatorBusy() const
{
    // kUnlatchCompleted as "still in progress" for command-queueing purposes.
    return (mLockActuatorState == LockActuatorState::kLockInitiated || mLockActuatorState == LockActuatorState::kUnlockInitiated ||
            mLockActuatorState == LockActuatorState::kUnlatchInitiated ||
            mLockActuatorState == LockActuatorState::kUnlatchCompleted);
}

bool AppTask::InitiateLockAction(LockAction aAction, bool fromButton)
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
    else if ((mLockActuatorState == LockActuatorState::kLockCompleted ||
              mLockActuatorState == LockActuatorState::kUnlockCompleted) &&
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
            ChipLogError(NotSpecified, "mLockTimer timer start() failed");
            appError(APP_ERROR_START_TIMER_FAILED);
        }

        mLockActuatorState = new_state;
        if (aAction == LockAction::kUnlock || aAction == LockAction::kLock)
        {
            bool locked = (aAction == LockAction::kLock);
            ChipLogDetail(Zcl, "%s Action has been initiated", locked ? "Lock" : "Unlock");
            sLockLED.Set(!locked);

#ifdef DISPLAY_ENABLED
            appInstance().GetLCD().WriteDemoUI(locked);
#endif // DISPLAY_ENABLED
        }
        else if (aAction == LockAction::kUnlatch)
        {
            ChipLogDetail(Zcl, "Unlatch Action has been initiated");
        }
        if (fromButton)
        {
            mSyncClusterToButtonAction = true;
        }
    }

    return action_initiated;
}

void AppTask::TimerEventHandler(void * timerCbArg)
{
    AppTask * lock = static_cast<AppTask *>(timerCbArg);

    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;
    event.Handler            = &CustomerAppTask::ActuatorMovementEventHandler;
    appInstance().PostEvent(&event);
}

void AppTask::UnlockAfterUnlatch(intptr_t /* context */)
{
    auto & app = appInstance();
    auto & ctx = app.mUnlatchContext;

    VerifyOrReturn(ctx.mEndpointId != kInvalidEndpointId,
                   ChipLogError(Zcl, "UnlockAfterUnlatch: no valid unlatch context, skipping auto-unlock"));

    Status status =
        DoorLockServer::Instance().SetLockState(ctx.mEndpointId, DlLockState::kNotFullyLocked, OperationSourceEnum::kRemote,
                                                NullNullable, NullNullable, ctx.mFabricIdx, ctx.mNodeId)
        ? Status::Success
        : Status::Failure;
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "ERR: setting transitional NotFullyLocked %x (auto-unlock after unlatch)", to_underlying(status));
    }

    LockRequest unlockRequest        = {};
    unlockRequest.endpointId         = ctx.mEndpointId;
    unlockRequest.action             = LockAction::kUnlock;
    unlockRequest.targetClusterState = DlLockState::kUnlocked;
    unlockRequest.fabricIdx          = ctx.mFabricIdx;
    unlockRequest.nodeId             = ctx.mNodeId;
    unlockRequest.userIndex          = ctx.mUserIndex;
    unlockRequest.credential         = ctx.mCredential;
    unlockRequest.hasCredential      = ctx.mHasCredential;

    VerifyOrReturn(sLockSharedStateMutex != nullptr,
                   ChipLogError(Zcl, "Door Lock App: remote-action mutex not initialized; dropping unlatch-unlock"));
    {
        osStatus_t mutexStatus = osMutexAcquire(sLockSharedStateMutex, osWaitForever);
        VerifyOrReturn(mutexStatus == osOK,
                       ChipLogError(Zcl, "Door Lock App: remote-action mutex acquire failed (%d); dropping unlatch-unlock",
                                    static_cast<int>(mutexStatus)));
        app.mActiveRemoteAction    = unlockRequest;
        app.mHasActiveRemoteAction = true;
        osMutexRelease(sLockSharedStateMutex);
    }

    ctx.mEndpointId = kInvalidEndpointId;
    ctx.mUserIndex.SetNull();
    ctx.mCredential    = {};
    ctx.mHasCredential = false;
    PostLockActionEvent(AppEvent::kEventType_Lock, LockAction::kUnlock);
}

void AppTask::UnlatchCallback(void * argument)
{
    (void) argument;
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(&CustomerAppTask::UnlockAfterUnlatch,
                                                                     reinterpret_cast<intptr_t>(nullptr));
}

void AppTask::ActuatorMovementEventHandler(AppEvent * aEvent)
{
    LockAction actionCompleted = LockAction::kInvalid;
    AppTask * lock             = static_cast<AppTask *>(aEvent->TimerEvent.Context);
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
        DlLockState stateToReport = DlLockState::kUnknownEnumValue;
        switch (actionCompleted)
        {
        case LockAction::kLock:
            ChipLogDetail(Zcl, "Lock Action has been completed");
            stateToReport = DlLockState::kLocked;
            break;
        case LockAction::kUnlatch:
            ChipLogDetail(Zcl, "Unlatch Action has been completed");
            StartUnlatchTimer(UNLATCH_TIME_MS);
            stateToReport = DlLockState::kUnlatched;
            break;
        case LockAction::kUnlock:
            ChipLogDetail(Zcl, "Unlock Action has been completed");
            stateToReport = DlLockState::kUnlocked;
            break;
        case LockAction::kInvalid:
            break;
        }
        if (lock->mSyncClusterToButtonAction)
        {
            if (stateToReport != DlLockState::kUnknownEnumValue)
            {
                TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(
                    UpdateClusterState, static_cast<intptr_t>(to_underlying(stateToReport)));
            }
            lock->mSyncClusterToButtonAction = false;
        }
        // Consume the remote-action slot under the dedicated mutex (writer is `UnlockAfterUnlatch`
        // on the chip thread and `HandleLockRequestOnAppTask` on this thread); then run the cluster
        // push outside the mutex so we never hold both this mutex and the chip lock at once.
        LockRequest remoteAction = {};
        bool hasRemoteAction     = false;
        if (sLockSharedStateMutex != nullptr && osMutexAcquire(sLockSharedStateMutex, osWaitForever) == osOK)
        {
            if (lock->mHasActiveRemoteAction)
            {
                remoteAction                 = lock->mActiveRemoteAction;
                lock->mHasActiveRemoteAction = false;
                hasRemoteAction              = true;
            }
            osMutexRelease(sLockSharedStateMutex);
        }
        if (hasRemoteAction)
        {
            DeviceLayer::PlatformMgr().LockChipStack();
            lock->PushClusterLockState(remoteAction.endpointId, remoteAction.targetClusterState, remoteAction.fabricIdx,
                                       remoteAction.nodeId, remoteAction.userIndex,
                                       remoteAction.hasCredential ? &remoteAction.credential : nullptr, remoteAction.hasCredential);
            DeviceLayer::PlatformMgr().UnlockChipStack();
        }
        if (lock->mHasPendingRequest &&
            (lock->mLockActuatorState == LockActuatorState::kLockCompleted ||
             lock->mLockActuatorState == LockActuatorState::kUnlockCompleted))
        {
            LockRequest req          = lock->mPendingRequest;
            lock->mHasPendingRequest = false;
            ChipLogDetail(Zcl, "Door Lock App: replaying queued %s (action=%u, target=%u)",
                          req.isButtonAction ? "button" : "remote", to_underlying(req.action),
                          to_underlying(req.targetClusterState));
            appInstance().HandleLockRequestOnAppTask(req);
        }
        if (lock->mLockActuatorState == LockActuatorState::kLockCompleted ||
            lock->mLockActuatorState == LockActuatorState::kUnlockCompleted)
        {
            LockRequest stagedReq;
            if (TryDrainStagedLockRequest(stagedReq))
            {
                ChipLogDetail(Zcl, "Door Lock App: draining staged lock request from action-complete fallback");
                appInstance().HandleLockRequestOnAppTask(stagedReq);
            }
        }
    }
}

// ---- Cross-thread `LockRequest` handoff -----------------------------------

void AppTask::EnqueueLockRequest(const LockRequest & request)
{
    VerifyOrReturn(sLockSharedStateMutex != nullptr,
                   ChipLogError(Zcl, "Door Lock App: staging mutex not initialized; dropping LockRequest"));

    osStatus_t mutexStatus = osMutexAcquire(sLockSharedStateMutex, osWaitForever);
    VerifyOrReturn(
        mutexStatus == osOK,
        ChipLogError(Zcl, "Door Lock App: staging mutex acquire failed (%d); dropping LockRequest", static_cast<int>(mutexStatus)));
    sStagedLockRequest      = request;
    sStagedLockRequestValid = true;
    osMutexRelease(sLockSharedStateMutex);

    AppEvent event                 = {};
    event.Type                     = AppEvent::kEventType_LockRequest;
    event.LockRequestEvent.Request = nullptr;
    event.Handler                  = &CustomerAppTask::LockRequestEventHandler;
    appInstance().PostEvent(&event);
}

bool AppTask::TryDrainStagedLockRequest(LockRequest & out)
{
    VerifyOrReturnValue(sLockSharedStateMutex != nullptr, false);
    VerifyOrReturnValue(osMutexAcquire(sLockSharedStateMutex, osWaitForever) == osOK, false);
    bool drained = sStagedLockRequestValid;
    if (drained)
    {
        out                     = sStagedLockRequest;
        sStagedLockRequestValid = false;
    }
    osMutexRelease(sLockSharedStateMutex);
    return drained;
}

void AppTask::LockRequestEventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_LockRequest,
                   ChipLogError(NotSpecified, "LockRequestEventHandler: unexpected event type %u", aEvent->Type));

    LockRequest req;
    VerifyOrReturn(TryDrainStagedLockRequest(req));

    appInstance().HandleLockRequestOnAppTask(req);
}

void AppTask::HandleLockRequestOnAppTask(const LockRequest & request)
{
    if (IsActuatorBusy())
    {
        ChipLogDetail(NotSpecified, "Door Lock App: actuator busy; queueing %s (replacing any prior pending)",
                      request.isButtonAction ? "button" : "remote");
        mPendingRequest    = request;
        mHasPendingRequest = true;
        return;
    }
    if (!request.isButtonAction)
    {
        const DlLockState currentTerminal =
            (mLockActuatorState == LockActuatorState::kLockCompleted) ? DlLockState::kLocked : DlLockState::kUnlocked;
        if (request.targetClusterState == currentTerminal)
        {
            ChipLogDetail(NotSpecified, "Door Lock App: remote request target (%u) matches current LockState; skipping no-op",
                          to_underlying(request.targetClusterState));
            return;
        }
    }
    if (InitiateLockAction(request.action, /*fromButton*/ request.isButtonAction))
    {
        if (!request.isButtonAction)
        {
            DeviceLayer::PlatformMgr().LockChipStack();
            if (request.isUnboltUnlatch)
            {
                mUnlatchContext.Update(request.endpointId, request.fabricIdx, request.nodeId, request.userIndex, request.credential,
                                       request.hasCredential);
            }
            PushClusterLockState(request.endpointId, DlLockState::kNotFullyLocked, request.fabricIdx, request.nodeId,
                                 request.userIndex, request.hasCredential ? &request.credential : nullptr, request.hasCredential);
            DeviceLayer::PlatformMgr().UnlockChipStack();

            VerifyOrReturn(sLockSharedStateMutex != nullptr,
                           ChipLogError(Zcl, "Door Lock App: remote-action mutex not initialized; dropping remote action"));
            osStatus_t mutexStatus = osMutexAcquire(sLockSharedStateMutex, osWaitForever);
            VerifyOrReturn(mutexStatus == osOK,
                           ChipLogError(Zcl, "Door Lock App: remote-action mutex acquire failed (%d); dropping remote action",
                                        static_cast<int>(mutexStatus)));
            mActiveRemoteAction    = request;
            mHasActiveRemoteAction = true;
            osMutexRelease(sLockSharedStateMutex);
        }
    }
    else
    {
        ChipLogDetail(NotSpecified, "Action is already in progress or active.");
    }
}

bool AppTask::DMDoorLockGetUser(EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    CHIP_ERROR error;
    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed
    userIndex--;
    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);
    ChipLogDetail(Zcl, "Door Lock App: AppTask::DMDoorLockGetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    // Get User struct from nvm3
    uint16_t size = kLockUserInfoSize;
    LockUserInfo userInStorage;
    StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);
    error                  = mStorage->SyncGetKeyValue(userKey.KeyName(), &userInStorage, size);

    // If no data is found at user key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        user.userStatus = UserStatusEnum::kAvailable;
        ChipLogDetail(Zcl, "No user data found");
        return true;
    }
    VerifyOrReturnValue(error == CHIP_NO_ERROR && size == kLockUserInfoSize, false);

    user.userStatus = userInStorage.userStatus;
    if (userInStorage.userStatus == UserStatusEnum::kAvailable)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d]", endpointId);
        return true;
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
    StorageKeyName credentialKey = LockUserCredentialMap(userIndex);
    uint16_t credentialSize      = static_cast<uint16_t>(kCredentialStructSize * userInStorage.currentCredentialCount);
    CredentialStruct credentials[kMaxCredentialsPerUser];
    error = mStorage->SyncGetKeyValue(credentialKey.KeyName(), credentials, credentialSize);
    if (error == CHIP_NO_ERROR)
    {
        // Check size out param matches what we expect to read
        VerifyOrReturnValue(credentialSize == static_cast<uint16_t>(kCredentialStructSize * userInStorage.currentCredentialCount),
                            false);
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

bool AppTask::DMDoorLockSetUser(EndpointId endpointId, uint16_t userIndex, FabricIndex creator, FabricIndex modifier,
                                const CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                                CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);
    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;
    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);
    VerifyOrReturnValue(userName.size() <= DOOR_LOCK_MAX_USER_NAME_SIZE, false,
                        ChipLogError(Zcl, "Cannot set user - user name is too long [endpoint=%d,index=%d]", endpointId, userIndex));
    VerifyOrReturnValue(
        totalCredentials <= mLockParams.numberOfCredentialsPerUser, false,
        ChipLogError(Zcl, "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,totalCredentials=%u]",
                     endpointId, userIndex, totalCredentials));

    LockUserInfo userInStorage = {};
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
    StorageKeyName credentialKey = LockUserCredentialMap(userIndex);
    VerifyOrReturnValue(mStorage->SyncSetKeyValue(credentialKey.KeyName(), credentials, kCredentialStructSize * totalCredentials) ==
                            CHIP_NO_ERROR,
                        false);

    // Save user in nvm3
    StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);
    VerifyOrReturnValue(mStorage->SyncSetKeyValue(userKey.KeyName(), &userInStorage, kLockUserInfoSize) == CHIP_NO_ERROR, false);

    ChipLogDetail(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool AppTask::DMDoorLockGetCredential(EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                      EmberAfPluginDoorLockCredentialInfo & credential)
{
    CHIP_ERROR error;
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);
    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);
    VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType), false);

    ChipLogDetail(Zcl, "Lock App: AppTask::DMDoorLockGetCredential [credentialType=%u], credentialIndex=%d",
                  to_underlying(credentialType), credentialIndex);

    StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);
    LockCredentialInfo credentialInStorage;
    uint16_t size = kLockCredentialInfoSize;
    error         = mStorage->SyncGetKeyValue(key.KeyName(), &credentialInStorage, size);

    // If no data is found at credential key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        credential.status = DlCredentialStatus::kAvailable;
        return true;
    }

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(error == CHIP_NO_ERROR && size == kLockCredentialInfoSize, false);

    credential.status = credentialInStorage.status;
    ChipLogDetail(Zcl, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);
    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential ");
        return true;
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

bool AppTask::DMDoorLockSetCredential(EndpointId endpointId, uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                      DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                      const ByteSpan & credentialData)
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

    LockCredentialInfo credentialInStorage = {};
    credentialInStorage.status             = credentialStatus;
    credentialInStorage.credentialType     = credentialType;
    credentialInStorage.createdBy          = creator;
    credentialInStorage.lastModifiedBy     = modifier;
    credentialInStorage.credentialDataSize = credentialData.size();
    // Copy credential data to the storage struct
    memmove(credentialInStorage.credentialData, credentialData.data(), credentialInStorage.credentialDataSize);

    StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);
    error              = mStorage->SyncSetKeyValue(key.KeyName(), &credentialInStorage, kLockCredentialInfoSize);
    VerifyOrReturnValue(error == CHIP_NO_ERROR, false, ChipLogError(Zcl, "Error reading from KVS key"));
    ChipLogDetail(Zcl, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));
    return true;
}

DlStatus AppTask::DMDoorLockGetWeekDaySchedule(EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
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
    uint16_t size                  = kWeekDayScheduleInfoSize;
    StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);
    error                          = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, size);
    VerifyOrReturnValue(error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, DlStatus::kNotFound,
                        ChipLogError(Zcl, "No schedule data found for user"));

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(error == CHIP_NO_ERROR && size == kWeekDayScheduleInfoSize, DlStatus::kFailure);
    VerifyOrReturnValue(weekDayScheduleInStorage.status != DlScheduleStatus::kAvailable, DlStatus::kNotFound);
    schedule = weekDayScheduleInStorage.schedule;
    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetWeekDaySchedule(EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                               DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour,
                                               uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    WeekDayScheduleInfo weekDayScheduleInStorage = {};
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
    StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);
    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, kWeekDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockGetYearDaySchedule(EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
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
    uint16_t size                  = kYearDayScheduleInfoSize;
    StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);
    error                          = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, size);
    VerifyOrReturnValue(error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, DlStatus::kNotFound,
                        ChipLogError(Zcl, "No schedule data found for user"));

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(error == CHIP_NO_ERROR && size == kYearDayScheduleInfoSize, DlStatus::kFailure);
    VerifyOrReturnValue(yearDayScheduleInStorage.status != DlScheduleStatus::kAvailable, DlStatus::kNotFound);
    schedule = yearDayScheduleInStorage.schedule;
    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetYearDaySchedule(EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                               DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    YearDayScheduleInfo yearDayScheduleInStorage = {};
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
    StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);
    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, kYearDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockGetHolidaySchedule(EndpointId endpointId, uint8_t holidayIndex,
                                               EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    CHIP_ERROR error;
    HolidayScheduleInfo holidayScheduleInStorage;
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    holidayIndex--;
    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    uint16_t size                  = kHolidayScheduleInfoSize;
    StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);
    error                          = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, size);
    VerifyOrReturnValue(error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, DlStatus::kNotFound,
                        ChipLogError(Zcl, "No schedule data found for user"));

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(error == CHIP_NO_ERROR && size == kHolidayScheduleInfoSize, DlStatus::kFailure);
    VerifyOrReturnValue(holidayScheduleInStorage.status != DlScheduleStatus::kAvailable, DlStatus::kNotFound);
    schedule = holidayScheduleInStorage.schedule;
    return DlStatus::kSuccess;
}

DlStatus AppTask::DMDoorLockSetHolidaySchedule(EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                               uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    HolidayScheduleInfo holidayScheduleInStorage = {};
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure);
    holidayIndex--;
    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    holidayScheduleInStorage.schedule.localStartTime = localStartTime;
    holidayScheduleInStorage.schedule.localEndTime   = localEndTime;
    holidayScheduleInStorage.schedule.operatingMode  = operatingMode;
    holidayScheduleInStorage.status                  = status;

    // Save schedule data in nvm3
    StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);
    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, kHolidayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

bool AppTask::ValidatePin(EndpointId endpointId, const Optional<ByteSpan> & pin, Nullable<uint16_t> & outUserIndex,
                          LockOpCredentials & outCred, bool & outHasCred, OperationErrorEnum & err)
{
    char userNameBuffer[DOOR_LOCK_MAX_USER_NAME_SIZE];
    CredentialStruct credentialsBuffer[kMaxCredentialsPerUser];
    uint8_t credentialDataBuffer[kMaxCredentialSize];
    outUserIndex.SetNull();
    outHasCred = false;
    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    // Assume pin is required until told otherwise
    bool requirePin = true;
    app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);
    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified [endpointId=%d]", endpointId);
        if (!requirePin)
        {
            return true;
        }
        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);
        err = OperationErrorEnum::kInvalidCredential;
        return false;
    }

    // Check all pin codes associated to all users to see if this pin code exists.
    for (uint32_t userIndex = 1; userIndex <= mLockParams.numberOfUsers; userIndex++)
    {
        EmberAfPluginDoorLockUserInfo user;
        user.userName    = MutableCharSpan(userNameBuffer);
        user.credentials = Span<CredentialStruct>(credentialsBuffer);
        VerifyOrReturnValue(
            DMDoorLockGetUser(endpointId, userIndex, user), false,
            ChipLogError(Zcl, "Unable to get the user - internal error [endpointId=%d,userIndex=%lu]", endpointId, userIndex));
        if (user.userStatus != UserStatusEnum::kOccupiedEnabled)
        {
            continue;
        }

        for (auto & userCredential : user.credentials)
        {
            if (userCredential.credentialType != CredentialTypeEnum::kPin)
            {
                continue;
            }

            EmberAfPluginDoorLockCredentialInfo credential;
            credential.credentialData = MutableByteSpan(credentialDataBuffer);
            VerifyOrReturnValue(
                DMDoorLockGetCredential(endpointId, userCredential.credentialIndex, userCredential.credentialType, credential),
                false,
                ChipLogError(Zcl, "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                             endpointId, to_underlying(userCredential.credentialType), userCredential.credentialIndex));
            if (credential.status != DlCredentialStatus::kOccupied)
            {
                continue;
            }
            if (credential.credentialData.data_equal(pin.Value()))
            {
                ChipLogDetail(Zcl, "Lock App: specified PIN code was found in the database [endpointId=%d]", endpointId);

                outUserIndex.SetNonNull(static_cast<uint16_t>(userIndex));
                outCred.credentialType  = CredentialTypeEnum::kPin;
                outCred.credentialIndex = userCredential.credentialIndex;
                outHasCred              = true;
                return true;
            }
        }
    }

    ChipLogDetail(Zcl, "Door Lock App: specified PIN code was not found in the database [endpointId=%d]", endpointId);
    err = OperationErrorEnum::kInvalidCredential;
    return false;
}

void AppTask::PushClusterLockState(EndpointId endpointId, DlLockState lockState, const Nullable<FabricIndex> & fabricIdx,
                                   const Nullable<NodeId> & nodeId, const Nullable<uint16_t> & userIndex,
                                   const LockOpCredentials * cred, bool hasCred)
{
    ChipLogDetail(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", LockStateToString(lockState),
                  endpointId);
    if (hasCred && cred != nullptr)
    {
        LockOpCredentials userCred[] = { *cred };
        auto userCredentials         = MakeNullable<List<const LockOpCredentials>>(userCred);
        DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, userIndex, userCredentials,
                                                fabricIdx, nodeId);
        return;
    }

    DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, NullNullable, NullNullable,
                                            fabricIdx, nodeId);
}
