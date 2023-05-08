/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#pragma once

#include <functional>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/k32w/common/RamStorage.h>
#include <platform/nxp/k32w/common/RamStorageKey.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *
 * This implementation uses the NXP Persistent Data Manager (PDM) library as the
 * underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class K32WConfig
{
public:
    using Key = RamStorageKey;

    /* Storage for factory data settings. Retained during factory reset. */
    static RamStorage sFactoryStorage;
    /* Storage for dynamic runtime config values. Cleared during factory reset. */
    static RamStorage sConfigStorage;
    /* Storage for dynamic counter values. Cleared during factory reset. */
    static RamStorage sCounterStorage;

    // Key definitions for well-known configuration values.
    // Factory Config Keys
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;

    // CHIP Config Keys
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_UniqueId;
    static const Key kConfigKey_SoftwareVersion;

    // CHIP Counter Keys
    static const Key kCounterKey_RebootCount;
    static const Key kCounterKey_UpTime;
    static const Key kCounterKey_TotalOperationalHours;
    static const Key kCounterKey_BootReason;

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    template <typename TValue>
    static CHIP_ERROR ReadConfigValue(Key key, TValue & val);
    template <typename TValue>
    static CHIP_ERROR WriteConfigValue(Key key, TValue val);
    template <typename TValue>
    static CHIP_ERROR WriteConfigValueSync(Key key, TValue val);

    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
};

template <typename TValue>
CHIP_ERROR K32WConfig::ReadConfigValue(Key key, TValue & val)
{
    CHIP_ERROR err;
    uint16_t valLen = sizeof(val);

    err = key.Read((uint8_t *) &val, valLen);
    SuccessOrExit(err);

exit:
    return err;
}

template <typename TValue>
CHIP_ERROR K32WConfig::WriteConfigValue(Key key, TValue val)
{
    return key.Write((uint8_t *) &val, sizeof(TValue));
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
