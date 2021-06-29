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
 *    @file
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for mbed platforms.
 */
/* this file behaves like a config.h, comes first */

// FIXME: Undefine the `sleep()` function included by the CHIPDeviceLayer.h
// from unistd.h to avoid a conflicting declaration with the `sleep()` provided
// by Mbed-OS in mbed_power_mgmt.h.
#define sleep unistd_sleep
#include <platform/internal/CHIPDeviceLayerInternal.h>
#undef sleep

#include <platform/internal/GenericConfigurationManagerImpl.cpp>

#include <platform/ConfigurationManager.h>

// mbed-os headers
#include "platform/mbed_power_mgmt.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    auto interface = WiFiInterface::get_default_instance();
    if (interface)
    {
        auto * mac_address = interface->get_mac_address();
        if (mac_address)
        {
            int last = -1;
            int rc =
                sscanf(mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%n", buf + 5, buf + 4, buf + 3, buf + 2, buf + 1, buf + 0, &last);
            if (rc != NSAPI_MAC_BYTES || last != (NSAPI_MAC_SIZE - 1))
            {
                ChipLogError(DeviceLayer, "Failed to extract the MAC address: %s, rc = %d, last = %d", mac_address, rc, last);
                return CHIP_ERROR_INTERNAL;
            }
            else
            {
                ChipLogError(DeviceLayer, "Extract the MAC address: %s", mac_address);
                return CHIP_NO_ERROR;
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to extract the MAC address: nothing returned by the interface");
            return CHIP_ERROR_INTERNAL;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to extract the MAC address: interface not available");
        return CHIP_ERROR_INTERNAL;
    }
}

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset(void)
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    CHIP_ERROR err = ReadConfigValue(key, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return WriteCounter(key, value);
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");
    const CHIP_ERROR err = FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", ErrorStr(err));
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    system_reset();
}

} // namespace DeviceLayer
} // namespace chip
