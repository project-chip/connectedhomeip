/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the ConfigurationManager object
 *          for the Beken platform.
 */
#pragma once

#include <platform/Beken/BekenConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

namespace chip {
namespace DeviceLayer {
/**
 * Concrete implementation of the ConfigurationManager singleton object for the Beken platform.
 */
class ConfigurationManagerImpl : public Internal::GenericConfigurationManagerImpl<Internal::BekenConfig>
{
public:
    // This returns an instance of this class.
    static ConfigurationManagerImpl & GetDefaultInstance();

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR Init(void);
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf);
    bool CanFactoryReset(void);
    void InitiateFactoryReset(void);
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value);
    CHIP_ERROR GetRebootCount(uint32_t & rebootCount);
    CHIP_ERROR StoreRebootCount(uint32_t rebootCount);
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours);
    CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours);
    CHIP_ERROR GetBootReason(uint32_t & bootReason);
    CHIP_ERROR StoreBootReason(uint32_t bootReason);

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.

    // ===== Members that implement the GenericConfigurationManagerImpl protected interface.
    CHIP_ERROR ReadConfigValue(Key key, bool & val);
    CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR WriteConfigValue(Key key, bool val);
    CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    void RunConfigUnitTest(void);

    // ===== Private members reserved for use by this class only.

    static void DoFactoryReset(intptr_t arg);
};

/**
 * Returns the platform-specific implementation of the ConfigurationManager object.
 *
 * Applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the selected platform.
 */
ConfigurationManager & ConfigurationMgrImpl();

} // namespace DeviceLayer
} // namespace chip
