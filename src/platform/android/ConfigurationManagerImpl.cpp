/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for Android platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ifaddrs.h>
#include <netpacket/packet.h>

#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConfigurationManager.h>
#include <platform/android/AndroidConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

void ConfigurationManagerImpl::InitializeWithObject(jobject managerObject)
{
    VerifyOrReturn(mConfigurationManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(DeviceLayer, "Failed to init mConfigurationManagerObject"));
    AndroidConfig::InitializeWithObject(managerObject);
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    // TODO(#742): query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset() {}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return AndroidConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return AndroidConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return AndroidConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return AndroidConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return AndroidConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return AndroidConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return AndroidConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return AndroidConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return AndroidConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return AndroidConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return AndroidConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    AndroidConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    return;
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersion(uint32_t & softwareVer)
{
    CHIP_ERROR err;
    uint32_t u32SoftwareVer = 0;
    err                     = ReadConfigValue(AndroidConfig::kConfigKey_SoftwareVersion, u32SoftwareVer);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        softwareVer = CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION;
    }
    else
    {
        softwareVer = u32SoftwareVer;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t u32SoftwareVerSize = 0; // without counting null-terminator
    err = ReadConfigValueStr(AndroidConfig::kConfigKey_SoftwareVersionString, buf, bufSize, u32SoftwareVerSize);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetUniqueId(char * buf, size_t bufSize)
{
    size_t dateLen;
    return ReadConfigValueStr(AndroidConfig::kConfigKey_UniqueId, buf, bufSize, dateLen);
}

CHIP_ERROR ConfigurationManagerImpl::GetDeviceTypeId(uint32_t & deviceType)
{
    CHIP_ERROR err;
    uint32_t u32DeviceTypeId = 0;
    err                      = ReadConfigValue(AndroidConfig::kConfigKey_DeviceTypeId, u32DeviceTypeId);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        deviceType = CHIP_DEVICE_CONFIG_DEVICE_TYPE;
    }
    else
    {
        deviceType = u32DeviceTypeId;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetCommissionableDeviceName(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t u32DeviceNameSize = 0;
    err                      = ReadConfigValueStr(AndroidConfig::kConfigKey_DeviceName, buf, bufSize, u32DeviceNameSize);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_NAME);
    }

    return CHIP_NO_ERROR;
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
