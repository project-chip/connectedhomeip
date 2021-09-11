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
 * or indirectly) by the ConfigurationManagerImpl class, which also appears as the template's ImplClass
 * parameter.
 */
template <class ImplClass>
class GenericConfigurationManagerImpl
{
public:
    // ===== Methods that implement the ConfigurationManager abstract interface.

    CHIP_ERROR _Init();
    CHIP_ERROR _GetVendorName(char * buf, size_t bufSize);
    CHIP_ERROR _GetVendorId(uint16_t & vendorId);
    CHIP_ERROR _GetProductName(char * buf, size_t bufSize);
    CHIP_ERROR _GetProductId(uint16_t & productId);
    CHIP_ERROR _GetProductRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR _GetProductRevision(uint16_t & productRev);
    CHIP_ERROR _StoreProductRevision(uint16_t productRev);
    CHIP_ERROR _GetFirmwareRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR _GetFirmwareRevision(uint32_t & firmwareRev);
    CHIP_ERROR _GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, uint8_t & hour, uint8_t & minute,
                                     uint8_t & second);
    CHIP_ERROR _GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen);
    CHIP_ERROR _StoreSerialNumber(const char * serialNum, size_t serialNumLen);
    CHIP_ERROR _GetPrimaryWiFiMACAddress(uint8_t * buf);
    CHIP_ERROR _StorePrimaryWiFiMACAddress(const uint8_t * buf);
    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR _GetPollPeriod(uint32_t & buf);
    CHIP_ERROR _StorePrimary802154MACAddress(const uint8_t * buf);
    CHIP_ERROR _GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
    CHIP_ERROR _StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen);
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId);
    CHIP_ERROR _GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    CHIP_ERROR _GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    CHIP_ERROR _GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    CHIP_ERROR _StoreDeviceId(uint64_t deviceId);
    CHIP_ERROR _StoreDeviceCertificate(const uint8_t * cert, size_t certLen);
    CHIP_ERROR _StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    CHIP_ERROR _StoreDevicePrivateKey(const uint8_t * key, size_t keyLen);
    CHIP_ERROR _ClearOperationalDeviceCredentials(void);
#endif
    CHIP_ERROR _GetManufacturerDeviceId(uint64_t & deviceId);
    CHIP_ERROR _StoreManufacturerDeviceId(uint64_t deviceId);
    CHIP_ERROR _GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    CHIP_ERROR _StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen);
    CHIP_ERROR _GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    CHIP_ERROR _StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    CHIP_ERROR _GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
    CHIP_ERROR _StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen);
    CHIP_ERROR _GetSetupPinCode(uint32_t & setupPinCode);
    CHIP_ERROR _StoreSetupPinCode(uint32_t setupPinCode);
    CHIP_ERROR _GetSetupDiscriminator(uint16_t & setupDiscriminator);
    CHIP_ERROR _StoreSetupDiscriminator(uint16_t setupDiscriminator);
    CHIP_ERROR _GetFabricId(uint64_t & fabricId);
    CHIP_ERROR _StoreFabricId(uint64_t fabricId);
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    CHIP_ERROR _GetLifetimeCounter(uint16_t & lifetimeCounter);
    CHIP_ERROR _IncrementLifetimeCounter();
#endif
    CHIP_ERROR _GetServiceId(uint64_t & serviceId);
    CHIP_ERROR _GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen);
    CHIP_ERROR _StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen);
    CHIP_ERROR _GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen);
    CHIP_ERROR _StorePairedAccountId(const char * accountId, size_t accountIdLen);
    CHIP_ERROR _StoreServiceProvisioningData(uint64_t serviceId, const uint8_t * serviceConfig, size_t serviceConfigLen,
                                             const char * accountId, size_t accountIdLen);
    CHIP_ERROR _ClearServiceProvisioningData();
    CHIP_ERROR _GetFailSafeArmed(bool & val);
    CHIP_ERROR _SetFailSafeArmed(bool val);
    CHIP_ERROR _GetQRCodeString(char * buf, size_t bufSize);
    CHIP_ERROR _GetWiFiAPSSID(char * buf, size_t bufSize);
    CHIP_ERROR _GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo);
    bool _IsCommissionableDeviceTypeEnabled();
    CHIP_ERROR _GetDeviceType(uint16_t & deviceType);
    bool _IsCommissionableDeviceNameEnabled();
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _GetInitialPairingHint(uint16_t & pairingHint);
    CHIP_ERROR _GetInitialPairingInstruction(char * buf, size_t bufSize);
    CHIP_ERROR _GetSecondaryPairingHint(uint16_t & pairingHint);
    CHIP_ERROR _GetSecondaryPairingInstruction(char * buf, size_t bufSize);
    CHIP_ERROR _GetRegulatoryLocation(uint32_t & location);
    CHIP_ERROR _StoreRegulatoryLocation(uint32_t location);
    CHIP_ERROR _GetCountryCode(char * buf, size_t bufSize, size_t & codeLen);
    CHIP_ERROR _StoreCountryCode(const char * code, size_t codeLen);
    CHIP_ERROR _GetBreadcrumb(uint64_t & breadcrumb);
    CHIP_ERROR _StoreBreadcrumb(uint64_t breadcrumb);
    CHIP_ERROR _ConfigureChipStack();
#if !defined(NDEBUG)
    CHIP_ERROR _RunUnitTests(void);
#endif
    bool _IsServiceProvisioned();
    bool _IsMemberOfFabric();
    bool _IsPairedToAccount();
    bool _IsFullyProvisioned();
    CHIP_ERROR _ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize);
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool _OperationalDeviceCredentialsProvisioned();
    void _UseManufacturerCredentialsAsOperational(bool val);
#endif
    void _LogDeviceConfig();

protected:
    enum class Flags : uint8_t
    {
        kIsServiceProvisioned                    = 0x01,
        kIsMemberOfFabric                        = 0x02,
        kIsPairedToAccount                       = 0x04,
        kOperationalDeviceCredentialsProvisioned = 0x08,
        kUseManufacturerCredentialsAsOperational = 0x10,
    };

    BitFlags<Flags> mFlags;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    chip::LifetimePersistedCounter mLifetimePersistedCounter;
#endif
    CHIP_ERROR PersistProvisioningData(ProvisioningDataSet & provData);

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool UseManufacturerCredentialsAsOperational();
#endif
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetVendorId(uint16_t & vendorId)
{
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductId(uint16_t & productId)
{
    productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetFirmwareRevision(uint32_t & firmwareRev)
{
    firmwareRev = static_cast<uint32_t>(CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceType(uint16_t & deviceType)
{
    deviceType = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_TYPE);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetInitialPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_INITIAL_HINT);
    return CHIP_NO_ERROR;
}

template <class ImplClass>
inline CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetSecondaryPairingHint(uint16_t & pairingHint)
{
    pairingHint = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_PAIRING_SECONDARY_HINT);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
