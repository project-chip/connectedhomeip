/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          Utilities for interacting with the the ESP32 "NVS" key-value store.
 */

#pragma once

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the ESP32.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class ESP32Config
{
public:
    struct Key;
    class KeyAllocator;

    // Maximum length of an NVS key name, as specified in the ESP-IDF documentation.
    static constexpr size_t kMaxConfigKeyNameLength = 15;

    // NVS namespaces used to store device configuration information.
    static const char kConfigNamespace_ChipFactory[];
    static const char kConfigNamespace_ChipConfig[];
    static const char kConfigNamespace_ChipCounters[];

    // Key definitions for well-known keys.
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_HardwareVersionString;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;
    static const Key kConfigKey_DACCert;
    static const Key kConfigKey_DACPrivateKey;
    static const Key kConfigKey_DACPublicKey;
    static const Key kConfigKey_PAICert;
    static const Key kConfigKey_CertDeclaration;
    static const Key kConfigKey_VendorId;
    static const Key kConfigKey_VendorName;
    static const Key kConfigKey_ProductId;
    static const Key kConfigKey_ProductName;
    static const Key kConfigKey_ProductLabel;
    static const Key kConfigKey_PartNumber;
    static const Key kConfigKey_ProductURL;
    static const Key kConfigKey_SupportedCalTypes;
    static const Key kConfigKey_SupportedLocaleSize;
    static const Key kConfigKey_RotatingDevIdUniqueId;
    static const Key kConfigKey_ProductFinish;
    static const Key kConfigKey_ProductColor;
    static const Key kConfigKey_LocationCapability;
    static const Key kConfigKey_PrimaryDeviceType;

    // CHIP Config keys
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_UniqueId;
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

    // If buf is NULL then outLen is set to the required length to fit the string/blob
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

    // NVS Namespace helper functions.
    static CHIP_ERROR EnsureNamespace(const char * ns);
    static CHIP_ERROR ClearNamespace(const char * ns);

    static void RunConfigUnitTest(void);

    static const char * GetPartitionLabelByNamespace(const char * ns);
};

struct ESP32Config::Key
{
    const char * Namespace;
    const char * Name;

    bool operator==(const Key & other) const;

    template <typename T, typename std::enable_if_t<std::is_convertible<T, const char *>::value, int> = 0>
    Key(const char * aNamespace, T aName) : Namespace(aNamespace), Name(aName)
    {}

    template <size_t N>
    Key(const char * aNamespace, const char (&aName)[N]) : Namespace(aNamespace), Name(aName)
    {
        // Note: N includes null-terminator.
        static_assert(N <= ESP32Config::kMaxConfigKeyNameLength + 1, "Key too long");
    }
};

inline bool ESP32Config::Key::operator==(const Key & other) const
{
    return strcmp(Namespace, other.Namespace) == 0 && strcmp(Name, other.Name) == 0;
}

class ESP32Config::KeyAllocator
{
public:
    static CHIP_ERROR Locale(char * key, size_t size, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "locale/%x", index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR FixedLabelCount(char * key, size_t size, uint16_t endpoint)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "fl-sz/%x", endpoint) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR FixedLabelKey(char * key, size_t size, uint16_t endpoint, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "fl-k/%x/%x", endpoint, index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR FixedLabelValue(char * key, size_t size, uint16_t endpoint, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "fl-v/%x/%x", endpoint, index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }

    // Supported modes

    static CHIP_ERROR SupportedModesCount(char * key, size_t size, uint16_t endpoint)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "sm-sz/%x", endpoint) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR SupportedModesLabel(char * key, size_t size, uint16_t endpoint, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "sm-label/%x/%x", endpoint, index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR SupportedModesValue(char * key, size_t size, uint16_t endpoint, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "sm-mode/%x/%x", endpoint, index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR SemanticTagsCount(char * key, size_t size, uint16_t endpoint, uint16_t index)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "sm-st-sz/%x/%x", endpoint, index) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR SemanticTagValue(char * key, size_t size, uint16_t endpoint, uint16_t index, uint16_t ind)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "st-v/%x/%x/%x", endpoint, index, ind) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    static CHIP_ERROR SemanticTagMfgCode(char * key, size_t size, uint16_t endpoint, uint16_t index, uint16_t ind)
    {
        VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
        return snprintf(key, size, "st-mfg/%x/%x/%x", endpoint, index, ind) > 0 ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
