/*
 *
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

#include <app/clusters/door-lock-server/door-lock-server.h>

#include <stddef.h>
#include <stdint.h>

#include "CHIPProjectConfig.h"

namespace SilabsDoorLock {

namespace LockInitParams {

struct LockParam
{
    // Read from zap attributes
    uint16_t numberOfUsers                   = 0;
    uint8_t numberOfCredentialsPerUser       = 0;
    uint8_t numberOfWeekdaySchedulesPerUser  = 0;
    uint8_t numberOfYeardaySchedulesPerUser = 0;
    uint8_t numberOfHolidaySchedules         = 0;
};

class ParamBuilder
{
public:
    ParamBuilder & SetNumberOfUsers(uint16_t numberOfUsers)
    {
        lockParam_.numberOfUsers = numberOfUsers;
        return *this;
    }
    ParamBuilder & SetNumberOfCredentialsPerUser(uint8_t numberOfCredentialsPerUser)
    {
        lockParam_.numberOfCredentialsPerUser = numberOfCredentialsPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfWeekdaySchedulesPerUser(uint8_t numberOfWeekdaySchedulesPerUser)
    {
        lockParam_.numberOfWeekdaySchedulesPerUser = numberOfWeekdaySchedulesPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfYeardaySchedulesPerUser(uint8_t numberOfYeardaySchedulesPerUser)
    {
        lockParam_.numberOfYeardaySchedulesPerUser = numberOfYeardaySchedulesPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfHolidaySchedules(uint8_t numberOfHolidaySchedules)
    {
        lockParam_.numberOfHolidaySchedules = numberOfHolidaySchedules;
        return *this;
    }
    LockParam GetLockParam() { return lockParam_; }

private:
    LockParam lockParam_;
};

} // namespace LockInitParams

namespace Storage {

using namespace SilabsDoorLockConfig::ResourceRanges;

struct WeekDayScheduleInfo
{
    DlScheduleStatus status;
    EmberAfPluginDoorLockWeekDaySchedule schedule;
};

struct YearDayScheduleInfo
{
    DlScheduleStatus status;
    EmberAfPluginDoorLockYearDaySchedule schedule;
};

struct HolidayScheduleInfo
{
    DlScheduleStatus status;
    EmberAfPluginDoorLockHolidaySchedule schedule;
};

struct LockUserInfo
{
    char userName[DOOR_LOCK_MAX_USER_NAME_SIZE];
    size_t userNameSize;
    uint32_t userUniqueId;
    UserStatusEnum userStatus;
    UserTypeEnum userType;
    CredentialRuleEnum credentialRule;
    chip::EndpointId endpointId;
    chip::FabricIndex createdBy;
    chip::FabricIndex lastModifiedBy;
    uint16_t currentCredentialCount;
};

struct LockCredentialInfo
{
    DlCredentialStatus status;
    CredentialTypeEnum credentialType;
    chip::FabricIndex createdBy;
    chip::FabricIndex lastModifiedBy;
    uint8_t credentialData[kMaxCredentialSize];
    size_t credentialDataSize;
};
} // namespace Storage
} // namespace SilabsDoorLock
