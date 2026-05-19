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

#pragma once

#include "AppTask.h"
#include "CRTPHelpers.h"

/**
 * @brief CRTP base for lock AppTask, exposing override hooks for customizable APIs.
 *
 * Each public method dispatches to `Derived::*Impl()`. Overrides are optional:
 * default `*Impl()` implementations in the private section forward to `AppTask`.
 * Override in `Derived` only for the behaviors you want to customize.
 *
 * @tparam Derived The derived class type (CRTP pattern)
 */
template <typename Derived>
class AppTaskImpl : public AppTask
{
public:
    // External triggers — entry points that pass context into AppTask (e.g. PostEvent).
    // Optional override: *Impl().
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    CHIP_ERROR InitLock() { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, InitLockImpl); }

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    // AppTask-thread event handler (queued event). Optional override: *Impl().
    static void LockButtonActionHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LockButtonActionHandlerImpl, aEvent);
    }

    // CMSIS osTimer expiry callback (unlatch timer). Optional override: *Impl().
    static void UnlatchCallback(void * argument)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, UnlatchCallbackImpl, argument);
    }

    // AppTask-thread event handler (queued actuator-movement timer event). Optional override: *Impl().
    static void ActuatorMovementEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ActuatorMovementEventHandlerImpl, aEvent);
    }

    // AppTask-thread event handler (queued lock-action event posted from the Matter / DoorLock
    // cluster-callback thread or from the FreeRTOS unlatch-timer trampoline). Optional override: *Impl().
    static void LockActionEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LockActionEventHandlerImpl, aEvent);
    }

    static void LockRequestEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LockRequestEventHandlerImpl, aEvent);
    }

    void HandleLockRequestOnAppTask(const AppTask::LockRequest & request)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, HandleLockRequestOnAppTaskImpl, request);
    }

    static void UnlockAfterUnlatch(intptr_t context)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, UnlockAfterUnlatchImpl, context);
    }

    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

    // ---------------------------------------------------------------------
    // DoorLock cluster data-model hooks. Default `*Impl()` forwards to
    // `AppTask::DM*`. Override in `Derived` to customize per-callback DM
    // behavior.
    // ---------------------------------------------------------------------

    bool DMDoorLockOnDoorLockCommand(chip::EndpointId endpointId,
                                     const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                     const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                     const chip::Optional<chip::ByteSpan> & pinCode,
                                     chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockOnDoorLockCommandImpl, endpointId, fabricIdx, nodeId, pinCode,
                                    err);
    }

    bool DMDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockOnDoorUnlockCommandImpl, endpointId, fabricIdx, nodeId, pinCode,
                                    err);
    }

    bool DMDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockOnDoorUnboltCommandImpl, endpointId, fabricIdx, nodeId, pinCode,
                                    err);
    }

    bool DMDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockGetCredentialImpl, endpointId, credentialIndex, credentialType,
                                    credential);
    }

    bool DMDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                 chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                 const chip::ByteSpan & credentialData)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockSetCredentialImpl, endpointId, credentialIndex, creator,
                                    modifier, credentialStatus, credentialType, credentialData);
    }

    bool DMDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockGetUserImpl, endpointId, userIndex, user);
    }

    bool DMDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                           const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                           CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockSetUserImpl, endpointId, userIndex, creator, modifier, userName,
                                    uniqueId, userStatus, usertype, credentialRule, credentials, totalCredentials);
    }

    DlStatus DMDoorLockGetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockGetWeekDayScheduleImpl, endpointId, weekdayIndex, userIndex,
                                    schedule);
    }

    DlStatus DMDoorLockSetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockSetWeekDayScheduleImpl, endpointId, weekdayIndex, userIndex,
                                    status, daysMask, startHour, startMinute, endHour, endMinute);
    }

    DlStatus DMDoorLockGetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockGetYearDayScheduleImpl, endpointId, yearDayIndex, userIndex,
                                    schedule);
    }

    DlStatus DMDoorLockSetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockSetYearDayScheduleImpl, endpointId, yearDayIndex, userIndex,
                                    status, localStartTime, localEndTime);
    }

    DlStatus DMDoorLockGetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockGetHolidayScheduleImpl, endpointId, holidayIndex,
                                    holidaySchedule);
    }

    DlStatus DMDoorLockSetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, DMDoorLockSetHolidayScheduleImpl, endpointId, holidayIndex, status,
                                    localStartTime, localEndTime, operatingMode);
    }

    void DMDoorLockOnAutoRelock(chip::EndpointId endpointId)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMDoorLockOnAutoRelockImpl, endpointId);
    }

private:
    friend Derived;

    /** Default implementations — override in Derived to customize. */

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    CHIP_ERROR InitLockImpl() { return AppTask::InitLock(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void LockButtonActionHandlerImpl(AppEvent * aEvent) { AppTask::LockButtonActionHandler(aEvent); }

    void UnlatchCallbackImpl(void * argument) { AppTask::UnlatchCallback(argument); }

    void ActuatorMovementEventHandlerImpl(AppEvent * aEvent) { AppTask::ActuatorMovementEventHandler(aEvent); }

    void LockActionEventHandlerImpl(AppEvent * aEvent) { AppTask::LockActionEventHandler(aEvent); }

    void LockRequestEventHandlerImpl(AppEvent * aEvent) { AppTask::LockRequestEventHandler(aEvent); }

    void HandleLockRequestOnAppTaskImpl(const AppTask::LockRequest & request) { AppTask::HandleLockRequestOnAppTask(request); }

    void UnlockAfterUnlatchImpl(intptr_t context) { AppTask::UnlockAfterUnlatch(context); }

    void DMPostAttributeChangeCallbackImpl(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                           uint8_t * value)
    {
        AppTask::DMPostAttributeChangeCallback(attributePath, type, size, value);
    }

    // DoorLock cluster DM defaults — forward to AppTask::DM*.
    bool DMDoorLockOnDoorLockCommandImpl(chip::EndpointId endpointId,
                                         const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                         const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                         const chip::Optional<chip::ByteSpan> & pinCode,
                                         chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        return AppTask::DMDoorLockOnDoorLockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
    }

    bool DMDoorLockOnDoorUnlockCommandImpl(chip::EndpointId endpointId,
                                           const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                           const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                           const chip::Optional<chip::ByteSpan> & pinCode,
                                           chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        return AppTask::DMDoorLockOnDoorUnlockCommand(endpointId, fabricIdx, nodeId, pinCode, err);
    }

    bool DMDoorLockOnDoorUnboltCommandImpl(chip::EndpointId endpointId,
                                           const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                           const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                           const chip::Optional<chip::ByteSpan> & pinCode,
                                           chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        return AppTask::DMDoorLockOnDoorUnboltCommand(endpointId, fabricIdx, nodeId, pinCode, err);
    }

    bool DMDoorLockGetCredentialImpl(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                     EmberAfPluginDoorLockCredentialInfo & credential)
    {
        return AppTask::DMDoorLockGetCredential(endpointId, credentialIndex, credentialType, credential);
    }

    bool DMDoorLockSetCredentialImpl(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                     chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                     CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
    {
        return AppTask::DMDoorLockSetCredential(endpointId, credentialIndex, creator, modifier, credentialStatus, credentialType,
                                                credentialData);
    }

    bool DMDoorLockGetUserImpl(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
    {
        return AppTask::DMDoorLockGetUser(endpointId, userIndex, user);
    }

    bool DMDoorLockSetUserImpl(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                               chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                               UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                               const CredentialStruct * credentials, size_t totalCredentials)
    {
        return AppTask::DMDoorLockSetUser(endpointId, userIndex, creator, modifier, userName, uniqueId, userStatus, usertype,
                                          credentialRule, credentials, totalCredentials);
    }

    DlStatus DMDoorLockGetWeekDayScheduleImpl(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                              EmberAfPluginDoorLockWeekDaySchedule & schedule)
    {
        return AppTask::DMDoorLockGetWeekDaySchedule(endpointId, weekdayIndex, userIndex, schedule);
    }

    DlStatus DMDoorLockSetWeekDayScheduleImpl(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                              DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                              uint8_t endHour, uint8_t endMinute)
    {
        return AppTask::DMDoorLockSetWeekDaySchedule(endpointId, weekdayIndex, userIndex, status, daysMask, startHour, startMinute,
                                                     endHour, endMinute);
    }

    DlStatus DMDoorLockGetYearDayScheduleImpl(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                              EmberAfPluginDoorLockYearDaySchedule & schedule)
    {
        return AppTask::DMDoorLockGetYearDaySchedule(endpointId, yearDayIndex, userIndex, schedule);
    }

    DlStatus DMDoorLockSetYearDayScheduleImpl(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                              DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
    {
        return AppTask::DMDoorLockSetYearDaySchedule(endpointId, yearDayIndex, userIndex, status, localStartTime, localEndTime);
    }

    DlStatus DMDoorLockGetHolidayScheduleImpl(chip::EndpointId endpointId, uint8_t holidayIndex,
                                              EmberAfPluginDoorLockHolidaySchedule & holidaySchedule)
    {
        return AppTask::DMDoorLockGetHolidaySchedule(endpointId, holidayIndex, holidaySchedule);
    }

    DlStatus DMDoorLockSetHolidayScheduleImpl(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                              uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
    {
        return AppTask::DMDoorLockSetHolidaySchedule(endpointId, holidayIndex, status, localStartTime, localEndTime, operatingMode);
    }

    void DMDoorLockOnAutoRelockImpl(chip::EndpointId endpointId) { AppTask::DMDoorLockOnAutoRelock(endpointId); }
};
