/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for Tizen platforms.
 */

#include "ConfigurationManagerImpl.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/Tizen/PosixConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "WiFiManager.h"
#endif

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
    CHIP_ERROR error;

    error = Internal::GenericConfigurationManagerImpl<Internal::PosixConfig>::Init();
    SuccessOrExit(error);

    if (!Internal::PosixConfig::ConfigValueExists(Internal::PosixConfig::kConfigKey_VendorId))
    {
        error = StoreVendorId(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
        SuccessOrExit(error);
    }

    if (!Internal::PosixConfig::ConfigValueExists(Internal::PosixConfig::kConfigKey_ProductId))
    {
        error = StoreProductId(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
        SuccessOrExit(error);
    }

    error = CHIP_NO_ERROR;

exit:
    return error;
}

CHIP_ERROR ConfigurationManagerImpl::StoreVendorId(uint16_t vendorId)
{
    return WriteConfigValue(Internal::PosixConfig::kConfigKey_VendorId, vendorId);
}

CHIP_ERROR ConfigurationManagerImpl::StoreProductId(uint16_t productId)
{
    return WriteConfigValue(Internal::PosixConfig::kConfigKey_ProductId, productId);
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    constexpr size_t kExpectedBufSize = ConfigurationManager::kPrimaryMACAddressLength;
    return Internal::WiFiMgr().GetDeviceMACAddress(buf, kExpectedBufSize);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset() {}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(Platform::PersistedStorage::Key key, uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(Platform::PersistedStorage::Key key, uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return Internal::PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint16_t & val)
{
    return Internal::PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return Internal::PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return Internal::PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return Internal::PosixConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return Internal::PosixConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return Internal::PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint16_t val)
{
    return Internal::PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return Internal::PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return Internal::PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return Internal::PosixConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return Internal::PosixConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return Internal::PosixConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest()
{
    Internal::PosixConfig::RunConfigUnitTest();
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
