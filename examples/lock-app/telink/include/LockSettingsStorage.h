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

#include <../Zephyr/ZephyrConfig.h>
#include <AppConfig.h>
#include <LockManager.h>

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
namespace chip {
namespace DeviceLayer {
namespace Internal {

class LockSettingsStorage : ZephyrConfig
{
public:
    static const ZephyrConfig::Key kConfigKey_LockUser;
    static const ZephyrConfig::Key kConfigKey_LockUserName;
    static const ZephyrConfig::Key kConfigKey_UserCredentials;
    static const ZephyrConfig::Key kConfigKey_WeekDaySchedules;
    static const ZephyrConfig::Key kConfigKey_YearDaySchedules;
    static const ZephyrConfig::Key kConfigKey_HolidaySchedules;
    static const char * kConfigKey_Credential[kNumCredentialTypes];
    static const char * kConfigKey_CredentialData[kNumCredentialTypes];
};
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif
