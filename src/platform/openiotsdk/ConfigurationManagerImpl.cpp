/*
 *
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

/**
 *    @file
 *          Provides the implementation of the Device Layer ConfigurationManager class
 *          for the Open IOT SDK platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <platform/ConfigurationManager.h>

#include "cmsis.h"

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

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<OpenIoTSDKConfig>::Init();
    SuccessOrExit(err);

exit:
    return err;
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset(void)
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    CHIP_ERROR err = ReadConfigValue(key, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return OpenIoTSDKConfig::WriteCounter(key, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return OpenIoTSDKConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return OpenIoTSDKConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return OpenIoTSDKConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return OpenIoTSDKConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return OpenIoTSDKConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return OpenIoTSDKConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return OpenIoTSDKConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return OpenIoTSDKConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return OpenIoTSDKConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return OpenIoTSDKConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return OpenIoTSDKConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    OpenIoTSDKConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");
    const CHIP_ERROR err = OpenIoTSDKConfig::FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", ErrorStr(err));
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    NVIC_SystemReset();
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
