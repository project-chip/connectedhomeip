/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <platform/CHIPDeviceConfig.h>

#if CHIP_HAVE_CONFIG_H
#include <setup_payload/CHIPAdditionalDataPayloadBuildConfig.h>
#endif

#include <lib/core/ClusterEnums.h>
#include <lib/support/Span.h>
#include <platform/PersistedStorage.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

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
        kMaxVendorNameLength            = 32,
        kMaxProductNameLength           = 32,
        kMaxLocationLength              = 2,
        kMaxHardwareVersionStringLength = 64,
        kMaxSoftwareVersionStringLength = 64,
        kMaxManufacturingDateLength     = 16,
        kMaxPartNumberLength            = 32,
        kMaxProductURLLength            = 256,
        kMaxProductLabelLength          = 64,
        kMaxSerialNumberLength          = 32,
        kMaxUniqueIDLength              = 32,
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
        kMinRotatingDeviceIDUniqueIDLength = 16,
        kRotatingDeviceIDUniqueIDLength    = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH,
#endif
        kEthernetMACAddressLength = 6,
        kThreadMACAddressLength   = 8,
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        kPrimaryMACAddressLength = kThreadMACAddressLength,
#else
        kPrimaryMACAddressLength = kEthernetMACAddressLength,
#endif
        kMaxMACAddressLength  = 8,
        kMaxLanguageTagLength = 5 // ISO 639-1 standard language codes
    };

    // Copies the primary MAC into a mutable span, which must be of size kPrimaryMACAddressLength.
    // Upon success, the span will be reduced to the size of the MAC address being returned, which
    // can be less than kPrimaryMACAddressLength on a device that supports Thread.
    virtual CHIP_ERROR GetPrimaryMACAddress(MutableByteSpan buf) = 0;

    // Copies the primary WiFi MAC into a buffer of size kEthernetMACAddressLength
    virtual CHIP_ERROR GetPrimaryWiFiMACAddress(uint8_t * buf) = 0;

    // Copies the primary Thread (802.15.4) MAC into a buffer of size kThreadMACAddressLength
    virtual CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf)                           = 0;
    virtual CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize)                = 0;
    virtual CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVer)                          = 0;
    virtual CHIP_ERROR GetFirmwareBuildChipEpochTime(System::Clock::Seconds32 & buildTime) = 0;
    virtual CHIP_ERROR SetFirmwareBuildChipEpochTime(System::Clock::Seconds32 buildTime) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    // Lifetime counter is monotonic counter that is incremented upon each commencement of advertising
    virtual CHIP_ERROR GetLifetimeCounter(uint16_t & lifetimeCounter)              = 0;
    virtual CHIP_ERROR IncrementLifetimeCounter()                                  = 0;
    virtual CHIP_ERROR SetRotatingDeviceIdUniqueId(const ByteSpan & uniqueIdSpan)  = 0;
    virtual CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) = 0;
#endif
    virtual CHIP_ERROR GetRegulatoryLocation(uint8_t & location)                       = 0;
    virtual CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)    = 0;
    virtual CHIP_ERROR StoreSerialNumber(const char * serialNum, size_t serialNumLen)  = 0;
    virtual CHIP_ERROR StoreManufacturingDate(const char * mfgDate, size_t mfgDateLen) = 0;
    virtual CHIP_ERROR StoreSoftwareVersion(uint32_t softwareVer)                      = 0;
    virtual CHIP_ERROR StoreHardwareVersion(uint16_t hardwareVer)                      = 0;
    virtual CHIP_ERROR StoreRegulatoryLocation(uint8_t location)                       = 0;
    virtual CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen)             = 0;
    virtual CHIP_ERROR GetRebootCount(uint32_t & rebootCount)                          = 0;
    virtual CHIP_ERROR StoreRebootCount(uint32_t rebootCount)                          = 0;
    virtual CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours)      = 0;
    virtual CHIP_ERROR StoreTotalOperationalHours(uint32_t totalOperationalHours)      = 0;
    virtual CHIP_ERROR GetBootReason(uint32_t & bootReason)                            = 0;
    virtual CHIP_ERROR StoreBootReason(uint32_t bootReason)                            = 0;
    virtual CHIP_ERROR GetUniqueId(char * buf, size_t bufSize)                         = 0;
    virtual CHIP_ERROR StoreUniqueId(const char * uniqueId, size_t uniqueIdLen)        = 0;
    virtual CHIP_ERROR GenerateUniqueId(char * buf, size_t bufSize)                    = 0;
    virtual CHIP_ERROR GetFailSafeArmed(bool & val)                                    = 0;
    virtual CHIP_ERROR SetFailSafeArmed(bool val)                                      = 0;

    virtual CHIP_ERROR GetBLEDeviceIdentificationInfo(Ble::ChipBLEDeviceIdentificationInfo & deviceIdInfo) = 0;

#if CHIP_CONFIG_TEST
    virtual void RunUnitTests() = 0;
#endif

    virtual bool IsFullyProvisioned()   = 0;
    virtual void InitiateFactoryReset() = 0;

    // Gets called when starting BLE/DNS-SD advertisement
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    virtual void NotifyOfAdvertisementStart() {}
#else
    void NotifyOfAdvertisementStart() {}
#endif

    virtual void LogDeviceConfig() = 0;

    virtual bool IsCommissionableDeviceTypeEnabled()                              = 0;
    virtual CHIP_ERROR GetDeviceTypeId(uint32_t & deviceType)                     = 0;
    virtual bool IsCommissionableDeviceNameEnabled()                              = 0;
    virtual CHIP_ERROR GetCommissionableDeviceName(char * buf, size_t bufSize)    = 0;
    virtual CHIP_ERROR GetInitialPairingHint(uint16_t & pairingHint)              = 0;
    virtual CHIP_ERROR GetInitialPairingInstruction(char * buf, size_t bufSize)   = 0;
    virtual CHIP_ERROR GetSecondaryPairingHint(uint16_t & pairingHint)            = 0;
    virtual CHIP_ERROR GetSecondaryPairingInstruction(char * buf, size_t bufSize) = 0;

    virtual CHIP_ERROR GetLocationCapability(uint8_t & location);

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
    virtual CHIP_ERROR ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value) = 0;
    virtual CHIP_ERROR WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)  = 0;

    // Construction/destruction limited to subclasses.
    ConfigurationManager()          = default;
    virtual ~ConfigurationManager() = default;

    // No copy, move or assignment.
    ConfigurationManager(const ConfigurationManager &)             = delete;
    ConfigurationManager(const ConfigurationManager &&)            = delete;
    ConfigurationManager & operator=(const ConfigurationManager &) = delete;
};

/**
 * Returns a reference to a ConfigurationManager object.
 *
 * Applications should use this to access features of the ConfigurationManager object
 * that are common to all platforms.
 */
ConfigurationManager & ConfigurationMgr();

/**
 * Returns the platform-specific implementation of the ConfigurationManager object.
 *
 * Applications can use this to gain access to features of the ConfigurationManager
 * that are specific to the selected platform.
 */
extern ConfigurationManager & ConfigurationMgrImpl();

/**
 * Sets a reference to a ConfigurationManager object.
 *
 * This must be called before any calls to ConfigurationMgr. If a nullptr is passed in,
 * no changes will be made.
 */
void SetConfigurationMgr(ConfigurationManager * configurationManager);

inline CHIP_ERROR ConfigurationManager::GetLocationCapability(uint8_t & location)
{
    location = to_underlying(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoor);
    return CHIP_NO_ERROR;
}

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
