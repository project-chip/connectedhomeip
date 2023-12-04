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

const ZephyrConfig::Key LockSettingsStorage::kConfigKey_LockUser             = CONFIG_KEY(NAMESPACE_CONFIG "lock-user");
const ZephyrConfig::Key LockSettingsStorage::kConfigKey_LockUserName         = CONFIG_KEY(NAMESPACE_CONFIG "lock-user-name");
const ZephyrConfig::Key LockSettingsStorage::kConfigKey_UserCredentials      = CONFIG_KEY(NAMESPACE_CONFIG "user-credentials");
const ZephyrConfig::Key LockSettingsStorage::kConfigKey_WeekDaySchedules     = CONFIG_KEY(NAMESPACE_CONFIG "week-day-schedules");
const ZephyrConfig::Key LockSettingsStorage::kConfigKey_YearDaySchedules     = CONFIG_KEY(NAMESPACE_CONFIG "year-day-schedules");
const ZephyrConfig::Key LockSettingsStorage::kConfigKey_HolidaySchedules     = CONFIG_KEY(NAMESPACE_CONFIG "holiday-schedules");
const char * LockSettingsStorage::kConfigKey_Credential[kNumCredentialTypes] = {
    CONFIG_KEY_CREDENTIAL("programming-pin"), CONFIG_KEY_CREDENTIAL("pin"),         CONFIG_KEY_CREDENTIAL("rfid"),
    CONFIG_KEY_CREDENTIAL("finger-print"),    CONFIG_KEY_CREDENTIAL("finger-vein"), CONFIG_KEY_CREDENTIAL("face")
};
const char * LockSettingsStorage::kConfigKey_CredentialData[kNumCredentialTypes] = {
    CONFIG_KEY_CREDENTIAL_DATA("programming-pin"), CONFIG_KEY_CREDENTIAL_DATA("pin"),         CONFIG_KEY_CREDENTIAL_DATA("rfid"),
    CONFIG_KEY_CREDENTIAL_DATA("finger-print"),    CONFIG_KEY_CREDENTIAL_DATA("finger-vein"), CONFIG_KEY_CREDENTIAL_DATA("face")
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif
