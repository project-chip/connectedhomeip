/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          for the PSoC6.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericConfigurationManagerImpl.cpp>

#include <platform/ConfigurationManager.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/P6/P6Config.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

/** Singleton instance of the ConfigurationManager implementation object for the PSoC6.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool failSafeArmed;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err);

    // If the fail-safe was armed when the device last shutdown, initiate a factory reset.
    if (_GetFailSafeArmed(failSafeArmed) == CHIP_NO_ERROR && failSafeArmed)
    {
        ChipLogProgress(DeviceLayer, "Detected fail-safe armed on reboot; initiating factory reset");
        _InitiateFactoryReset();
    }

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_wcm_mac_t mac;
    result = cy_wcm_get_mac_addr(CY_WCM_INTERFACE_TYPE_STA, &mac, 1);
    if (result != CY_RSLT_SUCCESS)
    {
        err = CHIP_ERROR_INTERNAL;
        ChipLogError(DeviceLayer, "_GetPrimaryWiFiMACAddress failed: %ld", result);
        return err;
    }

    /* Reverse mac address to buf pointer as it is expected by caller */
    buf[0] = mac[5];
    buf[1] = mac[4];
    buf[2] = mac[3];
    buf[3] = mac[2];
    buf[4] = mac[1];
    buf[5] = mac[0];

    return err;
}

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    // TODO: query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    uint32_t in    = 0;
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key, &in, 4);
    value          = in;
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key, static_cast<void *>(&value), 4);
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    err = FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", ErrorStr(err));
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    NVIC_SystemReset();
}

} // namespace DeviceLayer
} // namespace chip
