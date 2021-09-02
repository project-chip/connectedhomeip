/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Utilities for accessing persisted device configuration on
 *          Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/Zephyr/ZephyrConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <settings/settings.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Helper macro to define and check length of a configuration key
#define CONFIG_KEY(key)                                                                                                            \
    (key);                                                                                                                         \
    static_assert(sizeof(key) <= SETTINGS_MAX_NAME_LEN, "Config key too long: " key)

// Config namespaces
// clang-format off
#define NAMESPACE_FACTORY  "chip-fact/"
#define NAMESPACE_CONFIG   "chip-conf/"
#define NAMESPACE_COUNTERS "chip-cntr/"
// clang-format on

// Keys stored in the chip factory nam
const ZephyrConfig::Key ZephyrConfig::kConfigKey_SerialNum           = CONFIG_KEY(NAMESPACE_FACTORY "serial-num");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_MfrDeviceId         = CONFIG_KEY(NAMESPACE_FACTORY "device-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_MfrDeviceCert       = CONFIG_KEY(NAMESPACE_FACTORY "device-cert");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_MfrDeviceICACerts   = CONFIG_KEY(NAMESPACE_FACTORY "device-ca-certs");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_MfrDevicePrivateKey = CONFIG_KEY(NAMESPACE_FACTORY "device-key");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_ProductRevision     = CONFIG_KEY(NAMESPACE_FACTORY "product-rev");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_ManufacturingDate   = CONFIG_KEY(NAMESPACE_FACTORY "mfg-date");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_SetupPinCode        = CONFIG_KEY(NAMESPACE_FACTORY "pin-code");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_SetupDiscriminator  = CONFIG_KEY(NAMESPACE_FACTORY "discriminator");
// Keys stored in the chip config namespace
// NOTE: update sAllResettableConfigKeys definition when adding a new entry below
const ZephyrConfig::Key ZephyrConfig::kConfigKey_FabricId                    = CONFIG_KEY(NAMESPACE_CONFIG "fabric-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_ServiceConfig               = CONFIG_KEY(NAMESPACE_CONFIG "service-config");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_PairedAccountId             = CONFIG_KEY(NAMESPACE_CONFIG "account-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_ServiceId                   = CONFIG_KEY(NAMESPACE_CONFIG "service-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_FabricSecret                = CONFIG_KEY(NAMESPACE_CONFIG "fabric-secret");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_GroupKeyIndex               = CONFIG_KEY(NAMESPACE_CONFIG "group-key-index");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_LastUsedEpochKeyId          = CONFIG_KEY(NAMESPACE_CONFIG "last-ek-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_FailSafeArmed               = CONFIG_KEY(NAMESPACE_CONFIG "fail-safe-armed");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_OperationalDeviceId         = CONFIG_KEY(NAMESPACE_CONFIG "op-device-id");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_OperationalDeviceCert       = CONFIG_KEY(NAMESPACE_CONFIG "op-device-cert");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_OperationalDeviceICACerts   = CONFIG_KEY(NAMESPACE_CONFIG "op-device-ca-certs");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_OperationalDevicePrivateKey = CONFIG_KEY(NAMESPACE_CONFIG "op-device-key");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_RegulatoryLocation          = CONFIG_KEY(NAMESPACE_CONFIG "regulatory-location");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_CountryCode                 = CONFIG_KEY(NAMESPACE_CONFIG "country-code");
const ZephyrConfig::Key ZephyrConfig::kConfigKey_Breadcrumb                  = CONFIG_KEY(NAMESPACE_CONFIG "breadcrumb");
namespace {

constexpr const char * sAllResettableConfigKeys[] = { ZephyrConfig::kConfigKey_FabricId,
                                                      ZephyrConfig::kConfigKey_ServiceConfig,
                                                      ZephyrConfig::kConfigKey_PairedAccountId,
                                                      ZephyrConfig::kConfigKey_ServiceId,
                                                      ZephyrConfig::kConfigKey_FabricSecret,
                                                      ZephyrConfig::kConfigKey_GroupKeyIndex,
                                                      ZephyrConfig::kConfigKey_LastUsedEpochKeyId,
                                                      ZephyrConfig::kConfigKey_FailSafeArmed,
                                                      ZephyrConfig::kConfigKey_OperationalDeviceId,
                                                      ZephyrConfig::kConfigKey_OperationalDeviceCert,
                                                      ZephyrConfig::kConfigKey_OperationalDeviceICACerts,
                                                      ZephyrConfig::kConfigKey_OperationalDevicePrivateKey,
                                                      ZephyrConfig::kConfigKey_RegulatoryLocation,
                                                      ZephyrConfig::kConfigKey_CountryCode,
                                                      ZephyrConfig::kConfigKey_Breadcrumb };

// Data structure to be passed as a parameter of Zephyr's settings_load_subtree_direct() function
struct ReadRequest
{
    void * const destination; // NOTE: can be nullptr in which case `configSize` should still be returned
    const size_t bufferSize;  // size of destination buffer
    CHIP_ERROR result;        // [out] read result
    size_t configSize;        // [out] size of configuration value
};

// Callback for Zephyr's settings_load_subtree_direct() function
int ConfigValueCallback(const char * name, size_t configSize, settings_read_cb readCb, void * cbArg, void * param)
{
    // If requested a config key X, process just node X and ignore all its descendants: X/*
    if (settings_name_next(name, nullptr) > 0)
        return 0;

    ReadRequest & request = *reinterpret_cast<ReadRequest *>(param);

    if (!request.destination || configSize > request.bufferSize)
    {
        request.result     = CHIP_ERROR_BUFFER_TOO_SMALL;
        request.configSize = configSize;
        return 0;
    }

    // Found requested key
    const ssize_t bytesRead = readCb(cbArg, request.destination, request.bufferSize);
    request.result          = bytesRead > 0 ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    request.configSize      = bytesRead > 0 ? bytesRead : 0;
    return 0;
}

// Read configuration value of maximum size `bufferSize` and store the actual size in `configSize`.
CHIP_ERROR ReadConfigValueImpl(const ZephyrConfig::Key key, void * const destination, const size_t bufferSize, size_t & configSize)
{
    ReadRequest request{ destination, bufferSize, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND, 0 };
    settings_load_subtree_direct(key, ConfigValueCallback, &request);
    configSize = request.configSize;
    return request.result;
}

CHIP_ERROR WriteConfigValueImpl(const ZephyrConfig::Key key, const void * const source, const size_t length)
{
    if (settings_save_one(key, source, length) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

template <class T>
inline CHIP_ERROR ReadSimpleConfigValue(const ZephyrConfig::Key key, T & value)
{
    CHIP_ERROR result;
    T tempValue;
    size_t configSize;

    result = ReadConfigValueImpl(key, &tempValue, sizeof(T), configSize);
    SuccessOrExit(result);

    // For simple types require that size of the output variable matches size of the configuration value
    VerifyOrExit(configSize == sizeof(T), result = CHIP_ERROR_INVALID_ARGUMENT);
    value = tempValue;
exit:
    return result;
}
} // namespace

CHIP_ERROR ZephyrConfig::Init()
{
    if (settings_subsys_init() != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, bool & val)
{
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, uint32_t & val)
{
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR ZephyrConfig::ReadConfigValue(Key key, uint64_t & val)
{
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR ZephyrConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    // Pretend that the buffer is smaller by 1 to secure space for null-character
    const CHIP_ERROR result = ReadConfigValueImpl(key, buf, bufSize ? bufSize - 1 : 0, outLen);

    // Add trailing null-character unless it's already there (NOTE: Zephyr forbids configuration
    // values of size 0, so we put "\0" when a user wants to store an empty string).
    if (result == CHIP_NO_ERROR)
    {
        if (buf[outLen - 1]) // CHIP_NO_ERROR implies outLen > 0
            buf[outLen] = 0;
        else
            outLen--;
    }

    return result;
}

CHIP_ERROR ZephyrConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueImpl(key, buf, bufSize, outLen);
}

CHIP_ERROR ZephyrConfig::ReadConfigValueCounter(::chip::Platform::PersistedStorage::Key counterId, uint32_t & val)
{
    char key[SETTINGS_MAX_NAME_LEN];

    if (!BuildCounterConfigKey(counterId, key))
        return CHIP_ERROR_INVALID_ARGUMENT;

    return ReadConfigValue(key, val);
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, bool val)
{
    return WriteConfigValueImpl(key, &val, sizeof(bool));
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueImpl(key, &val, sizeof(uint32_t));
}

CHIP_ERROR ZephyrConfig::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueImpl(key, &val, sizeof(uint64_t));
}

CHIP_ERROR ZephyrConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, str ? strlen(str) : 0);
}

CHIP_ERROR ZephyrConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    // NOTE: Zephyr forbids configuration values of size 0, so we put "\0" in such a case
    if (str && strLen == 0)
    {
        str    = "\0";
        strLen = 1;
    }

    return WriteConfigValueImpl(key, str, strLen);
}

CHIP_ERROR ZephyrConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueImpl(key, data, dataLen);
}

CHIP_ERROR ZephyrConfig::WriteConfigValueCounter(::chip::Platform::PersistedStorage::Key counterId, uint32_t val)
{
    char key[SETTINGS_MAX_NAME_LEN];

    if (!BuildCounterConfigKey(counterId, key))
        return CHIP_ERROR_INVALID_ARGUMENT;

    return WriteConfigValue(key, val);
}

CHIP_ERROR ZephyrConfig::ClearConfigValue(Key key)
{
    if (settings_delete(key) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

bool ZephyrConfig::ConfigValueExists(Key key)
{
    size_t configSize;
    return ReadConfigValueImpl(key, nullptr, 0, configSize) == CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR ZephyrConfig::FactoryResetConfig(void)
{
    for (const auto key : sAllResettableConfigKeys)
        if (settings_delete(key) != 0)
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

void ZephyrConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<ZephyrConfig>();
}

bool ZephyrConfig::BuildCounterConfigKey(::chip::Platform::PersistedStorage::Key counterId, char key[SETTINGS_MAX_NAME_LEN])
{
    constexpr size_t KEY_PREFIX_LEN = sizeof(NAMESPACE_COUNTERS) - 1;
    const size_t keySuffixLen       = strlen(counterId) + 1; // including null-character

    if (KEY_PREFIX_LEN + keySuffixLen > SETTINGS_MAX_NAME_LEN)
        return false;

    memcpy(&key[0], NAMESPACE_COUNTERS, KEY_PREFIX_LEN);
    memcpy(&key[KEY_PREFIX_LEN], counterId, keySuffixLen);
    return true;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
