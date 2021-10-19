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
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

void ConfigurationManagerImpl::InitializeWithObject(jobject managerObject)
{
    JNIEnv * env                     = JniReferences::GetInstance().GetEnvForCurrentThread();
    mConfigurationManagerObject      = env->NewGlobalRef(managerObject);
    jclass configurationManagerClass = env->GetObjectClass(mConfigurationManagerObject);
    VerifyOrReturn(configurationManagerClass != nullptr, ChipLogError(DeviceLayer, "Failed to get KVS Java class"));

    AndroidConfig::InitializeWithObject(managerObject);
}

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    // TODO(#742): query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset() {}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
CHIP_ERROR ConfigurationManagerImpl::GetWiFiStationSecurityType(WiFiAuthSecurityType & secType)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::UpdateWiFiStationSecurityType(WiFiAuthSecurityType secType)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    return;
}

} // namespace DeviceLayer
} // namespace chip
