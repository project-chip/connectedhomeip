#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <LockSettingsStorage.h>
#include <lib/core/CHIPConfig.h>
#include <zephyr/settings/settings.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

    #define CONFIG_KEY(key)                                                                                                            \
    (key);                                                                                                                         \
    static_assert(sizeof(key) <= SETTINGS_MAX_NAME_LEN, "Config key too long: " key)

    #define NAMESPACE_CONFIG CHIP_DEVICE_CONFIG_SETTINGS_KEY "/cfg/"

    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_LockUser           = CONFIG_KEY(NAMESPACE_CONFIG "lock-user");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_Credential         = CONFIG_KEY(NAMESPACE_CONFIG "credential");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_LockUserName       = CONFIG_KEY(NAMESPACE_CONFIG "lock-user-name");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_CredentialData     = CONFIG_KEY(NAMESPACE_CONFIG "credential-data");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_UserCredentials    = CONFIG_KEY(NAMESPACE_CONFIG "user-credentials");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_WeekDaySchedules   = CONFIG_KEY(NAMESPACE_CONFIG "week-day-schedules");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_YearDaySchedules   = CONFIG_KEY(NAMESPACE_CONFIG "year-day-schedules");
    const ZephyrConfig::Key LockSettingsStorage::kConfigKey_HolidaySchedules   = CONFIG_KEY(NAMESPACE_CONFIG "holiday-schedules");
}
}
}
