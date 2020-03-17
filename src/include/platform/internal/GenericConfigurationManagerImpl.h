/*
 *
 *    Copyright (c) 2019-2020 Google LLC.
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
 *          Provides an generic implementation of ConfigurationManager features
 *          for use on various platforms.
 */

#ifndef GENERIC_CONFIGURATION_MANAGER_IMPL_H
#define GENERIC_CONFIGURATION_MANAGER_IMPL_H

namespace nl {
namespace Weave {
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
template<class ImplClass>
class GenericConfigurationManagerImpl
{
public:

    // ===== Methods that implement the ConfigurationManager abstract interface.

    WEAVE_ERROR _Init();
    WEAVE_ERROR _ConfigureWeaveStack();
    WEAVE_ERROR _GetVendorId(uint16_t & vendorId);
    WEAVE_ERROR _GetProductId(uint16_t & productId);
    WEAVE_ERROR _GetProductRevision(uint16_t & productRev);
    WEAVE_ERROR _StoreProductRevision(uint16_t productRev);
    WEAVE_ERROR _GetFirmwareRevision(char * buf, size_t bufSize, size_t & outLen);
    WEAVE_ERROR _GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth,
            uint8_t & hour, uint8_t & minute, uint8_t & second);
    WEAVE_ERROR _GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen);
    WEAVE_ERROR _StoreSerialNumber(const char * serialNum, size_t serialNumLen);
    WEAVE_ERROR _GetPrimaryWiFiMACAddress(uint8_t * buf);
    WEAVE_ERROR _StorePrimaryWiFiMACAddress(const uint8_t * buf);
    WEAVE_ERROR _GetPrimary802154MACAddress(uint8_t * buf);
    WEAVE_ERROR _StorePrimary802154MACAddress(const uint8_t * buf);
    WEAVE_ERROR _GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
    WEAVE_ERROR _StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen);
    WEAVE_ERROR _GetDeviceId(uint64_t & deviceId);
    WEAVE_ERROR _GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    WEAVE_ERROR _GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    WEAVE_ERROR _GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
#if WEAVE_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    WEAVE_ERROR _StoreDeviceId(uint64_t deviceId);
    WEAVE_ERROR _StoreDeviceCertificate(const uint8_t * cert, size_t certLen);
    WEAVE_ERROR _StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    WEAVE_ERROR _StoreDevicePrivateKey(const uint8_t * key, size_t keyLen);
    WEAVE_ERROR _ClearOperationalDeviceCredentials(void);
#endif
    WEAVE_ERROR _GetManufacturerDeviceId(uint64_t & deviceId);
    WEAVE_ERROR _StoreManufacturerDeviceId(uint64_t deviceId);
    WEAVE_ERROR _GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    WEAVE_ERROR _StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen);
    WEAVE_ERROR _GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    WEAVE_ERROR _StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    WEAVE_ERROR _GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
    WEAVE_ERROR _StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen);
    WEAVE_ERROR _GetPairingCode(char * buf, size_t bufSize, size_t & pairingCodeLen);
    WEAVE_ERROR _StorePairingCode(const char * pairingCode, size_t pairingCodeLen);
    WEAVE_ERROR _GetFabricId(uint64_t & fabricId);
    WEAVE_ERROR _StoreFabricId(uint64_t fabricId);
    WEAVE_ERROR _GetServiceId(uint64_t & serviceId);
    WEAVE_ERROR _GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen);
    WEAVE_ERROR _StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen);
    WEAVE_ERROR _GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen);
    WEAVE_ERROR _StorePairedAccountId(const char * accountId, size_t accountIdLen);
    WEAVE_ERROR _StoreServiceProvisioningData(uint64_t serviceId, const uint8_t * serviceConfig,
            size_t serviceConfigLen, const char * accountId, size_t accountIdLen);
    WEAVE_ERROR _ClearServiceProvisioningData();
    WEAVE_ERROR _GetFailSafeArmed(bool & val);
    WEAVE_ERROR _SetFailSafeArmed(bool val);
    WEAVE_ERROR _GetDeviceDescriptor(::nl::Weave::Profiles::DeviceDescription::WeaveDeviceDescriptor & deviceDesc);
    WEAVE_ERROR _GetDeviceDescriptorTLV(uint8_t * buf, size_t bufSize, size_t & encodedLen);
    WEAVE_ERROR _GetQRCodeString(char * buf, size_t bufSize);
    WEAVE_ERROR _GetWiFiAPSSID(char * buf, size_t bufSize);
    WEAVE_ERROR _GetBLEDeviceIdentificationInfo(Ble::WeaveBLEDeviceIdentificationInfo & deviceIdInfo);
    bool _IsServiceProvisioned();
    bool _IsMemberOfFabric();
    bool _IsPairedToAccount();
    bool _IsFullyProvisioned();
    WEAVE_ERROR _ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize);
#if WEAVE_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool _OperationalDeviceCredentialsProvisioned();
    void _UseManufacturerCredentialsAsOperational(bool val);
#endif

protected:

    enum
    {
        kFlag_IsServiceProvisioned                    = 0x01,
        kFlag_IsMemberOfFabric                        = 0x02,
        kFlag_IsPairedToAccount                       = 0x04,
        kFlag_OperationalDeviceCredentialsProvisioned = 0x08,
        kFlag_UseManufacturerCredentialsAsOperational = 0x10,
    };

    uint8_t mFlags;

    void LogDeviceConfig();
    WEAVE_ERROR PersistProvisioningData(ProvisioningDataSet & provData);

private:

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void HashLengthAndBase64Value(Platform::Security::SHA256 & hash, const uint8_t * val, uint16_t valLen);

#if WEAVE_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool UseManufacturerCredentialsAsOperational();
#endif
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

template<class ImplClass>
inline WEAVE_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetVendorId(uint16_t & vendorId)
{
    vendorId = (uint16_t)WEAVE_DEVICE_CONFIG_DEVICE_VENDOR_ID;
    return WEAVE_NO_ERROR;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetProductId(uint16_t & productId)
{
    productId = (uint16_t)WEAVE_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
    return WEAVE_NO_ERROR;
}




} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_CONFIGURATION_MANAGER_IMPL_H
