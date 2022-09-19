/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Utilities for interacting with the the BL702 Easyflash module.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define KCONFIG_SECT_FACTORY "chip-factory"
#define KCONFIG_SECT_CONFIG "chip-config"
#define KCONFIG_SECT_COUTNER "chip-counter"
#define KCONFIG_SECT_KVS "chip-kvs"
#define KCONFIG_SECT_PSV "chip-psv"
#define KCONFIG_MAX_LEN sizeof(KCONFIG_SECT_FACTORY)

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
    static constexpr const char * kConfigKey_SerialNum             = (KCONFIG_SECT_FACTORY "_serial-num");
    static constexpr const char * kConfigKey_MfrDeviceId           = (KCONFIG_SECT_FACTORY "_device-id");
    static constexpr const char * kConfigKey_MfrDeviceCert         = (KCONFIG_SECT_FACTORY "_device-cert");
    static constexpr const char * kConfigKey_MfrDeviceICACerts     = (KCONFIG_SECT_FACTORY "_device-ca-certs");
    static constexpr const char * kConfigKey_MfrDevicePrivateKey   = (KCONFIG_SECT_FACTORY "_device-key");
    static constexpr const char * kConfigKey_ManufacturingDate     = (KCONFIG_SECT_FACTORY "_mfg-date");
    static constexpr const char * kConfigKey_SetupPinCode          = (KCONFIG_SECT_FACTORY "_pin-code");
    static constexpr const char * kConfigKey_SetupDiscriminator    = (KCONFIG_SECT_FACTORY "_discriminator");
    static constexpr const char * kConfigKey_Spake2pIterationCount = (KCONFIG_SECT_FACTORY "_iteration-count");
    static constexpr const char * kConfigKey_Spake2pSalt           = (KCONFIG_SECT_FACTORY "_salt");
    static constexpr const char * kConfigKey_Spake2pVerifier       = (KCONFIG_SECT_FACTORY "_verifier");
    static constexpr const char * kConfigKey_UniqueId              = (KCONFIG_SECT_FACTORY "_unique-id");

    /** Config keys, which should be droped after a factory reset */
    static constexpr const char * kConfigKey_FabricId           = (KCONFIG_SECT_CONFIG "_fabric-id");
    static constexpr const char * kConfigKey_ServiceConfig      = (KCONFIG_SECT_CONFIG "_service-config");
    static constexpr const char * kConfigKey_PairedAccountId    = (KCONFIG_SECT_CONFIG "_account-id");
    static constexpr const char * kConfigKey_ServiceId          = (KCONFIG_SECT_CONFIG "_service-id");
    static constexpr const char * kConfigKey_FabricSecret       = (KCONFIG_SECT_CONFIG "_fabric-secret");
    static constexpr const char * kConfigKey_HardwareVersion    = (KCONFIG_SECT_CONFIG "_hardware-rev");
    static constexpr const char * kConfigKey_LastUsedEpochKeyId = (KCONFIG_SECT_CONFIG "_last-ek-id");
    static constexpr const char * kConfigKey_FailSafeArmed      = (KCONFIG_SECT_CONFIG "_fail-safe-armed");
    static constexpr const char * kConfigKey_WiFiStationSecType = (KCONFIG_SECT_CONFIG "_sta-sec-type");
    static constexpr const char * kConfigKey_RegulatoryLocation = (KCONFIG_SECT_CONFIG "_regulatory-location");
    static constexpr const char * kConfigKey_CountryCode        = (KCONFIG_SECT_CONFIG "_country-code");
    static constexpr const char * kConfigKey_ActiveLocale       = (KCONFIG_SECT_CONFIG "_active-locale");
    static constexpr const char * kConfigKey_Breadcrumb         = (KCONFIG_SECT_CONFIG "_breadcrumb");
    static constexpr const char * kConfigKey_GroupKeyIndex      = (KCONFIG_SECT_CONFIG "_group-key-index");
    static constexpr const char * kBLConfigKey_wifissid         = (KCONFIG_SECT_CONFIG "blConfig_wifi-ssid");
    static constexpr const char * kBLConfigKey_wifipassword     = (KCONFIG_SECT_CONFIG "blConfig_wifi-pwd");

    /** Counter Keys, diagnostic information  */
    static constexpr const char * kCounterKey_BootCount             = (KCONFIG_SECT_COUTNER "_boot-count");
    static constexpr const char * kCounterKey_TotalOperationalHours = (KCONFIG_SECT_COUTNER "_total-hours");
    static constexpr const char * kCounterKey_UpTime                = (KCONFIG_SECT_COUTNER "_up-time");

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
