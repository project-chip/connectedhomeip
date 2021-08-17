/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Defines the public interface for the Device Layer ConfigurationManager object.
 */

#pragma once

#include <cstdint>

#include <platform/CHIPDeviceBuildConfig.h>
#include <platform/PersistedStorage.h>

namespace chip {
namespace Ble {
struct ChipBLEDeviceIdentificationInfo;
}
} // namespace chip

namespace chip {
namespace DeviceLayer {

class PlatformManagerImpl;
class ConfigurationManagerImpl;
namespace Internal {
template <class>
class GenericPlatformManagerImpl;
template <class>
class GenericPlatformManagerImpl_POSIX;
} // namespace Internal

/**
 * Provides access to runtime and build-time configuration information for a chip device.
 */
class ConfigurationManager
{
public:
    // ===== Members that define the public interface of the ConfigurationManager

    enum
    {
        kMaxPairingCodeLength      = 16,
        kMaxSerialNumberLength     = 32,
        kMaxFirmwareRevisionLength = 32,
    };

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize);
    CHIP_ERROR GetVendorId(uint16_t & vendorId);
    CHIP_ERROR GetProductName(char * buf, size_t bufSize);
    CHIP_ERROR GetProductId(uint16_t & productId);
    CHIP_ERROR GetProductRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR GetProductRevision(uint16_t & productRev);
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen);
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf);
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
    CHIP_ERROR GetFirmwareRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR GetFirmwareRevision(uint32_t & firmwareRev);
    CHIP_ERROR GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, uint8_t & hour, uint8_t & minute,
                                    uint8_t & second);
    CHIP_ERROR GetDeviceId(uint64_t & deviceId);
    CHIP_ERROR GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    CHIP_ERROR GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    CHIP_ERROR GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
    CHIP_ERROR GetManufacturerDeviceId(uint64_t & deviceId);
    CHIP_ERROR GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen);
    CHIP_ERROR GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen);
    CHIP_ERROR GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen);
    CHIP_ERROR GetSetupPinCode(uint32_t & setupPinCode);
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator);
    CHIP_ERROR GetServiceId(uint64_t & serviceId);
    CHIP_ERROR GetFabricId(uint64_t & fabricId);
    CHIP_ERROR GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen);
    CHIP_ERROR GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen);
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    // Lifetime counter is monotonic counter that is incremented only in the case of a factory reset
    CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter);
#endif
    CHIP_ERROR GetRegulatoryLocation(uint32_t & location);
    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen);
    CHIP_ERROR GetBreadcrumb(uint64_t & breadcrumb);
    CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen);
    CHIP_ERROR StorePrimaryWiFiMACAddress(const uint8_t * buf);
    CHIP_ERROR StorePrimary802154MACAddress(const uint8_t * buf);
    CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen);
    CHIP_ERROR StoreProductRevision(uint16_t productRev);
    CHIP_ERROR StoreFabricId(uint64_t fabricId);
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    CHIP_ERROR StoreDeviceId(uint64_t deviceId);
    CHIP_ERROR StoreDeviceCertificate(const uint8_t * cert, size_t certLen);
    CHIP_ERROR StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    CHIP_ERROR StoreDevicePrivateKey(const uint8_t * key, size_t keyLen);
#endif
    CHIP_ERROR StoreManufacturerDeviceId(uint64_t deviceId);
    CHIP_ERROR StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen);
    CHIP_ERROR StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen);
    CHIP_ERROR StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen);
    CHIP_ERROR StoreSetupPinCode(uint32_t setupPinCode);
    CHIP_ERROR StoreSetupDiscriminator(uint16_t setupDiscriminator);
    CHIP_ERROR StoreServiceProvisioningData(uint64_t serviceId, const uint8_t * serviceConfig, size_t serviceConfigLen,
                                            const char * accountId, size_t accountIdLen);
    CHIP_ERROR ClearServiceProvisioningData();
    CHIP_ERROR StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen);
    CHIP_ERROR StorePairedAccountId(const char * accountId, size_t accountIdLen);
    CHIP_ERROR StoreRegulatoryLocation(uint32_t location);
    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen);
    CHIP_ERROR StoreBreadcrumb(uint64_t breadcrumb);

    CHIP_ERROR GetQRCodeString(char * buf, size_t bufSize);

    CHIP_ERROR GetWiFiAPSSID(char * buf, size_t bufSize);

    CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo);

#if !defined(NDEBUG)
    CHIP_ERROR RunUnitTests();
#endif

    bool IsServiceProvisioned();
    bool IsPairedToAccount();
    bool IsMemberOfFabric();
    bool IsFullyProvisioned();
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    bool OperationalDeviceCredentialsProvisioned();
#endif

    void InitiateFactoryReset();

    CHIP_ERROR ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize);

    void LogDeviceConfig();

    bool IsCommissionableDeviceTypeEnabled();
    CHIP_ERROR GetDeviceType(uint16_t & deviceType);
    bool IsCommissionableDeviceNameEnabled();
    CHIP_ERROR GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint);
    CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize);
    CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint);
    CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize);

private:
    // ===== Members for internal use by the following friends.

    friend class ::chip::DeviceLayer::PlatformManagerImpl;
    template <class>
    friend class ::chip::DeviceLayer::Internal::GenericPlatformManagerImpl;
    template <class>
    friend class ::chip::DeviceLayer::Internal::GenericPlatformManagerImpl_POSIX;
    // Parentheses used to fix clang parsing issue with these declarations
    friend CHIP_ERROR(::chip::Platform::PersistedStorage::Read)(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    friend CHIP_ERROR(::chip::Platform::PersistedStorage::Write)(::chip::Platform::PersistedStorage::Key key, uint32_t value);

    using ImplClass = ::chip::DeviceLayer::ConfigurationManagerImpl;

    CHIP_ERROR Init();
    CHIP_ERROR ConfigureChipStack();
    bool CanFactoryReset();
    CHIP_ERROR GetFailSafeArmed(bool & val);
    CHIP_ERROR SetFailSafeArmed(bool val);
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value);
#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING
    CHIP_ERROR ClearOperationalDeviceCredentials(void);
    void UseManufacturerCredentialsAsOperational(bool val);
#endif

protected:
    // Construction/destruction limited to subclasses.
    ConfigurationManager()  = default;
    ~ConfigurationManager() = default;

    // No copy, move or assignment.
    ConfigurationManager(const ConfigurationManager &)  = delete;
    ConfigurationManager(const ConfigurationManager &&) = delete;
    ConfigurationManager & operator=(const ConfigurationManager &) = delete;
};

/**
 * Returns a reference to the public interface of the ConfigurationManager singleton object.
 *
 * chip application should use this to access features of the ConfigurationManager object
 * that are common to all platforms.
 */
extern ConfigurationManager & ConfigurationMgr();

/**
 * Returns the platform-specific implementation of the ConfigurationManager singleton object.
 *
 * chip applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the selected platform.
 */
extern ConfigurationManagerImpl & ConfigurationMgrImpl();

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the ConfigurationManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_CONFIGURATIONMANAGERIMPL_HEADER
#include EXTERNAL_CONFIGURATIONMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define CONFIGURATIONMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/ConfigurationManagerImpl.h>
#include CONFIGURATIONMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

/**
 * Name of the vendor that produced the device.
 */
inline CHIP_ERROR ConfigurationManager::GetVendorName(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetVendorName(buf, bufSize);
}

/**
 * Id of the vendor that produced the device.
 */
inline CHIP_ERROR ConfigurationManager::GetVendorId(uint16_t & vendorId)
{
    return static_cast<ImplClass *>(this)->_GetVendorId(vendorId);
}

/**
 * Name of the product assigned by the vendor.
 */
inline CHIP_ERROR ConfigurationManager::GetProductName(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetProductName(buf, bufSize);
}

/**
 * Device product id assigned by the vendor.
 */
inline CHIP_ERROR ConfigurationManager::GetProductId(uint16_t & productId)
{
    return static_cast<ImplClass *>(this)->_GetProductId(productId);
}

/**
 * Product revision string assigned by the vendor.
 */
inline CHIP_ERROR ConfigurationManager::GetProductRevisionString(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetProductRevisionString(buf, bufSize);
}

/**
 * Product revision number assigned by the vendor.
 */
inline CHIP_ERROR ConfigurationManager::GetProductRevision(uint16_t & productRev)
{
    return static_cast<ImplClass *>(this)->_GetProductRevision(productRev);
}

inline CHIP_ERROR ConfigurationManager::GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen)
{
    return static_cast<ImplClass *>(this)->_GetSerialNumber(buf, bufSize, serialNumLen);
}

inline CHIP_ERROR ConfigurationManager::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return static_cast<ImplClass *>(this)->_GetPrimaryWiFiMACAddress(buf);
}

inline CHIP_ERROR ConfigurationManager::GetPrimary802154MACAddress(uint8_t * buf)
{
    return static_cast<ImplClass *>(this)->_GetPrimary802154MACAddress(buf);
}

inline CHIP_ERROR ConfigurationManager::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth)
{
    return static_cast<ImplClass *>(this)->_GetManufacturingDate(year, month, dayOfMonth);
}

inline CHIP_ERROR ConfigurationManager::GetFirmwareRevisionString(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetFirmwareRevisionString(buf, bufSize);
}

inline CHIP_ERROR ConfigurationManager::GetFirmwareRevision(uint32_t & firmwareRev)
{
    return static_cast<ImplClass *>(this)->_GetFirmwareRevision(firmwareRev);
}

inline CHIP_ERROR ConfigurationManager::GetFirmwareBuildTime(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, uint8_t & hour,
                                                             uint8_t & minute, uint8_t & second)
{
    return static_cast<ImplClass *>(this)->_GetFirmwareBuildTime(year, month, dayOfMonth, hour, minute, second);
}

inline CHIP_ERROR ConfigurationManager::GetDeviceId(uint64_t & deviceId)
{
    return static_cast<ImplClass *>(this)->_GetDeviceId(deviceId);
}

inline CHIP_ERROR ConfigurationManager::GetDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen)
{
    return static_cast<ImplClass *>(this)->_GetDeviceCertificate(buf, bufSize, certLen);
}

inline CHIP_ERROR ConfigurationManager::GetDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen)
{
    return static_cast<ImplClass *>(this)->_GetDeviceIntermediateCACerts(buf, bufSize, certsLen);
}

inline CHIP_ERROR ConfigurationManager::GetDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen)
{
    return static_cast<ImplClass *>(this)->_GetDevicePrivateKey(buf, bufSize, keyLen);
}

inline CHIP_ERROR ConfigurationManager::GetManufacturerDeviceId(uint64_t & deviceId)
{
    return static_cast<ImplClass *>(this)->_GetManufacturerDeviceId(deviceId);
}

inline CHIP_ERROR ConfigurationManager::GetManufacturerDeviceCertificate(uint8_t * buf, size_t bufSize, size_t & certLen)
{
    return static_cast<ImplClass *>(this)->_GetManufacturerDeviceCertificate(buf, bufSize, certLen);
}

inline CHIP_ERROR ConfigurationManager::GetManufacturerDeviceIntermediateCACerts(uint8_t * buf, size_t bufSize, size_t & certsLen)
{
    return static_cast<ImplClass *>(this)->_GetManufacturerDeviceIntermediateCACerts(buf, bufSize, certsLen);
}

inline CHIP_ERROR ConfigurationManager::GetManufacturerDevicePrivateKey(uint8_t * buf, size_t bufSize, size_t & keyLen)
{
    return static_cast<ImplClass *>(this)->_GetManufacturerDevicePrivateKey(buf, bufSize, keyLen);
}

inline CHIP_ERROR ConfigurationManager::GetSetupPinCode(uint32_t & setupPinCode)
{
    return static_cast<ImplClass *>(this)->_GetSetupPinCode(setupPinCode);
}

inline CHIP_ERROR ConfigurationManager::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    return static_cast<ImplClass *>(this)->_GetSetupDiscriminator(setupDiscriminator);
}

inline CHIP_ERROR ConfigurationManager::GetServiceId(uint64_t & serviceId)
{
    return static_cast<ImplClass *>(this)->_GetServiceId(serviceId);
}

inline CHIP_ERROR ConfigurationManager::GetFabricId(uint64_t & fabricId)
{
    return static_cast<ImplClass *>(this)->_GetFabricId(fabricId);
}

inline CHIP_ERROR ConfigurationManager::GetServiceConfig(uint8_t * buf, size_t bufSize, size_t & serviceConfigLen)
{
    return static_cast<ImplClass *>(this)->_GetServiceConfig(buf, bufSize, serviceConfigLen);
}

inline CHIP_ERROR ConfigurationManager::GetPairedAccountId(char * buf, size_t bufSize, size_t & accountIdLen)
{
    return static_cast<ImplClass *>(this)->_GetPairedAccountId(buf, bufSize, accountIdLen);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
inline CHIP_ERROR ConfigurationManager::GetLifetimeCounter(uint16_t & lifetimeCounter)
{
    return static_cast<ImplClass *>(this)->_GetLifetimeCounter(lifetimeCounter);
}
#endif

inline CHIP_ERROR ConfigurationManager::GetRegulatoryLocation(uint32_t & location)
{
    return static_cast<ImplClass *>(this)->_GetRegulatoryLocation(location);
}

inline CHIP_ERROR ConfigurationManager::GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
{
    return static_cast<ImplClass *>(this)->_GetCountryCode(buf, bufSize, codeLen);
}

inline CHIP_ERROR ConfigurationManager::GetBreadcrumb(uint64_t & breadcrumb)
{
    return static_cast<ImplClass *>(this)->_GetBreadcrumb(breadcrumb);
}

inline CHIP_ERROR ConfigurationManager::StoreSerialNumber(const char * serialNum, size_t serialNumLen)
{
    return static_cast<ImplClass *>(this)->_StoreSerialNumber(serialNum, serialNumLen);
}

inline CHIP_ERROR ConfigurationManager::StorePrimaryWiFiMACAddress(const uint8_t * buf)
{
    return static_cast<ImplClass *>(this)->_StorePrimaryWiFiMACAddress(buf);
}

inline CHIP_ERROR ConfigurationManager::StorePrimary802154MACAddress(const uint8_t * buf)
{
    return static_cast<ImplClass *>(this)->_StorePrimary802154MACAddress(buf);
}

inline CHIP_ERROR ConfigurationManager::StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen)
{
    return static_cast<ImplClass *>(this)->_StoreManufacturingDate(mfgDate, mfgDateLen);
}

inline CHIP_ERROR ConfigurationManager::StoreProductRevision(uint16_t productRev)
{
    return static_cast<ImplClass *>(this)->_StoreProductRevision(productRev);
}

inline CHIP_ERROR ConfigurationManager::StoreFabricId(uint64_t fabricId)
{
    return static_cast<ImplClass *>(this)->_StoreFabricId(fabricId);
}

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

inline CHIP_ERROR ConfigurationManager::StoreDeviceId(uint64_t deviceId)
{
    return static_cast<ImplClass *>(this)->_StoreDeviceId(deviceId);
}

inline CHIP_ERROR ConfigurationManager::StoreDeviceCertificate(const uint8_t * cert, size_t certLen)
{
    return static_cast<ImplClass *>(this)->_StoreDeviceCertificate(cert, certLen);
}

inline CHIP_ERROR ConfigurationManager::StoreDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen)
{
    return static_cast<ImplClass *>(this)->_StoreDeviceIntermediateCACerts(certs, certsLen);
}

inline CHIP_ERROR ConfigurationManager::StoreDevicePrivateKey(const uint8_t * key, size_t keyLen)
{
    return static_cast<ImplClass *>(this)->_StoreDevicePrivateKey(key, keyLen);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

inline CHIP_ERROR ConfigurationManager::StoreManufacturerDeviceId(uint64_t deviceId)
{
    return static_cast<ImplClass *>(this)->_StoreManufacturerDeviceId(deviceId);
}

inline CHIP_ERROR ConfigurationManager::StoreManufacturerDeviceCertificate(const uint8_t * cert, size_t certLen)
{
    return static_cast<ImplClass *>(this)->_StoreManufacturerDeviceCertificate(cert, certLen);
}

inline CHIP_ERROR ConfigurationManager::StoreManufacturerDeviceIntermediateCACerts(const uint8_t * certs, size_t certsLen)
{
    return static_cast<ImplClass *>(this)->_StoreManufacturerDeviceIntermediateCACerts(certs, certsLen);
}

inline CHIP_ERROR ConfigurationManager::StoreManufacturerDevicePrivateKey(const uint8_t * key, size_t keyLen)
{
    return static_cast<ImplClass *>(this)->_StoreManufacturerDevicePrivateKey(key, keyLen);
}

inline CHIP_ERROR ConfigurationManager::StoreSetupPinCode(uint32_t setupPinCode)
{
    return static_cast<ImplClass *>(this)->_StoreSetupPinCode(setupPinCode);
}

inline CHIP_ERROR ConfigurationManager::StoreSetupDiscriminator(uint16_t setupDiscriminator)
{
    return static_cast<ImplClass *>(this)->_StoreSetupDiscriminator(setupDiscriminator);
}

inline CHIP_ERROR ConfigurationManager::StoreServiceProvisioningData(uint64_t serviceId, const uint8_t * serviceConfig,
                                                                     size_t serviceConfigLen, const char * accountId,
                                                                     size_t accountIdLen)
{
    return static_cast<ImplClass *>(this)->_StoreServiceProvisioningData(serviceId, serviceConfig, serviceConfigLen, accountId,
                                                                         accountIdLen);
}

inline CHIP_ERROR ConfigurationManager::StoreRegulatoryLocation(uint32_t location)
{
    return static_cast<ImplClass *>(this)->_StoreRegulatoryLocation(location);
}

inline CHIP_ERROR ConfigurationManager::StoreCountryCode(const char * code, size_t codeLen)
{
    return static_cast<ImplClass *>(this)->_StoreCountryCode(code, codeLen);
}

inline CHIP_ERROR ConfigurationManager::StoreBreadcrumb(uint64_t breadcrumb)
{
    return static_cast<ImplClass *>(this)->_StoreBreadcrumb(breadcrumb);
}

inline CHIP_ERROR ConfigurationManager::ClearServiceProvisioningData()
{
    return static_cast<ImplClass *>(this)->_ClearServiceProvisioningData();
}

inline CHIP_ERROR ConfigurationManager::StoreServiceConfig(const uint8_t * serviceConfig, size_t serviceConfigLen)
{
    return static_cast<ImplClass *>(this)->_StoreServiceConfig(serviceConfig, serviceConfigLen);
}

inline CHIP_ERROR ConfigurationManager::StorePairedAccountId(const char * accountId, size_t accountIdLen)
{
    return static_cast<ImplClass *>(this)->_StorePairedAccountId(accountId, accountIdLen);
}

inline CHIP_ERROR ConfigurationManager::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    return static_cast<ImplClass *>(this)->_ReadPersistedStorageValue(key, value);
}

inline CHIP_ERROR ConfigurationManager::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return static_cast<ImplClass *>(this)->_WritePersistedStorageValue(key, value);
}

inline CHIP_ERROR ConfigurationManager::GetQRCodeString(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetQRCodeString(buf, bufSize);
}

inline CHIP_ERROR ConfigurationManager::GetWiFiAPSSID(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetWiFiAPSSID(buf, bufSize);
}

inline CHIP_ERROR ConfigurationManager::GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo)
{
    return static_cast<ImplClass *>(this)->_GetBLEDeviceIdentificationInfo(deviceIdInfo);
}

inline bool ConfigurationManager::IsServiceProvisioned()
{
    return static_cast<ImplClass *>(this)->_IsServiceProvisioned();
}

inline bool ConfigurationManager::IsPairedToAccount()
{
    return static_cast<ImplClass *>(this)->_IsPairedToAccount();
}

inline bool ConfigurationManager::IsMemberOfFabric()
{
    return static_cast<ImplClass *>(this)->_IsMemberOfFabric();
}

inline bool ConfigurationManager::IsFullyProvisioned()
{
    return static_cast<ImplClass *>(this)->_IsFullyProvisioned();
}

inline void ConfigurationManager::InitiateFactoryReset()
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    static_cast<ImplClass *>(this)->_IncrementLifetimeCounter();
#endif
    static_cast<ImplClass *>(this)->_InitiateFactoryReset();
}

#if !defined(NDEBUG)
inline CHIP_ERROR ConfigurationManager::RunUnitTests()
{
    return static_cast<ImplClass *>(this)->_RunUnitTests();
}
#endif

inline CHIP_ERROR ConfigurationManager::ComputeProvisioningHash(uint8_t * hashBuf, size_t hashBufSize)
{
    return static_cast<ImplClass *>(this)->_ComputeProvisioningHash(hashBuf, hashBufSize);
}

inline CHIP_ERROR ConfigurationManager::Init()
{
    return static_cast<ImplClass *>(this)->_Init();
}

inline CHIP_ERROR ConfigurationManager::ConfigureChipStack()
{
    return static_cast<ImplClass *>(this)->_ConfigureChipStack();
}

inline bool ConfigurationManager::CanFactoryReset()
{
    return static_cast<ImplClass *>(this)->_CanFactoryReset();
}

inline CHIP_ERROR ConfigurationManager::GetFailSafeArmed(bool & val)
{
    return static_cast<ImplClass *>(this)->_GetFailSafeArmed(val);
}

inline CHIP_ERROR ConfigurationManager::SetFailSafeArmed(bool val)
{
    return static_cast<ImplClass *>(this)->_SetFailSafeArmed(val);
}

#if CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

inline bool ConfigurationManager::OperationalDeviceCredentialsProvisioned()
{
    return static_cast<ImplClass *>(this)->_OperationalDeviceCredentialsProvisioned();
}

inline CHIP_ERROR ConfigurationManager::ClearOperationalDeviceCredentials(void)
{
    return static_cast<ImplClass *>(this)->_ClearOperationalDeviceCredentials();
}

inline void ConfigurationManager::UseManufacturerCredentialsAsOperational(bool val)
{
    static_cast<ImplClass *>(this)->_UseManufacturerCredentialsAsOperational(val);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING

inline void ConfigurationManager::LogDeviceConfig()
{
    static_cast<ImplClass *>(this)->_LogDeviceConfig();
}

/**
 * True if device type in DNS-SD advertisement is enabled
 */
inline bool ConfigurationManager::IsCommissionableDeviceTypeEnabled()
{
    return static_cast<ImplClass *>(this)->_IsCommissionableDeviceNameEnabled();
}

/**
 * Device type id.
 */
inline CHIP_ERROR ConfigurationManager::GetDeviceType(uint16_t & deviceType)
{
    return static_cast<ImplClass *>(this)->_GetDeviceType(deviceType);
}

/**
 * True if device name in DNS-SD advertisement is enabled
 */
inline bool ConfigurationManager::IsCommissionableDeviceNameEnabled()
{
    return static_cast<ImplClass *>(this)->_IsCommissionableDeviceNameEnabled();
}

/**
 * Name of the device.
 */
inline CHIP_ERROR ConfigurationManager::GetDeviceName(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetDeviceName(buf, bufSize);
}

/**
 * Initial pairing hint.
 */
inline CHIP_ERROR ConfigurationManager::GetInitialPairingHint(uint16_t & pairingHint)
{
    return static_cast<ImplClass *>(this)->_GetInitialPairingHint(pairingHint);
}

/**
 * Secondary pairing hint.
 */
inline CHIP_ERROR ConfigurationManager::GetSecondaryPairingHint(uint16_t & pairingHint)
{
    return static_cast<ImplClass *>(this)->_GetSecondaryPairingHint(pairingHint);
}

/**
 * Initial pairing instruction.
 */
inline CHIP_ERROR ConfigurationManager::GetInitialPairingInstruction(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetInitialPairingInstruction(buf, bufSize);
}

/**
 * Secondary pairing instruction.
 */
inline CHIP_ERROR ConfigurationManager::GetSecondaryPairingInstruction(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetSecondaryPairingInstruction(buf, bufSize);
}

} // namespace DeviceLayer
} // namespace chip
