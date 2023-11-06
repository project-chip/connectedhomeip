/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include "lib/dnssd/platform/Dnssd.h"
#include "platform/CHIPDeviceLayer.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/OpenThread/OpenThreadDnssdImpl.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/ESP32/WiFiDnssdImpl.h>
#endif

using namespace ::chip::DeviceLayer;

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ReturnErrorOnFailure(WiFiDnssdInit(initCallback, errorCallback, context));
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ReturnErrorOnFailure(OpenThreadDnssdInit(initCallback, errorCallback, context));
#endif
    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown() {}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(WiFiDnssdPublishService(service, callback, context));
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdPublishService(service, callback, context));
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(WiFiDnssdRemoveServices());
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdRemoveServices());
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdFinalizeServiceUpdate());
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(WiFiDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(WiFiDnssdResolve(service, interface, callback, context));
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdResolve(service, interface, callback, context));
    }
#endif
    return CHIP_NO_ERROR;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName) {}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
