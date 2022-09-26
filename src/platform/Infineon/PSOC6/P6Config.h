/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Utilities for interacting with the the P6 key-value store.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <mtb_kvstore.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the P6.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class P6Config
{
public:
    struct Key;

    // Maximum length of a key name
    static constexpr size_t kMaxConfigKeyNameLength = MTB_KVSTORE_MAX_KEY_SIZE;

    // Namespaces used to store device configuration information.
    static const char kConfigNamespace_ChipFactory[];
    static const char kConfigNamespace_ChipConfig[];
    static const char kConfigNamespace_ChipCounters[];

    // Key definitions for well-known keys.
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_UniqueId;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_SoftwareVersion;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_WiFiStationSecType;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_WiFiSSID;
    static const Key kConfigKey_WiFiPassword;
    static const Key kConfigKey_WiFiSecurity;
    static const Key kConfigKey_WiFiMode;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;
    static const Key kConfigKey_LockUser;
    static const Key kConfigKey_Credential;
    static const Key kConfigKey_LockUserName;
    static const Key kConfigKey_CredentialData;
    static const Key kConfigKey_UserCredentials;
    static const Key kConfigKey_WeekDaySchedules;
    static const Key kConfigKey_YearDaySchedules;
    static const Key kConfigKey_HolidaySchedules;

    // CHIP Counter keys
    static const Key kCounterKey_RebootCount;
    static const Key kCounterKey_UpTime;
    static const Key kCounterKey_TotalOperationalHours;

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);

    static CHIP_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);
};

struct P6Config::Key
{
    const char * Namespace;
    const char * Name;

    CHIP_ERROR to_str(char * buf, size_t buf_size) const;
    size_t len() const;
    bool operator==(const Key & other) const;
};

inline CHIP_ERROR P6Config::Key::to_str(char * buf, size_t buf_size) const
{
    if (buf_size < len() + 1)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    strcpy(buf, Namespace);
    strcat(buf, ";");
    strcat(buf, Name);

    return CHIP_NO_ERROR;
}

// Length of key str (not including terminating null char)
inline size_t P6Config::Key::len() const
{
    // + 1 for separating ';'
    size_t out_size = strlen(Namespace) + strlen(Name) + 1;
    return out_size;
}

inline bool P6Config::Key::operator==(const Key & other) const
{
    return strcmp(Namespace, other.Namespace) == 0 && strcmp(Name, other.Name) == 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
