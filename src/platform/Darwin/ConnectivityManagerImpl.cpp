/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#if TARGET_OS_OSX
#import <CoreFoundation/CoreFoundation.h>
#import <SystemConfiguration/SCNetworkConfiguration.h>
#endif // TARGET_OS_OSX

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ReturnErrorOnFailure(GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init());
#endif

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

CHIP_ERROR ConnectivityManagerImpl::GetEthernetInterfaceName(char * outName, size_t maxLen)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#if TARGET_OS_OSX
    if (!CanCastTo<CFIndex>(maxLen))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    CFArrayRef interfaces = SCNetworkInterfaceCopyAll();
    VerifyOrReturnError(interfaces != nullptr, CHIP_ERROR_INTERNAL);

    err = CHIP_ERROR_KEY_NOT_FOUND;

    CFIndex count = CFArrayGetCount(interfaces);
    for (CFIndex i = 0; i < count; i++)
    {
        const SCNetworkInterfaceRef interface = static_cast<const SCNetworkInterfaceRef>(CFArrayGetValueAtIndex(interfaces, i));

        CFStringRef interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
        if (interfaceType == nullptr || interfaceType != kSCNetworkInterfaceTypeEthernet)
        {
            continue;
        }

        CFStringRef interfaceName = SCNetworkInterfaceGetBSDName(interface);
        if (interfaceName == nullptr)
        {
            continue;
        }

        if (!CFStringGetCString(interfaceName, outName, static_cast<CFIndex>(maxLen), kCFStringEncodingUTF8))
        {
            continue;
        }
        outName[maxLen - 1] = '\0';
        err                 = CHIP_NO_ERROR;
        break;
    }

    CFRelease(interfaces);
#endif // TARGET_OS_OSX
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::GetInterfaceStatus(const char * interfaceName, bool * status)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#if TARGET_OS_OSX
    SCDynamicStoreRef store = SCDynamicStoreCreate(nullptr, CFSTR("Matter"), nullptr, nullptr);
    if (store == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    auto path = CFStringCreateWithFormat(nullptr, nullptr, CFSTR("State:/Network/Interface/%s/Link"), interfaceName);
    if (path == nullptr)
    {
        CFRelease(store);
        return CHIP_ERROR_NO_MEMORY;
    }

    auto dict = static_cast<CFDictionaryRef>(SCDynamicStoreCopyValue(store, path));
    if (dict == nullptr)
    {
        CFRelease(path);
        CFRelease(store);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CFBooleanRef linkActive = static_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSCPropNetLinkActive));
    if (linkActive == nullptr)
    {
        CFRelease(dict);
        CFRelease(path);
        CFRelease(store);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *status = CFBooleanGetValue(linkActive);

    CFRelease(dict);
    CFRelease(path);
    CFRelease(store);

    err = CHIP_NO_ERROR;
#endif // TARGET_OS_OSX
    return err;
}

} // namespace DeviceLayer
} // namespace chip
