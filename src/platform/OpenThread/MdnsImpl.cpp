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

#include "lib/mdns/platform/Mdns.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <support/CodeUtils.h>

using namespace ::chip::DeviceLayer;

namespace chip {
namespace Mdns {

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    // Intentionally empty
    return CHIP_NO_ERROR;
}

const char * GetProtocolString(MdnsServiceProtocol protocol)
{
    return protocol == MdnsServiceProtocol::kMdnsProtocolUdp ? "_udp" : "_tcp";
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ReturnErrorCodeIf(service == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (strcmp(service->mHostName, "") != 0)
    {
        ReturnErrorOnFailure(ThreadStackMgr().SetupSrpHost(service->mHostName));
    }

    char serviceType[chip::Mdns::kMdnsTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", service->mType, GetProtocolString(service->mProtocol));

    Span<const char * const> subTypes(service->mSubTypes, service->mSubTypeSize);
    Span<const TextEntry> textEntries(service->mTextEntries, service->mTextEntrySize);
    return ThreadStackMgr().AddSrpService(service->mName, serviceType, service->mPort, subTypes, textEntries);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipMdnsStopPublish()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    return ThreadStackMgr().RemoveAllSrpServices();
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipMdnsStopPublishService(const MdnsService * service)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    if (service == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    char serviceType[chip::Mdns::kMdnsTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", service->mType, GetProtocolString(service->mProtocol));

    return ThreadStackMgr().RemoveSrpService(service->mName, serviceType);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, Inet::IPAddressType addressType,
                          Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (type == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    char serviceType[chip::Mdns::kMdnsTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", type, GetProtocolString(protocol));

    return ThreadStackMgr().DnsBrowse(serviceType, callback, context);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
}

CHIP_ERROR ChipMdnsResolve(MdnsService * browseResult, Inet::InterfaceId interface, MdnsResolveCallback callback, void * context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (browseResult == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    char serviceType[chip::Mdns::kMdnsTypeAndProtocolMaxSize + 1];
    snprintf(serviceType, sizeof(serviceType), "%s.%s", browseResult->mType, GetProtocolString(browseResult->mProtocol));

    return ThreadStackMgr().DnsResolve(serviceType, browseResult->mName, callback, context);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
}

} // namespace Mdns
} // namespace chip
