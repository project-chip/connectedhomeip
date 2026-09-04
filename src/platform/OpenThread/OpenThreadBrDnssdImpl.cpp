/*
 *
 *    Copyright (c) 2024, 2026 Project CHIP Authors
 *    Copyright 2024, 2026 NXP
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "lib/dnssd/platform/Dnssd.h"
#include <inet/InetInterface.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/FixedBufferAllocator.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/OpenThread/OpenThreadUtils.h>

#include <openthread/mdns.h>
#include <openthread/srp_server.h>

#include <platform/OpenThread/OpenThreadBrDnssdImpl.h>

using namespace ::chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace Dnssd {

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

enum ResolveStep : uint8_t
{
    kResolveStepSrv = 0,
    kResolveStepTxt,
    kResolveStepIpAddr,
};

enum NameType : uint8_t
{
    kNameTypeInstance = 0,
    kNameTypeHost,
    kNameTypeService,
};

struct DnsServiceTxtEntries
{
    uint8_t mBuffer[kTotalMdnsServiceTxtBufferSize];
    Dnssd::TextEntry mTxtEntries[kMaxMdnsServiceTxtEntriesNumber];
};

struct mDnsQueryCtx : public IntrusiveListNodeBase<>
{
    void * mMatterCtx;
    chip::Dnssd::DnssdService mMdnsService;
    DnsServiceTxtEntries mServiceTxtEntry;
    char mServiceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1];
    Inet::IPAddress mResolvedAddresses[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES];
    uint8_t mResolvedAddressCount;
    ResolveStep mResolveStep;
    CHIP_ERROR mError;
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
        mMatterCtx            = context;
        mDnsBrowseCallback    = aBrowseCallback;
        mResolvedAddressCount = 0;
        mResolveStep          = kResolveStepSrv;
        mError                = CHIP_NO_ERROR;
    }
    mDnsQueryCtx(void * context, DnsResolveCallback aResolveCallback)
    {
        mMatterCtx            = context;
        mDnsResolveCallback   = aResolveCallback;
        mResolvedAddressCount = 0;
        mResolveStep          = kResolveStepSrv;
        mError                = CHIP_NO_ERROR;
    }
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
static void DispatchResolveSrp(intptr_t context);
static void DispatchResolveError(intptr_t context);

static void HandleResolveCleanup(mDnsQueryCtx & resolveContext, ResolveStep stepType, bool dispatchResolveError);

static mDnsQueryCtx * GetResolveElement(const char * instanceName, NameType aType);
static bool IsInResolveList(const mDnsQueryCtx * pResolveContext);

static CHIP_ERROR ResolveBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context, DnssdService * mdnsReq);
static CHIP_ERROR BrowseBySrp(otInstance * thrInstancePtr, char * serviceName, mDnsQueryCtx * context);
static CHIP_ERROR FromSrpCacheToMdnsData(const otSrpServerService * service, const otSrpServerHost * host,
                                         const DnssdService * mdnsQueryReq, chip::Dnssd::DnssdService & mdnsService,
                                         DnsServiceTxtEntries & serviceTxtEntries, Inet::IPAddress * resolvedAddresses,
                                         uint8_t & resolvedAddressCount);

static CHIP_ERROR FromServiceTypeToMdnsData(chip::Dnssd::DnssdService & mdnsService, const char * aServiceType);

// ID 0 is reserved for host
static uint32_t mRegisterServiceId = 1;
static uint32_t mNetifIndex        = 0;

namespace {

otInstance * mOtInstance = nullptr;
Inet::InterfaceId mExternalNetIf;
Inet::InterfaceId mThreadNetIf;

// IntrusiveList requires an empty list at destruction time. OpenThreadBrDnssdShutdown()
// empties both lists, so static storage duration is safe here.
IntrusiveList<mDnsQueryCtx> mResolveList;
IntrusiveList<mDnsQueryCtx> mBrowseList;

} // namespace

static otMdnsService * mServiceList[kServiceListSize] = {};
static uint32_t mServiceListFreeIndex;

CHIP_ERROR OpenThreadBrDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    mOtInstance = ThreadStackMgrImpl().OTInstance();

    mExternalNetIf = ConnectivityMgr().GetExternalInterface();
    mThreadNetIf   = ConnectivityMgr().GetThreadInterface();

    if (mExternalNetIf == Inet::InterfaceId::Null() || mThreadNetIf == Inet::InterfaceId::Null())
    {
        Inet::InterfaceIterator iter;
        while (iter.HasCurrent())
        {
            Inet::InterfaceType ifType;
            if (iter.IsUp() && iter.GetInterfaceType(ifType) == CHIP_NO_ERROR)
            {
                if (mExternalNetIf == Inet::InterfaceId::Null() &&
                    (ifType == Inet::InterfaceType::Ethernet || ifType == Inet::InterfaceType::WiFi))
                {
                    mExternalNetIf = iter.GetInterfaceId();
                }
                else if (mThreadNetIf == Inet::InterfaceId::Null() && ifType == Inet::InterfaceType::Thread)
                {
                    mThreadNetIf = iter.GetInterfaceId();
                }
            }

            if (mExternalNetIf != Inet::InterfaceId::Null() && mThreadNetIf != Inet::InterfaceId::Null())
            {
                break;
            }
            iter.Next();
        }
    }

    VerifyOrReturnError(mExternalNetIf != Inet::InterfaceId::Null(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "External interface not set, DNS-SD init failed"));

    if (mThreadNetIf == Inet::InterfaceId::Null())
    {
        ChipLogError(DeviceLayer, "Thread interface not set; SRP cache results will report a null interface");
    }

    mNetifIndex = mExternalNetIf.GetInterfaceIndex();

    // The mDNS module is ready
    if (otMdnsIsEnabled(mOtInstance))
    {
        initCallback(context, CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }
    else
    {
        errorCallback(context, CHIP_ERROR_INCORRECT_STATE);
    }

    return CHIP_ERROR_INCORRECT_STATE;
}

void OpenThreadBrDnssdShutdown()
{
    while (!mBrowseList.Empty())
    {
        mDnsQueryCtx * pQueryContext = &(*mBrowseList.begin());
        otMdnsStopBrowser(mOtInstance, &pQueryContext->mBrowseInfo);
        mBrowseList.Remove(pQueryContext);
        Platform::Delete<mDnsQueryCtx>(pQueryContext);
    }

    // Stop all resolve operations and clean the resolve list
    while (!mResolveList.Empty())
    {
        mDnsQueryCtx * pQueryContext = &(*mResolveList.begin());
        HandleResolveCleanup(*pQueryContext, pQueryContext->mResolveStep, /* dispatchResolveError */ false);
        mResolveList.Remove(pQueryContext);
        Platform::Delete<mDnsQueryCtx>(pQueryContext);
    }

    mExternalNetIf = Inet::InterfaceId::Null();
    mThreadNetIf   = Inet::InterfaceId::Null();
    mNetifIndex    = 0;
}

CHIP_ERROR OpenThreadBrDnssdRemoveServices()
{
    otMdnsIterator * iterator = nullptr;
    ChipError error           = CHIP_NO_ERROR;
    otError otErr             = OT_ERROR_NONE;
    otMdnsEntryState state;

    const char * hostName = otMdnsGetLocalHostName(mOtInstance);

    iterator = otMdnsAllocateIterator(mOtInstance);
    VerifyOrExit(iterator != nullptr, error = CHIP_ERROR_NO_MEMORY);

    mServiceListFreeIndex = 0;

    while (mServiceListFreeIndex < kServiceListSize)
    {
        // allocate memory for new entry if the entry is not allready allocated from previous iteration
        if (mServiceList[mServiceListFreeIndex] == nullptr)
        {
            mServiceList[mServiceListFreeIndex] = static_cast<otMdnsService *>(Platform::MemoryAlloc(sizeof(otMdnsService)));
            VerifyOrExit(mServiceList[mServiceListFreeIndex] != nullptr, error = CHIP_ERROR_NO_MEMORY);
        }

        otErr = otMdnsGetNextService(mOtInstance, iterator, mServiceList[mServiceListFreeIndex], &state);
        if (otErr == OT_ERROR_NOT_FOUND)
        {
            Platform::MemoryFree(mServiceList[mServiceListFreeIndex]);
            mServiceList[mServiceListFreeIndex] = nullptr;
            break;
        }

        if ((0 == strcmp(mServiceList[mServiceListFreeIndex]->mHostName, hostName)) &&
            ((0 == strcmp(mServiceList[mServiceListFreeIndex]->mServiceType, "_matter._tcp")) ||
             (0 == strcmp(mServiceList[mServiceListFreeIndex]->mServiceType, "_matterc._udp"))) &&
            (mServiceList[mServiceListFreeIndex]->mTtl > 0))
        // if mTtl = 0 , the service is already in the removal process but not yet removed
        {
            mServiceListFreeIndex++;
        }
    }

exit:
    if (iterator != nullptr)
    {
        otMdnsFreeIterator(mOtInstance, iterator);
    }
    return error;
}

CHIP_ERROR OpenThreadBrDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t txtBufferOffset    = 0;
    otError otErr               = OT_ERROR_NONE;
    otMdnsService otServiceData = { 0 };
    bool bRegisterService       = true;

    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1] = "";
    snprintf(serviceType, sizeof(serviceType), "%s.%s", service->mType, GetProtocolString(service->mProtocol));

    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtBufferSize] = { 0 };

    // Don't try to do anything until the mDNS server is started
    VerifyOrReturnValue(otMdnsIsEnabled(mOtInstance), CHIP_NO_ERROR);

    //  Create TXT Data as one string from multiple key entries
    for (uint32_t i = 0; i < service->mTextEntrySize; i++)
    {
        uint32_t keySize = strlen(service->mTextEntries[i].mKey);
        // add TXT entry len, + 1 is for '='
        *(txtBuffer + txtBufferOffset++) = static_cast<uint8_t>(keySize + service->mTextEntries[i].mDataSize + 1);

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

    if (bRegisterService)
    {
        if (strcmp(service->mHostName, "") != 0)
        {
            otServiceData.mHostName = service->mHostName;
        }

        VerifyOrReturnError(service->mSubTypeSize <= UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);

        otServiceData.mServiceInstance     = service->mName;
        otServiceData.mServiceType         = serviceType;
        otServiceData.mSubTypeLabels       = service->mSubTypes;
        otServiceData.mSubTypeLabelsLength = static_cast<uint16_t>(service->mSubTypeSize);
        otServiceData.mPort                = service->mPort;
        otServiceData.mTtl                 = service->mTtlSeconds;
        otServiceData.mTxtData             = txtBuffer;
        otServiceData.mTxtDataLength       = static_cast<uint16_t>(txtBufferOffset);

        otErr = otMdnsRegisterService(mOtInstance, &otServiceData, mRegisterServiceId++, NULL);
    }

    return MapOpenThreadError(otErr);
}

CHIP_ERROR OpenThreadBrDnssdFinalizeServiceUpdate()
{
    for (uint32_t i = 0; i < mServiceListFreeIndex; i++)
    {
        if (mServiceList[i] != nullptr)
        {
            otMdnsUnregisterService(mOtInstance, mServiceList[i]);
            Platform::MemoryFree(mServiceList[i]);
            mServiceList[i] = nullptr;
        }
    }

    mServiceListFreeIndex = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenThreadBrDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                                   Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                                   intptr_t * browseIdentifier)
{
    *browseIdentifier             = reinterpret_cast<intptr_t>(nullptr);
    CHIP_ERROR error              = CHIP_NO_ERROR;
    CHIP_ERROR srpBrowseError     = CHIP_NO_ERROR;
    mDnsQueryCtx * pBrowseContext = nullptr;
    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + 1];

    if (type == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    snprintf(serviceType, sizeof(serviceType), "%s.%s", type, GetProtocolString(protocol));
    pBrowseContext = reinterpret_cast<mDnsQueryCtx *>(GetResolveElement(serviceType, kNameTypeService));
    if (pBrowseContext != nullptr)
    {
        // In case there is an ongoing query let it continue using OT's mDNS management
        return CHIP_NO_ERROR;
    }

    pBrowseContext = Platform::New<mDnsQueryCtx>(context, callback);
    VerifyOrReturnError(pBrowseContext != nullptr, CHIP_ERROR_NO_MEMORY);

    Platform::CopyString(pBrowseContext->mServiceType, sizeof(pBrowseContext->mServiceType), serviceType);

    // First try to browse the service in the SRP cache
    // After browsing in the SRP cache we will continue with regular mDNS browse
    srpBrowseError = BrowseBySrp(mOtInstance, pBrowseContext->mServiceType, pBrowseContext);

    // Proceed to generate a mDNS query
    pBrowseContext->mBrowseInfo.mServiceType  = pBrowseContext->mServiceType;
    pBrowseContext->mBrowseInfo.mSubTypeLabel = nullptr;
    pBrowseContext->mBrowseInfo.mInfraIfIndex = mNetifIndex;
    pBrowseContext->mBrowseInfo.mCallback     = OtBrowseCallback;

    mBrowseList.PushBack(pBrowseContext);

    error = MapOpenThreadError(otMdnsStartBrowser(mOtInstance, &pBrowseContext->mBrowseInfo));

    if (CHIP_NO_ERROR == error)
    {
        *browseIdentifier = reinterpret_cast<intptr_t>(pBrowseContext);
    }
    else
    {
        mBrowseList.Remove(pBrowseContext);
        if (srpBrowseError == CHIP_NO_ERROR)
        {
            // In this case, we need to send a final browse indication to signal the Matter App that there are no more
            // browse results coming but the result is no error since we have a match in the SRP cache.
            error                  = CHIP_NO_ERROR;
            pBrowseContext->mError = CHIP_NO_ERROR;
            if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(pBrowseContext)) !=
                CHIP_NO_ERROR)
            {
                Platform::Delete<mDnsQueryCtx>(pBrowseContext);
                pBrowseContext = nullptr;
                error          = CHIP_ERROR_NO_MEMORY;
            }
        }
        else
        {
            Platform::Delete<mDnsQueryCtx>(pBrowseContext);
            pBrowseContext = nullptr;
        }
    }
    return error;
}

CHIP_ERROR OpenThreadBrDnssdStopBrowse(intptr_t browseIdentifier)
{
    mDnsQueryCtx * pBrowseContext = reinterpret_cast<mDnsQueryCtx *>(browseIdentifier);
    otError error                 = OT_ERROR_INVALID_ARGS;

    // The Matter stack can call this function even with a browseContext that has been
    // freed in DispatchBrowseEmpty. If the browseContext is successfully freed from the
    // browse list then we consider it valid and proceed to stop the mDNS browse operation.
    bool found = false;
    for (auto & ctx : mBrowseList)
    {
        if (&ctx == pBrowseContext)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        error                  = otMdnsStopBrowser(mOtInstance, &pBrowseContext->mBrowseInfo);
        pBrowseContext->mError = MapOpenThreadError(error);
        mBrowseList.Remove(pBrowseContext);

        // browse context will be freed in DispatchBrowseEmpty
        DispatchBrowseEmpty(reinterpret_cast<intptr_t>(pBrowseContext));
    }
    return MapOpenThreadError(error);
}

CHIP_ERROR OpenThreadBrDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback,
                                    void * context)
{
    ChipError error                = CHIP_ERROR_NOT_FOUND;
    mDnsQueryCtx * pResolveContext = nullptr;

    if (browseResult == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    pResolveContext = reinterpret_cast<mDnsQueryCtx *>(GetResolveElement(browseResult->mName, kNameTypeInstance));
    if (pResolveContext != nullptr)
    {
        // In case there is an ongoing query let it continue using OT's mDNS management
        return CHIP_NO_ERROR;
    }

    pResolveContext = Platform::New<mDnsQueryCtx>(context, callback);
    VerifyOrReturnError(pResolveContext != nullptr, CHIP_ERROR_NO_MEMORY);

    // First try to find the service in the SRP cache, use default.service.arpa as domain name
    snprintf(pResolveContext->mServiceType, sizeof(pResolveContext->mServiceType), "%s.%s", browseResult->mType,
             GetProtocolString(browseResult->mProtocol));

    error = ResolveBySrp(mOtInstance, pResolveContext->mServiceType, pResolveContext, browseResult);
    // If the SRP cache returns not found, proceed to generate a MDNS query
    if (CHIP_ERROR_NOT_FOUND == error)
    {
        // The otMdnsSrvResolver structure contains only pointers to instance name and service type strings
        // Use the memory from mMdnsService.mName to store the instance name string we are looking for
        Platform::CopyString(pResolveContext->mMdnsService.mName, sizeof(pResolveContext->mMdnsService.mName), browseResult->mName);

        pResolveContext->mSrvInfo.mInfraIfIndex    = mNetifIndex;
        pResolveContext->mSrvInfo.mCallback        = OtServiceCallback;
        pResolveContext->mSrvInfo.mServiceInstance = pResolveContext->mMdnsService.mName;
        pResolveContext->mSrvInfo.mServiceType     = pResolveContext->mServiceType;

        mResolveList.PushBack(pResolveContext);

        error = MapOpenThreadError(otMdnsStartSrvResolver(mOtInstance, &pResolveContext->mSrvInfo));
    }

    if (error != CHIP_NO_ERROR)
    {
        mResolveList.Remove(pResolveContext);
        Platform::Delete<mDnsQueryCtx>(pResolveContext);
    }

    return error;
}
void OpenThreadBrDnssdResolveNoLongerNeeded(const char * instanceName)
{
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(GetResolveElement(instanceName, kNameTypeInstance));
    if (pResolveContext != nullptr)
    {
        if (strcmp(instanceName, pResolveContext->mMdnsService.mName) == 0)
        {
            HandleResolveCleanup(*pResolveContext, pResolveContext->mResolveStep, /* dispatchResolveError */ false);

            mResolveList.Remove(pResolveContext);
            Platform::Delete<mDnsQueryCtx>(pResolveContext);
        }
    }
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
                mDnsQueryCtx * serviceContext = Platform::New<mDnsQueryCtx>(context->mMatterCtx, context->mDnsBrowseCallback);
                if (serviceContext != nullptr)
                {
                    if (CHIP_NO_ERROR ==
                        FromSrpCacheToMdnsData(service, host, nullptr, serviceContext->mMdnsService,
                                               serviceContext->mServiceTxtEntry, serviceContext->mResolvedAddresses,
                                               serviceContext->mResolvedAddressCount))
                    {
                        // Set error to CHIP_NO_ERROR to signal that there was at least one service found in the cache
                        error = CHIP_NO_ERROR;
                        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(serviceContext)) !=
                            CHIP_NO_ERROR)
                        {
                            Platform::Delete<mDnsQueryCtx>(serviceContext);
                        }
                    }
                    else
                    {
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
                error = FromSrpCacheToMdnsData(service, host, mdnsReq, context->mMdnsService, context->mServiceTxtEntry,
                                               context->mResolvedAddresses, context->mResolvedAddressCount);
                if (error == CHIP_NO_ERROR)
                {
                    if (context->mResolvedAddressCount > 0)
                    {
                        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveSrp, reinterpret_cast<intptr_t>(context)) !=
                            CHIP_NO_ERROR)
                        {
                            // Failed to shcedule work
                            error = CHIP_ERROR_NO_MEMORY;
                        }
                    }
                    else
                    {
                        // The SRP cache entry had no usable addresses. Treat it as not found so the caller
                        // falls back to the regular mDNS resolve path instead of reporting a successful
                        // resolve with an empty address list.
                        error = CHIP_ERROR_NOT_FOUND;
                    }
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
                                  DnsServiceTxtEntries & serviceTxtEntries, Inet::IPAddress * resolvedAddresses,
                                  uint8_t & resolvedAddressCount)
{
    const char * tmpName;
    const uint8_t * txtStringPtr;
    size_t substringSize;
    uint8_t addrNum = 0;
    uint16_t txtDataLen;
    const otIp6Address * ip6AddrPtr = otSrpServerHostGetAddresses(host, &addrNum);
    const char * dot;

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
        dot = strchr(tmpName, '.');
        VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        substringSize = dot - tmpName;
        if (substringSize >= MATTER_ARRAY_SIZE(mdnsService.mName))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Platform::CopyString(mdnsService.mName, substringSize + 1, tmpName);

        // Extract from the <instance>.<type>.<protocol>.<domain-name>. the <type> part.
        tmpName = tmpName + substringSize + 1;
        dot     = strchr(tmpName, '.');
        VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        substringSize = dot - tmpName;
        if (substringSize >= MATTER_ARRAY_SIZE(mdnsService.mType))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Platform::CopyString(mdnsService.mType, substringSize + 1, tmpName);

        // Extract from the <instance>.<type>.<protocol>.<domain-name>. the <type> part.
        tmpName = tmpName + substringSize + 1;
        dot     = strchr(tmpName, '.');
        VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        substringSize = dot - tmpName;
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
    tmpName = otSrpServerHostGetFullName(host);
    dot     = strchr(tmpName, '.');
    VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    substringSize = dot - tmpName;
    if (substringSize >= MATTER_ARRAY_SIZE(mdnsService.mHostName))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(mdnsService.mHostName, substringSize + 1, tmpName);
    mdnsService.mPort = otSrpServerServiceGetPort(service);

    // All SRP cache hits come from the Thread Netif
    mdnsService.mInterface = mThreadNetIf;

    mdnsService.mAddressType = Inet::IPAddressType::kIPv6;

    for (uint8_t i = 0; i < addrNum && resolvedAddressCount < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES; i++)
    {
        resolvedAddresses[resolvedAddressCount++] = ToIPAddress(ip6AddrPtr[i]);
    }

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

        VerifyOrReturnError(!alloc.AnyAllocFailed(), CHIP_ERROR_BUFFER_TOO_SMALL);

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
    const char * dot;

    // Extract from the <type>.<protocol> the <type> part.
    dot = strchr(aServiceType, '.');
    VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    substringSize = dot - aServiceType;
    if (substringSize >= MATTER_ARRAY_SIZE(mdnsService.mType))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(mdnsService.mType, substringSize + 1, aServiceType);

    // Extract from the <type>.<protocol>. the .<protocol> part.
    protocolSubstringStart = aServiceType + substringSize;

    // Check that the protocolSubstringStart starts wit a '.' to be sure we are in the right place
    if (strchr(protocolSubstringStart, '.') == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Jump over '.' in protocolSubstringStart and substract the string terminator from the size
    substringSize = strlen(++protocolSubstringStart) - 1;
    if (substringSize >= MATTER_ARRAY_SIZE(protocol))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(protocol, MATTER_ARRAY_SIZE(protocol), protocolSubstringStart);

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

    // All mDNS replies come from the External Netif
    mdnsService.mInterface = mExternalNetIf;

    return CHIP_NO_ERROR;
}

static void OtBrowseCallback(otInstance * aInstance, const otMdnsBrowseResult * aResult)
{
    CHIP_ERROR error;
    mDnsQueryCtx * pBrowseContext = nullptr;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    pBrowseContext = reinterpret_cast<mDnsQueryCtx *>(GetResolveElement(aResult->mServiceType, kNameTypeService));
    VerifyOrReturn(pBrowseContext != nullptr);

    mDnsQueryCtx * tmpContext = Platform::New<mDnsQueryCtx>(pBrowseContext->mMatterCtx, pBrowseContext->mDnsBrowseCallback);
    VerifyOrReturn(tmpContext != nullptr);

    Platform::CopyString(tmpContext->mMdnsService.mName, sizeof(tmpContext->mMdnsService.mName), aResult->mServiceInstance);
    error = FromServiceTypeToMdnsData(tmpContext->mMdnsService, aResult->mServiceType);

    if (CHIP_NO_ERROR == error)
    {
        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(tmpContext)) != CHIP_NO_ERROR)
        {
            Platform::Delete<mDnsQueryCtx>(tmpContext);
        }
    }
    else
    {
        Platform::Delete<mDnsQueryCtx>(tmpContext);
    }
}

static void OtServiceCallback(otInstance * aInstance, const otMdnsSrvResult * aResult)
{
    CHIP_ERROR error;
    mDnsQueryCtx * pResolveContext = nullptr;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    pResolveContext = GetResolveElement(aResult->mServiceInstance, kNameTypeInstance);
    VerifyOrReturn(pResolveContext != nullptr);

    error                   = FromServiceTypeToMdnsData(pResolveContext->mMdnsService, aResult->mServiceType);
    pResolveContext->mError = error;

    if (CHIP_NO_ERROR == error)
    {
        Platform::CopyString(pResolveContext->mMdnsService.mName, sizeof(pResolveContext->mMdnsService.mName),
                             aResult->mServiceInstance);
        Platform::CopyString(pResolveContext->mMdnsService.mHostName, sizeof(pResolveContext->mMdnsService.mHostName),
                             aResult->mHostName);

        pResolveContext->mMdnsService.mPort       = aResult->mPort;
        pResolveContext->mMdnsService.mTtlSeconds = aResult->mTtl;
        TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(DispatchTxtResolve,
                                                                         reinterpret_cast<intptr_t>(pResolveContext));
    }
    else
    {
        HandleResolveCleanup(*pResolveContext, kResolveStepSrv, /* dispatchResolveError*/ true);
    }
}

static void OtTxtCallback(otInstance * aInstance, const otMdnsTxtResult * aResult)
{
    bool bSendDispatch             = true;
    mDnsQueryCtx * pResolveContext = nullptr;

    // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
    VerifyOrReturn(aResult->mTtl > 0);

    pResolveContext = GetResolveElement(aResult->mServiceInstance, kNameTypeInstance);
    VerifyOrReturn(pResolveContext != nullptr);

    // Check if TXT record was included in the response.
    if (aResult->mTxtDataLength != 0)
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, aResult->mTxtData, aResult->mTxtDataLength);

        otDnsTxtEntry txtEntry;
        chip::FixedBufferAllocator alloc(pResolveContext->mServiceTxtEntry.mBuffer);

        uint8_t entryIndex = 0;
        while ((otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE) && entryIndex < 64)
        {
            if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
                continue;

            pResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mKey  = alloc.Clone(txtEntry.mKey);
            pResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mData = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
            pResolveContext->mServiceTxtEntry.mTxtEntries[entryIndex].mDataSize = txtEntry.mValueLength;
            entryIndex++;
        }

        if (alloc.AnyAllocFailed())
        {
            bSendDispatch           = false;
            pResolveContext->mError = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            pResolveContext->mMdnsService.mTextEntries   = pResolveContext->mServiceTxtEntry.mTxtEntries;
            pResolveContext->mMdnsService.mTextEntrySize = entryIndex;
        }
    }
    else
    {
        pResolveContext->mMdnsService.mTextEntrySize = 0;
    }

    if (bSendDispatch)
    {
        TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(DispatchAddressResolve,
                                                                         reinterpret_cast<intptr_t>(pResolveContext));
    }
    else
    {
        HandleResolveCleanup(*pResolveContext, kResolveStepTxt, /* dispatchResolveError*/ true);
    }
}

static void OtAddressCallback(otInstance * aInstance, const otMdnsAddressResult * aResult)
{
    VerifyOrReturn(aResult->mAddressesLength > 0);

    mDnsQueryCtx * pResolveContext = GetResolveElement(aResult->mHostName, kNameTypeHost);
    VerifyOrReturn(pResolveContext != nullptr);

    pResolveContext->mMdnsService.mAddressType = Inet::IPAddressType::kIPv6;
    pResolveContext->mResolvedAddressCount     = 0;

    for (uint16_t i = 0;
         i < aResult->mAddressesLength && pResolveContext->mResolvedAddressCount < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES;
         i++)
    {
        // Ingnore reponses with TTL 0, the record is no longer valid and was removed from the mDNS cache
        if (aResult->mAddresses[i].mTtl > 0)
        {
            Inet::IPAddress addr = ToIPAddress(aResult->mAddresses[i].mAddress);

            if (addr.IsIPv4())
            {
                continue;
            }
            pResolveContext->mResolvedAddresses[pResolveContext->mResolvedAddressCount++] = addr;
        }
    }

    VerifyOrReturn(pResolveContext->mResolvedAddressCount > 0);

    if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(pResolveContext)) != CHIP_NO_ERROR)
    {
        (void) otMdnsStopIp6AddressResolver(mOtInstance, &pResolveContext->mAddrInfo);
        pResolveContext->mError = CHIP_ERROR_NO_MEMORY;
        DispatchResolveError(reinterpret_cast<intptr_t>(pResolveContext));
    }
}

static void DispatchBrowseEmpty(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    browseContext->mDnsBrowseCallback(browseContext->mMatterCtx, nullptr, 0, true, browseContext->mError);
    Platform::Delete<mDnsQueryCtx>(browseContext);
}

static void DispatchBrowse(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    browseContext->mDnsBrowseCallback(browseContext->mMatterCtx, &browseContext->mMdnsService, 1, false, browseContext->mError);
    Platform::Delete<mDnsQueryCtx>(browseContext);
}

static void DispatchTxtResolve(intptr_t context)
{
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    otError error;

    VerifyOrReturn(IsInResolveList(pResolveContext));

    // Stop SRV resolver before starting TXT one, ignore error as it will only happen if mMDS module is not initialized
    (void) otMdnsStopSrvResolver(mOtInstance, &pResolveContext->mSrvInfo);

    pResolveContext->mTxtInfo.mServiceInstance = pResolveContext->mMdnsService.mName;
    pResolveContext->mTxtInfo.mServiceType     = pResolveContext->mServiceType;
    pResolveContext->mTxtInfo.mCallback        = OtTxtCallback;
    pResolveContext->mTxtInfo.mInfraIfIndex    = mNetifIndex;

    error = otMdnsStartTxtResolver(mOtInstance, &pResolveContext->mTxtInfo);
    if (error != OT_ERROR_NONE)
    {
        pResolveContext->mError = MapOpenThreadError(error);
        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveError, reinterpret_cast<intptr_t>(pResolveContext)) !=
            CHIP_NO_ERROR)
        {
            DispatchResolveError(reinterpret_cast<intptr_t>(pResolveContext));
        }
    }
    else
    {
        pResolveContext->mResolveStep = kResolveStepTxt;
    }
}

static void DispatchAddressResolve(intptr_t context)
{
    otError error;
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(context);

    VerifyOrReturn(IsInResolveList(pResolveContext));

    // Stop TXT resolver before starting address one, ignore error as it will only happen if mMDS module is not initialized
    (void) otMdnsStopTxtResolver(mOtInstance, &pResolveContext->mTxtInfo);

    pResolveContext->mAddrInfo.mCallback     = OtAddressCallback;
    pResolveContext->mAddrInfo.mHostName     = pResolveContext->mMdnsService.mHostName;
    pResolveContext->mAddrInfo.mInfraIfIndex = mNetifIndex;

    error = otMdnsStartIp6AddressResolver(mOtInstance, &pResolveContext->mAddrInfo);
    if (error != OT_ERROR_NONE)
    {
        pResolveContext->mError = MapOpenThreadError(error);
        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveError, reinterpret_cast<intptr_t>(pResolveContext)) !=
            CHIP_NO_ERROR)
        {
            DispatchResolveError(reinterpret_cast<intptr_t>(pResolveContext));
        }
    }
    else
    {
        pResolveContext->mResolveStep = kResolveStepIpAddr;
    }
}

static void DispatchResolve(intptr_t context)
{
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(context);

    VerifyOrReturn(IsInResolveList(pResolveContext));

    Dnssd::DnssdService & service = pResolveContext->mMdnsService;

    // Stop Address resolver, we have finished resolving the service. Ignore error as it will only happen if
    // mMDS module is not initialized
    (void) otMdnsStopIp6AddressResolver(mOtInstance, &pResolveContext->mAddrInfo);

    // The context will be freed and the resolve operation is stopped. Matter will
    // try to stop it again on the mDnsResolveCallback but nothing will happen because the
    // element is no longer present in the list.
    mResolveList.Remove(pResolveContext);

    Span<Inet::IPAddress> ipAddrs(pResolveContext->mResolvedAddresses, pResolveContext->mResolvedAddressCount);

    pResolveContext->mDnsResolveCallback(pResolveContext->mMatterCtx, &service, ipAddrs, pResolveContext->mError);
    Platform::Delete<mDnsQueryCtx>(pResolveContext);
}

static void DispatchResolveSrp(intptr_t context)
{
    // When processing this function, the context is valid and not added to the resolve list. The OpenThread service
    // resolver was not started, and a call to OpenThreadBrDnssdShutdown or OpenThreadBrDnssdResolveNoLongerNeeded will have
    // no effect on this context. The only place that the context is being freed is below.
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    Dnssd::DnssdService & service  = pResolveContext->mMdnsService;
    Span<Inet::IPAddress> ipAddrs(pResolveContext->mResolvedAddresses, pResolveContext->mResolvedAddressCount);

    // Address resolver was not started and the resolve context was not added to the resolve list
    // when the result came directly from SRP.

    pResolveContext->mDnsResolveCallback(pResolveContext->mMatterCtx, &service, ipAddrs, pResolveContext->mError);
    Platform::Delete<mDnsQueryCtx>(pResolveContext);
}

static void DispatchResolveError(intptr_t context)
{
    mDnsQueryCtx * pResolveContext = reinterpret_cast<mDnsQueryCtx *>(context);

    VerifyOrReturn(IsInResolveList(pResolveContext));

    Span<Inet::IPAddress> ipAddrs;

    // The context will be freed and the resolve operation is stopped. Matter will
    // try to stop it again on the mDnsResolveCallback but nothing will happen because the
    // element is no longer present in the list.
    mResolveList.Remove(pResolveContext);

    pResolveContext->mDnsResolveCallback(pResolveContext->mMatterCtx, nullptr, ipAddrs, pResolveContext->mError);
    Platform::Delete<mDnsQueryCtx>(pResolveContext);
}

static void HandleResolveCleanup(mDnsQueryCtx & resolveContext, ResolveStep stepType, bool dispatchResolveError)
{
    switch (stepType)
    {
    case kResolveStepSrv:
        otMdnsStopSrvResolver(mOtInstance, &resolveContext.mSrvInfo);
        break;
    case kResolveStepTxt:
        otMdnsStopTxtResolver(mOtInstance, &resolveContext.mTxtInfo);
        break;
    case kResolveStepIpAddr:
        otMdnsStopIp6AddressResolver(mOtInstance, &resolveContext.mAddrInfo);
        break;
    }

    if (dispatchResolveError)
    {
        // In this case the resolve operation could not be completed successfully so we need to call
        // DispatchResolveError to handle the Matter callback with an error case. No IP address is reported and
        // the address resolve operation doesn’t need to be stopped again as was not started in the first place
        // or it's already handled by HandleResolveCleanup.
        if (DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveError, reinterpret_cast<intptr_t>(&resolveContext)) !=
            CHIP_NO_ERROR)
        {
            DispatchResolveError(reinterpret_cast<intptr_t>(&resolveContext));
        }
    }
}

static mDnsQueryCtx * GetResolveElement(const char * aName, NameType aType)
{
    IntrusiveList<mDnsQueryCtx> & list = (aType == kNameTypeService) ? mBrowseList : mResolveList;

    for (auto & ctx : list)
    {
        if (aType == kNameTypeInstance && strcmp(aName, ctx.mMdnsService.mName) == 0)
        {
            return &ctx;
        }
        if (aType == kNameTypeHost && strcmp(aName, ctx.mMdnsService.mHostName) == 0)
        {
            return &ctx;
        }
        if (aType == kNameTypeService && strcmp(aName, ctx.mServiceType) == 0)
        {
            return &ctx;
        }
    }

    return nullptr;
}

static bool IsInResolveList(const mDnsQueryCtx * pResolveContext)
{
    for (auto & ctx : mResolveList)
    {
        if (&ctx == pResolveContext)
        {
            return true;
        }
    }
    return false;
}
} // namespace Dnssd
} // namespace chip
