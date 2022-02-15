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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/ConfigurationManager.h>
#include <platform/Darwin/DiagnosticDataProviderImpl.h>
#include <platform/Darwin/PosixConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    kern_return_t kernResult;
    io_object_t interfaceService;
    io_object_t controllerService;

    while ((interfaceService = IOIteratorNext(primaryInterfaceIterator)))
    {
        CFTypeRef MACAddressAsCFData = nullptr;
        kernResult                   = IORegistryEntryGetParentEntry(interfaceService, kIOServicePlane, &controllerService);
        VerifyOrExit(KERN_SUCCESS == kernResult, err = CHIP_ERROR_INTERNAL);

        MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
        VerifyOrExit(MACAddressAsCFData != nullptr, err = CHIP_ERROR_INTERNAL);

        CFDataGetBytes((CFDataRef) MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), buf);
        CFRelease(MACAddressAsCFData);

        kernResult = IOObjectRelease(controllerService);
        VerifyOrExit(KERN_SUCCESS == kernResult, err = CHIP_ERROR_INTERNAL);

        kernResult = IOObjectRelease(interfaceService);
        VerifyOrExit(KERN_SUCCESS == kernResult, err = CHIP_ERROR_INTERNAL);
    }

exit:
    if (IOObjectGetRetainCount(interfaceService))
    {
        IOObjectRelease(interfaceService);
    }

    if (IOObjectGetRetainCount(controllerService))
    {
        IOObjectRelease(controllerService);
    }

    return err;
}
#endif // TARGET_OS_OSX

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    // Initialize the generic implementation base class.
    ReturnErrorOnFailure(Internal::GenericConfigurationManagerImpl<PosixConfig>::Init());

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
        ReturnErrorOnFailure(StoreBootReason(DiagnosticDataProvider::BootReasonType::Unspecified));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_RegulatoryLocation))
    {
        uint32_t location = to_underlying(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoor);
        ReturnErrorOnFailure(WriteConfigValue(PosixConfig::kConfigKey_RegulatoryLocation, location));
    }

    if (!PosixConfig::ConfigValueExists(PosixConfig::kConfigKey_LocationCapability))
    {
        uint32_t location = to_underlying(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoor);
        ReturnErrorOnFailure(WriteConfigValue(PosixConfig::kConfigKey_LocationCapability, location));
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(PosixConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(PosixConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(PosixConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(PosixConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::GetRegulatoryLocation(uint8_t & location)
{
    uint32_t value = 0;

    CHIP_ERROR err = ReadConfigValue(PosixConfig::kConfigKey_RegulatoryLocation, value);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(value <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        location = static_cast<uint8_t>(value);
    }

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetLocationCapability(uint8_t & location)
{
    uint32_t value = 0;

    CHIP_ERROR err = ReadConfigValue(PosixConfig::kConfigKey_LocationCapability, value);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(value <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        location = static_cast<uint8_t>(value);
    }

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    PosixConfig::Key configKey{ PosixConfig::kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    PosixConfig::Key configKey{ PosixConfig::kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return PosixConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return PosixConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return PosixConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return PosixConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return PosixConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return PosixConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return PosixConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    PosixConfig::RunConfigUnitTest();
}

} // namespace DeviceLayer
} // namespace chip
