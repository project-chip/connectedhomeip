#include <LockManager.h>
#include <platform/silabs/SilabsConfig.h>
#include <lib/support/CodeUtils.h>
#include <app/server/Server.h>

using namespace chip::app::Clusters;
using namespace chip::DeviceLayer::Internal;
using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;

namespace {
namespace Legacy {

static constexpr uint16_t kMaxUserName               = 10;
static constexpr uint16_t kMaxUsers                  = 6;
static constexpr uint8_t kMaxCredentialsPerUser      = 5;
static constexpr uint8_t kMaxWeekdaySchedulesPerUser = 10;
static constexpr uint8_t kMaxYeardaySchedulesPerUser = 10;
static constexpr uint8_t kMaxHolidaySchedules        = 10;
static constexpr uint8_t kMaxCredentialSize          = 20;
static constexpr uint8_t kNumCredentialTypes         = 6;
static constexpr uint8_t kMaxCredentials             = kMaxCredentialsPerUser;

static constexpr SilabsConfig::Key kConfigKey_LockUser           = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x10);
static constexpr SilabsConfig::Key kConfigKey_Credential         = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x11);
static constexpr SilabsConfig::Key kConfigKey_LockUserName       = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x12);
static constexpr SilabsConfig::Key kConfigKey_CredentialData     = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x13);
static constexpr SilabsConfig::Key kConfigKey_UserCredentials    = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x14);
static constexpr SilabsConfig::Key kConfigKey_WeekDaySchedules   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x15);
static constexpr SilabsConfig::Key kConfigKey_YearDaySchedules   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x16);
static constexpr SilabsConfig::Key kConfigKey_HolidaySchedules   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x17);

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

bool ReadKey(SilabsConfig::Key key, uint8_t *&p, size_t &size)
{
    uint32_t type;
    VerifyOrReturnValue(SilabsConfig::ValidConfigKey(key), false);
    VerifyOrReturnValue(0 == nvm3_getObjectInfo(nvm3_defaultHandle, key, &type, &size), false);
    p = (uint8_t *)Platform::MemoryAlloc(size);
    VerifyOrReturnValue(0 == nvm3_readData(nvm3_defaultHandle, key, p, size), false);
    return true;
}

bool MigrateCredentials(chip::EndpointId endpoint_id)
{
    constexpr size_t single_type_creds_size = Legacy::kMaxCredentials * Legacy::kMaxCredentialSize;
    uint8_t *creds_data_buffer = nullptr;
    uint8_t *creds_info_buffer = nullptr;
    bool success = true;
    //
    // Credentials Data
    //   uint8_t mCredentialData[kNumCredentialTypes][kMaxCredentials][kMaxCredentialSize]
    //   kConfigKey_CredentialData = 0x87313
    {
        size_t creds_data_size = 0;
        size_t creds_type_count = 0;
        VerifyOrReturnValue(ReadKey(SilabsConfig::kConfigKey_CredentialData, creds_data_buffer, creds_data_size), false);
        VerifyOrReturnValue(nullptr != creds_data_buffer, false);
        VerifyOrExit((creds_data_size > single_type_creds_size) && (0 == creds_data_size % single_type_creds_size), success=false);
        creds_type_count = creds_data_size / single_type_creds_size;
        VerifyOrExit(kNumCredentialTypes == creds_type_count, success=false);
    }

    //
    // Credentials Info
    //   EmberAfPluginDoorLockCredentialInfo mLockCredentials[kNumCredentialTypes][kMaxCredentials];
    //   kConfigKey_Credential = 0x87311
    {
        constexpr size_t single_type_info_size = Legacy::kMaxCredentials * sizeof(Legacy::EmberAfPluginDoorLockCredentialInfo);
        Legacy::EmberAfPluginDoorLockCredentialInfo *creds_info = nullptr;
        size_t creds_info_size = 0;
        size_t type_info_count = 0;
        VerifyOrExit(ReadKey(SilabsConfig::kConfigKey_Credential, creds_info_buffer, creds_info_size), success=false);
        VerifyOrExit(nullptr != creds_info_buffer, success=false);
        VerifyOrExit((creds_info_size > single_type_info_size) && (0 == creds_info_size % single_type_info_size), success=false);
        type_info_count = creds_info_size / single_type_info_size;
        VerifyOrExit(kNumCredentialTypes == type_info_count, success=false);
        creds_info = (Legacy::EmberAfPluginDoorLockCredentialInfo*)creds_info_buffer;

        for (size_t type_idx = 0; type_idx < type_info_count; type_idx++)
        {
            // Credential type
            bool success = true;
            for (size_t cred_idx = 0; success && cred_idx < Legacy::kMaxCredentials; cred_idx++)
            {
                // Credential index (per type)
                Legacy::EmberAfPluginDoorLockCredentialInfo &info = creds_info[(type_idx *  Legacy::kMaxCredentials) + cred_idx];
                if(DlCredentialStatus::kOccupied == info.status)
                {
                    uint8_t *data = creds_data_buffer + (type_idx * single_type_creds_size + cred_idx * Legacy::kMaxCredentialSize);
                    const chip::ByteSpan data_span(data, info.credentialData.size());
                    success = LockMgr().SetCredential(endpoint_id, cred_idx, info.createdBy, info.lastModifiedBy, info.status, info.credentialType, data_span);
                }
            }
        }
    }

exit:
    Platform::MemoryFree(creds_data_buffer);
    Platform::MemoryFree(creds_info_buffer);
    return success;
}

bool MigrateUsers(chip::EndpointId endpoint_id)
{
    constexpr size_t single_name_size = Legacy::kMaxUserName;
    constexpr size_t single_user_creds_size = sizeof(Legacy::CredentialStruct) * Legacy::kMaxCredentials;
    Legacy::CredentialStruct *all_user_creds = nullptr;
    Legacy::EmberAfPluginDoorLockUserInfo *users_info = nullptr;
    uint8_t *user_creds_buffer = nullptr;
    uint8_t *names_buffer = nullptr;
    uint8_t *users_buffer = nullptr;
    size_t user_count = 0;
    bool success = true;

    //
    // User Credentials
    //   CredentialStruct mCredentials[kMaxUsers][kMaxCredentials]; // Wasteful
    //   kMaxCredentials = kMaxUsers * kMaxCredentialsPerUser;
    //   Read/write: LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser
    //   kConfigKey_UserCredentials = 0x87314
    {
        size_t user_creds_size = 0;
        size_t user_creds_count = 0;

        VerifyOrReturnValue(ReadKey(SilabsConfig::kConfigKey_UserCredentials, user_creds_buffer, user_creds_size), false);
        VerifyOrReturnValue(nullptr != user_creds_buffer, false);
        VerifyOrExit((user_creds_size > single_user_creds_size) && (0 == user_creds_size % single_user_creds_size), success=false);
        user_creds_count = user_creds_size / single_user_creds_size;
        VerifyOrExit(Legacy::kMaxUsers == user_creds_count, success=false);
        all_user_creds = (Legacy::CredentialStruct *)user_creds_buffer;
    }

    //
    // User Names
    //   char mUserNames[kMaxUsers][kMaxUserName];
    //   kConfigKey_LockUserName = 0x87312
    {
        size_t names_size = 0;
        size_t names_count = 0;
        VerifyOrExit(ReadKey(SilabsConfig::kConfigKey_LockUserName, names_buffer, names_size), success=false);
        VerifyOrExit(nullptr != names_buffer, success=false);
        VerifyOrExit((names_size > single_name_size) && (0 == names_size % single_name_size), success=false);
        names_count = names_size / single_name_size;
        VerifyOrExit(Legacy::kMaxUsers == names_count, success=false);
    }

    //
    // Users Info
    //  EmberAfPluginDoorLockUserInfo mLockUsers[kMaxUsers];
    //  kConfigKey_LockUser = 0x87310
    {
        constexpr size_t single_info_size = sizeof(Legacy::EmberAfPluginDoorLockUserInfo);
        size_t total_info_size = 0;
        VerifyOrExit(ReadKey(SilabsConfig::kConfigKey_LockUser, users_buffer, total_info_size), success=false);
        VerifyOrExit(nullptr != users_buffer, success=false);
        VerifyOrExit((total_info_size > single_info_size) && (0 == total_info_size % single_info_size), success=false);
        user_count = total_info_size / single_info_size;
        VerifyOrExit(Legacy::kMaxUsers == user_count, success=false);
        users_info = (Legacy::EmberAfPluginDoorLockUserInfo*) users_buffer;
    }

    for (size_t user_idx = 0; success && (user_idx < user_count); user_idx++)
    {
        const char *name = (const char *)(names_buffer + single_name_size * user_idx);
        Legacy::EmberAfPluginDoorLockUserInfo &info = users_info[user_idx];
        Legacy::CredentialStruct *creds = all_user_creds + (user_idx * single_user_creds_size);
        CredentialStruct creds2[Legacy::kMaxCredentials];
        VerifyOrExit(Legacy::kMaxCredentials >= info.credentials.size(), success=false);
        // Translate credentials
        for (size_t i = 0; i < info.credentials.size(); i++)
        {
            creds2[i].credentialType = creds[i].credentialType;
            creds2[i].credentialIndex = creds[i].credentialIndex;
        }
        success = LockMgr().SetUser(endpoint_id, 1 + user_idx, info.createdBy, info.lastModifiedBy,
                        chip::CharSpan(name, info.userName.size()), info.userUniqueId,
                        info.userStatus, info.userType,
                        info.credentialRule, creds2, info.credentials.size());
    }

exit:
    Platform::MemoryFree(user_creds_buffer);
    Platform::MemoryFree(names_buffer);
    Platform::MemoryFree(users_buffer);
    return success;
}

bool MigrateSchedules(chip::EndpointId endpoint_id)
{
    Legacy::WeekDaysScheduleInfo *week_schedules = nullptr;
    Legacy::YearDayScheduleInfo *year_schedules = nullptr;
    uint8_t *week_schedules_buffer = nullptr;
    uint8_t *year_schedules_buffer = nullptr;
    uint8_t *holiday_schedules_buffer = nullptr;
    size_t week_schedules_count = 0;
    size_t year_schedules_count = 0;
    size_t user_count = 0;
    bool success = true;

    //
    // WeekDay Schedules
    //   WeekDaysScheduleInfo mWeekdaySchedule[kMaxUsers][kMaxWeekdaySchedulesPerUser];
    //   Read/write: sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser * LockParams.numberOfUsers
    //   kConfigKey_WeekDaySchedules = 0x87315
    {
        constexpr size_t single_schedule_size = sizeof(Legacy::WeekDaysScheduleInfo);
        size_t schedules_size = 0;
        VerifyOrReturnValue(ReadKey(SilabsConfig::kConfigKey_WeekDaySchedules, week_schedules_buffer, schedules_size), false);
        VerifyOrReturnValue(nullptr != week_schedules_buffer, false);
        VerifyOrExit((schedules_size > single_schedule_size) && (0 == schedules_size % single_schedule_size), success=false);
        week_schedules_count = schedules_size / single_schedule_size;
        user_count = week_schedules_count / Legacy::kMaxWeekdaySchedulesPerUser;
        VerifyOrExit(Legacy::kMaxUsers == user_count, success=false);
        VerifyOrExit(Legacy::kMaxUsers * Legacy::kMaxWeekdaySchedulesPerUser == week_schedules_count, success=false);
        week_schedules = (Legacy::WeekDaysScheduleInfo *) week_schedules_buffer;
    }

    // YearDay Schedules
    //   YearDayScheduleInfo mYeardaySchedule[kMaxUsers][kMaxYeardaySchedulesPerUser];
    //   sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser * LockParams.numberOfUsers;
    //   kConfigKey_YearDaySchedules = 0x87316
    {
        constexpr size_t single_schedule_size = sizeof(Legacy::YearDayScheduleInfo);
        size_t schedules_size = 0;
        VerifyOrExit(ReadKey(SilabsConfig::kConfigKey_YearDaySchedules, year_schedules_buffer, schedules_size), success=false);
        VerifyOrExit(nullptr != year_schedules_buffer, success=false);
        VerifyOrExit((schedules_size > single_schedule_size) && (0 == schedules_size % single_schedule_size), success=false);
        year_schedules_count = schedules_size / single_schedule_size;
        user_count = year_schedules_count / Legacy::kMaxYeardaySchedulesPerUser;
        VerifyOrExit(Legacy::kMaxUsers == user_count, success=false);
        VerifyOrExit(Legacy::kMaxUsers * Legacy::kMaxYeardaySchedulesPerUser == year_schedules_count, success=false);
        year_schedules = (Legacy::YearDayScheduleInfo *) year_schedules_buffer;
    }

    for (size_t user_idx = 0; user_idx < Legacy::kMaxUsers; user_idx++)
    {
        // Week schedule
        for (size_t sched_idx = 0; sched_idx < Legacy::kMaxWeekdaySchedulesPerUser; sched_idx++)
        {
            Legacy::WeekDaysScheduleInfo &week = week_schedules[user_idx * Legacy::kMaxWeekdaySchedulesPerUser + sched_idx];
            if(DlScheduleStatus::kOccupied == week.status)
            {
                VerifyOrExit(DlStatus::kSuccess == LockMgr().SetWeekdaySchedule(endpoint_id, 1 + sched_idx, 1 + user_idx,
                                            week.status, week.schedule.daysMask,
                                            week.schedule.startHour, week.schedule.startMinute,
                                            week.schedule.endHour, week.schedule.endMinute), success=false);
            }
        }
        // Year schedule
        for (size_t sched_idx = 0; sched_idx < Legacy::kMaxYeardaySchedulesPerUser; sched_idx++)
        {
            Legacy::YearDayScheduleInfo &year = year_schedules[user_idx * Legacy::kMaxYeardaySchedulesPerUser + sched_idx];
            if(DlScheduleStatus::kOccupied == year.status)
            {
                VerifyOrExit(DlStatus::kSuccess == LockMgr().SetYeardaySchedule(endpoint_id, 1 + sched_idx, 1 + user_idx, year.status,
                    year.schedule.localStartTime, year.schedule.localEndTime), success=false);
            }
        }
    }

    //
    // Holiday Schedule
    //   HolidayScheduleInfo mHolidaySchedule[kMaxHolidaySchedules]; // 10
    //   Read/write: sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules
    //   kConfigKey_HolidaySchedules = 0x87317
    {
        constexpr size_t single_schedule_size = sizeof(Legacy::HolidayScheduleInfo);
        Legacy::HolidayScheduleInfo *holiday_schedules = nullptr;
        size_t holiday_schedules_count = 0;
        size_t schedules_size = 0;
        VerifyOrExit(ReadKey(SilabsConfig::kConfigKey_HolidaySchedules, holiday_schedules_buffer, schedules_size), success=false);
        VerifyOrExit(nullptr != holiday_schedules_buffer, success=false);
        VerifyOrExit((schedules_size > single_schedule_size) && (0 == schedules_size % single_schedule_size), success=false);
        holiday_schedules_count = schedules_size / single_schedule_size;
        VerifyOrExit(Legacy::kMaxHolidaySchedules == holiday_schedules_count, success=false);
        holiday_schedules = (Legacy::HolidayScheduleInfo *) holiday_schedules_buffer;

        for (size_t sched_idx = 0; sched_idx < Legacy::kMaxHolidaySchedules; sched_idx++)
        {
            Legacy::HolidayScheduleInfo &info = holiday_schedules[sched_idx];
            if(DlScheduleStatus::kOccupied == info.status)
            {
                VerifyOrExit(DlStatus::kSuccess == LockMgr().SetHolidaySchedule(endpoint_id, 1 + sched_idx,
                    info.status, info.schedule.localStartTime, info.schedule.localEndTime, info.schedule.operatingMode), success=false);
            }
        }
    }

exit:
    Platform::MemoryFree(week_schedules_buffer);
    Platform::MemoryFree(year_schedules_buffer);
    Platform::MemoryFree(holiday_schedules_buffer);
    return success;
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {

bool MigrateLockManager()
{
    static constexpr size_t kEnpointCount = MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
    
    for (chip::EndpointId endpoint_id = 1; endpoint_id <= kEnpointCount; ++endpoint_id)
    {
        if (emberAfContainsServer(endpoint_id, DoorLock::Id))
        {
            bool success = MigrateCredentials(endpoint_id) &&
                      MigrateUsers(endpoint_id) &&
                      MigrateSchedules(endpoint_id);
            ChipLogProgress(Zcl, "LockMigration: %s", success ? "Success" : "ERROR");
            return success;
        }
    }
    return false;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
