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
 *          for K32W platforms using the NXP SDK.
 */

#ifndef CONFIGURATION_MANAGER_IMPL_H
#define CONFIGURATION_MANAGER_IMPL_H

#include "K32WConfig.h"
#include <platform/internal/GenericConfigurationManagerImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {
class NetworkProvisioningServerImpl;
}

/**
 * Concrete implementation of the ConfigurationManager singleton object for the K32W platform.
 */
class ConfigurationManagerImpl final : public ConfigurationManager,
                                       public Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>,
                                       private Internal::K32WConfig
{
    // Allow the ConfigurationManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConfigurationManager;

    // Allow the GenericConfigurationManagerImpl base class to access helper methods and types
    // defined on this class.
    friend class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR _Init(void);
    CHIP_ERROR _GetPrimaryWiFiMACAddress(uint8_t * buf);
    bool _CanFactoryReset(void);
    void _InitiateFactoryReset(void);
    CHIP_ERROR _ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    CHIP_ERROR _WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value);

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.

    // ===== Members for internal use by the following friends.

    friend class Internal::NetworkProvisioningServerImpl;
    friend ConfigurationManager & ConfigurationMgr(void);
    friend ConfigurationManagerImpl & ConfigurationMgrImpl(void);

    static ConfigurationManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    static void DoFactoryReset(intptr_t arg);
};

/**
 * Returns the public interface of the ConfigurationManager singleton object.
 *
 * Chip applications should use this to access features of the ConfigurationManager object
 * that are common to all platforms.
 */
inline ConfigurationManager & ConfigurationMgr(void)
{
    return ConfigurationManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConfigurationManager singleton object.
 *
 * Chio applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the K32W platform.
 */
inline ConfigurationManagerImpl & ConfigurationMgrImpl(void)
{
    return ConfigurationManagerImpl::sInstance;
}

inline CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CONFIGURATION_MANAGER_IMPL_H
