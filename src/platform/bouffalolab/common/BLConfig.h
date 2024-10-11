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

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the Bouffalo Lab platform.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class BLConfig
{
public:
    using Key = const char *;

    /** Key definitions for well-known keys */
    /** Manufacturing config keys, which should be saved in a specified place */
    static constexpr char kConfigKey_SerialNum[]             = ("serial-num");
    static constexpr char kConfigKey_MfrDeviceId[]           = ("device-id");
    static constexpr char kConfigKey_MfrDeviceCert[]         = ("device-cert");
    static constexpr char kConfigKey_MfrDeviceICACerts[]     = ("device-ca-certs");
    static constexpr char kConfigKey_MfrDevicePrivateKey[]   = ("device-key");
    static constexpr char kConfigKey_ManufacturingDate[]     = ("mfg-date");
    static constexpr char kConfigKey_SetupPinCode[]          = ("pin-code");
    static constexpr char kConfigKey_SetupDiscriminator[]    = ("discriminator");
    static constexpr char kConfigKey_Spake2pIterationCount[] = ("iteration-count");
    static constexpr char kConfigKey_Spake2pSalt[]           = ("salt");
    static constexpr char kConfigKey_Spake2pVerifier[]       = ("verifier");
    static constexpr char kConfigKey_UniqueId[]              = ("unique-id");

    /** Config keys, which should be droped after a factory reset */
    static constexpr char kConfigKey_FabricId[]                    = ("fabric-id");
    static constexpr char kConfigKey_ServiceConfig[]               = ("service-config");
    static constexpr char kConfigKey_PairedAccountId[]             = ("account-id");
    static constexpr char kConfigKey_ServiceId[]                   = ("service-id");
    static constexpr char kConfigKey_FabricSecret[]                = ("fabric-secret");
    static constexpr char kConfigKey_HardwareVersion[]             = ("hardware-ver");
    static constexpr char kConfigKey_LastUsedEpochKeyId[]          = ("last-ek-id");
    static constexpr char kConfigKey_FailSafeArmed[]               = ("fail-safe-armed");
    static constexpr char kConfigKey_OperationalDeviceId[]         = ("op-device-id");
    static constexpr char kConfigKey_OperationalDeviceCert[]       = ("op-device-cert");
    static constexpr char kConfigKey_OperationalDeviceICACerts[]   = ("op-device-ca-certs");
    static constexpr char kConfigKey_OperationalDevicePrivateKey[] = ("op-device-key");
    static constexpr char kConfigKey_RegulatoryLocation[]          = ("regulatory-location");
    static constexpr char kConfigKey_CountryCode[]                 = ("country-code");
    static constexpr char kConfigKey_ActiveLocale[]                = ("active-locale");
    static constexpr char kConfigKey_Breadcrumb[]                  = ("breadcrumb");
    static constexpr char kConfigKey_GroupKeyIndex[]               = ("group-key-index");
    static constexpr char kConfigKey_LifeTimeCounter[]             = ("life-time-counter");

    static constexpr char kConfigKey_WiFiSSID[]     = ("bl-wifi-ssid");
    static constexpr char kConfigKey_WiFiPassword[] = ("bl-wifi-pass");

    /** Counter Keys, diagnostic information  */
    static constexpr char kCounterKey_RebootCount[]           = ("reboot-count");
    static constexpr char kCounterKey_TotalOperationalHours[] = ("total-hours");
    static constexpr char kCounterKey_UpTime[]                = ("up-time");

    static constexpr char kBLKey_factoryResetFlag[] = ("__factory_reset_pending");

    static void Init(void);

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
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
