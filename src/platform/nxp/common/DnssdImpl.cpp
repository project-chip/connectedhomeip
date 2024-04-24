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

#include <openthread/mdns_server.h>

using namespace ::chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace Dnssd {

#define LOCAL_DOMAIN_STRING_SIZE 7
#define ARPA_DOMAIN_STRING_SIZE 22
#define MATTER_DNS_TXT_SIZE 128

// Support both operational and commissionable discovery, so buffers sizes must be worst case.
static constexpr uint8_t kMaxMdnsServiceTxtEntriesNumber =
    std::max(Dnssd::CommissionAdvertisingParameters::kTxtMaxNumber, Dnssd::OperationalAdvertisingParameters::kTxtMaxNumber);
static constexpr size_t kTotalMdnsServiceTxtValueSize = std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                 Dnssd::OperationalAdvertisingParameters::kTxtTotalValueSize);
static constexpr size_t kTotalMdnsServiceTxtKeySize =
    std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalKeySize, Dnssd::OperationalAdvertisingParameters::kTxtTotalKeySize);

static constexpr size_t kTotalMdnsServiceTxtBufferSize =
    kTotalMdnsServiceTxtKeySize + kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtValueSize;

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
    char mServiceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE + 1];
    CHIP_ERROR error;

    mDnsQueryCtx(void * context, CHIP_ERROR aError)
    {
        matterCtx = context;
        error     = aError;
    }
};

static const char * GetProtocolString(DnssdServiceProtocol protocol);

static void OtBrowseCallback(otError aError, const otDnsBrowseResponse * aResponse, void * aContext);
static void OtServiceCallback(otError aError, const otDnsServiceResponse * aResponse, void * aContext);

static void DispatchBrowseEmpty(intptr_t context);
static void DispatchBrowse(intptr_t context);
static void DispatchBrowseNoMemory(intptr_t context);

void DispatchAddressResolve(intptr_t context);
void DispatchResolve(intptr_t context);
void DispatchResolveNoMemory(intptr_t context);

static DnsBrowseCallback mDnsBrowseCallback;
static DnsResolveCallback mDnsResolveCallback;

CHIP_ERROR ResolveBySrp(DnssdService * mdnsReq, otInstance * thrInstancePtr, char * instanceName, void * context);
CHIP_ERROR BrowseBySrp(otInstance * thrInstancePtr, char * serviceName, void * context);

CHIP_ERROR FromSrpCacheToMdnsData(const otSrpServerService * service, const otSrpServerHost * host,
                                  const DnssdService * mdnsQueryReq, chip::Dnssd::DnssdService & mdnsService,
                                  DnsServiceTxtEntries & serviceTxtEntries);

static bool bBrowseInProgress = false;

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error            = CHIP_NO_ERROR;
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();

    uint8_t macBuffer[ConfigurationManager::kPrimaryMACAddressLength];
    MutableByteSpan mac(macBuffer);
    char hostname[kHostNameMaxLength + LOCAL_DOMAIN_STRING_SIZE + 1] = "";
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac));
    MakeHostName(hostname, sizeof(hostname), mac);
    snprintf(hostname + strlen(hostname), sizeof(hostname), ".local.");

    error = MapOpenThreadError(otMdnsServerSetHostName(thrInstancePtr, hostname));

    initCallback(context, error);
    return error;
}

void ChipDnssdShutdown()
{
    otMdnsServerStop(ThreadStackMgrImpl().OTInstance());
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();

    otMdnsServerMarkServiceForRemoval(thrInstancePtr, nullptr, "_matter._tcp.local.");
    otMdnsServerMarkServiceForRemoval(thrInstancePtr, nullptr, "_matterc._udp.local.");

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    ReturnErrorCodeIf(service == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    otError otErr;
    otDnsTxtEntry aTxtEntry;
    uint32_t txtBufferOffset = 0;

    char fullInstName[Common::kInstanceNameMaxLength + chip::Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE + 1] =
        "";
    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE + 1] = "";
    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtBufferSize] = { 0 };

    if ((strcmp(service->mHostName, "") != 0) && (nullptr == otMdnsServerGetHostName(thrInstancePtr)))
    {
        char hostname[kHostNameMaxLength + LOCAL_DOMAIN_STRING_SIZE + 1] = "";
        snprintf(hostname, sizeof(hostname), "%s.local.", service->mHostName);
        otMdnsServerSetHostName(thrInstancePtr, hostname);
    }

    snprintf(serviceType, sizeof(serviceType), "%s.%s.local.", service->mType, GetProtocolString(service->mProtocol));
    snprintf(fullInstName, sizeof(fullInstName), "%s.%s", service->mName, serviceType);

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
    aTxtEntry.mKey         = nullptr;
    aTxtEntry.mValue       = txtBuffer;
    aTxtEntry.mValueLength = txtBufferOffset;

    otErr = otMdnsServerAddService(thrInstancePtr, fullInstName, serviceType, service->mSubTypes, service->mSubTypeSize,
                                   service->mPort, &aTxtEntry, 1);
    // Ignore duplicate error and threat it as error none
    if (otErr == OT_ERROR_DUPLICATED)
        otErr = OT_ERROR_NONE;

    return MapOpenThreadError(otErr);
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    otMdnsServerRemoveMarkedServices(ThreadStackMgrImpl().OTInstance());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                           Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context, intptr_t * browseIdentifier)
{
    *browseIdentifier                                                                         = reinterpret_cast<intptr_t>(nullptr);
    CHIP_ERROR error                                                                          = CHIP_NO_ERROR;
    CHIP_ERROR srpBrowseError                                                                 = CHIP_NO_ERROR;
    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + ARPA_DOMAIN_STRING_SIZE + 1] = ""; // +1 for null-terminator

    if (type == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    mDnsBrowseCallback          = callback;

    mDnsQueryCtx * browseContext = Platform::New<mDnsQueryCtx>(context, CHIP_NO_ERROR);
    VerifyOrReturnError(browseContext != nullptr, CHIP_ERROR_NO_MEMORY);

    // First try to browse the service in the SRP cache, use default.service.arpa as domain name
    snprintf(serviceType, sizeof(serviceType), "%s.%s.default.service.arpa.", type, GetProtocolString(protocol));
    // After browsing in the SRP cache we will continue with regular mDNS browse
    srpBrowseError = BrowseBySrp(thrInstancePtr, serviceType, context);

    // Proceed to generate a mDNS query
    snprintf(browseContext->mServiceType, sizeof(browseContext->mServiceType), "%s.%s.local.", type, GetProtocolString(protocol));

    error = MapOpenThreadError(otMdnsServerBrowse(thrInstancePtr, browseContext->mServiceType, OtBrowseCallback, browseContext));

    if (CHIP_NO_ERROR == error)
    {
        bBrowseInProgress = true;
        *browseIdentifier = reinterpret_cast<intptr_t>(browseContext);
    }
    else
    {
        if (srpBrowseError == CHIP_NO_ERROR)
        {
            // In this case, we need to send a final browse indication to signal the Matter App that there are no more
            // browse results coming
            browseContext->error = error;
            DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(browseContext));
        }
        else
        {
            Platform::Delete<mDnsQueryCtx>(browseContext);
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
    // that has been freed in DispatchBrowseEmpty before.
    if ((true == bBrowseInProgress) && (browseContext))
    {
        browseContext->error = MapOpenThreadError(otMdnsServerStopQuery(thrInstancePtr, browseContext->mServiceType));

        // browse context will be freed in DispatchBrowseEmpty
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(browseContext));
    }
    return MapOpenThreadError(error);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
    ChipError error;
    if (browseResult == nullptr || callback == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
    mDnsResolveCallback         = callback;

    char serviceType[chip::Dnssd::kDnssdTypeAndProtocolMaxSize + ARPA_DOMAIN_STRING_SIZE + 1] = ""; // +1 for null-terminator
    char fullInstName[Common::kInstanceNameMaxLength + chip::Dnssd::kDnssdTypeAndProtocolMaxSize + ARPA_DOMAIN_STRING_SIZE + 1] =
        "";

    // First try to find the service in the SRP cache, use default.service.arpa as domain name
    snprintf(serviceType, sizeof(serviceType), "%s.%s.default.service.arpa.", browseResult->mType,
             GetProtocolString(browseResult->mProtocol));
    snprintf(fullInstName, sizeof(fullInstName), "%s.%s", browseResult->mName, serviceType);

    error = ResolveBySrp(browseResult, thrInstancePtr, fullInstName, context);
    if (CHIP_ERROR_NOT_FOUND == error)
    {
        // If the SRP cache returns not found, proceed to generate a MDNS query
        memset(serviceType, 0, sizeof(serviceType));
        memset(fullInstName, 0, sizeof(fullInstName));

        snprintf(serviceType, sizeof(serviceType), "%s.%s.local.", browseResult->mType, GetProtocolString(browseResult->mProtocol));
        snprintf(fullInstName, sizeof(fullInstName), "%s.%s", browseResult->mName, serviceType);

        return MapOpenThreadError(otMdnsServerResolveService(thrInstancePtr, fullInstName, OtServiceCallback, context));
    }
    else
    {
        return error;
    }
}

CHIP_ERROR BrowseBySrp(otInstance * thrInstancePtr, char * serviceName, void * context)
{
    const otSrpServerHost * host       = nullptr;
    const otSrpServerService * service = nullptr;
    CHIP_ERROR error                   = CHIP_ERROR_NOT_FOUND;

    while ((host = otSrpServerGetNextHost(thrInstancePtr, host)) != nullptr)
    {
        service = otSrpServerHostFindNextService(host, service, OT_SRP_SERVER_FLAGS_ANY_TYPE_ACTIVE_SERVICE, serviceName, nullptr);
        if (service != nullptr)
        {
            mDnsQueryCtx * serviceContext;

            serviceContext = Platform::New<mDnsQueryCtx>(context, CHIP_NO_ERROR);
            if (serviceContext != nullptr)
            {
                if (CHIP_NO_ERROR ==
                    FromSrpCacheToMdnsData(service, host, nullptr, serviceContext->mMdnsService, serviceContext->mServiceTxtEntry))
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
    return error;
}

CHIP_ERROR ResolveBySrp(DnssdService * mdnsReq, otInstance * thrInstancePtr, char * instanceName, void * context)
{
    const otSrpServerHost * host       = nullptr;
    const otSrpServerService * service = nullptr;
    CHIP_ERROR error                   = CHIP_ERROR_NOT_FOUND;

    while ((host = otSrpServerGetNextHost(thrInstancePtr, host)) != nullptr)
    {
        service = otSrpServerHostFindNextService(
            host, service, (OT_SRP_SERVER_SERVICE_FLAG_BASE_TYPE | OT_SRP_SERVER_SERVICE_FLAG_ACTIVE), nullptr, instanceName);
        if (service != nullptr)
        {
            error = CHIP_NO_ERROR;
            mDnsQueryCtx * serviceContext;

            serviceContext = Platform::New<mDnsQueryCtx>(context, CHIP_NO_ERROR);
            if (serviceContext != nullptr)
            {
                error =
                    FromSrpCacheToMdnsData(service, host, mdnsReq, serviceContext->mMdnsService, serviceContext->mServiceTxtEntry);
                if (error == CHIP_NO_ERROR)
                {
                    DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(serviceContext));
                }
                else
                {
                    Platform::Delete<mDnsQueryCtx>(serviceContext);
                }
            }
            else
            {
                error = CHIP_ERROR_NO_MEMORY;
            }
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
    mdnsService.mAddress     = MakeOptional(ToIPAddress(*ip6AddrPtr));

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

CHIP_ERROR FromOtDnsResponseToMdnsData(otDnsServiceInfo & serviceInfo, const char * serviceType,
                                       chip::Dnssd::DnssdService & mdnsService, DnsServiceTxtEntries & serviceTxtEntries,
                                       otError error)
{
    char protocol[chip::Dnssd::kDnssdProtocolTextMaxSize + 1];

    if (strchr(serviceType, '.') == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Extract from the <type>.<protocol>.<domain-name>. the <type> part.
    size_t substringSize = strchr(serviceType, '.') - serviceType;
    if (substringSize >= ArraySize(mdnsService.mType))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(mdnsService.mType, substringSize + 1, serviceType);

    // Extract from the <type>.<protocol>.<domain-name>. the <protocol> part.
    const char * protocolSubstringStart = serviceType + substringSize + 1;

    if (strchr(protocolSubstringStart, '.') == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    substringSize = strchr(protocolSubstringStart, '.') - protocolSubstringStart;
    if (substringSize >= ArraySize(protocol))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    Platform::CopyString(protocol, substringSize + 1, protocolSubstringStart);

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

    // Check if SRV record was included in DNS response.
    if (error != OT_ERROR_NOT_FOUND)
    {
        if (strchr(serviceInfo.mHostNameBuffer, '.') == nullptr)
            return CHIP_ERROR_INVALID_ARGUMENT;

        // Extract from the <hostname>.<domain-name>. the <hostname> part.
        substringSize = strchr(serviceInfo.mHostNameBuffer, '.') - serviceInfo.mHostNameBuffer;
        if (substringSize >= ArraySize(mdnsService.mHostName))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Platform::CopyString(mdnsService.mHostName, substringSize + 1, serviceInfo.mHostNameBuffer);

        mdnsService.mPort = serviceInfo.mPort;
    }

    // All mDNS replies come from the External Netif
    mdnsService.mInterface = ConnectivityManagerImpl().GetExternalInterface();

    // Check if AAAA record was included in DNS response.
    if (!otIp6IsAddressUnspecified(&serviceInfo.mHostAddress))
    {
        mdnsService.mAddressType = Inet::IPAddressType::kIPv6;
        mdnsService.mAddress     = MakeOptional(ToIPAddress(serviceInfo.mHostAddress));
    }

    // Check if TXT record was included in DNS response.
    if (serviceInfo.mTxtDataSize != 0)
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, serviceInfo.mTxtData, serviceInfo.mTxtDataSize);

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

    return CHIP_NO_ERROR;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName) {}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

static void OtBrowseCallback(otError aError, const otDnsBrowseResponse * aResponse, void * aContext)
{
    CHIP_ERROR error;
    // type buffer size is kDnssdTypeAndProtocolMaxSize + . + kMaxDomainNameSize + . + termination character
    char type[Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE + 3];
    // hostname buffer size is kHostNameMaxLength + . + kMaxDomainNameSize + . + termination character
    char hostname[Dnssd::kHostNameMaxLength + LOCAL_DOMAIN_STRING_SIZE + 3];
    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtBufferSize];

    mDnsQueryCtx * browseContext = reinterpret_cast<mDnsQueryCtx *>(aContext);
    otDnsServiceInfo serviceInfo;
    uint16_t index = 0;

    /// TODO: check this code, might be remvoed, or if not free browseContext
    if (mDnsBrowseCallback == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid dns browse callback");
        return;
    }

    VerifyOrExit(aError == OT_ERROR_NONE, error = MapOpenThreadError(aError));

    error = MapOpenThreadError(otDnsBrowseResponseGetServiceName(aResponse, type, sizeof(type)));

    VerifyOrExit(error == CHIP_NO_ERROR, );

    char serviceName[Dnssd::Common::kInstanceNameMaxLength + 1];
    while (otDnsBrowseResponseGetServiceInstance(aResponse, index, serviceName, sizeof(serviceName)) == OT_ERROR_NONE)
    {
        serviceInfo.mHostNameBuffer     = hostname;
        serviceInfo.mHostNameBufferSize = sizeof(hostname);
        serviceInfo.mTxtData            = txtBuffer;
        serviceInfo.mTxtDataSize        = sizeof(txtBuffer);

        otError err = otDnsBrowseResponseGetServiceInfo(aResponse, serviceName, &serviceInfo);
        error       = MapOpenThreadError(err);

        VerifyOrExit(err == OT_ERROR_NOT_FOUND || err == OT_ERROR_NONE, );

        mDnsQueryCtx * tmpContext = Platform::New<mDnsQueryCtx>(browseContext->matterCtx, CHIP_NO_ERROR);

        VerifyOrExit(tmpContext != nullptr, error = CHIP_ERROR_NO_MEMORY);

        error = FromOtDnsResponseToMdnsData(serviceInfo, type, tmpContext->mMdnsService, tmpContext->mServiceTxtEntry, err);
        if (CHIP_NO_ERROR == error)
        {
            // Invoke callback for every service one by one instead of for the whole
            // list due to large memory size needed to allocate on stack.
            static_assert(ArraySize(tmpContext->mMdnsService.mName) >= ArraySize(serviceName),
                          "The target buffer must be big enough");
            Platform::CopyString(tmpContext->mMdnsService.mName, serviceName);
            DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(tmpContext));
        }
        else
        {
            Platform::Delete<mDnsQueryCtx>(tmpContext);
        }
        index++;
    }

exit:
    // Invoke callback to notify about end-of-browse when OT_ERROR_RESPONSE_TIMEOUT is received, otherwise ignore errors
    if (aError == OT_ERROR_RESPONSE_TIMEOUT)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(browseContext));
    }
}
static void OtServiceCallback(otError aError, const otDnsServiceResponse * aResponse, void * aContext)
{
    CHIP_ERROR error;
    otError otErr;
    otDnsServiceInfo serviceInfo;
    mDnsQueryCtx * serviceContext;
    bool bStopQuery = false;

    // If error is timeout we don't need to inform the Matter app and we can just exit
    VerifyOrReturn(aError != OT_ERROR_RESPONSE_TIMEOUT, );

    bStopQuery     = true;
    serviceContext = Platform::New<mDnsQueryCtx>(aContext, MapOpenThreadError(aError));
    VerifyOrExit(serviceContext != nullptr, error = CHIP_ERROR_NO_MEMORY);

    // type buffer size is kDnssdTypeAndProtocolMaxSize + . + kMaxDomainNameSize + . + termination character
    char type[Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE + 3];
    // hostname buffer size is kHostNameMaxLength + . + kMaxDomainNameSize + . + termination character
    char hostname[Dnssd::kHostNameMaxLength + LOCAL_DOMAIN_STRING_SIZE + 3];
    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxMdnsServiceTxtEntriesNumber + kTotalMdnsServiceTxtBufferSize];

    if (mDnsResolveCallback == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid dns resolve callback");
        return;
    }

    VerifyOrExit(aError == OT_ERROR_NONE, error = MapOpenThreadError(aError));

    error = MapOpenThreadError(otDnsServiceResponseGetServiceName(aResponse, serviceContext->mMdnsService.mName,
                                                                  sizeof(serviceContext->mMdnsService.mName), type, sizeof(type)));

    VerifyOrExit(error == CHIP_NO_ERROR, );

    serviceInfo.mHostNameBuffer     = hostname;
    serviceInfo.mHostNameBufferSize = sizeof(hostname);
    serviceInfo.mTxtData            = txtBuffer;
    serviceInfo.mTxtDataSize        = sizeof(txtBuffer);

    otErr = otDnsServiceResponseGetServiceInfo(aResponse, &serviceInfo);
    error = MapOpenThreadError(otErr);

    VerifyOrExit(error == CHIP_NO_ERROR, );

    error = FromOtDnsResponseToMdnsData(serviceInfo, type, serviceContext->mMdnsService, serviceContext->mServiceTxtEntry, otErr);

exit:
    if (serviceContext == nullptr)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveNoMemory, reinterpret_cast<intptr_t>(aContext));
        return;
    }

    serviceContext->error = error;

    // If IPv6 address in unspecified (AAAA record not present), send additional DNS query to obtain IPv6 address.
    if (otIp6IsAddressUnspecified(&serviceInfo.mHostAddress))
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchAddressResolve, reinterpret_cast<intptr_t>(serviceContext));
    }
    else
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(serviceContext));
    }

    if (bStopQuery)
    {
        char fullInstName[Common::kInstanceNameMaxLength + chip::Dnssd::kDnssdTypeAndProtocolMaxSize + LOCAL_DOMAIN_STRING_SIZE +
                          1] = "";
        snprintf(fullInstName, sizeof(fullInstName), "%s.%s", serviceContext->mMdnsService.mName, type);

        otInstance * thrInstancePtr = ThreadStackMgrImpl().OTInstance();
        otMdnsServerStopQuery(thrInstancePtr, fullInstName);
    }
}

void DispatchBrowseEmpty(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    mDnsBrowseCallback(browseContext->matterCtx, nullptr, 0, true, browseContext->error);
    Platform::Delete<mDnsQueryCtx>(browseContext);
    bBrowseInProgress = false;
}

void DispatchBrowse(intptr_t context)
{
    auto * browseContext = reinterpret_cast<mDnsQueryCtx *>(context);
    mDnsBrowseCallback(browseContext->matterCtx, &browseContext->mMdnsService, 1, false, browseContext->error);
    Platform::Delete<mDnsQueryCtx>(browseContext);
}

void DispatchBrowseNoMemory(intptr_t context)
{
    mDnsBrowseCallback(reinterpret_cast<void *>(context), nullptr, 0, true, CHIP_ERROR_NO_MEMORY);
}

void DispatchAddressResolve(intptr_t context)
{
    CHIP_ERROR error = CHIP_ERROR_NO_MEMORY; // ResolveAddress(context, OnDnsAddressResolveResult);

    // In case of address resolve failure, fill the error code field and dispatch method to end resolve process.
    if (error != CHIP_NO_ERROR)
    {
        mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
        resolveContext->error         = error;

        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(resolveContext));
    }
}

void DispatchResolve(intptr_t context)
{
    mDnsQueryCtx * resolveContext = reinterpret_cast<mDnsQueryCtx *>(context);
    Dnssd::DnssdService & service = resolveContext->mMdnsService;
    Span<Inet::IPAddress> ipAddrs;

    if (service.mAddress.HasValue())
    {
        ipAddrs = Span<Inet::IPAddress>(&service.mAddress.Value(), 1);
    }

    mDnsResolveCallback(resolveContext->matterCtx, &service, ipAddrs, resolveContext->error);
    Platform::Delete<mDnsQueryCtx>(resolveContext);
}

void DispatchResolveNoMemory(intptr_t context)
{
    Span<Inet::IPAddress> ipAddrs;
    mDnsResolveCallback(reinterpret_cast<void *>(context), nullptr, ipAddrs, CHIP_ERROR_NO_MEMORY);
}

} // namespace Dnssd
} // namespace chip
