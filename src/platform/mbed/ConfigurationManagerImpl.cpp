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
    // TODO: Implement once Mbed interface integration is in place.
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    return ReadCounter(key, value);
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
