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

#include "DnssdImpl.h"
#include "lib/dnssd/platform/Dnssd.h"

#include <esp_err.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

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

struct MdnsQuery
{
    GenericContext * ctx;
    MdnsQuery * next;
};

static MdnsQuery * sQueryList = nullptr;
void MdnsQueryNotifier(mdns_search_once_t * searchHandle);

CHIP_ERROR AddQueryList(GenericContext * ctx)
{
    MdnsQuery * ret = static_cast<MdnsQuery *>(chip::Platform::MemoryAlloc(sizeof(MdnsQuery)));
    if (ret == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to alloc memory for MdnsQuery");
        return CHIP_ERROR_NO_MEMORY;
    }
    ret->ctx   = ctx;
    ret->next  = sQueryList;
    sQueryList = ret;
    return CHIP_NO_ERROR;
}

GenericContext * FindMdnsQuery(mdns_search_once_t * searchHandle)
{
    MdnsQuery * current = sQueryList;
    while (current)
    {
        if (current->ctx && current->ctx->mSearchHandle == searchHandle)
        {
            return current->ctx;
        }
        current = current->next;
    }
    return nullptr;
}

CHIP_ERROR RemoveMdnsQuery(GenericContext * ctx)
{
    MdnsQuery * current = sQueryList;
    MdnsQuery * front   = nullptr;

    VerifyOrReturnError(ctx != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    while (current)
    {
        if (current->ctx == ctx)
        {
            break;
        }
        front   = current;
        current = current->next;
    }
    if (!current)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    if (front)
    {
        front->next = current->next;
    }
    else
    {
        sQueryList = current->next;
    }
    if (current->ctx->mContextType == ContextType::Browse)
    {
        chip::Platform::Delete(reinterpret_cast<BrowseContext *>(current->ctx));
    }
    else if (ctx->mContextType == ContextType::Resolve)
    {
        chip::Platform::Delete(reinterpret_cast<ResolveContext *>(current->ctx));
    }
    chip::Platform::MemoryFree(current);
    return CHIP_NO_ERROR;
}

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

void ChipDnssdShutdown() {}

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
    switch (ip_protocol)
    {
#if INET_CONFIG_ENABLE_IPV4
    case MDNS_IP_PROTOCOL_V4:
        return Inet::IPAddressType::kIPv4;
#endif
    case MDNS_IP_PROTOCOL_V6:
        return Inet::IPAddressType::kIPv6;
    default:
        return Inet::IPAddressType::kAny;
    }
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

CHIP_ERROR GetIPAddress(Inet::IPAddress & outIPAddress, mdns_ip_addr_t * mdnsIPAddr)
{
    if (!mdnsIPAddr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (mdnsIPAddr->addr.type == ESP_IPADDR_TYPE_V6)
    {
        memcpy(outIPAddress.Addr, mdnsIPAddr->addr.u_addr.ip6.addr, sizeof(mdnsIPAddr->addr.u_addr.ip6.addr));
    }
    else if (mdnsIPAddr->addr.type == ESP_IPADDR_TYPE_V4)
    {
        outIPAddress.Addr[0] = 0;
        outIPAddress.Addr[1] = 0;
        outIPAddress.Addr[2] = htonl(0xFFFF);
        outIPAddress.Addr[3] = mdnsIPAddr->addr.u_addr.ip4.addr;
    }
    else
    {
        outIPAddress = Inet::IPAddress::Any;
    }
    return CHIP_NO_ERROR;
}

size_t GetResultSize(mdns_result_t * result)
{
    size_t ret = 0;
    while (result)
    {
        ret++;
        result = result->next;
    }
    return ret;
}

CHIP_ERROR OnBrowseDone(BrowseContext * ctx)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    mdns_result_t * currentResult = nullptr;
    size_t servicesIndex          = 0;
    VerifyOrExit(ctx && ctx->mBrowseCb, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (ctx->mResult)
    {
        ctx->mServiceSize = GetResultSize(ctx->mResult);
        if (ctx->mServiceSize > 0)
        {
            ctx->mServices = static_cast<DnssdService *>(chip::Platform::MemoryCalloc(ctx->mServiceSize, sizeof(DnssdService)));
            if (!ctx->mServices)
            {
                ChipLogError(DeviceLayer, "Failed to alloc memory for Dnssd services");
                ctx->mServiceSize = 0;
                error             = CHIP_ERROR_NO_MEMORY;
                ExitNow();
            }
            currentResult = ctx->mResult;
            servicesIndex = 0;
            while (currentResult)
            {
                Platform::CopyString(ctx->mServices[servicesIndex].mName, currentResult->instance_name);
                Platform::CopyString(ctx->mServices[servicesIndex].mHostName, currentResult->hostname);
                Platform::CopyString(ctx->mServices[servicesIndex].mType, currentResult->service_type);
                ctx->mServices[servicesIndex].mProtocol      = ctx->mProtocol;
                ctx->mServices[servicesIndex].mAddressType   = MapAddressType(currentResult->ip_protocol);
                ctx->mServices[servicesIndex].mTransportType = ctx->mAddressType;
                ctx->mServices[servicesIndex].mPort          = currentResult->port;
                ctx->mServices[servicesIndex].mInterface     = ctx->mInterfaceId;
                ctx->mServices[servicesIndex].mTextEntries =
                    GetTextEntry(currentResult->txt, currentResult->txt_value_len, currentResult->txt_count);
                ctx->mServices[servicesIndex].mTextEntrySize = currentResult->txt_count;
                ctx->mServices[servicesIndex].mSubTypes      = NULL;
                ctx->mServices[servicesIndex].mSubTypeSize   = 0;
                if (currentResult->addr)
                {
                    Inet::IPAddress IPAddr;
                    error = GetIPAddress(IPAddr, currentResult->addr);
                    SuccessOrExit(error);
                    ctx->mServices[servicesIndex].mAddress.SetValue(IPAddr);
                }
                currentResult = currentResult->next;
                servicesIndex++;
            }
        }
    }
exit:
    ctx->mBrowseCb(ctx->mCbContext, ctx->mServices, ctx->mServiceSize, true, error);
    return RemoveMdnsQuery(reinterpret_cast<GenericContext *>(ctx));
}

size_t GetAddressCount(mdns_ip_addr_t * addr)
{
    size_t ret = 0;
    while (addr)
    {
        ret++;
        addr = addr->next;
    }
    return ret;
}

CHIP_ERROR OnResolveQuerySrvDone(ResolveContext * ctx)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    size_t addressIndex = 0;

    VerifyOrExit(ctx && ctx->mResolveCb, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ctx->mService == nullptr && ctx->mResolveState == ResolveContext::ResolveState::QuerySrv,
                 error = CHIP_ERROR_INCORRECT_STATE);
    if (ctx->mResult)
    {
        ctx->mService = static_cast<DnssdService *>(chip::Platform::MemoryAlloc(sizeof(DnssdService)));
        VerifyOrExit(ctx->mService != nullptr, error = CHIP_ERROR_NO_MEMORY);
        Platform::CopyString(ctx->mService->mName, ctx->mResult->instance_name);
        Platform::CopyString(ctx->mService->mHostName, ctx->mResult->hostname);
        Platform::CopyString(ctx->mService->mType, ctx->mResult->service_type);
        ctx->mService->mProtocol      = ctx->mProtocol;
        ctx->mService->mAddressType   = MapAddressType(ctx->mResult->ip_protocol);
        ctx->mService->mTransportType = ctx->mService->mAddressType;
        ctx->mService->mPort          = ctx->mResult->port;
        ctx->mService->mInterface     = ctx->mInterfaceId;
        ctx->mService->mSubTypes      = nullptr;
        ctx->mService->mSubTypeSize   = 0;

        if (ctx->mResult->addr)
        {
            ctx->mAddressCount = GetAddressCount(ctx->mResult->addr);
            if (ctx->mAddressCount > 0)
            {
                ctx->mAddresses =
                    static_cast<Inet::IPAddress *>(chip::Platform::MemoryCalloc(ctx->mAddressCount, sizeof(Inet::IPAddress)));
                if (ctx->mAddresses == nullptr)
                {
                    ChipLogError(DeviceLayer, "Failed to alloc memory for addresses");
                    error              = CHIP_ERROR_NO_MEMORY;
                    ctx->mAddressCount = 0;
                    ExitNow();
                }
                auto * addr = ctx->mResult->addr;
                while (addr)
                {
                    GetIPAddress(ctx->mAddresses[addressIndex], addr);
                    addressIndex++;
                    addr = addr->next;
                }
            }
            else
            {
                ctx->mAddresses    = nullptr;
                ctx->mAddressCount = 0;
            }
        }
    }
exit:
    if (error != CHIP_NO_ERROR)
    {
        ctx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(nullptr, 0), error);
        RemoveMdnsQuery(reinterpret_cast<GenericContext *>(ctx));
        return error;
    }
    mdns_query_results_free(ctx->mResult);
    mdns_query_async_delete(ctx->mSearchHandle);
    ctx->mResult       = nullptr;
    ctx->mResolveState = ResolveContext::ResolveState::QueryTxt;
    // then query the text entries
    ctx->mSearchHandle = mdns_query_async_new(ctx->mInstanceName, ctx->mType, GetProtocolString(ctx->mProtocol), MDNS_TYPE_TXT,
                                              kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnResolveQueryTxtDone(ResolveContext * ctx)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(ctx && ctx->mResolveCb, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ctx->mService && ctx->mResolveState == ResolveContext::ResolveState::QueryTxt, error = CHIP_ERROR_INCORRECT_STATE);
    if (ctx->mResult)
    {
        ctx->mService->mTextEntries   = GetTextEntry(ctx->mResult->txt, ctx->mResult->txt_value_len, ctx->mResult->txt_count);
        ctx->mService->mTextEntrySize = ctx->mResult->txt_count;
    }
    else
    {
        ctx->mService->mTextEntries   = nullptr;
        ctx->mService->mTextEntrySize = 0;
    }
exit:
    if (error != CHIP_NO_ERROR)
    {
        ctx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(nullptr, 0), error);
    }
    else
    {
        ctx->mResolveCb(ctx->mCbContext, ctx->mService, Span<Inet::IPAddress>(ctx->mAddresses, ctx->mAddressCount), error);
    }
    RemoveMdnsQuery(reinterpret_cast<GenericContext *>(ctx));
    return error;
}

void MdnsQueryDone(intptr_t context)
{
    if (!context)
    {
        return;
    }
    mdns_search_once_t * searchHandle = reinterpret_cast<mdns_search_once_t *>(context);
    GenericContext * ctx              = FindMdnsQuery(searchHandle);
    if (mdns_query_async_get_results(searchHandle, kTimeoutMilli, &(ctx->mResult)))
    {
        if (ctx->mContextType == ContextType::Browse)
        {
            OnBrowseDone(reinterpret_cast<BrowseContext *>(ctx));
        }
        else if (ctx->mContextType == ContextType::Resolve)
        {
            ResolveContext * resolveCtx = reinterpret_cast<ResolveContext *>(ctx);
            if (resolveCtx->mResolveState == ResolveContext::ResolveState::QuerySrv)
            {
                OnResolveQuerySrvDone(resolveCtx);
            }
            else if (resolveCtx->mResolveState == ResolveContext::ResolveState::QueryTxt)
            {
                OnResolveQueryTxtDone(resolveCtx);
            }
        }
    }
}

void MdnsQueryNotifier(mdns_search_once_t * searchHandle)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(MdnsQueryDone, reinterpret_cast<intptr_t>(searchHandle));
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    mdns_search_once_t * searchHandle =
        mdns_query_async_new(NULL, type, GetProtocolString(protocol), MDNS_TYPE_PTR, kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    BrowseContext * ctx =
        chip::Platform::New<BrowseContext>(type, protocol, interface, searchHandle, addressType, callback, context);
    if (!ctx)
    {
        ChipLogError(DeviceLayer, "Failed to alloc memory for browse context");
        mdns_query_async_delete(searchHandle);
        return CHIP_ERROR_NO_MEMORY;
    }
    error = AddQueryList(reinterpret_cast<GenericContext *>(ctx));
    if (error != CHIP_NO_ERROR)
    {
        chip::Platform::Delete(ctx);
    }
    else
    {
        *browseIdentifier = reinterpret_cast<intptr_t>(nullptr);
    }
    return error;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    CHIP_ERROR error                  = CHIP_NO_ERROR;
    mdns_search_once_t * searchHandle = mdns_query_async_new(service->mName, service->mType, GetProtocolString(service->mProtocol),
                                                             MDNS_TYPE_SRV, kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    ResolveContext * ctx              = chip::Platform::New<ResolveContext>(service, interface, searchHandle, callback, context);
    if (!ctx)
    {
        ChipLogError(DeviceLayer, "Failed to alloc memory for resolve context");
        mdns_query_async_delete(searchHandle);
        return CHIP_ERROR_NO_MEMORY;
    }
    error = AddQueryList(reinterpret_cast<GenericContext *>(ctx));
    if (error != CHIP_NO_ERROR)
    {
        chip::Platform::Delete(ctx);
    }
    return error;
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
