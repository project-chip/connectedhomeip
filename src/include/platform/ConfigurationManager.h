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

#include <lib/support/Span.h>
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
        kMaxSerialNumberLength     = 32,
        kMaxFirmwareRevisionLength = 32,
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        kPrimaryMACAddressLength = 8,
#else
        kPrimaryMACAddressLength = 6,
#endif
        kMaxMACAddressLength = 8,
    };

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize);
    CHIP_ERROR GetVendorId(uint16_t & vendorId);
    CHIP_ERROR GetProductName(char * buf, size_t bufSize);
    CHIP_ERROR GetProductId(uint16_t & productId);
    CHIP_ERROR GetProductRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR GetProductRevision(uint16_t & productRev);
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize, size_t & serialNumLen);
    CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan buf);
    CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf);
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
    CHIP_ERROR GetFirmwareRevisionString(char * buf, size_t bufSize);
    CHIP_ERROR GetFirmwareRevision(uint32_t & firmwareRev);
    CHIP_ERROR GetSetupPinCode(uint32_t & setupPinCode);
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator);
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
    CHIP_ERROR StoreSetupPinCode(uint32_t setupPinCode);
    CHIP_ERROR StoreSetupDiscriminator(uint16_t setupDiscriminator);
    CHIP_ERROR StoreRegulatoryLocation(uint32_t location);
    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen);
    CHIP_ERROR StoreBreadcrumb(uint64_t breadcrumb);

    CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo);

#if !defined(NDEBUG)
    CHIP_ERROR RunUnitTests();
#endif

    bool IsFullyProvisioned();
    void InitiateFactoryReset();

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
    bool CanFactoryReset();
    CHIP_ERROR GetFailSafeArmed(bool & val);
    CHIP_ERROR SetFailSafeArmed(bool val);
    CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value);

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

inline CHIP_ERROR ConfigurationManager::GetPrimaryMACAddress(MutableByteSpan buf)
{
    return static_cast<ImplClass *>(this)->_GetPrimaryMACAddress(buf);
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

inline CHIP_ERROR ConfigurationManager::GetSetupPinCode(uint32_t & setupPinCode)
{
    return static_cast<ImplClass *>(this)->_GetSetupPinCode(setupPinCode);
}

inline CHIP_ERROR ConfigurationManager::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    return static_cast<ImplClass *>(this)->_GetSetupDiscriminator(setupDiscriminator);
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

inline CHIP_ERROR ConfigurationManager::StoreSetupPinCode(uint32_t setupPinCode)
{
    return static_cast<ImplClass *>(this)->_StoreSetupPinCode(setupPinCode);
}

inline CHIP_ERROR ConfigurationManager::StoreSetupDiscriminator(uint16_t setupDiscriminator)
{
    return static_cast<ImplClass *>(this)->_StoreSetupDiscriminator(setupDiscriminator);
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

inline CHIP_ERROR ConfigurationManager::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    return static_cast<ImplClass *>(this)->_ReadPersistedStorageValue(key, value);
}

inline CHIP_ERROR ConfigurationManager::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    return static_cast<ImplClass *>(this)->_WritePersistedStorageValue(key, value);
}

inline CHIP_ERROR ConfigurationManager::GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo)
{
    return static_cast<ImplClass *>(this)->_GetBLEDeviceIdentificationInfo(deviceIdInfo);
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

inline CHIP_ERROR ConfigurationManager::Init()
{
    return static_cast<ImplClass *>(this)->_Init();
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

inline void ConfigurationManager::LogDeviceConfig()
{
    static_cast<ImplClass *>(this)->_LogDeviceConfig();
}

/**
 * True if device type in DNS-SD advertisement is enabled
 */
inline bool ConfigurationManager::IsCommissionableDeviceTypeEnabled()
{
    return static_cast<ImplClass *>(this)->_IsCommissionableDeviceTypeEnabled();
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
