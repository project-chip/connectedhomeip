/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 * @file
 * Provides the implementation of the Device Layer ConfigurationManager object for Renesas platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint32_t rebootCount;
    if (RenesasConfig::ConfigValueExists(RenesasConfig::kConfigKey_RebootCount))
    {
        err = GetRebootCount(rebootCount);
        SuccessOrExit(err);

        err = StoreRebootCount(rebootCount + 1);
        SuccessOrExit(err);
    }
    else
    {
        // The first boot after factory reset of the Node.
        err = StoreRebootCount(1);
        SuccessOrExit(err);
    }

    if (!RenesasConfig::ConfigValueExists(RenesasConfig::kConfigKey_BootReason))
    {
        err = StoreBootReason(to_underlying(BootReasonType::kUnspecified));
        SuccessOrExit(err);
    }

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<RenesasConfig>::Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(RenesasConfig::kConfigKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(RenesasConfig::kConfigKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(RenesasConfig::kConfigKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(RenesasConfig::kConfigKey_BootReason, bootReason);
}

bool ConfigurationManagerImpl::CanFactoryReset(void)
{
    // Implement InitiateFactoryReset first before enabling
    return false;
}

void ConfigurationManagerImpl::InitiateFactoryReset(void)
{
    // TODO implement
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrReturnError(keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    return PersistedStorage::KeyValueStoreMgr().Get(key, &value, sizeof(value));
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrReturnError(keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    return PersistedStorage::KeyValueStoreMgr().Put(key, &value, sizeof(value));
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return RenesasConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return RenesasConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return RenesasConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return RenesasConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return RenesasConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return RenesasConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return RenesasConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    RenesasConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    /// Implement factory reset
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
