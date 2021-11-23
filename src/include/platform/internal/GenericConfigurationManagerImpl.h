/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an generic implementation of ConfigurationManager features
 *          for use on various platforms.
 */

#pragma once

#include <lib/support/BitFlags.h>
#include <platform/ConfigurationManager.h>

#if CHIP_ENABLE_ROTATING_DEVICE_ID
#include <lib/support/LifetimePersistedCounter.h>
#endif

namespace chip {
namespace DeviceLayer {

class ProvisioningDataSet;

namespace Internal {

/**
 * Provides a generic implementation of ConfigurationManager features that works on multiple platforms.
 *
 * This template contains implementations of select features from the ConfigurationManager abstract
 * interface that are suitable for use on all platforms.  It is intended to be inherited (directly
 * or indirectly) by the ConfigurationManagerImpl class.
 */
template <class ConfigClass>
class GenericConfigurationManagerImpl : public ConfigurationManager
{
public:
    // ===== Methods that implement the ConfigurationManager abstract interface.

    CHIP_ERROR Init() override;
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetProductRevisionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductRevision(uint16_t & productRev) override;
    CHIP_ERROR StoreProductRevision(uint16_t productRev) override;
    CHIP_ERROR GetFirmwareRevisionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetFirmwareRevision(uint16_t & firmwareRev) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen) override;
    CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan buf) override;
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) override;
    CHIP_ERROR StorePrimaryWiFiMACAddress(const uint8_t * buf) override;
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf) override;
    CHIP_ERROR StorePrimary802154MACAddress(const uint8_t * buf) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth) override;
    CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) override;
    CHIP_ERROR GetSetupPinCode(uint32_t & setupPinCode) override;
    CHIP_ERROR StoreSetupPinCode(uint32_t setupPinCode) override;
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR StoreSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter) override;
    CHIP_ERROR _IncrementLifetimeCounter();
    CHIP_ERROR GetFailSafeArmed(bool & val) override;
    CHIP_ERROR SetFailSafeArmed(bool val) override;
    CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo) override;
    bool IsCommissionableDeviceTypeEnabled() override;
    CHIP_ERROR GetDeviceTypeId(uint16_t & deviceType) override;
    bool IsCommissionableDeviceNameEnabled() override;
    CHIP_ERROR GetCommissionableDeviceName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint) override;
    CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize) override;
    CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint) override;
    CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRegulatoryLocation(uint32_t & location) override;
    CHIP_ERROR StoreRegulatoryLocation(uint32_t location) override;
    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen) override;
    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen) override;
    CHIP_ERROR GetBreadcrumb(uint64_t & breadcrumb) override;
    CHIP_ERROR StoreBreadcrumb(uint64_t breadcrumb) override;
    CHIP_ERROR GetRebootCount(uint32_t & rebootCount) override;
    CHIP_ERROR StoreRebootCount(uint32_t rebootCount) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours) override;
    CHIP_ERROR GetBootReason(uint32_t & bootReason) override;
    CHIP_ERROR StoreBootReason(uint32_t bootReason) override;
    CHIP_ERROR RunUnitTests(void) override;
    bool IsFullyProvisioned() override;
    void InitiateFactoryReset() override;
    void LogDeviceConfig() override;

    virtual ~GenericConfigurationManagerImpl() = default;

protected:
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    chip::LifetimePersistedCounter mLifetimePersistedCounter;
#endif
    CHIP_ERROR PersistProvisioningData(ProvisioningDataSet & provData);

    // Methods to read and write configuration values, as well as run the configuration unit test.
    typedef typename ConfigClass::Key Key;
    virtual CHIP_ERROR ReadConfigValue(Key key, bool & val)                                        = 0;
    virtual CHIP_ERROR ReadConfigValue(Key key, uint32_t & val)                                    = 0;
    virtual CHIP_ERROR ReadConfigValue(Key key, uint64_t & val)                                    = 0;
    virtual CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)    = 0;
    virtual CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen) = 0;
    virtual CHIP_ERROR WriteConfigValue(Key key, bool val)                                         = 0;
    virtual CHIP_ERROR WriteConfigValue(Key key, uint32_t val)                                     = 0;
    virtual CHIP_ERROR WriteConfigValue(Key key, uint64_t val)                                     = 0;
    virtual CHIP_ERROR WriteConfigValueStr(Key key, const char * str)                              = 0;
    virtual CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen)               = 0;
    virtual CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)          = 0;
    virtual void RunConfigUnitTest(void)                                                           = 0;
};

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetVendorId(uint16_t & vendorId)
{
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetProductId(uint16_t & productId)
{
    productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetFirmwareRevision(uint16_t & firmwareRev)
{
    firmwareRev = static_cast<uint32_t>(CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetDeviceTypeId(uint16_t & deviceType)
{
    deviceType = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_TYPE);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetInitialPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT);
    return CHIP_NO_ERROR;
}

template <class ConfigClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ConfigClass>::GetSecondaryPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
