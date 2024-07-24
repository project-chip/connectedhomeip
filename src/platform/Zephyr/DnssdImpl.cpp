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


#include "platform/CHIPDeviceLayer.h"

#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <inet/IPAddress.h>

#include <zephyr/net/dns_sd.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/OpenThread/OpenThreadDnssdImpl.h>
#endif


using namespace ::chip::DeviceLayer;

namespace chip {
namespace Dnssd {

const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Do nothing because DNS is initialized by zephyr with KCONFIG
    VerifyOrReturnError(initCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ReturnErrorOnFailure(OpenThreadDnssdInit(initCallback, errorCallback, context));
#endif
    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown() 
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    switch(service->mAddressType)
    {
#if INET_CONFIG_ENABLE_IPV4
        case chip::Inet::IPAddressType::kIPv4 :
            DNS_SD_REGISTER_SERVICE (
                var,
                CONFIG_NET_HOSTNAME,
                service->mHostName,
                GetProtocolString(service->mProtocol),
                "local",
                service->mTextEntries,
                &(service->mPort)
            );
            break;
#endif // INET_CONFIG_ENABLE_IPV4
        case chip::Inet::IPAddressType::kIPv6 :
            DNS_SD_REGISTER_SERVICE (
                var,
                CONFIG_NET_HOSTNAME,
                service->mHostName,
                GetProtocolString(service->mProtocol),
                "local",
                service->mTextEntries,
                &(service->mPort)
            );
            break;
        default :
            return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
#endif //CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdPublishService(service, callback, context));
    }
#endif //CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    // We can't remove services in Zephyr
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
//TODO verify if implementation is needed
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
//TODO verify if implementation is needed
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
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
