/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#pragma once
#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConfigurationManager singleton object for the fake platform.
 */
class ConfigurationManagerImpl : public ConfigurationManager
{
public:
    virtual ~ConfigurationManagerImpl() = default;
    // NOTE: This method is required by the tests.
    // This returns an instance of this class.
    static ConfigurationManagerImpl & GetDefaultInstance();

private:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StoreHardwareVersion(uint16_t hardwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetFirmwareBuildChipEpochTime(System::Clock::Seconds32 & buildTime) override
    {
        buildTime = mFirmwareBuildChipEpochTime;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetFirmwareBuildChipEpochTime(System::Clock::Seconds32 buildTime) override
    {
        mFirmwareBuildChipEpochTime = buildTime;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreSoftwareVersion(uint32_t softwareVer) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan & buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR IncrementLifetimeCounter() override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    CHIP_ERROR GetFailSafeArmed(bool & val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetFailSafeArmed(bool val) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    bool IsCommissionableDeviceTypeEnabled() override { return false; }
    CHIP_ERROR GetDeviceTypeId(uint32_t & deviceType) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    bool IsCommissionableDeviceNameEnabled() override { return false; }
    CHIP_ERROR GetCommissionableDeviceName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRegulatoryLocation(uint8_t & location) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreRegulatoryLocation(uint8_t location) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRebootCount(uint32_t & rebootCount) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreRebootCount(uint32_t rebootCount) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetBootReason(uint32_t & bootReason) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreBootReason(uint32_t bootReason) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR StoreUniqueId(const char * uniqueId, size_t uniqueIdLen) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GenerateUniqueId(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_CONFIG_TEST
    void RunUnitTests() override {}
#endif
    bool IsFullyProvisioned() override { return false; }
    void LogDeviceConfig() override {}
    bool CanFactoryReset() override { return true; }
    void InitiateFactoryReset() override {}
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    System::Clock::Seconds32 mFirmwareBuildChipEpochTime = System::Clock::Seconds32(0);
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
