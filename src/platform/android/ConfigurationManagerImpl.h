/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          Provides an implementation of the ConfigurationManager object
 *          for Android platforms.
 */

#pragma once

#include <platform/android/AndroidConfig.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

#include <jni.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConfigurationManager singleton object for the Android platform.
 */
class ConfigurationManagerImpl : public Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>,
                                 private Internal::AndroidConfig
{
    // Allow the GenericConfigurationManagerImpl base class to access helper methods and types
    // defined on this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;
#endif

public:
    void InitializeWithObject(jobject managerObject);
    static ConfigurationManagerImpl & GetDefaultInstance();

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR Init() override;
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override;
    bool CanFactoryReset() override;
    void InitiateFactoryReset() override;
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) override;
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value) override;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    CHIP_ERROR GetWiFiStationSecurityType(Internal::WiFiAuthSecurityType & secType);
    CHIP_ERROR UpdateWiFiStationSecurityType(Internal::WiFiAuthSecurityType secType);
#endif

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.

    // ===== Private members reserved for use by this class only.

    static void DoFactoryReset(intptr_t arg);

    jobject mConfigurationManagerObject = nullptr;
};

} // namespace DeviceLayer
} // namespace chip
