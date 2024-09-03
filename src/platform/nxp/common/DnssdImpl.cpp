/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/FixedBufferAllocator.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/OpenThread/OpenThreadUtils.h>

#include <platform/nxp/common/ConnectivityManagerImpl.h>

#include <openthread/mdns.h>
#include <openthread/srp_server.h>

using namespace ::chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace Dnssd {

#define USE_MDNS_NEXT_SERVICE_API 1

// Support both operational and commissionable discovery, so buffers sizes must be worst case.
static constexpr uint8_t kMaxMdnsServiceTxtEntriesNumber =
    std::max(Dnssd::CommissionAdvertisingParameters::kTxtMaxNumber, Dnssd::OperationalAdvertisingParameters::kTxtMaxNumber);
static constexpr size_t kTotalMdnsServiceTxtValueSize = std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                 Dnssd::OperationalAdvertisingParameters::kTxtTotalValueSize);
static constexpr size_t kTotalMdnsServiceTxtKeySize =
    std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalKeySize, Dnssd::OperationalAdvertisingParameters::kTxtTotalKeySize);

static constexpr size_t kTotalMdnsServiceTxtBufferSize =
    kTotalMdnsServiceTxtKeySize + kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtValueSize;

// For each fabric we can register one _matter._tcp and one _matterc._udp service
static constexpr uint32_t kServiceListSize = CHIP_CONFIG_MAX_FABRICS * 2;

static const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp";
}

struct DnsServiceTxtEntries
{
    uint8_t mBuffer[kTotalMdnsServiceTxtBufferSize];
    Dnssd::TextEntry mTxtEntries[kMaxMdnsServiceTxtEntriesNumber];
};

struct mDnsQueryCtx
{
    void * matterCtx;
    chip::Dnssd::DnssdService mMdnsService;
    DnsServiceTxtEntries mServiceTxtEntry;
    char mServiceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1];
    CHIP_ERROR error;
    union
    {
        otMdnsBrowser mBrowseInfo;
        otMdnsSrvResolver mSrvInfo;
        otMdnsTxtResolver mTxtInfo;
        otMdnsAddressResolver mAddrInfo;
    };
    union
    {
        DnsBrowseCallback mDnsBrowseCallback;
        DnsResolveCallback mDnsResolveCallback;
    };

    mDnsQueryCtx(void * context, DnsBrowseCallback aBrowseCallback)
    {
        matterCtx          = context;
        mDnsBrowseCallback = aBrowseCallback;
        error              = CHIP_NO_ERROR;
    }
    mDnsQueryCtx(void * context, DnsResolveCallback aResolveCallback)
    {
        matterCtx           = context;
        mDnsResolveCallback = aResolveCallback;
        error               = CHIP_NO_ERROR;
    }
};

enum ResolveStep : uint8_t
{
    kResolveStepSrv = 0,
    kResolveStepTxt,
    kResolveStepIpAddr,
};

static const char * GetProtocolString(DnssdServiceProtocol protocol);

static void OtBrowseCallback(otInstance * aInstance, const otMdnsBrowseResult * aResult);
static void OtServiceCallback(otInstance * aInstance, const otMdnsSrvResult * aResult);
static void OtTxtCallback(otInstance * aInstance, const otMdnsTxtResult * aResult);
static void OtAddressCallback(otInstance * aInstance, const otMdnsAddressResult * aResult);

static void DispatchBrowseEmpty(intptr_t context);
static void DispatchBrowse(intptr_t context);

static void DispatchTxtResolve(intptr_t context);
static void DispatchAddressResolve(intptr_t context);
static void DispatchResolve(intptr_t context);
static void DispatchResolveError(intptr_t context);

static void HandleResolveCleanup(mDnsQueryCtx & resolveContext, ResolveStep stepType);

static CHIP_ERROR ResolveBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context, DnssdService * mdnsReq);
static CHIP_ERROR BrowseBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context);
static CHIP_ERROR FromSrpCacheToMdnsData(const otSrpServerService * service, const otSrpServerHost * host,
                                         const DnssdService * mdnsQueryReq, chip::Dnssd::DnssdService & mdnsService,
                                         DnsServiceTxtEntries & serviceTxtEntries);

static CHIP_ERROR FromServiceTypeToMdnsData(chip::Dnssd::DnssdService & mdnsService, const char * aServiceType);

static bool bBrowseInProgress = false;
// ID 0 is reserved for host
static uint32_t mRegisterServiceId = 1;
static uint8_t mNetifIndex         = 0;

// Matter currently only supports one browse and resolve operation at a time so there is no need to create a list
// If the Matter implementation evolves in the future this functionality can be extended to a list.
static mDnsQueryCtx * mBrowseContext  = nullptr;
static mDnsQueryCtx * mResolveContext = nullptr;

#if USE_MDNS_NEXT_SERVICE_API
static otMdnsService * mServiceList[kServiceListSize];
static uint32_t mServiceListFreeIndex;
#endif

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error            = CHIP_NO_ERROR;
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    struct netif * extNetif     = (ConnectivityManagerImpl().GetExternalInterface()).GetPlatformInterface();

    // Don't try to do anything until the mDNS server is started
    VerifyOrExit(otMdnsIsEnabled(thrInstancePtr), error = CHIP_ERROR_INCORRECT_STATE);

    mNetifIndex = netif_get_index(extNetif);

exit:
    initCallback(context, error);
    return error;
}

void ChipDnssdShutdown()
{
    otMdnsSetEnabled(ThreadStackMgrImpl().OTInstance(), false, 0);
}
#if USE_MDNS_NEXT_SERVICE_API
CHIP_ERROR ChipDnssdRemoveServices()
{
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    otMdnsService otServiceData = { 0 };
    otMdnsIterator * iterator   = nullptr;
    ChipError error             = CHIP_NO_ERROR;
    otError otError             = OT_ERROR_NONE;
    otMdnsEntryState state;

    const char * hostName = ConnectivityManagerImpl().GetHostName();

    iterator = otMdnsAllocateIterator(thrInstancePtr);
    VerifyOrExit(iterator != nullptr, error = CHIP_ERROR_NO_MEMORY);

    mServiceListFreeIndex = 0;

    while (mServiceListFreeIndex <= kServiceListSize)
    {
        // allocate memory for new entry if the entry is not allready allocated from previous iteration
        if (mServiceList[mServiceListFreeIndex] == nullptr)
        {
            mServiceList[mServiceListFreeIndex] = static_cast<otMdnsService *>(Platform::MemoryAlloc(sizeof(otMdnsService)));
            VerifyOrExit(mServiceList[mServiceListFreeIndex] != nullptr, error = CHIP_ERROR_NO_MEMORY);
        }

        otError = otMdnsGetNextService(thrInstancePtr, iterator, mServiceList[mServiceListFreeIndex], &state);
        if (otError == OT_ERROR_NOT_FOUND)
        {
            Platform::MemoryFree(mServiceList[mServiceListFreeIndex]);
            mServiceList[mServiceListFreeIndex] = nullptr;
            break;
        }

        if ((0 == strcmp(mServiceList[mServiceListFreeIndex]->mHostName, hostName)) &&
            ((0 == strcmp(mServiceList[mServiceListFreeIndex]->mServiceType, "_matter._tcp")) ||
             (0 == strcmp(mServiceList[mServiceListFreeIndex]->mServiceType, "_matterc._udp"))))
        {
            mServiceListFreeIndex++;
        }
    }

exit:
    if (iterator != nullptr)
    {
        otMdnsFreeIterator(thrInstancePtr, iterator);
    }
    return error;
}
#else
CHIP_ERROR ChipDnssdRemoveServices()
{
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    otMdnsService otServiceData = { 0 };

    otServiceData.mHostName = ConnectivityManagerImpl().GetHostName();

    otServiceData.mServiceType = "_matter._tcp";
    otMdnsUnregisterServiceType(thrInstancePtr, &otServiceData, OT_MDNS_SERVICE_MARK_FOR_UNREGISTER);
    otServiceData.mServiceType = "_matterc._udp";
    otMdnsUnregisterServiceType(thrInstancePtr, &otServiceData, OT_MDNS_SERVICE_MARK_FOR_UNREGISTER);

    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    ReturnErrorCodeIf(service == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    uint32_t txtBufferOffset    = 0;
    otError otErr               = OT_ERROR_NONE;
    otMdnsService otServiceData = { 0 };

#if USE_MDNS_NEXT_SERVICE_API
    bool bRegisterService = true;
#endif

    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1] = "";
    snprintf(serviceType, sizeof(serviceType), "%s.%s", service->mType, GetProtocolString(service->mProtocol));

    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtBufferSize] = { 0 };

    // Don't try to do anything until the mDNS server is started
    VerifyOrReturnValue(otMdnsIsEnabled(thrInstancePtr), CHIP_NO_ERROR);

    //  Create TXT Data as one string from multiple key entries
    for (uint32_t i = 0; i < service->mTextEntrySize; i++)
    {
        uint32_t keySize = strlen(service->mTextEntries[i].mKey);
        // add TXT entry len, + 1 is for '='
        *(txtBuffer + txtBufferOffset++) = keySize + service->mTextEntries[i].mDataSize + 1;

        // add TXT entry key
        memcpy(txtBuffer + txtBufferOffset, service->mTextEntries[i].mKey, keySize);
        txtBufferOffset += keySize;

        // add TXT entry value if pointer is not null, if pointer is null it means we have bool value
        if (service->mTextEntries[i].mData)
        {
            *(txtBuffer + txtBufferOffset++) = '=';
            memcpy(txtBuffer + txtBufferOffset, service->mTextEntries[i].mData, service->mTextEntries[i].mDataSize);
            txtBufferOffset += service->mTextEntries[i].mDataSize;
        }
    }

#if USE_MDNS_NEXT_SERVICE_API
    for (uint32_t i = 0; i < mServiceListFreeIndex; i++)
    {
        if ((0 == strcmp(mServiceList[i]->mHostName, service->mHostName)) &&
            (0 == strcmp(mServiceList[i]->mServiceInstance, service->mName)) &&
            (0 == strcmp(mServiceList[i]->mServiceType, serviceType)))
        {
            if ((mServiceList[i]->mTxtDataLength == txtBufferOffset) &&
                (0 == memcmp(txtBuffer, mServiceList[i]->mTxtData, txtBufferOffset)))
            {
                // In this case the service is
                bRegisterService = false;
            }
            Platform::MemoryFree(mServiceList[i]);
            if (i < --mServiceListFreeIndex)
            {
                // move last element in place of the removed one
                mServiceList[i]                     = mServiceList[mServiceListFreeIndex];
                mServiceList[mServiceListFreeIndex] = nullptr;
            }
            else
            {
                mServiceList[i] = nullptr;
            }
            break;
        }
    }
#endif
    if (bRegisterService)
    {
        if (strcmp(service->mHostName, "") != 0)
        {
            otServiceData.mHostName = service->mHostName;
        }

        otServiceData.mServiceInstance     = service->mName;
        otServiceData.mServiceType         = serviceType;
        otServiceData.mSubTypeLabels       = service->mSubTypes;
        otServiceData.mSubTypeLabelsLength = service->mSubTypeSize;
        otServiceData.mPort                = service->mPort;
        otServiceData.mTtl                 = service->mTtlSeconds;
        otServiceData.mTxtData             = txtBuffer;
        otServiceData.mTxtDataLength       = txtBufferOffset;

        otErr = otMdnsRegisterService(thrInstancePtr, &otServiceData, mRegisterServiceId++, NULL);
    }

    return MapOpenThreadError(otErr);
}

#if USE_MDNS_NEXT_SERVICE_API
CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();

    for (uint32_t i = 0; i < mServiceListFreeIndex; i++)
    {
        if (mServiceList[i] != nullptr)
        {
            otMdnsUnregisterService(thrInstancePtr, mServiceList[i]);
            Platform::MemoryFree(mServiceList[i]);
            mServiceList[i] = nullptr;
        }
    }

    mServiceListFreeIndex = 0;
    return CHIP_NO_ERROR;
}

#else
CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    otMdnsService otServiceData = { 0 };

    otServiceData.mHostName = ConnectivityManagerImpl().GetHostName();

    otServiceData.mServiceType = "_matter._tcp";
    otMdnsUnregisterServiceType(thrInstancePtr, &otServiceData, OT_MDNS_SERVICE_UNREGISTER_MARKED_SERVICE);
    otServiceData.mServiceType = "_matterc._udp";
    otMdnsUnregisterServiceType(thrInstancePtr, &otServiceData, OT_MDNS_SERVICE_UNREGISTER_MARKED_SERVICE);

    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                           Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context, intptr_t * browseIdentifier)
{
    *browseIdentifier           = reinterpret_cast<intptr_t>(nullptr);
    CHIP_ERROR error            = CHIP_NO_ERROR;
    CHIP_ERROR srpBrowseError   = CHIP_NO_ERROR;
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();

    if (type == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    mBrowseContext = Platform::New<mDnsQueryCtx>(context, callback);
    VerifyOrReturnError(mBrowseContext != nullptr, CHIP_ERROR_NO_MEMORY);

    // First try to browse the service in the SRP cache
    snprintf(mBrowseContext->mServiceType, sizeof(mBrowseContext->mServiceType), "%s.%s", type, GetProtocolString(protocol));
    // After browsing in the SRP cache we will continue with regular mDNS browse
    srpBrowseError = BrowseBySrp(thrInstancePtr, mBrowseContext->mServiceType, mBrowseContext);

    // Proceed to generate a mDNS query
    mBrowseContext->mBrowseInfo.mServiceType  = mBrowseContext->mServiceType;
    mBrowseContext->mBrowseInfo.mSubTypeLabel = nullptr;
    mBrowseContext->mBrowseInfo.mInfraIfIndex = mNetifIndex;
    mBrowseContext->mBrowseInfo.mCallback     = OtBrowseCallback;

    error = MapOpenThreadError(otMdnsStartBrowser(thrInstancePtr, &mBrowseContext->mBrowseInfo));

    if (CHIP_NO_ERROR == error)
    {
        bBrowseInProgress = true;
        *browseIdentifier = reinterpret_cast<intptr_t>(mBrowseContext);
    }
    else
    {
        if (srpBrowseError == CHIP_NO_ERROR)
        {
            // In this case, we need to send a final browse indication to signal the Matter App that there are no more
            // browse results coming
            mBrowseContext->error = error;
            DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(mBrowseContext));
        }
        else
        {
            Platform::Delete<mDnsQueryCtx>(mBrowseContext);
            mBrowseContext = nullptr;
        }
    }
    return error;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    mDnsQueryCtx * browseContext = reinterpret_cast<mDnsQueryCtx *>(browseIdentifier);
    otInstance * thrInstancePtr  = ThreadStackMgrImpl().OTInstance();
    otError error                = OT_ERROR_INVALID_ARGS;

    // browseContext is only valid when bBrowseInProgress is true. The Matter stack can call this function even with a browseContext
    // that has been freed in DispatchBrowseEmpty.
    if ((true == bBrowseInProgress) && (browseContext))
    {
        browseContext->error = MapOpenThreadError(otMdnsStopBrowser(thrInstancePtr, &browseContext->mBrowseInfo));

        // browse context will be freed in DispatchBrowseEmpty
        DispatchBrowseEmpty(reinterpret_cast<intptr_t>(browseContext));
    }
    return MapOpenThreadError(error);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
    ChipError error = CHIP_ERROR_NOT_FOUND;

    if (browseResult == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();

    mResolveContext = Platform::New<mDnsQueryCtx>(context, callback);
    VerifyOrReturnError(mResolveContext != nullptr, CHIP_ERROR_NO_MEMORY);

    // First try to find the service in the SRP cache, use default.service.arpa as domain name
    snprintf(mResolveContext->mServiceType, sizeof(mResolveContext->mServiceType), "%s.%s", browseResult->mType,
             GetProtocolString(browseResult->mProtocol));

    error = ResolveBySrp(thrInstancePtr, mResolveContext->mServiceType, mResolveContext, browseResult);
    // If the SRP cache returns not found, proceed to generate a MDNS query
    if (CHIP_ERROR_NOT_FOUND == error)
    {
        // The otMdnsSrvResolver structure contains only pointers to instance name and service type strings
        // Use the memory from mMdnsService.mName to store the instance name string we are looking for
        Platform::CopyString(mResolveContext->mMdnsService.mName, sizeof(mResolveContext->mMdnsService.mName), browseResult->mName);

        mResolveContext->mSrvInfo.mInfraIfIndex    = mNetifIndex;
        mResolveContext->mSrvInfo.mCallback        = OtServiceCallback;
        mResolveContext->mSrvInfo.mServiceInstance = mResolveContext->mMdnsService.mName;
        mResolveContext->mSrvInfo.mServiceType     = mResolveContext->mServiceType;

        return MapOpenThreadError(otMdnsStartSrvResolver(thrInstancePtr, &mResolveContext->mSrvInfo));
    }
    else
    {
        return error;
    }
}
void ChipDnssdResolveNoLongerNeeded(const char * instanceName)
{
    if (mResolveContext != nullptr)
    {
        if (strcmp(instanceName, mResolveContext->mMdnsService.mName) == 0)
        {
            otMdnsStopSrvResolver(ThreadStackMgrImpl().OTInstance(), &mResolveContext->mSrvInfo);

            Platform::Delete<mDnsQueryCtx>(mResolveContext);
            mResolveContext = nullptr;
        }
    }
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BrowseBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context)
{
    const otSrpServerHost * host       = nullptr;
    const otSrpServerService * service = nullptr;
    CHIP_ERROR error                   = CHIP_ERROR_NOT_FOUND;

    while ((host = otSrpServerGetNextHost(thrInstancePtr, host)) != nullptr)
    {
        while ((service = otSrpServerHostGetNextService(host, service)) != nullptr)
        {
            if ((false == otSrpServerServiceIsDeleted(service)) &&
                (0 == strncmp(otSrpServerServiceGetServiceName(service), serviceName, strlen(serviceName))))
            {
                mDnsQueryCtx * serviceContext = Platform::New<mDnsQueryCtx>(context->matterCtx, context->mDnsBrowseCallback);
                if (serviceContext != nullptr)
                {
                    if (CHIP_NO_ERROR ==
                        FromSrpCacheToMdnsData(service, host, nullptr, serviceContext->mMdnsService,
                                               serviceContext->mServiceTxtEntry))
                    {
                        // Set error to CHIP_NO_ERROR to signal that there was at least one service found in the cache
                        error = CHIP_NO_ERROR;
                        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(serviceContext));
                    }
                    else
                    {
                        Platform::Delete<mDnsQueryCtx>(serviceContext);
                    }
                }
            }
        }
    }
    return error;
}

CHIP_ERROR ResolveBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context, DnssdService * mdnsReq)
{
    const otSrpServerHost * host       = nullptr;
    const otSrpServerService * service = nullptr;
    CHIP_ERROR error                   = CHIP_ERROR_NOT_FOUND;

    while ((host = otSrpServerGetNextHost(thrInstancePtr, host)) != nullptr)
    {
        while ((service = otSrpServerHostGetNextService(host, service)) != nullptr)
        {
            if ((false == otSrpServerServiceIsDeleted(service)) &&
                (0 == strncmp(otSrpServerServiceGetServiceName(service), serviceName, strlen(serviceName))) &&
                (0 == strncmp(otSrpServerServiceGetInstanceName(service), mdnsReq->mName, strlen(mdnsReq->mName))))
            {
                error = FromSrpCacheToMdnsData(service, host, mdnsReq, context->mMdnsService, context->mServiceTxtEntry);
                if (error == CHIP_NO_ERROR)
                {
                    DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(context));
                }
                break;
            }
        }

        if (error == CHIP_NO_ERROR)
        {
            break;
        }
    }

    return error;
}

CHIP_ERROR FromSrpCacheToMdnsData(const otSrpServerService * service, const otSrpServerHost * host,
                                  const DnssdService * mdnsQueryReq, chip::Dnssd::DnssdService & mdnsService,
                                  DnsServiceTxtEntries & serviceTxtEntries)
{
    const char * tmpName;
    const uint8_t * txtStringPtr;
    size_t substringSize;
    uint8_t addrNum = 0;
    uint16_t txtDataLen;
    const otIp6Address * ip6AddrPtr = otSrpServerHostGetAddresses(host, &addrNum);

    if (mdnsQueryReq != nullptr)
    {
        Platform::CopyString(mdnsService.mName, sizeof(mdnsService.mName), mdnsQueryReq->mName);
        Platform::CopyString(mdnsService.mType, sizeof(mdnsService.mType), mdnsQueryReq->mType);
        mdnsService.mProtocol = mdnsQueryReq->mProtocol;
    }
    else
    {
        tmpName = otSrpServerServiceGetInstanceName(service);
        // Extract from the <instance>.<type>.<protocol>.<domain-name>. the <instance> part
        size_t substringSize = strchr(tmpName, '.') - tmpName;
        if (substringSize >= ArraySize(mdnsService.mName))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Platform::CopyString(mdnsService.mName, substringSize + 1, tmpName);

        // Extract from the <instance>.<type>.<protocol>.<domain-name>. the <type> part.
        tmpName       = tmpName + substringSize + 1;
        substringSize = strchr(tmpName, '.') - tmpName;
        if (substringSize >= ArraySize(mdnsService.mType))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Platform::CopyString(mdnsService.mType, substringSize + 1, tmpName);

        // Extract from the <instance>.<type>.<protocol>.<domain-name>. the <type> part.
        tmpName       = tmpName + substringSize + 1;
        substringSize = strchr(tmpName, '.') - tmpName;
        if (substringSize >= (chip::Dnssd::kDnssdProtocolTextMaxSize + 1))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (strncmp(tmpName, "_udp", substringSize) == 0)
        {
            mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUdp;
        }
        else if (strncmp(tmpName, "_tcp", substringSize) == 0)
        {
            mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolTcp;
        }
        else
        {
            mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUnknown;
        }
    }

    // Extract from the <hostname>.<domain-name>. the <hostname> part.
    tmpName       = otSrpServerHostGetFullName(host);
    substringSize = strchr(tmpName, '.') - tmpName;
    if (substringSize >= ArraySize(mdnsService.mHostName))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(mdnsService.mHostName, substringSize + 1, tmpName);
    mdnsService.mPort = otSrpServerServiceGetPort(service);

    // All SRP cache hits come from the Thread Netif
    mdnsService.mInterface = ConnectivityManagerImpl().GetThreadInterface();

    mdnsService.mAddressType = Inet::IPAddressType::kIPv6;
    mdnsService.mAddress     = std::optional(ToIPAddress(*ip6AddrPtr));

    // Extract TXT record SRP service
    txtStringPtr = otSrpServerServiceGetTxtData(service, &txtDataLen);
    if (txtDataLen != 0)
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, txtStringPtr, txtDataLen);

        otDnsTxtEntry txtEntry;
        chip::FixedBufferAllocator alloc(serviceTxtEntries.mBuffer);

        uint8_t entryIndex = 0;
        while ((otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE) && entryIndex < 64)
        {
            if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
                continue;

            serviceTxtEntries.mTxtEntries[entryIndex].mKey      = alloc.Clone(txtEntry.mKey);
            serviceTxtEntries.mTxtEntries[entryIndex].mData     = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
            serviceTxtEntries.mTxtEntries[entryIndex].mDataSize = txtEntry.mValueLength;
            entryIndex++;
        }

        ReturnErrorCodeIf(alloc.AnyAllocFailed(), CHIP_ERROR_BUFFER_TOO_SMALL);

        mdnsService.mTextEntries   = serviceTxtEntries.mTxtEntries;
        mdnsService.mTextEntrySize = entryIndex;
    }
    else
    {
        mdnsService.mTextEntrySize = 0;
    }

    mdnsService.mSubTypes    = nullptr;
    mdnsService.mSubTypeSize = 0;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR FromServiceTypeToMdnsData(chip::Dnssd::DnssdService & mdnsService, const char * aServiceType)
{
    char protocol[chip::Dnssd::kDnssdProtocolTextMaxSize + 1];
    const char * protocolSubstringStart;
    size_t substringSize;

    // Extract from the <type>.<protocol> the <type> part.
    substringSize = strchr(aServiceType, '.') - aServiceType;
    if (substringSize >= ArraySize(mdnsService.mType))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(mdnsService.mType, ArraySize(mdnsService.mType), aServiceType);

    // Extract from the <type>.<protocol>. the .<protocol> part.
    protocolSubstringStart = aServiceType + substringSize;

    // Check that the protocolSubstringStart starts wit a '.' to be sure we are in the right place
    if (strchr(protocolSubstringStart, '.') == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Jump over '.' in protocolSubstringStart and substract the string terminator from the size
    substringSize = strlen(++protocolSubstringStart) - 1;
    if (substringSize >= ArraySize(protocol))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(protocol, ArraySize(protocol), protocolSubstringStart);

    if (strncmp(protocol, "_udp", chip::Dnssd::kDnssdProtocolTextMaxSize) == 0)
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUdp;
    }
    else if (strncmp(protocol, "_tcp", chip::Dnssd::kDnssdProtocolTextMaxSize) == 0)
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolTcp;
    }
    else
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUnknown;
    }

    return CHIP_NO_ERROR;
}

static void OtBrowseCallback(otInstance * aInstance, const otMdnsBrowseResult * aResult)
{
    CHIP_ERROR error;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    mDnsQueryCtx * tmpContext = Platform::New<mDnsQueryCtx>(mBrowseContext->matterCtx, mBrowseContext->mDnsBrowseCallback);
    VerifyOrReturn(tmpContext != nullptr);

    Platform::CopyString(tmpContext->mMdnsService.mName, sizeof(tmpContext->mMdnsService.mName), aResult->mServiceInstance);
    error = FromServiceTypeToMdnsData(tmpContext->mMdnsService, aResult->mServiceType);

    if (CHIP_NO_ERROR == error)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(tmpContext));
    }
    else
    {
        Platform::Delete<mDnsQueryCtx>(tmpContext);
    }
}

static void OtServiceCallback(otInstance * aInstance, const otMdnsSrvResult * aResult)
{
    CHIP_ERROR error;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    VerifyOrReturn(mResolveContext != nullptr);

    error                  = FromServiceTypeToMdnsData(mResolveContext->mMdnsService, aResult->mServiceType);
    mResolveContext->error = error;

    if (CHIP_NO_ERROR == error)
    {
        Platform::CopyString(mResolveContext->mMdnsService.mName, sizeof(mResolveContext->mMdnsService.mName),
                             aResult->mServiceInstance);
        Platform::CopyString(mResolveContext->mMdnsService.mHostName, sizeof(mResolveContext->mMdnsService.mHostName),
                             aResult->mHostName);

        mResolveContext->mMdnsService.mPort       = aResult->mPort;
        mResolveContext->mMdnsService.mTtlSeconds = aResult->mTtl;
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchTxtResolve, reinterpret_cast<intptr_t>(mResolveContext));
    }
    else
    {
        HandleResolveCleanup(*mResolveContext, kResolveStepSrv);
    }
}

static void OtTxtCallback(otInstance * aInstance, const otMdnsTxtResult * aResult)
{
    bool bSendDispatch = true;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    VerifyOrReturn(mResolveContext != nullptr);

    // Check if TXT record was included in the response.
    if (aResult->mTxtDataLength != 0)
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, aResult->mTxtData, aResult->mTxtDataLength);

        otDnsTxtEntry txtEntry;
        chip::FixedBufferAllocator alloc(mResolveContext->mServiceTxtEntry.mBuffer);

        uint8_t entryIndex = 0;
        while ((otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE) && entryIndex < 64)
        {
            if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
                continue;

            mResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mKey  = alloc.Clone(txtEntry.mKey);
            mResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mData = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
            mResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mDataSize = txtEntry.mValueLength;
            entryIndex++;
        }

        if (alloc.AnyAllocFailed())
        {
            bSendDispatch = false;
        }
        else
        {
            mResolveContext->mMdnsService.mTextEntries   = mResolveContext->mServiceTxtEntry.mTxtEntries;
            mResolveContext->mMdnsService.mTextEntrySize = entryIndex;
        }
    }
    else
    {
        mResolveContext->mMdnsService.mTextEntrySize = 0;
    }

    if (bSendDispatch)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchAddressResolve, reinterpret_cast<intptr_t>(mResolveContext));
    }
    else
    {
        HandleResolveCleanup(*mResolveContext, kResolveStepTxt);
    }
}

static void OtAddressCallback(otInstance * aInstance, const otMdnsAddressResult * aResult)
{
    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn((aResult->mAddressesLength > 0) && (aResult->mAddresses[0].mTtl > 0));

    VerifyOrReturn(mResolveContext != nullptr);

    mResolveContext->mMdnsService.mAddressType = Inet::IPAddressType::kIPv6;
    mResolveContext->mMdnsService.mAddress     = std::optional(ToIPAddress(aResult->mAddresses[0].mAddress));

    DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(mResolveContext));
}

static void DispatchBrowseEmpty(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    browseContext->mDnsBrowseCallback(browseContext->matterCtx, nullptr, 0, true, browseContext->error);
    Platform::Delete<mDnsQueryCtx>(browseContext);
    mBrowseContext    = nullptr;
    bBrowseInProgress = false;
}

static void DispatchBrowse(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    browseContext->mDnsBrowseCallback(browseContext->matterCtx, &browseContext->mMdnsService, 1, false, browseContext->error);
    Platform::Delete<mDnsQueryCtx>(browseContext);
}

static void DispatchTxtResolve(intptr_t context)
{
    mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    otError error;

    // Stop SRV resolver before starting TXT one, ignore error as it will only happen if mMDS module is not initialized
    otMdnsStopSrvResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext->mSrvInfo);

    resolveContext->mTxtInfo.mServiceInstance = resolveContext->mMdnsService.mName;
    resolveContext->mTxtInfo.mServiceType     = resolveContext->mServiceType;
    resolveContext->mTxtInfo.mCallback        = OtTxtCallback;
    resolveContext->mTxtInfo.mInfraIfIndex    = mNetifIndex;

    error = otMdnsStartTxtResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext->mTxtInfo);
    if (error != OT_ERROR_NONE)
    {
        resolveContext->error = MapOpenThreadError(error);
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveError, reinterpret_cast<intptr_t>(resolveContext));
    }
}

static void DispatchAddressResolve(intptr_t context)
{
    otError error;
    mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    // Stop TXT resolver before starting address one, ignore error as it will only happen if mMDS module is not initialized
    otMdnsStopTxtResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext->mTxtInfo);

    resolveContext->mAddrInfo.mCallback     = OtAddressCallback;
    resolveContext->mAddrInfo.mHostName     = resolveContext->mMdnsService.mHostName;
    resolveContext->mAddrInfo.mInfraIfIndex = mNetifIndex;

    error = otMdnsStartIp6AddressResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext->mAddrInfo);
    if (error != OT_ERROR_NONE)
    {
        resolveContext->error = MapOpenThreadError(error);
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveError, reinterpret_cast<intptr_t>(resolveContext));
    }
}

static void DispatchResolve(intptr_t context)
{
    mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    Dnssd::DnssdService & service = resolveContext->mMdnsService;
    Span<Inet::IPAddress> ipAddrs;

    // Stop Address resolver, we have finished resolving the service
    otMdnsStopIp6AddressResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext->mAddrInfo);

    if (service.mAddress.has_value())
    {
        ipAddrs = Span<Inet::IPAddress>(&*service.mAddress, 1);
    }

    // Signal that the context will be freed and that the resolve operation is stopped because Matter will
    // try do stop it again on the mDnsResolveCallback
    mResolveContext = nullptr;

    resolveContext->mDnsResolveCallback(resolveContext->matterCtx, &service, ipAddrs, resolveContext->error);
    Platform::Delete<mDnsQueryCtx>(resolveContext);
}

static void DispatchResolveError(intptr_t context)
{
    mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    Span<Inet::IPAddress> ipAddrs;

    // Signal that the context will be freed and that the resolve operation is stopped because Matter will
    // try do stop it again on the mDnsResolveCallback
    mResolveContext = nullptr;

    resolveContext->mDnsResolveCallback(resolveContext->matterCtx, nullptr, ipAddrs, resolveContext->error);
    Platform::Delete<mDnsQueryCtx>(resolveContext);
}

void HandleResolveCleanup(mDnsQueryCtx & resolveContext, ResolveStep stepType)
{
    switch (stepType)
    {
    case kResolveStepSrv:
        otMdnsStopSrvResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext.mSrvInfo);
        break;
    case kResolveStepTxt:
        otMdnsStopTxtResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext.mTxtInfo);
        break;
    case kResolveStepIpAddr:
        otMdnsStopIp6AddressResolver(ThreadStackMgrImpl().OTInstance(), &resolveContext.mAddrInfo);
        break;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(&resolveContext));
}

} // namespace Dnssd
} // namespace chip
