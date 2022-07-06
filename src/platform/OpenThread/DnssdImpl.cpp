/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/OpenThreadUtils.h>

using namespace ::chip::DeviceLayer;

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    ReturnErrorOnFailure(ThreadStackMgr().SetSrpDnsCallbacks(initCallback, errorCallback, context));

    uint8_t macBuffer[ConfigurationManager::kPrimaryMACAddressLength];
    MutableByteSpan mac(macBuffer);
    char hostname[kHostNameMaxLength + 1] = "";
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac));
    MakeHostName(hostname, sizeof(hostname), mac);

    return ThreadStackMgr().ClearSrpHost(hostname);
}

void ChipDnssdShutdown() {}

const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp";
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ReturnErrorCodeIf(service == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (strcmp(service->mHostName, "") != 0)
    {
        ReturnErrorOnFailure(ThreadStackMgr().SetupSrpHost(service->mHostName));
    }

    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", service->mType, GetProtocolString(service->mProtocol));

    Span<const char * const> subTypes(service->mSubTypes, service->mSubTypeSize);
    Span<const TextEntry> textEntries(service->mTextEntries, service->mTextEntrySize);
    return ThreadStackMgr().AddSrpService(service->mName, serviceType, service->mPort, subTypes, textEntries);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipDnssdRemoveServices()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ThreadStackMgr().InvalidateAllSrpServices();
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    return ThreadStackMgr().RemoveInvalidSrpServices();
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                           Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (type == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    char serviceType[Dnssd::kDnssdFullTypeAndProtocolMaxSize + 1]; // +1 for null-terminator
    snprintf(serviceType, sizeof(serviceType), "%s.%s", type, GetProtocolString(protocol));

    return ThreadStackMgr().DnsBrowse(serviceType, callback, context);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (browseResult == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", browseResult->mType, GetProtocolString(browseResult->mProtocol));

    return ThreadStackMgr().DnsResolve(serviceType, browseResult->mName, callback, context);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
}

} // namespace Dnssd
} // namespace chip
