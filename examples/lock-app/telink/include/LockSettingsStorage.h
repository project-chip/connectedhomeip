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
    static const char * kConfigKey_LockUser[APP_MAX_USERS];
    static const char * kConfigKey_LockUserName[APP_MAX_USERS];
    static const char * kConfigKey_UserCredentials[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_WeekDaySchedules[APP_MAX_USERS][APP_MAX_WEEKDAY_SCHEDULE_PER_USER];
    static const char * kConfigKey_YearDaySchedules[APP_MAX_USERS][APP_MAX_YEARDAY_SCHEDULE_PER_USER];
    static const char * kConfigKey_HolidaySchedules[APP_MAX_HOLYDAY_SCHEDULE_PER_USER];

    static const char * kConfigKey_CredentialPin[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialFace[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialFingerprint[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialFingervein[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialRfid[APP_MAX_CREDENTIAL];

    static const char * kConfigKey_CredentialData[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialDataPin[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialDataFace[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialDataFingerprint[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialDataFingerVein[APP_MAX_CREDENTIAL];
    static const char * kConfigKey_CredentialDataRfid[APP_MAX_CREDENTIAL];
};
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif
