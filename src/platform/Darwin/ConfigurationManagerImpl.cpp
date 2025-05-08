/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for Darwin platforms.
 */

#include <platform/Darwin/ConfigurationManagerImpl.h>

#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/Global.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/Darwin/DiagnosticDataProviderImpl.h>
#include <platform/Darwin/PosixConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IONetworkMedium.h>
#endif // TARGET_OS_OSX

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

#if TARGET_OS_OSX
CHIP_ERROR FindInterfaces(io_iterator_t * primaryInterfaceIterator)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    kern_return_t kernResult;
    CFMutableDictionaryRef matchingDict     = nullptr;
    CFMutableDictionaryRef primaryInterface = nullptr;

    matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);
    VerifyOrExit(matchingDict != nullptr, err = CHIP_ERROR_INTERNAL);

    primaryInterface =
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    VerifyOrExit(primaryInterface != nullptr, err = CHIP_ERROR_INTERNAL);

    CFDictionarySetValue(primaryInterface, CFSTR(kIOPrimaryInterface), kCFBooleanTrue);
    CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), primaryInterface);

    // IOServiceGetMatchingServices will consume matchingDict, so there is no need to call CFRelease afterwards
    kernResult   = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, primaryInterfaceIterator);
    matchingDict = nullptr;
    VerifyOrExit(KERN_SUCCESS == kernResult, err = CHIP_ERROR_INTERNAL);

exit:
    if (matchingDict != nullptr)
    {
        CFRelease(matchingDict);
    }

    if (primaryInterface != nullptr)
    {
        CFRelease(primaryInterface);
    }

    return err;
}

CHIP_ERROR GetMACAddressFromInterfaces(io_iterator_t primaryInterfaceIterator, uint8_t * buf)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;

    kern_return_t kernResult;
    io_object_t interfaceService;
    io_object_t controllerService = 0;

    while ((interfaceService = IOIteratorNext(primaryInterfaceIterator)))
    {
        kernResult = IORegistryEntryGetParentEntry(interfaceService, kIOServicePlane, &controllerService);
        IOObjectRelease(interfaceService);
        VerifyOrExit(KERN_SUCCESS == kernResult, err = CHIP_ERROR_INTERNAL);

        CFTypeRef linkStatusAsCFNumber =
            IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOLinkStatus), kCFAllocatorDefault, 0);
        VerifyOrExit(linkStatusAsCFNumber != nullptr, err = CHIP_ERROR_INTERNAL);

        uint64_t linkStatus;
        CFNumberGetValue((CFNumberRef) linkStatusAsCFNumber, CFNumberType::kCFNumberLongType, &linkStatus);
        CFRelease(linkStatusAsCFNumber);

        if ((linkStatus & kIONetworkLinkValid) && (linkStatus & kIONetworkLinkActive))
        {
            CFTypeRef MACAddressAsCFData =
                IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
            VerifyOrExit(MACAddressAsCFData != nullptr, err = CHIP_ERROR_INTERNAL);

            CFDataGetBytes((CFDataRef) MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), buf);
            CFRelease(MACAddressAsCFData);
            ExitNow(err = CHIP_NO_ERROR);
        }

        IOObjectRelease(controllerService);
        controllerService = 0;
    }

exit:
    if (controllerService)
    {
        IOObjectRelease(controllerService);
    }

    return err;
}
#endif // TARGET_OS_OSX

namespace {
AtomicGlobal<ConfigurationManagerImpl> gInstance;
}

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    return gInstance.get();
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_NO_ERROR;
#else  // CHIP_DISABLE_PLATFORM_KVS
    size_t len;

    // Initialize the generic implementation base class.
    ReturnErrorOnFailure(Internal::GenericConfigurationManagerImpl<PosixConfig>::Init());

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_VendorId))
    {
        ReturnErrorOnFailure(StoreVendorId(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_ProductId))
    {
        ReturnErrorOnFailure(StoreProductId(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_SoftwareVersionString))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxSoftwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(StoreSoftwareVersionString(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING, len));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_HardwareVersionString))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxHardwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(StoreHardwareVersionString(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING, len));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_VendorName))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxVendorNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(StoreVendorName(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME, len));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_ProductName))
    {
        len = strlen(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxProductNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(StoreProductName(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME, len));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_SerialNum))
    {
        len = strlen(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        VerifyOrReturnError(len <= ConfigurationManager::kMaxSerialNumberLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(StoreSerialNumber(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, len));
    }

    uint32_t rebootCount;
    if (!PosixConfig::ConfigValueExists(PosixConfig::kCounterKey_RebootCount))
    {
        // The first boot after factory reset of the Node.
        ReturnErrorOnFailure(StoreRebootCount(1));
    }
    else
    {
        ReturnErrorOnFailure(GetRebootCount(rebootCount));
        ReturnErrorOnFailure(StoreRebootCount(rebootCount + 1));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kCounterKey_TotalOperationalHours))
    {
        ReturnErrorOnFailure(StoreTotalOperationalHours(0));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kCounterKey_BootReason))
    {
        ReturnErrorOnFailure(StoreBootReason(to_underlying(BootReasonType::kUnspecified)));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_RegulatoryLocation))
    {
        uint32_t location = to_underlying(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoor);
        ReturnErrorOnFailure(WriteConfigValue(PosixConfig::kConfigKey_RegulatoryLocation, location));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_LocationCapability))
    {
        uint32_t location = to_underlying(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoor);
        ReturnErrorOnFailure(WriteConfigValue(PosixConfig::kConfigKey_LocationCapability, location));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_ConfigurationVersion))
    {
        ReturnErrorOnFailure(StoreConfigurationVersion(1));
    }

    return CHIP_NO_ERROR;
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
#if TARGET_OS_OSX
    CHIP_ERROR err = CHIP_NO_ERROR;

    io_iterator_t primaryInterfaceIterator;
    err = FindInterfaces(&primaryInterfaceIterator);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err);

    err = GetMACAddressFromInterfaces(primaryInterfaceIterator, buf);
    IOObjectRelease(primaryInterfaceIterator);

    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // TARGET_OS_OSX
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    // TODO(#742): query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset()
{
    ChipLogError(DeviceLayer, "InitiateFactoryReset not implemented");
}

CHIP_ERROR ConfigurationManagerImpl::StoreVendorId(uint16_t vendorId)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kConfigKey_VendorId, vendorId);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreProductId(uint16_t productId)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kConfigKey_ProductId, productId);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreSoftwareVersionString(const char * buf, size_t bufSize)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxSoftwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_SoftwareVersionString, buf, bufSize);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    // Read from Posix config.
    size_t outLen;
    auto err = ReadConfigValueStr(PosixConfig::kConfigKey_SoftwareVersionString, buf, bufSize, outLen);

    // If not found use genereic implementation, which gets value from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        return Internal::GenericConfigurationManagerImpl<Internal::PosixConfig>::GetSoftwareVersionString(buf, bufSize);

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreHardwareVersionString(const char * buf, size_t bufSize)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxHardwareVersionStringLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_HardwareVersionString, buf, bufSize);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreVendorName(const char * buf, size_t bufSize)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxVendorNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_VendorName, buf, bufSize);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreProductName(const char * buf, size_t bufSize)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    VerifyOrReturnError(bufSize <= ConfigurationManager::kMaxProductNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    return WriteConfigValueStr(PosixConfig::kConfigKey_ProductName, buf, bufSize);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return ReadConfigValue(PosixConfig::kCounterKey_RebootCount, rebootCount);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kCounterKey_RebootCount, rebootCount);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return ReadConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return ReadConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetRegulatoryLocation(uint8_t & location)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    uint32_t value = 0;

    CHIP_ERROR err = ReadConfigValue(PosixConfig::kConfigKey_RegulatoryLocation, value);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(value <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        location = static_cast<uint8_t>(value);
    }

    return err;
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetLocationCapability(uint8_t & location)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    uint32_t value = 0;

    CHIP_ERROR err = ReadConfigValue(PosixConfig::kConfigKey_LocationCapability, value);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(value <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        location = static_cast<uint8_t>(value);
    }

    return err;
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::GetConfigurationVersion(uint32_t & configurationVersion)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return ReadConfigValue(PosixConfig::kConfigKey_ConfigurationVersion, configurationVersion);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::StoreConfigurationVersion(uint32_t configurationVersion)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return WriteConfigValue(PosixConfig::kConfigKey_ConfigurationVersion, configurationVersion);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    PosixConfig::Key configKey{ PosixConfig::kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    PosixConfig::Key configKey{ PosixConfig::kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint16_t & val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint16_t val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValue(key, val);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValueStr(key, str);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValueStr(key, str, strLen);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
#if CHIP_DISABLE_PLATFORM_KVS
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else  // CHIP_DISABLE_PLATFORM_KVS
    return PosixConfig::WriteConfigValueBin(key, data, dataLen);
#endif // CHIP_DISABLE_PLATFORM_KVS
}

void ConfigurationManagerImpl::RunConfigUnitTest()
{
#if CHIP_DISABLE_PLATFORM_KVS
    return;
#else  // CHIP_DISABLE_PLATFORM_KVS
    PosixConfig::RunConfigUnitTest();
#endif // CHIP_DISABLE_PLATFORM_KVS
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
