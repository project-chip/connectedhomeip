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

    virtual CHIP_ERROR GetVendorName(char * buf, size_t bufSize)                                    = 0;
    virtual CHIP_ERROR GetVendorId(uint16_t & vendorId)                                             = 0;
    virtual CHIP_ERROR GetProductName(char * buf, size_t bufSize)                                   = 0;
    virtual CHIP_ERROR GetProductId(uint16_t & productId)                                           = 0;
    virtual CHIP_ERROR GetProductRevisionString(char * buf, size_t bufSize)                         = 0;
    virtual CHIP_ERROR GetProductRevision(uint16_t & productRev)                                    = 0;
    virtual CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize)                                  = 0;
    virtual CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan buf)                                    = 0;
    virtual CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf)                                      = 0;
    virtual CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf)                                    = 0;
    virtual CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth) = 0;
    virtual CHIP_ERROR GetFirmwareRevisionString(char * buf, size_t bufSize)                        = 0;
    virtual CHIP_ERROR GetFirmwareRevision(uint16_t & firmwareRev)                                  = 0;
    virtual CHIP_ERROR GetSetupPinCode(uint32_t & setupPinCode)                                     = 0;
    virtual CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator)                         = 0;
    // Lifetime counter is monotonic counter that is incremented only in the case of a factory reset
    virtual CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter)                  = 0;
    virtual CHIP_ERROR GetRegulatoryLocation(uint32_t & location)                      = 0;
    virtual CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)    = 0;
    virtual CHIP_ERROR GetBreadcrumb(uint64_t & breadcrumb)                            = 0;
    virtual CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen)  = 0;
    virtual CHIP_ERROR StorePrimaryWiFiMACAddress(const uint8_t * buf)                 = 0;
    virtual CHIP_ERROR StorePrimary802154MACAddress(const uint8_t * buf)               = 0;
    virtual CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) = 0;
    virtual CHIP_ERROR StoreProductRevision(uint16_t productRev)                       = 0;
    virtual CHIP_ERROR StoreSetupPinCode(uint32_t setupPinCode)                        = 0;
    virtual CHIP_ERROR StoreSetupDiscriminator(uint16_t setupDiscriminator)            = 0;
    virtual CHIP_ERROR StoreRegulatoryLocation(uint32_t location)                      = 0;
    virtual CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen)             = 0;
    virtual CHIP_ERROR StoreBreadcrumb(uint64_t breadcrumb)                            = 0;
    virtual CHIP_ERROR GetRebootCount(uint32_t & rebootCount)                          = 0;
    virtual CHIP_ERROR StoreRebootCount(uint32_t rebootCount)                          = 0;
    virtual CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours)      = 0;
    virtual CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours)      = 0;
    virtual CHIP_ERROR GetBootReason(uint32_t & bootReason)                            = 0;
    virtual CHIP_ERROR StoreBootReason(uint32_t bootReason)                            = 0;

    virtual CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo) = 0;

    virtual CHIP_ERROR RunUnitTests() = 0;

    virtual bool IsFullyProvisioned()   = 0;
    virtual void InitiateFactoryReset() = 0;

    virtual void LogDeviceConfig() = 0;

    virtual bool IsCommissionableDeviceTypeEnabled()                              = 0;
    virtual CHIP_ERROR GetDeviceTypeId(uint16_t & deviceType)                     = 0;
    virtual bool IsCommissionableDeviceNameEnabled()                              = 0;
    virtual CHIP_ERROR GetCommissionableDeviceName(char * buf, size_t bufSize)    = 0;
    virtual CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint)              = 0;
    virtual CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize)   = 0;
    virtual CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint)            = 0;
    virtual CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize) = 0;

protected:
    // ===== Members for internal use by the following friends.

    friend class ::chip::DeviceLayer::PlatformManagerImpl;
    template <class>
    friend class ::chip::DeviceLayer::Internal::GenericPlatformManagerImpl;
    template <class>
    friend class ::chip::DeviceLayer::Internal::GenericPlatformManagerImpl_POSIX;
    // Parentheses used to fix clang parsing issue with these declarations
    friend CHIP_ERROR(::chip::Platform::PersistedStorage::Read)(::chip::Platform::PersistedStorage::Key key, uint32_t & value);
    friend CHIP_ERROR(::chip::Platform::PersistedStorage::Write)(::chip::Platform::PersistedStorage::Key key, uint32_t value);

    virtual CHIP_ERROR Init()                                                                                   = 0;
    virtual bool CanFactoryReset()                                                                              = 0;
    virtual CHIP_ERROR GetFailSafeArmed(bool & val)                                                             = 0;
    virtual CHIP_ERROR SetFailSafeArmed(bool val)                                                               = 0;
    virtual CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) = 0;
    virtual CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)  = 0;

    // Construction/destruction limited to subclasses.
    ConfigurationManager()          = default;
    virtual ~ConfigurationManager() = default;

    // No copy, move or assignment.
    ConfigurationManager(const ConfigurationManager &)  = delete;
    ConfigurationManager(const ConfigurationManager &&) = delete;
    ConfigurationManager & operator=(const ConfigurationManager &) = delete;
};

/**
 * Returns a reference to a ConfigurationManager object.
 *
 * Applications should use this to access the features of the ConfigurationManager.
 */
extern ConfigurationManager & ConfigurationMgr();

/**
 * Sets a reference to a ConfigurationManager object.
 *
 * This must be called before any calls to ConfigurationMgr. If a nullptr is passed in,
 * no changes will be made.
 */
extern void SetConfigurationMgr(ConfigurationManager * configurationManager);

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
