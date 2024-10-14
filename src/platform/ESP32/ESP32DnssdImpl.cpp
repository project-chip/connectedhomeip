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

#include "ESP32DnssdImpl.h"
#include "lib/dnssd/platform/Dnssd.h"

#include <esp_err.h>
#include <esp_netif_net_stack.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32Utils.h>

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
static void MdnsQueryNotifier(mdns_search_once_t * queryHandle);

static CHIP_ERROR AddQueryList(GenericContext * ctx)
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

static GenericContext * FindMdnsQuery(mdns_search_once_t * queryHandle)
{
    MdnsQuery * current = sQueryList;
    while (current)
    {
        if (current->ctx)
        {
            if (current->ctx->mContextType == ContextType::Browse)
            {
                BrowseContext * browseCtx = reinterpret_cast<BrowseContext *>(current->ctx);
                if (browseCtx->mPtrQueryHandle == queryHandle)
                {
                    return current->ctx;
                }
            }
            else if (current->ctx->mContextType == ContextType::Resolve)
            {
                ResolveContext * resolveCtx = reinterpret_cast<ResolveContext *>(current->ctx);
                if (resolveCtx->mSrvQueryHandle == queryHandle || resolveCtx->mTxtQueryHandle == queryHandle)
                {
                    return current->ctx;
                }
            }
        }
        current = current->next;
    }
    return nullptr;
}

static CHIP_ERROR RemoveMdnsQuery(GenericContext * ctx)
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

CHIP_ERROR EspDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
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

static const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR EspDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    CHIP_ERROR error        = CHIP_NO_ERROR;
    mdns_txt_item_t * items = nullptr;
    esp_err_t espError;

    if (strcmp(service->mHostName, "") != 0)
    {
        VerifyOrExit(mdns_hostname_set(service->mHostName) == ESP_OK, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(service->mTextEntrySize <= UINT8_MAX, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (service->mTextEntries && service->mTextEntrySize > 0)
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

    // Remove service before adding it
    if (mdns_service_exists(service->mType, GetProtocolString(service->mProtocol), nullptr))
    {
        mdns_service_remove_for_host(service->mName, service->mType, GetProtocolString(service->mProtocol), nullptr);
    }

    espError = mdns_service_add(service->mName, service->mType, GetProtocolString(service->mProtocol), service->mPort, items,
                                service->mTextEntrySize);
    // The mdns_service_add will return error if we try to add an existing service
    if (espError != ESP_OK && espError != ESP_ERR_NO_MEM)
    {
        espError = mdns_service_txt_set(service->mType, GetProtocolString(service->mProtocol), items,
                                        static_cast<uint8_t>(service->mTextEntrySize));
    }
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    for (size_t i = 0; i < service->mSubTypeSize; i++)
    {
        mdns_service_subtype_add_for_host(service->mName, service->mType, GetProtocolString(service->mProtocol), service->mHostName,
                                          service->mSubTypes[i]);
    }
#endif
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (items != nullptr)
    {
        chip::Platform::MemoryFree(items);
    }

    return error;
}

CHIP_ERROR EspDnssdRemoveServices()
{
    if (mdns_service_exists("_matter", "_tcp", nullptr))
    {
        mdns_service_remove("_matter", "_tcp");
    }
    if (mdns_service_exists("_matterc", "_udp", nullptr))
    {
        mdns_service_remove("_matterc", "_udp");
    }
    if (mdns_service_exists("_matterd", "_udp", nullptr))
    {
        mdns_service_remove("_matterd", "_udp");
    }
    return CHIP_NO_ERROR;
}

static Inet::InterfaceId GetServiceInterfaceId(esp_netif_t * esp_netif)
{
    if (!esp_netif)
    {
        // If the InterfaceId in the context and esp_netif in current result is Null,
        // we will use the Station or Ethernet netif by default.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        return Inet::InterfaceId(DeviceLayer::Internal::ESP32Utils::GetStationNetif());
#elif CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
        return Inet::InterfaceId(
            DeviceLayer::Internal::ESP32Utils::GetNetif(DeviceLayer::Internal::ESP32Utils::kDefaultEthernetNetifKey));
#endif
    }
    return Inet::InterfaceId(static_cast<struct netif *>(esp_netif_get_netif_impl(esp_netif)));
}

static Inet::IPAddressType MapAddressType(mdns_ip_protocol_t ip_protocol)
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

static TextEntry * GetTextEntry(mdns_txt_item_t * txt_array, uint8_t * txt_value_len, size_t txt_count)
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

static CHIP_ERROR GetIPAddress(Inet::IPAddress & outIPAddress, mdns_ip_addr_t * mdnsIPAddr)
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

static CHIP_ERROR OnBrowseDone(BrowseContext * ctx)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    mdns_result_t * currentResult = nullptr;
    size_t servicesIndex          = 0;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mdns_result_t * delegatedResults = nullptr;
#endif
    VerifyOrExit(ctx && ctx->mBrowseCb, error = CHIP_ERROR_INVALID_ARGUMENT);
    ctx->mServiceSize = GetResultSize(ctx->mPtrQueryResult);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mdns_lookup_delegated_service(NULL, ctx->mType, GetProtocolString(ctx->mProtocol), kMaxResults - ctx->mServiceSize,
                                  &delegatedResults);
    while (delegatedResults)
    {
        mdns_result_t * tmp    = delegatedResults->next;
        delegatedResults->next = ctx->mPtrQueryResult;
        ctx->mPtrQueryResult   = delegatedResults;
        delegatedResults       = tmp;
        ctx->mServiceSize++;
    }
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)

    if (ctx->mPtrQueryResult && ctx->mServiceSize > 0)
    {
        if (ctx->mServiceSize > 0)
        {
            ctx->mService = static_cast<DnssdService *>(chip::Platform::MemoryCalloc(ctx->mServiceSize, sizeof(DnssdService)));
            if (!ctx->mService)
            {
                ChipLogError(DeviceLayer, "Failed to alloc memory for Dnssd services");
                ctx->mServiceSize = 0;
                error             = CHIP_ERROR_NO_MEMORY;
                ExitNow();
            }
            currentResult = ctx->mPtrQueryResult;
            servicesIndex = 0;
            while (currentResult)
            {
                Platform::CopyString(ctx->mService[servicesIndex].mName, currentResult->instance_name);
                Platform::CopyString(ctx->mService[servicesIndex].mHostName, currentResult->hostname);
                Platform::CopyString(ctx->mService[servicesIndex].mType, currentResult->service_type);
                ctx->mService[servicesIndex].mProtocol      = ctx->mProtocol;
                ctx->mService[servicesIndex].mAddressType   = MapAddressType(currentResult->ip_protocol);
                ctx->mService[servicesIndex].mTransportType = ctx->mAddressType;
                ctx->mService[servicesIndex].mPort          = currentResult->port;
                ctx->mService[servicesIndex].mTextEntries =
                    GetTextEntry(currentResult->txt, currentResult->txt_value_len, currentResult->txt_count);
                ctx->mService[servicesIndex].mTextEntrySize = currentResult->txt_count;
                ctx->mService[servicesIndex].mSubTypes      = NULL;
                ctx->mService[servicesIndex].mSubTypeSize   = 0;
                ctx->mService[servicesIndex].mInterface     = ctx->mInterfaceId != chip::Inet::InterfaceId::Null()
                        ? ctx->mInterfaceId
                        : GetServiceInterfaceId(currentResult->esp_netif);
                if (currentResult->addr)
                {
                    Inet::IPAddress IPAddr;
                    error = GetIPAddress(IPAddr, currentResult->addr);
                    SuccessOrExit(error);
                    ctx->mService[servicesIndex].mAddress.emplace(IPAddr);
                }
                currentResult = currentResult->next;
                servicesIndex++;
            }
        }
    }
exit:
    ctx->mBrowseCb(ctx->mCbContext, ctx->mService, ctx->mServiceSize, true, error);
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

static CHIP_ERROR ParseIPAddresses(ResolveContext * ctx)
{
    size_t addressIndex = 0;
    if (ctx->mAddrQueryResult && ctx->mAddrQueryResult->addr)
    {
        ctx->mAddressCount = GetAddressCount(ctx->mAddrQueryResult->addr);
        if (ctx->mAddressCount > 0)
        {
            ctx->mAddresses =
                static_cast<Inet::IPAddress *>(chip::Platform::MemoryCalloc(ctx->mAddressCount, sizeof(Inet::IPAddress)));
            if (ctx->mAddresses == nullptr)
            {
                ChipLogError(DeviceLayer, "Failed to alloc memory for addresses");
                ctx->mAddressCount = 0;
                return CHIP_ERROR_NO_MEMORY;
            }
            auto * addr = ctx->mAddrQueryResult->addr;
            while (addr)
            {
                GetIPAddress(ctx->mAddresses[addressIndex], addr);
                addressIndex++;
                addr = addr->next;
            }
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

static CHIP_ERROR ParseSrvResult(ResolveContext * ctx)
{
    if (ctx->mSrvQueryResult)
    {
        if (!ctx->mService)
        {
            ctx->mService = static_cast<DnssdService *>(chip::Platform::MemoryAlloc(sizeof(DnssdService)));
        }
        VerifyOrReturnError(ctx->mService, CHIP_ERROR_NO_MEMORY);
        ctx->mServiceSize = 1;
        Platform::CopyString(ctx->mService->mName, ctx->mSrvQueryResult->instance_name);
        Platform::CopyString(ctx->mService->mHostName, ctx->mSrvQueryResult->hostname);
        Platform::CopyString(ctx->mService->mType, ctx->mSrvQueryResult->service_type);
        ctx->mService->mProtocol      = ctx->mProtocol;
        ctx->mService->mAddressType   = MapAddressType(ctx->mSrvQueryResult->ip_protocol);
        ctx->mService->mTransportType = ctx->mService->mAddressType;
        ctx->mService->mPort          = ctx->mSrvQueryResult->port;
        ctx->mService->mSubTypes      = nullptr;
        ctx->mService->mSubTypeSize   = 0;
        ctx->mService->mInterface     = ctx->mInterfaceId != chip::Inet::InterfaceId::Null()
                ? ctx->mInterfaceId
                : GetServiceInterfaceId(ctx->mSrvQueryResult->esp_netif);
        return CHIP_NO_ERROR;
    }
    else
    {
        ctx->mService     = nullptr;
        ctx->mServiceSize = 0;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// ParseTxtResult should be called after ParseSrvResult
static CHIP_ERROR ParseTxtResult(ResolveContext * ctx)
{
    VerifyOrReturnError(ctx->mService, CHIP_ERROR_INCORRECT_STATE);
    if (ctx->mTxtQueryResult)
    {
        ctx->mService->mTextEntries =
            GetTextEntry(ctx->mTxtQueryResult->txt, ctx->mTxtQueryResult->txt_value_len, ctx->mTxtQueryResult->txt_count);
        ctx->mService->mTextEntrySize = ctx->mTxtQueryResult->txt_count;
    }
    else
    {
        ctx->mService->mTextEntries   = nullptr;
        ctx->mService->mTextEntrySize = 0;
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OnResolveDone(ResolveContext * ctx)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(ctx && ctx->mResolveCb, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!ctx->mService && ctx->mSrvAddrQueryFinished && ctx->mTxtQueryFinished, error = CHIP_ERROR_INCORRECT_STATE);
    error = ParseSrvResult(ctx);
    SuccessOrExit(error);
    error = ParseIPAddresses(ctx);
    SuccessOrExit(error);
    error = ParseTxtResult(ctx);
    SuccessOrExit(error);
exit:
    if (error != CHIP_NO_ERROR)
    {
        ctx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(), error);
    }
    else
    {
        ctx->mResolveCb(ctx->mCbContext, ctx->mService, Span<Inet::IPAddress>(ctx->mAddresses, ctx->mAddressCount), error);
    }
    RemoveMdnsQuery(reinterpret_cast<GenericContext *>(ctx));
    return error;
}

static mdns_result_t * MdnsQueryGetResults(mdns_search_once_t * queryHandle)
{
    mdns_result_t * ret = nullptr;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    if (mdns_query_async_get_results(queryHandle, kTimeoutMilli, &ret, NULL))
#else
    if (mdns_query_async_get_results(queryHandle, kTimeoutMilli, &ret))
#endif
    {
        return ret;
    }
    return nullptr;
}

static void MdnsQueryDone(intptr_t context)
{
    if (!context)
    {
        return;
    }
    mdns_search_once_t * queryHandle = reinterpret_cast<mdns_search_once_t *>(context);
    mdns_result_t * result           = MdnsQueryGetResults(queryHandle);
    GenericContext * ctx             = FindMdnsQuery(queryHandle);
    if (!ctx)
    {
        mdns_query_results_free(result);
        mdns_query_async_delete(queryHandle);
        return;
    }
    if (ctx->mContextType == ContextType::Browse)
    {
        BrowseContext * browseCtx  = reinterpret_cast<BrowseContext *>(ctx);
        browseCtx->mPtrQueryResult = result;
        OnBrowseDone(browseCtx);
    }
    else if (ctx->mContextType == ContextType::Resolve)
    {

        ResolveContext * resolveCtx = reinterpret_cast<ResolveContext *>(ctx);
        if (resolveCtx->mSrvQueryHandle == queryHandle)
        {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
            // No result found, look up delegated services.
            if (!result)
            {
                mdns_lookup_delegated_service(resolveCtx->mInstanceName, resolveCtx->mType,
                                              GetProtocolString(resolveCtx->mProtocol), kMaxResults, &result);
            }
#endif
            if (!result)
            {
                resolveCtx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_INVALID_ARGUMENT);
                RemoveMdnsQuery(ctx);
                return;
            }
            // If SRV Query Result is empty, the result is for SRV Query.
            if (!resolveCtx->mSrvQueryResult)
            {
                resolveCtx->mSrvQueryResult = result;
                if (result->addr)
                {
                    resolveCtx->mAddrQueryResult      = result;
                    resolveCtx->mSrvAddrQueryFinished = true;
                }
                else
                {
                    // If there is no A/AAAA records in SRV query response, we will send an AAAA query for the IP addresses.
                    mdns_query_async_delete(resolveCtx->mSrvQueryHandle);
                    resolveCtx->mAddrQueryResult = nullptr;
                    resolveCtx->mSrvQueryHandle  = mdns_query_async_new(result->hostname, NULL, NULL, MDNS_TYPE_AAAA, kTimeoutMilli,
                                                                        kMaxResults, MdnsQueryNotifier);
                    if (!resolveCtx->mSrvQueryHandle)
                    {
                        resolveCtx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_NO_MEMORY);
                        RemoveMdnsQuery(ctx);
                        return;
                    }
                }
            }
            else if (!resolveCtx->mAddrQueryResult)
            {
                resolveCtx->mAddrQueryResult      = result;
                resolveCtx->mSrvAddrQueryFinished = true;
            }
            else
            {
                resolveCtx->mResolveCb(ctx->mCbContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_INCORRECT_STATE);
                RemoveMdnsQuery(ctx);
                return;
            }
        }
        else if (resolveCtx->mTxtQueryHandle == queryHandle)
        {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
            // No result found, look up delegated services.
            if (!result)
            {
                mdns_lookup_delegated_service(resolveCtx->mInstanceName, resolveCtx->mType,
                                              GetProtocolString(resolveCtx->mProtocol), kMaxResults, &result);
            }
#endif
            resolveCtx->mTxtQueryResult   = result;
            resolveCtx->mTxtQueryFinished = true;
        }
        if (resolveCtx->mTxtQueryFinished && resolveCtx->mSrvAddrQueryFinished)
        {
            OnResolveDone(resolveCtx);
        }
    }
}

static void MdnsQueryNotifier(mdns_search_once_t * searchHandle)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(MdnsQueryDone, reinterpret_cast<intptr_t>(searchHandle));
}

CHIP_ERROR EspDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                          intptr_t * browseIdentifier)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    mdns_search_once_t * queryHandle =
        mdns_query_async_new(NULL, type, GetProtocolString(protocol), MDNS_TYPE_PTR, kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    VerifyOrReturnError(queryHandle, CHIP_ERROR_NO_MEMORY);
    BrowseContext * ctx =
        chip::Platform::New<BrowseContext>(type, protocol, interface, queryHandle, addressType, callback, context);
    if (!ctx)
    {
        ChipLogError(DeviceLayer, "Failed to alloc memory for browse context");
        mdns_query_async_delete(queryHandle);
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

CHIP_ERROR EspDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    mdns_search_once_t * querySrv = mdns_query_async_new(service->mName, service->mType, GetProtocolString(service->mProtocol),
                                                         MDNS_TYPE_SRV, kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    VerifyOrReturnError(querySrv, CHIP_ERROR_NO_MEMORY);
    mdns_search_once_t * queryTxt = mdns_query_async_new(service->mName, service->mType, GetProtocolString(service->mProtocol),
                                                         MDNS_TYPE_TXT, kTimeoutMilli, kMaxResults, MdnsQueryNotifier);
    if (!queryTxt)
    {
        mdns_query_async_delete(querySrv);
        return CHIP_ERROR_NO_MEMORY;
    }
    ResolveContext * ctx = chip::Platform::New<ResolveContext>(service, interface, querySrv, queryTxt, callback, context);
    if (!ctx)
    {
        ChipLogError(DeviceLayer, "Failed to alloc memory for resolve context");
        mdns_query_async_delete(querySrv);
        mdns_query_async_delete(queryTxt);
        return CHIP_ERROR_NO_MEMORY;
    }
    error = AddQueryList(reinterpret_cast<GenericContext *>(ctx));
    if (error != CHIP_NO_ERROR)
    {
        chip::Platform::Delete(ctx);
    }
    return error;
}

} // namespace Dnssd
} // namespace chip
