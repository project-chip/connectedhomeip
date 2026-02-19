/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <LockManager.h>
#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <nvm3_default.h>
#include <platform/silabs/SilabsConfig.h>

using namespace chip::app::Clusters;
using namespace chip::DeviceLayer::Internal;
using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

namespace {
namespace Legacy {

//
// These constants must match the configuration used to generate the data.
//
static constexpr uint16_t kMaxUserName       = 10;
static constexpr uint8_t kMaxCredentialSize  = 20;
static constexpr uint8_t kNumCredentialTypes = 6;

static constexpr SilabsConfig::Key kConfigKey_LockUser         = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x10);
static constexpr SilabsConfig::Key kConfigKey_Credential       = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x11);
static constexpr SilabsConfig::Key kConfigKey_LockUserName     = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x12);
static constexpr SilabsConfig::Key kConfigKey_CredentialData   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x13);
static constexpr SilabsConfig::Key kConfigKey_UserCredentials  = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x14);
static constexpr SilabsConfig::Key kConfigKey_WeekDaySchedules = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x15);
static constexpr SilabsConfig::Key kConfigKey_YearDaySchedules = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x16);
static constexpr SilabsConfig::Key kConfigKey_HolidaySchedules = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x17);

struct CredentialStruct
{
public:
    CredentialTypeEnum credentialType = static_cast<CredentialTypeEnum>(0);
    uint16_t credentialIndex          = static_cast<uint16_t>(0);
};

struct EmberAfPluginDoorLockUserInfo
{
    chip::CharSpan userName;
    chip::Span<const Legacy::CredentialStruct> credentials;
    uint32_t userUniqueId;
    UserStatusEnum userStatus = UserStatusEnum::kAvailable;
    UserTypeEnum userType;
    CredentialRuleEnum credentialRule;
    DlAssetSource creationSource;
    chip::FabricIndex createdBy;
    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy;
};

struct EmberAfPluginDoorLockCredentialInfo
{
    DlCredentialStatus status = DlCredentialStatus::kAvailable;
    CredentialTypeEnum credentialType;
    chip::ByteSpan credentialData;
    DlAssetSource creationSource;
    chip::FabricIndex createdBy;
    DlAssetSource modificationSource;
    chip::FabricIndex lastModifiedBy;
};

struct WeekDaysScheduleInfo
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

} // namespace Legacy

/**
 * @brief Reads a value from NVM3, allocating the its full size.
 *
 * @param[in]  key   The configuration key to read from NVM3 storage.
 * @param[out] p     Pointer to the allocated memory, if suscessful.
 *                   The caller is responsible for freeing this memory.
 * @param[out] size  Size of the allocated data
 * @return true if the key is valid and the data was successfully read; false otherwise.
 */
bool ReadKey(SilabsConfig::Key key, uint8_t *& p, size_t & size)
{
    uint32_t type;
    VerifyOrReturnValue(SilabsConfig::ValidConfigKey(key), false);
    VerifyOrReturnValue(0 == nvm3_getObjectInfo(nvm3_defaultHandle, key, &type, &size), false);
    p = (uint8_t *) Platform::MemoryAlloc(size);
    VerifyOrReturnValue(0 == nvm3_readData(nvm3_defaultHandle, key, p, size), false);
    return true;
}

/**
 * @brief Verifies if a given configuration key exists and contains data in NVM3 storage.
 * @param[in] key  The configuration key to verify in NVM3 storage.
 * @return true if the key is valid and contains data; false otherwise.
 */
bool VerifyKey(SilabsConfig::Key key)
{
    uint32_t type;
    size_t size = 0;
    VerifyOrReturnValue(SilabsConfig::ValidConfigKey(key), false);
    VerifyOrReturnValue(0 == nvm3_getObjectInfo(nvm3_defaultHandle, key, &type, &size), false);
    return size > 0;
}

/**
 * @return true if there is a SilabsConfig key that needs migration.
 */
bool IsMigrationNeeded()
{
    return VerifyKey(Legacy::kConfigKey_LockUser) || VerifyKey(Legacy::kConfigKey_Credential) ||
        VerifyKey(Legacy::kConfigKey_LockUserName) || VerifyKey(Legacy::kConfigKey_CredentialData) ||
        VerifyKey(Legacy::kConfigKey_UserCredentials) || VerifyKey(Legacy::kConfigKey_WeekDaySchedules) ||
        VerifyKey(Legacy::kConfigKey_YearDaySchedules) || VerifyKey(Legacy::kConfigKey_HolidaySchedules);
}

bool MigrateCredentials(chip::EndpointId endpoint_id, const SilabsDoorLock::LockInitParams::LockParam & params)
{
    const size_t kSingleTypeCredsSize = params.numberOfCredentialsPerUser * Legacy::kMaxCredentialSize;
    uint8_t * creds_data_buffer       = nullptr;
    uint8_t * creds_info_buffer       = nullptr;
    bool success                      = true;
    //
    // Credentials Data
    //   uint8_t mCredentialData[kNumCredentialTypes][kMaxCredentials][kMaxCredentialSize]
    //   kConfigKey_CredentialData = 0x87313
    {
        size_t creds_data_size  = 0;
        size_t creds_type_count = 0;
        VerifyOrReturnValue(ReadKey(Legacy::kConfigKey_CredentialData, creds_data_buffer, creds_data_size), false);
        VerifyOrReturnValue(nullptr != creds_data_buffer, false);
        VerifyOrExit((creds_data_size > kSingleTypeCredsSize) && (0 == creds_data_size % kSingleTypeCredsSize), success = false);
        creds_type_count = creds_data_size / kSingleTypeCredsSize;
        VerifyOrExit(kNumCredentialTypes == creds_type_count, success = false);
    }

    //
    // Credentials Info
    //   EmberAfPluginDoorLockCredentialInfo mLockCredentials[kNumCredentialTypes][kMaxCredentials];
    //   kConfigKey_Credential = 0x87311
    {
        const size_t kSingleTypeInfoSize = params.numberOfCredentialsPerUser * sizeof(Legacy::EmberAfPluginDoorLockCredentialInfo);
        Legacy::EmberAfPluginDoorLockCredentialInfo * creds_info = nullptr;
        size_t creds_info_size                                   = 0;
        size_t type_info_count                                   = 0;
        VerifyOrExit(ReadKey(Legacy::kConfigKey_Credential, creds_info_buffer, creds_info_size), success = false);
        VerifyOrExit(nullptr != creds_info_buffer, success = false);
        VerifyOrExit((creds_info_size > kSingleTypeInfoSize) && (0 == creds_info_size % kSingleTypeInfoSize), success = false);
        type_info_count = creds_info_size / kSingleTypeInfoSize;
        VerifyOrExit(kNumCredentialTypes == type_info_count, success = false);
        creds_info = (Legacy::EmberAfPluginDoorLockCredentialInfo *) creds_info_buffer;

        for (size_t type_idx = 0; type_idx < type_info_count; type_idx++)
        {
            // Credential type
            bool success = true;
            for (size_t cred_idx = 0; success && cred_idx < params.numberOfCredentialsPerUser; cred_idx++)
            {
                // Credential index (per type)
                Legacy::EmberAfPluginDoorLockCredentialInfo & info =
                    creds_info[(type_idx * params.numberOfCredentialsPerUser) + cred_idx];
                if (DlCredentialStatus::kOccupied == info.status)
                {
                    uint8_t * data = creds_data_buffer + (type_idx * kSingleTypeCredsSize + cred_idx * Legacy::kMaxCredentialSize);
                    const chip::ByteSpan data_span(data, info.credentialData.size());
                    success = LockMgr().SetCredential(endpoint_id, cred_idx, info.createdBy, info.lastModifiedBy, info.status,
                                                      info.credentialType, data_span);
                }
            }
        }
    }

    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_CredentialData);
    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_Credential);
exit:
    Platform::MemoryFree(creds_data_buffer);
    Platform::MemoryFree(creds_info_buffer);
    return success;
}

bool MigrateUsers(chip::EndpointId endpoint_id, const SilabsDoorLock::LockInitParams::LockParam & params)
{
    const size_t kSingleUserCredsSize                  = sizeof(Legacy::CredentialStruct) * params.numberOfCredentialsPerUser;
    Legacy::CredentialStruct * all_user_creds          = nullptr;
    Legacy::EmberAfPluginDoorLockUserInfo * users_info = nullptr;
    uint8_t * user_creds_buffer                        = nullptr;
    uint8_t * names_buffer                             = nullptr;
    uint8_t * users_buffer                             = nullptr;
    CredentialStruct * new_creds                       = nullptr;
    size_t user_count                                  = 0;
    bool success                                       = true;

    //
    // User Credentials
    //   CredentialStruct mCredentials[kMaxUsers][kMaxCredentials];
    //   kMaxCredentials = kMaxUsers * kMaxCredentialsPerUser;
    //   Read/write: LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser
    //   kConfigKey_UserCredentials = 0x87314
    {
        size_t user_creds_size  = 0;
        size_t user_creds_count = 0;

        VerifyOrReturnValue(ReadKey(Legacy::kConfigKey_UserCredentials, user_creds_buffer, user_creds_size), false);
        VerifyOrReturnValue(nullptr != user_creds_buffer, false);
        VerifyOrExit((user_creds_size > kSingleUserCredsSize) && (0 == user_creds_size % kSingleUserCredsSize), success = false);
        user_creds_count = user_creds_size / kSingleUserCredsSize;
        VerifyOrExit(params.numberOfUsers == user_creds_count, success = false);
        all_user_creds = (Legacy::CredentialStruct *) user_creds_buffer;
        new_creds      = (CredentialStruct *) Platform::MemoryAlloc(params.numberOfCredentialsPerUser * sizeof(CredentialStruct));
        VerifyOrExit(nullptr != new_creds, success = false);
        VerifyOrExit(params.numberOfUsers == user_creds_count, success = false);
    }

    //
    // User Names
    //   char mUserNames[kMaxUsers][kMaxUserName];
    //   kConfigKey_LockUserName = 0x87312
    {
        size_t names_size  = 0;
        size_t names_count = 0;
        VerifyOrExit(ReadKey(Legacy::kConfigKey_LockUserName, names_buffer, names_size), success = false);
        VerifyOrExit(nullptr != names_buffer, success = false);
        VerifyOrExit((names_size > Legacy::kMaxUserName) && (0 == names_size % Legacy::kMaxUserName), success = false);
        names_count = names_size / Legacy::kMaxUserName;
        VerifyOrExit(params.numberOfUsers == names_count, success = false);
    }

    //
    // Users Info
    //  EmberAfPluginDoorLockUserInfo mLockUsers[kMaxUsers];
    //  kConfigKey_LockUser = 0x87310
    {
        constexpr size_t kSingleInfoSize = sizeof(Legacy::EmberAfPluginDoorLockUserInfo);
        size_t total_info_size           = 0;
        VerifyOrExit(ReadKey(Legacy::kConfigKey_LockUser, users_buffer, total_info_size), success = false);
        VerifyOrExit(nullptr != users_buffer, success = false);
        VerifyOrExit((total_info_size > kSingleInfoSize) && (0 == total_info_size % kSingleInfoSize), success = false);
        user_count = total_info_size / kSingleInfoSize;
        VerifyOrExit(params.numberOfUsers == user_count, success = false);
        users_info = (Legacy::EmberAfPluginDoorLockUserInfo *) users_buffer;
    }

    for (size_t user_idx = 0; success && (user_idx < user_count); user_idx++)
    {
        const char * name                            = (const char *) (names_buffer + Legacy::kMaxUserName * user_idx);
        Legacy::EmberAfPluginDoorLockUserInfo & info = users_info[user_idx];
        Legacy::CredentialStruct * creds             = all_user_creds + (user_idx * kSingleUserCredsSize);

        VerifyOrExit(params.numberOfCredentialsPerUser >= info.credentials.size(), success = false);
        // Translate credentials
        for (size_t i = 0; i < info.credentials.size(); i++)
        {
            new_creds[i].credentialType  = creds[i].credentialType;
            new_creds[i].credentialIndex = creds[i].credentialIndex;
        }
        success = LockMgr().SetUser(endpoint_id, 1 + user_idx, info.createdBy, info.lastModifiedBy,
                                    chip::CharSpan(name, info.userName.size()), info.userUniqueId, info.userStatus, info.userType,
                                    info.credentialRule, new_creds, info.credentials.size());
    }

    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_UserCredentials);
    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_LockUserName);
    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_LockUser);
exit:
    Platform::MemoryFree(user_creds_buffer);
    Platform::MemoryFree(names_buffer);
    Platform::MemoryFree(users_buffer);
    Platform::MemoryFree(new_creds);
    return success;
}

bool MigrateSchedules(chip::EndpointId endpoint_id, const SilabsDoorLock::LockInitParams::LockParam & params)
{
    Legacy::WeekDaysScheduleInfo * week_schedules = nullptr;
    Legacy::YearDayScheduleInfo * year_schedules  = nullptr;
    uint8_t * week_schedules_buffer               = nullptr;
    uint8_t * year_schedules_buffer               = nullptr;
    uint8_t * holiday_schedules_buffer            = nullptr;
    size_t week_schedules_count                   = 0;
    size_t year_schedules_count                   = 0;
    size_t user_count                             = 0;
    bool success                                  = true;

    //
    // WeekDay Schedules
    //   WeekDaysScheduleInfo mWeekdaySchedule[kMaxUsers][kMaxWeekdaySchedulesPerUser];
    //   Read/write: sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser *
    //   LockParams.numberOfUsers kConfigKey_WeekDaySchedules = 0x87315
    {
        constexpr size_t kSingleScheduleSize = sizeof(Legacy::WeekDaysScheduleInfo);
        size_t schedules_size                = 0;
        VerifyOrReturnValue(ReadKey(Legacy::kConfigKey_WeekDaySchedules, week_schedules_buffer, schedules_size), false);
        VerifyOrReturnValue(nullptr != week_schedules_buffer, false);
        VerifyOrExit((schedules_size > kSingleScheduleSize) && (0 == schedules_size % kSingleScheduleSize), success = false);
        week_schedules_count = schedules_size / kSingleScheduleSize;
        user_count           = week_schedules_count / params.numberOfWeekdaySchedulesPerUser;
        VerifyOrExit(params.numberOfUsers == user_count, success = false);
        VerifyOrExit(params.numberOfUsers * params.numberOfWeekdaySchedulesPerUser == week_schedules_count, success = false);
        week_schedules = (Legacy::WeekDaysScheduleInfo *) week_schedules_buffer;
    }

    // YearDay Schedules
    //   YearDayScheduleInfo mYeardaySchedule[kMaxUsers][kMaxYeardaySchedulesPerUser];
    //   sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser * LockParams.numberOfUsers;
    //   kConfigKey_YearDaySchedules = 0x87316
    {
        constexpr size_t kSingleScheduleSize = sizeof(Legacy::YearDayScheduleInfo);
        size_t schedules_size                = 0;
        VerifyOrExit(ReadKey(Legacy::kConfigKey_YearDaySchedules, year_schedules_buffer, schedules_size), success = false);
        VerifyOrExit(nullptr != year_schedules_buffer, success = false);
        VerifyOrExit((schedules_size > kSingleScheduleSize) && (0 == schedules_size % kSingleScheduleSize), success = false);
        year_schedules_count = schedules_size / kSingleScheduleSize;
        user_count           = year_schedules_count / params.numberOfYeardaySchedulesPerUser;
        VerifyOrExit(params.numberOfUsers == user_count, success = false);
        VerifyOrExit(params.numberOfUsers * params.numberOfYeardaySchedulesPerUser == year_schedules_count, success = false);
        year_schedules = (Legacy::YearDayScheduleInfo *) year_schedules_buffer;
    }

    for (size_t user_idx = 0; user_idx < params.numberOfUsers; user_idx++)
    {
        // Week schedule
        for (size_t sched_idx = 0; sched_idx < params.numberOfWeekdaySchedulesPerUser; sched_idx++)
        {
            Legacy::WeekDaysScheduleInfo & week = week_schedules[user_idx * params.numberOfWeekdaySchedulesPerUser + sched_idx];
            if (DlScheduleStatus::kOccupied == week.status)
            {
                VerifyOrExit(DlStatus::kSuccess ==
                                 LockMgr().SetWeekdaySchedule(endpoint_id, 1 + sched_idx, 1 + user_idx, week.status,
                                                              week.schedule.daysMask, week.schedule.startHour,
                                                              week.schedule.startMinute, week.schedule.endHour,
                                                              week.schedule.endMinute),
                             success = false);
            }
        }
        // Year schedule
        for (size_t sched_idx = 0; sched_idx < params.numberOfYeardaySchedulesPerUser; sched_idx++)
        {
            Legacy::YearDayScheduleInfo & year = year_schedules[user_idx * params.numberOfYeardaySchedulesPerUser + sched_idx];
            if (DlScheduleStatus::kOccupied == year.status)
            {
                VerifyOrExit(DlStatus::kSuccess ==
                                 LockMgr().SetYeardaySchedule(endpoint_id, 1 + sched_idx, 1 + user_idx, year.status,
                                                              year.schedule.localStartTime, year.schedule.localEndTime),
                             success = false);
            }
        }
    }

    //
    // Holiday Schedule
    //   HolidayScheduleInfo mHolidaySchedule[kMaxHolidaySchedules]; // 10
    //   Read/write: sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules
    //   kConfigKey_HolidaySchedules = 0x87317
    {
        constexpr size_t kSingleScheduleSize            = sizeof(Legacy::HolidayScheduleInfo);
        Legacy::HolidayScheduleInfo * holiday_schedules = nullptr;
        size_t holiday_schedules_count                  = 0;
        size_t schedules_size                           = 0;
        VerifyOrExit(ReadKey(Legacy::kConfigKey_HolidaySchedules, holiday_schedules_buffer, schedules_size), success = false);
        VerifyOrExit(nullptr != holiday_schedules_buffer, success = false);
        VerifyOrExit((schedules_size > kSingleScheduleSize) && (0 == schedules_size % kSingleScheduleSize), success = false);
        holiday_schedules_count = schedules_size / kSingleScheduleSize;
        VerifyOrExit(params.numberOfHolidaySchedules == holiday_schedules_count, success = false);
        holiday_schedules = (Legacy::HolidayScheduleInfo *) holiday_schedules_buffer;

        for (size_t sched_idx = 0; sched_idx < params.numberOfHolidaySchedules; sched_idx++)
        {
            Legacy::HolidayScheduleInfo & info = holiday_schedules[sched_idx];
            if (DlScheduleStatus::kOccupied == info.status)
            {
                VerifyOrExit(DlStatus::kSuccess ==
                                 LockMgr().SetHolidaySchedule(endpoint_id, 1 + sched_idx, info.status, info.schedule.localStartTime,
                                                              info.schedule.localEndTime, info.schedule.operatingMode),
                             success = false);
            }
        }
    }

    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_WeekDaySchedules);
    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_YearDaySchedules);
    TEMPORARY_RETURN_IGNORED SilabsConfig::ClearConfigValue(Legacy::kConfigKey_HolidaySchedules);
exit:
    Platform::MemoryFree(week_schedules_buffer);
    Platform::MemoryFree(year_schedules_buffer);
    Platform::MemoryFree(holiday_schedules_buffer);
    return success;
}

} // namespace

bool LockManager::MigrateConfig(const SilabsDoorLock::LockInitParams::LockParam & params)
{
    if (IsMigrationNeeded())
    {
        static constexpr size_t kEnpointCount =
            MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
        // Migrate old data into the FIRST endpoint found
        for (chip::EndpointId endpoint_id = 1; endpoint_id <= kEnpointCount; ++endpoint_id)
        {
            if (emberAfContainsServer(endpoint_id, DoorLock::Id))
            {
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                bool success = MigrateCredentials(endpoint_id, params) && MigrateUsers(endpoint_id, params) &&
                    MigrateSchedules(endpoint_id, params);
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                ChipLogProgress(Zcl, "LockMigration: %s", success ? "Success" : "ERROR");
                return success;
            }
        }
    }
    return false;
}
