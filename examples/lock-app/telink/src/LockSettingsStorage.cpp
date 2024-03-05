/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <LockSettingsStorage.h>
#include <lib/core/CHIPConfig.h>

#include <zephyr/settings/settings.h>

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define CONFIG_KEY(key)                                                                                                            \
    (key);                                                                                                                         \
    static_assert(sizeof(key) <= SETTINGS_MAX_NAME_LEN, "Config key too long: " key)

#define CONFIG_KEY_CREDENTIAL(type) (NAMESPACE_CONFIG "credential-" type)
#define CONFIG_KEY_CREDENTIAL_DATA(type) (NAMESPACE_CONFIG "credential-" type "-data")

#define NAMESPACE_CONFIG CHIP_DEVICE_CONFIG_SETTINGS_KEY "/cfg/"

#define CONFIG_KEY_USER(index, _) (NAMESPACE_CONFIG "lock-user-" #index)
#define CONFIG_KEY_USER_NAMES(index, _) (NAMESPACE_CONFIG "lock-user-name-" #index)
#define CONFIG_KEY_USER_CREDENTIALS(index, _) (NAMESPACE_CONFIG "user-credentials-" #index)
#define CONFIG_KEY_WEEK_DAY_SCH(index, _) (NAMESPACE_CONFIG "week-day-schedules-" #index)
#define CONFIG_KEY_YEAR_DAY_SCH(index, _) (NAMESPACE_CONFIG "year-day-schedules-" #index)
#define CONFIG_KEY_HOLYDAY_SCH(index, _) (NAMESPACE_CONFIG "holiday-schedules-" #index)

#define CONFIG_KEY_CREDENTIALS_PIN(index, _) (NAMESPACE_CONFIG "credentials-pin" #index)
#define CONFIG_KEY_CREDENTIALS_FACE(index, _) (NAMESPACE_CONFIG "credentials-face" #index)
#define CONFIG_KEY_CREDENTIALS_FINGERPTRINT(index, _) (NAMESPACE_CONFIG "credentials-fp" #index)
#define CONFIG_KEY_CREDENTIALS_FINGERVEIN(index, _) (NAMESPACE_CONFIG "credentials-fv" #index)
#define CONFIG_KEY_CREDENTIALS_RFID(index, _) (NAMESPACE_CONFIG "credentials-rfid" #index)

#define CONFIG_KEY_USER_CREDENTIALS_DATA_PIN(index, _) (NAMESPACE_CONFIG "credentials-datapin-" #index)
#define CONFIG_KEY_USER_CREDENTIALS_DATA_FACE(index, _) (NAMESPACE_CONFIG "credentials-dataface-" #index)
#define CONFIG_KEY_USER_CREDENTIALS_DATA_FINGERPTRINT(index, _) (NAMESPACE_CONFIG "credentials-datafp-" #index)
#define CONFIG_KEY_USER_CREDENTIALS_DATA_FINGERVEIN(index, _) (NAMESPACE_CONFIG "credentials-datafv-" #index)
#define CONFIG_KEY_USER_CREDENTIALS_DATA_RFID(index, _) (NAMESPACE_CONFIG "credentials-datarfid-" #index)

const char * LockSettingsStorage::kConfigKey_LockUser[APP_MAX_USERS]     = { LISTIFY(APP_MAX_USERS, CONFIG_KEY_USER, (, )) };
const char * LockSettingsStorage::kConfigKey_LockUserName[APP_MAX_USERS] = { LISTIFY(APP_MAX_USERS, CONFIG_KEY_USER_NAMES, (, )) };
const char * LockSettingsStorage::kConfigKey_UserCredentials[APP_MAX_CREDENTIAL] = { LISTIFY(APP_MAX_CREDENTIAL,
                                                                                             CONFIG_KEY_USER_CREDENTIALS, (, )) };
const char * LockSettingsStorage::kConfigKey_WeekDaySchedules[APP_MAX_USERS][APP_MAX_WEEKDAY_SCHEDULE_PER_USER] = { LISTIFY(
    APP_MAX_SCHEDULES_TOTAL, CONFIG_KEY_WEEK_DAY_SCH, (, )) };
const char * LockSettingsStorage::kConfigKey_YearDaySchedules[APP_MAX_USERS][APP_MAX_YEARDAY_SCHEDULE_PER_USER] = { LISTIFY(
    APP_MAX_SCHEDULES_TOTAL, CONFIG_KEY_YEAR_DAY_SCH, (, )) };
const char * LockSettingsStorage::kConfigKey_HolidaySchedules[APP_MAX_HOLYDAY_SCHEDULE_PER_USER]                = { LISTIFY(
    APP_MAX_HOLYDAY_SCHEDULE_PER_USER, CONFIG_KEY_HOLYDAY_SCH, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialPin[APP_MAX_CREDENTIAL] = { LISTIFY(APP_MAX_CREDENTIAL,
                                                                                           CONFIG_KEY_CREDENTIALS_PIN, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialFace[APP_MAX_CREDENTIAL] = { LISTIFY(APP_MAX_CREDENTIAL,
                                                                                            CONFIG_KEY_CREDENTIALS_FACE, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialFingerprint[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_CREDENTIALS_FINGERPTRINT, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialFingervein[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_CREDENTIALS_FINGERVEIN, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialRfid[APP_MAX_CREDENTIAL] = { LISTIFY(APP_MAX_CREDENTIAL,
                                                                                            CONFIG_KEY_CREDENTIALS_RFID, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialDataPin[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_USER_CREDENTIALS_DATA_PIN, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialDataFace[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_USER_CREDENTIALS_DATA_FACE, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialDataFingerprint[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_USER_CREDENTIALS_DATA_FINGERPTRINT, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialDataFingerVein[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_USER_CREDENTIALS_DATA_FINGERVEIN, (, )) };

const char * LockSettingsStorage::kConfigKey_CredentialDataRfid[APP_MAX_CREDENTIAL] = { LISTIFY(
    APP_MAX_CREDENTIAL, CONFIG_KEY_USER_CREDENTIALS_DATA_RFID, (, )) };

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif
