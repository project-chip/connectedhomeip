/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          Provides an implementation of the ConfigurationManager object
 *          for MW320 platforms using the NXP SDK.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#include "wifi.h"
#ifdef __cplusplus
}
#endif //__cplusplus

#include "MW320Config.h"
#include <platform/internal/GenericConfigurationManagerImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {
class NetworkProvisioningServerImpl;
}

/**
 * Concrete implementation of the ConfigurationManager singleton object for the MW320 platform.
 */
// class ConfigurationManagerImpl final : public ConfigurationManager,
//                                       public Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>,
//                                       private Internal::MW320Config
class ConfigurationManagerImpl final : public Internal::GenericConfigurationManagerImpl<Internal::MW320Config>
{
public:
    // This returns an instance of this class.
    static ConfigurationManagerImpl & GetDefaultInstance();

    // Allow the ConfigurationManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    //    friend class ConfigurationManager;

    // Allow the GenericConfigurationManagerImpl base class to access helper methods and types
    // defined on this class.
    //#ifndef DOXYGEN_SHOULD_SKIP_THIS
    //    friend class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;
    //#endif

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR Init(void) override;
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override;
    bool CanFactoryReset(void) override;
    void InitiateFactoryReset(void) override;
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) override;
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value) override;
    CHIP_ERROR GetRebootCount(uint32_t & rebootCount) override;
    CHIP_ERROR StoreRebootCount(uint32_t rebootCount) override;
    CHIP_ERROR GetBootReason(uint32_t & bootReasons) override;
    CHIP_ERROR StoreBootReason(uint32_t bootReasons) override;

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.

    // ===== Members that implement the GenericConfigurationManagerImpl protected interface.
    CHIP_ERROR ReadConfigValue(Key key, bool & val) override;
    CHIP_ERROR ReadConfigValue(Key key, uint32_t & val) override;
    CHIP_ERROR ReadConfigValue(Key key, uint64_t & val) override;
    CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen) override;
    CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen) override;
    CHIP_ERROR WriteConfigValue(Key key, bool val) override;
    CHIP_ERROR WriteConfigValue(Key key, uint32_t val) override;
    CHIP_ERROR WriteConfigValue(Key key, uint64_t val) override;
    CHIP_ERROR WriteConfigValueStr(Key key, const char * str) override;
    CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen) override;
    CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen) override;
    void RunConfigUnitTest(void) override;

    // ===== Members for internal use by the following friends.

    friend class Internal::NetworkProvisioningServerImpl;

    //    static ConfigurationManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    static void DoFactoryReset(intptr_t arg);
};

inline CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    wifi_mac_addr_t mac_addr;
    wifi_get_device_mac_addr(&mac_addr);

    memcpy(buf, mac_addr.mac, 6);
    PRINTF("MAC: %02x:%02x:%02x:%02x:%02x:%02x \r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    // return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    return CHIP_NO_ERROR;
}

/**
 * Returns the platform-specific implementation of the ConfigurationManager object.
 *
 * Applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the selected platform.
 */
ConfigurationManager & ConfigurationMgrImpl();

} // namespace DeviceLayer
} // namespace chip
