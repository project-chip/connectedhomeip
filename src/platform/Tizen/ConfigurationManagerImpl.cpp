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
    size_t len;

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

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_SoftwareVersionString))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxSoftwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        err = StoreSoftwareVersionString(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING, len);
        SuccessOrExit(err);
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_HardwareVersionString))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxHardwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        err = StoreHardwareVersionString(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING, len);
        SuccessOrExit(err);
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_VendorName))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxVendorNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        err = StoreVendorName(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME, len);
        SuccessOrExit(err);
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_ProductName))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxProductNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        err = StoreProductName(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME, len);
        SuccessOrExit(err);
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_SerialNum))
    {
        len = strlen(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxSerialNumberLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        err = StoreSerialNumber(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, len);
        SuccessOrExit(err);
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

CHIP_ERROR ConfigurationManagerImpl::StoreSoftwareVersionString(const char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxSoftwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_SoftwareVersionString, buf, bufSize);
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    // Read from Posix config.
    size_t outLen;
    auto err = ReadConfigValueStr(PosixConfig::kConfigKey_SoftwareVersionString, buf, bufSize, outLen);

    // If not found use genereic implementation, which gets value from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        return Internal::GenericConfigurationManagerImpl<Internal::PosixConfig>::GetSoftwareVersionString(buf, bufSize);

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::StoreHardwareVersionString(const char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxHardwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_HardwareVersionString, buf, bufSize);
}

CHIP_ERROR ConfigurationManagerImpl::StoreVendorName(const char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxVendorNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_VendorName, buf, bufSize);
}

CHIP_ERROR ConfigurationManagerImpl::StoreProductName(const char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxProductNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_ProductName, buf, bufSize);
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
