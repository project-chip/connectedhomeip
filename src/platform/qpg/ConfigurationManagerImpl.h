/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for Qorvo QPG platforms.
 */

#pragma once

#include <platform/internal/GenericConfigurationManagerImpl.h>
#include <platform/qpg/qpgConfig.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConfigurationManager singleton object for the platform.
 */
class ConfigurationManagerImpl : public Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>,
                                 private Internal::QPGConfig
{
    // Allow the GenericConfigurationManagerImpl base class to access helper methods and types
    // defined on this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;
#endif

public:
    // This returns an instance of this class.
    static ConfigurationManagerImpl & GetDefaultInstance();

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR Init(void) override;
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override;
    bool CanFactoryReset(void) override;
    void InitiateFactoryReset(void) override;
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) override;
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value) override;

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.

    // ===== Private members reserved for use by this class only.

    static void DoFactoryReset(intptr_t arg);
};

inline CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace DeviceLayer
} // namespace chip
