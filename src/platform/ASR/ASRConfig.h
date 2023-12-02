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
 *          Utilities for interacting with the the ASR key-value store.
 */

#pragma once

#include "ASRFactoryDataParser.h"
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the ASR.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class ASRConfig
{
public:
    struct Key;

    // Maximum length of a key name
    static constexpr size_t kMaxConfigKeyNameLength = 128;

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
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_WiFiStationSecType;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_WiFiSSID;
    static const Key kConfigKey_WiFiPassword;
    static const Key kConfigKey_WiFiSecurity;
    static const Key kConfigKey_WiFiMode;

#if !CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
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
#endif
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
#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
    static CHIP_ERROR ReadFactoryConfigValue(asr_matter_partition_t matter_partition, uint8_t * buf, size_t bufSize,
                                             size_t & outLen);
    static CHIP_ERROR ReadFactoryConfigValue(asr_matter_partition_t matter_partition, uint32_t & val);
#endif
    static CHIP_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);
};

struct ASRConfig::Key
{
    const char * Namespace;
    const char * Name;

    CHIP_ERROR to_str(char * buf, size_t buf_size) const;
    size_t len() const;
    bool operator==(const Key & other) const;
};

inline CHIP_ERROR ASRConfig::Key::to_str(char * buf, size_t buf_size) const
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
inline size_t ASRConfig::Key::len() const
{
    // + 1 for separating ';'
    size_t out_size = strlen(Namespace) + strlen(Name) + 1;
    return out_size;
}

inline bool ASRConfig::Key::operator==(const Key & other) const
{
    return strcmp(Namespace, other.Namespace) == 0 && strcmp(Name, other.Name) == 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
