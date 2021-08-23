/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
class ConfigurationManagerImpl final : public ConfigurationManager
{
    // Allow the ConfigurationManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConfigurationManager;

private:
    CHIP_ERROR _Init() { return CHIP_NO_ERROR; }
    CHIP_ERROR _GetVendorName(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetVendorId(uint16_t & vendorId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetProductName(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetProductId(uint16_t & productId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetProductRevisionString(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetProductRevision(uint16_t & productRev) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreProductRevision(uint16_t productRev) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetFirmwareRevisionString(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetFirmwareRevision(uint32_t & firmwareRev) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, uint8_t & hour, uint8_t & minute,
                                     uint8_t & second)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreSerialNumber(const char * serialNum, size_t serialNumLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetPrimaryWiFiMACAddress(uint8_t * buf) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StorePrimaryWiFiMACAddress(const uint8_t * buf) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetPollPeriod(uint32_t & buf) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StorePrimary802154MACAddress(const uint8_t * buf) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    CHIP_ERROR _StoreDeviceId(uint64_t deviceId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreDeviceCertificate(const uint8_t * cert, size_t certLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreDevicePrivateKey(const uint8_t * key, size_t keyLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _ClearOperationalDeviceCredentials(void) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    CHIP_ERROR _GetManufacturerDeviceId(uint64_t & deviceId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreManufacturerDeviceId(uint64_t deviceId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetSetupPinCode(uint32_t & setupPinCode) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreSetupPinCode(uint32_t setupPinCode) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetSetupDiscriminator(uint16_t & setupDiscriminator) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreSetupDiscriminator(uint16_t setupDiscriminator) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetFabricId(uint64_t & fabricId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreFabricId(uint64_t fabricId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    CHIP_ERROR _GetLifetimeCounter(uint16_t & lifetimeCounter) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _IncrementLifetimeCounter() { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    CHIP_ERROR _GetServiceId(uint64_t & serviceId) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StorePairedAccountId(const char * accountId, size_t accountIdLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreServiceProvisioningData(uint64_t serviceId, const uint8_t * serviceConfig, size_t serviceConfigLen,
                                             const char * accountId, size_t accountIdLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _ClearServiceProvisioningData() { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetFailSafeArmed(bool & val) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _SetFailSafeArmed(bool val) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetQRCodeString(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetWiFiAPSSID(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo);
    bool _IsCommissionableDeviceTypeEnabled() { return false; }
    CHIP_ERROR _GetDeviceType(uint16_t & deviceType) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    bool _IsCommissionableDeviceNameEnabled() { return false; }
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetInitialPairingHint(uint16_t & pairingHint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetInitialPairingInstruction(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetSecondaryPairingHint(uint16_t & pairingHint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetSecondaryPairingInstruction(char * buf, size_t bufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetRegulatoryLocation(uint32_t & location) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreRegulatoryLocation(uint32_t location) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetCountryCode(char * buf, size_t bufSize, size_t & codeLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreCountryCode(const char * code, size_t codeLen) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _GetBreadcrumb(uint64_t & breadcrumb) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _StoreBreadcrumb(uint64_t breadcrumb) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR _ConfigureChipStack() { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if !defined(NDEBUG)
    CHIP_ERROR _RunUnitTests(void) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    bool _IsServiceProvisioned() { return false; }
    bool _IsMemberOfFabric() { return false; }
    bool _IsPairedToAccount() { return false; }
    bool _IsFullyProvisioned() { return false; }
    CHIP_ERROR _ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool _OperationalDeviceCredentialsProvisioned() { return false; }
    void _UseManufacturerCredentialsAsOperational(bool val) { return false; }
#endif
    void _UseManufacturerCredentialsAsOperational(bool val) {}
    void _LogDeviceConfig() {}
    bool _CanFactoryReset() { return true; }
    void _InitiateFactoryReset() {}
    CHIP_ERROR _ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR _WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // NOTE: Other public interface methods are implemented by GenericConfigurationManagerImpl<>.
    // ===== Members for internal use by the following friends.

    friend ConfigurationManager & ConfigurationMgr(void);
    friend ConfigurationManagerImpl & ConfigurationMgrImpl(void);

    static ConfigurationManagerImpl sInstance;
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
 * Chip applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the ESP32 platform.
 */
inline ConfigurationManagerImpl & ConfigurationMgrImpl(void)
{
    return ConfigurationManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
