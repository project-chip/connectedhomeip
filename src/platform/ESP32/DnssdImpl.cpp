/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <esp_err.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>
#include <mdns.h>

#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    esp_err_t espError;

    espError = mdns_init();
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (espError != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp mdns internal error: %s", esp_err_to_name(espError));
    }
    initCallback(context, error);

    return error;
}

CHIP_ERROR ChipDnssdShutdown()
{
    return CHIP_NO_ERROR;
}

static const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    CHIP_ERROR error        = CHIP_NO_ERROR;
    mdns_txt_item_t * items = nullptr;
    esp_err_t espError;

    if (strcmp(service->mHostName, "") != 0)
    {
        VerifyOrExit(mdns_hostname_set(service->mHostName) == ESP_OK, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(service->mTextEntrySize <= UINT8_MAX, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (service->mTextEntries)
    {
        items = static_cast<mdns_txt_item_t *>(chip::Platform::MemoryCalloc(service->mTextEntrySize, sizeof(mdns_txt_item_t)));
        VerifyOrExit(items != nullptr, error = CHIP_ERROR_NO_MEMORY);
        for (size_t i = 0; i < service->mTextEntrySize; i++)
        {
            items[i].key = service->mTextEntries[i].mKey;
            // Unfortunately ESP mdns stack doesn't support arbitrary binary data
            items[i].value = reinterpret_cast<const char *>(service->mTextEntries[i].mData);
        }
    }

    espError = mdns_service_add(service->mName, service->mType, GetProtocolString(service->mProtocol), service->mPort, items,
                                service->mTextEntrySize);
    // The mdns_service_add will return error if we try to add an existing service
    if (espError != ESP_OK && espError != ESP_ERR_NO_MEM)
    {
        espError = mdns_service_txt_set(service->mType, GetProtocolString(service->mProtocol), items,
                                        static_cast<uint8_t>(service->mTextEntrySize));
    }
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (items != nullptr)
    {
        chip::Platform::MemoryFree(items);
    }

    return error;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    mdns_service_remove("_matter", "_tcp");
    mdns_service_remove("_matterc", "_udp");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

Inet::IPAddressType MapAddressType(mdns_ip_protocol_t ip_protocol)
{
    if (ip_protocol == MDNS_IP_PROTOCOL_V4)
        return Inet::IPAddressType::kIPv4;
    else if (ip_protocol == MDNS_IP_PROTOCOL_V6)
        return Inet::IPAddressType::kIPv6;
    else
        return Inet::IPAddressType::kAny;
}

TextEntry * GetTextEntry(mdns_txt_item_t * txt_array, uint8_t * txt_value_len, size_t txt_count)
{
    if (txt_count == 0 || txt_array == NULL)
    {
        return NULL;
    }
    TextEntry * ret = static_cast<TextEntry *>(chip::Platform::MemoryCalloc(txt_count, sizeof(TextEntry)));
    if (ret)
    {
        for (size_t TextEntryIndex = 0; TextEntryIndex < txt_count; ++TextEntryIndex)
        {
            ret[TextEntryIndex].mKey      = txt_array[TextEntryIndex].key;
            ret[TextEntryIndex].mData     = reinterpret_cast<const uint8_t *>(txt_array[TextEntryIndex].value);
            ret[TextEntryIndex].mDataSize = txt_value_len[TextEntryIndex];
        }
    }
    return ret;
}

CHIP_ERROR GetIPAddress(Inet::IPAddress & outIPAddress, mdns_ip_addr_t * MdnsIPAddr)
{
    if (!MdnsIPAddr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (MdnsIPAddr->addr.type == ESP_IPADDR_TYPE_V6)
    {
        memcpy(outIPAddress.Addr, MdnsIPAddr->addr.u_addr.ip6.addr, sizeof(MdnsIPAddr->addr.u_addr.ip6.addr));
    }
    else if (MdnsIPAddr->addr.type == ESP_IPADDR_TYPE_V4)
    {
        outIPAddress.Addr[0] = 0;
        outIPAddress.Addr[1] = 0;
        outIPAddress.Addr[2] = htonl(0xFFFF);
        outIPAddress.Addr[3] = MdnsIPAddr->addr.u_addr.ip4.addr;
    }
    else
    {
        outIPAddress = Inet::IPAddress::Any;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
    CHIP_ERROR error               = CHIP_NO_ERROR;
    esp_err_t espError             = ESP_OK;
    mdns_result_t * browse_results = NULL;
    mdns_result_t * current_result = NULL;
    DnssdService * ServicesArray   = NULL;
    size_t ServicesArraySize       = 0;
    size_t ServicesArrayIndex      = 0;

    VerifyOrExit(type != NULL && callback != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    espError = mdns_query_ptr(type, GetProtocolString(protocol), kTimeoutMilli, kMaxResults, &browse_results);
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);
    current_result = browse_results;
    while (current_result)
    {
        ServicesArraySize++;
        current_result = current_result->next;
    }
    if (ServicesArraySize > 0)
    {
        ServicesArray = static_cast<DnssdService *>(chip::Platform::MemoryCalloc(ServicesArraySize, sizeof(DnssdService)));
        VerifyOrExit(ServicesArray != NULL, error = CHIP_ERROR_NO_MEMORY);
        current_result     = browse_results;
        ServicesArrayIndex = 0;
        while (current_result)
        {
            strncpy(ServicesArray[ServicesArrayIndex].mName, current_result->instance_name,
                    strnlen(current_result->instance_name, Common::kInstanceNameMaxLength));
            strncpy(ServicesArray[ServicesArrayIndex].mHostName, current_result->hostname,
                    strnlen(current_result->hostname, kHostNameMaxLength));
            strncpy(ServicesArray[ServicesArrayIndex].mType, current_result->service_type,
                    strnlen(current_result->service_type, kDnssdTypeMaxSize));
            ServicesArray[ServicesArrayIndex].mProtocol      = protocol;
            ServicesArray[ServicesArrayIndex].mAddressType   = MapAddressType(current_result->ip_protocol);
            ServicesArray[ServicesArrayIndex].mTransportType = addressType;
            ServicesArray[ServicesArrayIndex].mPort          = current_result->port;
            ServicesArray[ServicesArrayIndex].mInterface     = interface;
            ServicesArray[ServicesArrayIndex].mTextEntries =
                GetTextEntry(current_result->txt, current_result->txt_value_len, current_result->txt_count);
            ServicesArray[ServicesArrayIndex].mTextEntrySize = current_result->txt_count;
            ServicesArray[ServicesArrayIndex].mSubTypes      = NULL;
            ServicesArray[ServicesArrayIndex].mSubTypeSize   = 0;
            if (current_result->addr)
            {
                Inet::IPAddress IPAddr;
                error = GetIPAddress(IPAddr, current_result->addr);
                SuccessOrExit(error);
                ServicesArray[ServicesArrayIndex].mAddress.SetValue(IPAddr);
            }
            current_result = current_result->next;
            ServicesArrayIndex++;
        }
    }
    callback(context, ServicesArray, ServicesArraySize, CHIP_NO_ERROR);

exit:
    if (ServicesArray)
    {
        for (ServicesArrayIndex = 0; ServicesArrayIndex < ServicesArraySize; ++ServicesArrayIndex)
        {
            if (ServicesArray[ServicesArrayIndex].mTextEntries)
            {
                chip::Platform::MemoryFree(ServicesArray[ServicesArrayIndex].mTextEntries);
            }
        }
        chip::Platform::MemoryFree(ServicesArray);
    }
    if (browse_results)
    {
        mdns_query_results_free(browse_results);
    }
    return error;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    CHIP_ERROR error               = CHIP_NO_ERROR;
    esp_err_t espError             = ESP_OK;
    mdns_result_t * resolve_result = NULL;
    mdns_ip_addr_t * resolve_addr  = NULL;
    mdns_ip_addr_t * extra_addr    = NULL;
    Inet::IPAddress * extraIPs     = NULL;
    size_t extraIPsSize            = 0;
    size_t extraIPsIndex           = 0;

    VerifyOrExit(service != NULL && callback != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    // query_srv to obtain IPAddress
    espError =
        mdns_query_srv(service->mName, service->mType, GetProtocolString(service->mProtocol), kTimeoutMilli, &resolve_result);
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);
    if (resolve_result)
    {
        if (resolve_result->addr)
        {
            Inet::IPAddress IPAddr;
            resolve_addr = resolve_result->addr;
            error        = GetIPAddress(IPAddr, resolve_addr);
            SuccessOrExit(error);
            service->mAddress.SetValue(IPAddr);
            service->mPort = resolve_result->port;
            // fill the extra IP address
            extra_addr = resolve_addr->next;
            while (extra_addr)
            {
                extraIPsSize++;
                extra_addr = extra_addr->next;
            }
            if (extraIPsSize > 0)
            {
                extra_addr = resolve_addr->next;
                extraIPs   = (Inet::IPAddress *) chip::Platform::MemoryCalloc(extraIPsSize, sizeof(Inet::IPAddress));
                VerifyOrExit(extraIPs != NULL, error = CHIP_ERROR_NO_MEMORY);
                while (extra_addr)
                {
                    error = GetIPAddress(extraIPs[extraIPsIndex], extra_addr);
                    SuccessOrExit(error);
                    extra_addr = extra_addr->next;
                    extraIPsIndex++;
                }
            }
        }
        strncpy(service->mHostName, resolve_result->hostname, strnlen(resolve_result->hostname, kHostNameMaxLength));
        mdns_query_results_free(resolve_result);
        resolve_result = NULL;
    }
    // query_txt to obtain text entries
    espError =
        mdns_query_txt(service->mName, service->mType, GetProtocolString(service->mProtocol), kTimeoutMilli, &resolve_result);
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);
    if (resolve_result)
    {
        service->mTextEntries   = GetTextEntry(resolve_result->txt, resolve_result->txt_value_len, resolve_result->txt_count);
        service->mTextEntrySize = resolve_result->txt_count;
    }

    callback(context, service, Span<Inet::IPAddress>(extraIPs, extraIPsSize), CHIP_NO_ERROR);
exit:
    if (extraIPs)
    {
        chip::Platform::MemoryFree(extraIPs);
    }
    if (service->mTextEntries)
    {
        chip::Platform::MemoryFree(service->mTextEntries);
    }
    if (resolve_result)
    {
        mdns_query_results_free(resolve_result);
    }
    return error;
}

} // namespace Dnssd
} // namespace chip
