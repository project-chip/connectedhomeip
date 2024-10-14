/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#include "chef-lock-endpoint.h"
#include <app/clusters/door-lock-server/door-lock-server.h>

#include <cstdint>
#include <vector>

class LockManager
{
public:
    LockManager() = default;

    bool InitEndpoint(chip::EndpointId endpointId);

    bool SetDoorState(chip::EndpointId endpointId, DoorStateEnum doorState);

    bool SendLockAlarm(chip::EndpointId endpointId, AlarmCodeEnum alarmCode);

    bool Lock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
              const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource);
    bool Unlock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource);
    bool Unbolt(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource);

    bool GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);
    bool SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                 const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                 CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials);

    bool GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential);

    bool SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                       const chip::ByteSpan & credentialData);

    DlStatus GetSchedule(chip::EndpointId endpointId, uint8_t weekDayIndex, uint16_t userIndex,
                         EmberAfPluginDoorLockWeekDaySchedule & schedule);
    DlStatus GetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                         EmberAfPluginDoorLockYearDaySchedule & schedule);
    DlStatus GetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule);

    DlStatus SetSchedule(chip::EndpointId endpointId, uint8_t weekDayIndex, uint16_t userIndex, DlScheduleStatus status,
                         DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute);
    DlStatus SetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                         uint32_t localStartTime, uint32_t localEndTime);
    DlStatus SetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime,
                         uint32_t localEndTime, OperatingModeEnum operatingMode);

    static LockManager & Instance();

private:
    LockEndpoint * getEndpoint(chip::EndpointId endpointId);

    std::vector<LockEndpoint> mEndpoints;

    static LockManager instance;
};
