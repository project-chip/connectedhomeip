/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define KCONFIG_SECT_PSV "PSV"
/**
 * Provides functions and definitions for accessing device configuration information on the BL702 platform.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class BL702Config
{
public:
    using Key = const char *;

    /** Key definitions for well-known keys */
    /** Manufacturing config keys, which should be saved in a specified place */
    static constexpr const char * kConfigKey_SerialNum             = ("serial-num");
    static constexpr const char * kConfigKey_MfrDeviceId           = ("device-id");
    static constexpr const char * kConfigKey_MfrDeviceCert         = ("device-cert");
    static constexpr const char * kConfigKey_MfrDeviceICACerts     = ("device-ca-certs");
    static constexpr const char * kConfigKey_MfrDevicePrivateKey   = ("device-key");
    static constexpr const char * kConfigKey_ManufacturingDate     = ("mfg-date");
    static constexpr const char * kConfigKey_SetupPinCode          = ("pin-code");
    static constexpr const char * kConfigKey_SetupDiscriminator    = ("discriminator");
    static constexpr const char * kConfigKey_Spake2pIterationCount = ("iteration-count");
    static constexpr const char * kConfigKey_Spake2pSalt           = ("salt");
    static constexpr const char * kConfigKey_Spake2pVerifier       = ("verifier");
    static constexpr const char * kConfigKey_UniqueId              = ("unique-id");

    /** Config keys, which should be droped after a factory reset */
    static constexpr const char * kConfigKey_FabricId                    = ("fabric-id");
    static constexpr const char * kConfigKey_ServiceConfig               = ("service-config");
    static constexpr const char * kConfigKey_PairedAccountId             = ("account-id");
    static constexpr const char * kConfigKey_ServiceId                   = ("service-id");
    static constexpr const char * kConfigKey_FabricSecret                = ("fabric-secret");
    static constexpr const char * kConfigKey_HardwareVersion             = ("hardware-ver");
    static constexpr const char * kConfigKey_LastUsedEpochKeyId          = ("last-ek-id");
    static constexpr const char * kConfigKey_FailSafeArmed               = ("fail-safe-armed");
    static constexpr const char * kConfigKey_WiFiStationSecType          = ("sta-sec-type");
    static constexpr const char * kConfigKey_OperationalDeviceId         = ("op-device-id");
    static constexpr const char * kConfigKey_OperationalDeviceCert       = ("op-device-cert");
    static constexpr const char * kConfigKey_OperationalDeviceICACerts   = ("op-device-ca-certs");
    static constexpr const char * kConfigKey_OperationalDevicePrivateKey = ("op-device-key");
    static constexpr const char * kConfigKey_RegulatoryLocation          = ("regulatory-location");
    static constexpr const char * kConfigKey_CountryCode                 = ("country-code");
    static constexpr const char * kConfigKey_ActiveLocale                = ("active-locale");
    static constexpr const char * kConfigKey_Breadcrumb                  = ("breadcrumb");
    static constexpr const char * kConfigKey_GroupKeyIndex               = ("group-key-index");
    static constexpr const char * kBLConfigKey_wifissid                  = ("blConfig_wifi-ssid");
    static constexpr const char * kBLConfigKey_wifipassword              = ("blConfig_wifi-pwd");

    /** Counter Keys, diagnostic information  */
    static constexpr const char * kCounterKey_RebootCount           = ("reboot-count");
    static constexpr const char * kCounterKey_TotalOperationalHours = ("total-hours");
    static constexpr const char * kCounterKey_UpTime                = ("up-time");

    static CHIP_ERROR Init(void);

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(const char * key, uint8_t * val, size_t size, size_t & readsize);
    static CHIP_ERROR ReadConfigValue(const char * key, bool & val);
    static CHIP_ERROR ReadConfigValue(const char * key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(const char * key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen);

    static CHIP_ERROR WriteConfigValue(const char * key, uint8_t * val, size_t size);
    static CHIP_ERROR WriteConfigValue(const char * key, bool val);
    static CHIP_ERROR WriteConfigValue(const char * key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(const char * key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(const char * key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(const char * key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(const char * key, const uint8_t * data, size_t dataLen);

    static CHIP_ERROR ClearConfigValue(const char * key);
    static bool ConfigValueExists(const char * key);
    static CHIP_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);

    // internal to the platform for KeyValueStoreManagerImpl.cpp
    static CHIP_ERROR ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes);
    static CHIP_ERROR WriteKVS(const char * key, const void * value, size_t value_size);
    static CHIP_ERROR ClearKVS(const char * key);

#ifdef CFG_USE_PSRAM
    static bool isPsramRegion(void * paddr);
#endif
    static CHIP_ERROR WriteWiFiInfo(const char * ssid, const char * password);
    static CHIP_ERROR ReadWiFiInfo(const char * ssid, uint32_t ssid_size, const char * passwd, uint32_t passwd_size);
    static bool isWiFiInfoSaved();
    static CHIP_ERROR ClearWiFiInfo(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
