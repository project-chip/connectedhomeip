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
#include <platform/Darwin/PosixConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

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

/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
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

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    // TODO(#742): query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset()
{
    ChipLogError(DeviceLayer, "InitiateFactoryReset not implemented");
}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    PosixConfig::Key configKey{ kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    PosixConfig::Key configKey{ kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

} // namespace DeviceLayer
} // namespace chip
