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
#include "MdnsError.h"
#include "lib/dnssd/platform/Dnssd.h"

#include "dns_sd.h"
#include "lwip/mld6.h"
#include "mdns.h"
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "lwip/sockets.h"
#undef write
#undef read
#include <cstdio>
#include <sstream>
#include <string.h>

#include "mDNSDebug.h"
#include "task.h"
#include "task_def.h"
#include <errno.h>

#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::Dnssd;

extern "C" {
extern void mDNSPlatformWriteLogRedirect(void (*)(const char *, const char *));
}

namespace {
constexpr char kLocalDot[]              = "local.";
constexpr char kProtocolTcp[]           = "._tcp";
constexpr char kProtocolUdp[]           = "._udp";
static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

constexpr DNSServiceFlags kRegisterFlags        = kDNSServiceFlagsNoAutoRename;
constexpr DNSServiceFlags kBrowseFlags          = 0;
constexpr DNSServiceFlags kGetAddrInfoFlags     = kDNSServiceFlagsTimeout | kDNSServiceFlagsShareConnection;
constexpr DNSServiceFlags kResolveFlags         = kDNSServiceFlagsShareConnection;
constexpr DNSServiceFlags kReconfirmRecordFlags = 0;

bool IsSupportedProtocol(DnssdServiceProtocol protocol)
{
    return (protocol == DnssdServiceProtocol::kDnssdProtocolUdp) || (protocol == DnssdServiceProtocol::kDnssdProtocolTcp);
}

uint32_t GetInterfaceId(chip::Inet::InterfaceId interfaceId)
{
    return interfaceId.IsPresent() ? (uint32_t) (void *) interfaceId.GetPlatformInterface() : kDNSServiceInterfaceIndexAny;
}

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kProtocolUdp : kProtocolTcp);
    return typeBuilder.str();
}

std::string GetFullType(const DnssdService * service)
{
    return GetFullType(service->mType, service->mProtocol);
}

std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol, const char * subTypes[], size_t subTypeSize)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kProtocolUdp : kProtocolTcp);
    for (int i = 0; i < (int) subTypeSize; i++)
    {
        typeBuilder << ",";
        typeBuilder << subTypes[i];
    }
    return typeBuilder.str();
}

std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol)
{
    auto fullType = GetFullType(type, protocol);

    std::string subtypeDelimiter = "._sub.";
    size_t position              = fullType.find(subtypeDelimiter);
    if (position != std::string::npos)
    {
        fullType = fullType.substr(position + subtypeDelimiter.size()) + "," + fullType.substr(0, position);
    }

    return fullType;
}

std::string GetFullTypeWithSubTypes(const DnssdService * service)
{
    return GetFullTypeWithSubTypes(service->mType, service->mProtocol, service->mSubTypes, service->mSubTypeSize);
}

std::string GetHostNameWithDomain(const char * hostname)
{
    return std::string(hostname) + '.' + kLocalDot;
}

void LogOnFailure(const char * name, DNSServiceErrorType err)
{
    if (kDNSServiceErr_NoError != err)
    {
        ChipLogError(Discovery, "%s (%s)", name, Error::ToString(err));
    }
}

class ScopedTXTRecord
{
public:
    ScopedTXTRecord() {}

    ~ScopedTXTRecord()
    {
        if (mDataSize != 0)
        {
            TXTRecordDeallocate(&mRecordRef);
        }
    }

    CHIP_ERROR Init(TextEntry * textEntries, size_t textEntrySize)
    {
        VerifyOrReturnError(textEntrySize <= kDnssdTextMaxSize, CHIP_ERROR_INVALID_ARGUMENT);

        TXTRecordCreate(&mRecordRef, sizeof(mRecordBuffer), mRecordBuffer);

        for (size_t i = 0; i < textEntrySize; i++)
        {
            TextEntry entry = textEntries[i];
            VerifyOrReturnError(chip::CanCastTo<uint8_t>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);

            auto err = TXTRecordSetValue(&mRecordRef, entry.mKey, static_cast<uint8_t>(entry.mDataSize), entry.mData);
            VerifyOrReturnError(err == kDNSServiceErr_NoError, CHIP_ERROR_INVALID_ARGUMENT);
        }

        mDataSize = TXTRecordGetLength(&mRecordRef);
        if (mDataSize == 0)
        {
            TXTRecordDeallocate(&mRecordRef);
        }

        mData = TXTRecordGetBytesPtr(&mRecordRef);
        return CHIP_NO_ERROR;
    }

    uint16_t size() { return mDataSize; }
    const void * data() { return mData; }

private:
    uint16_t mDataSize = 0;
    const void * mData = nullptr;

    TXTRecordRef mRecordRef;
    char mRecordBuffer[kDnssdTextMaxSize];
};
} // namespace

namespace chip {
namespace Dnssd {

#define SERVICE_DOMAIN ("local")

MdnsContexts MdnsContexts::sInstance;
static DNSServiceRef BrowseClient = NULL;
static TaskHandle_t gResolveTask  = NULL;
static EventGroupHandle_t gResolveTaskWakeEvent;

void ChipDnssdMdnsLog(const char * level, const char * msg)
{
    ChipLogProgress(ServiceProvisioning, "%s %s", StringOrNullMarker(level), StringOrNullMarker(msg));
}

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    ChipLogDetail(Discovery, "Mdns: %s name: %s, type: %s, domain: %s, flags: %ld", __func__, name, type, domain, flags);

    auto sdCtx = reinterpret_cast<RegisterContext *>(context);
    sdCtx->Finalize(err);
};

CHIP_ERROR Register(void * context, DnssdPublishCallback callback, uint32_t interfaceId, const char * type, const char * name,
                    uint16_t port, ScopedTXTRecord & record, Inet::IPAddressType addressType, const char * hostname)
{
    ChipLogDetail(Discovery, "Registering service %s on host %s with port %u and type: %s on interface id: %" PRIu32, name,
                  hostname, port, type, interfaceId);

    RegisterContext * sdCtx = nullptr;
    if (CHIP_NO_ERROR == MdnsContexts::GetInstance().GetRegisterContextOfType(type, &sdCtx))
    {
        auto err = DNSServiceUpdateRecord(sdCtx->serviceRef, nullptr, kRegisterFlags, record.size(), record.data(), 0 /* ttl */);
        VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
        return CHIP_NO_ERROR;
    }

    sdCtx = chip::Platform::New<RegisterContext>(type, name, callback, context);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    DNSServiceRef sdRef;
    auto err = DNSServiceRegister(&sdRef, kRegisterFlags, interfaceId, name, type, kLocalDot, hostname, htons(port), record.size(),
                                  record.data(), OnRegister, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

static void mdnsd_entry(void * not_used)
{
    ChipLogProgress(ServiceProvisioning, "mdnsd_entry start");
    mdnsd_start();
    ChipLogProgress(ServiceProvisioning, "mdnsd_entry return");
    vTaskDelete(NULL);
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
#if LWIP_IPV6
    struct netif * sta_if = netif_default;
    ip6_addr_t mld_address;
#endif

#if LWIP_IPV6
    sta_if->ip6_autoconfig_enabled = 1;
    ip6_addr_set_solicitednode(&mld_address, netif_ip6_addr(sta_if, 0)->addr[3]);
    mld6_joingroup(netif_ip6_addr(sta_if, 0), &mld_address);
#endif

    ChipLogProgress(ServiceProvisioning, "create mdnsd_task");

    mDNSPlatformWriteLogRedirect(ChipDnssdMdnsLog);

    // xTaskHandle create mDNS daemon task
    if (pdPASS != xTaskCreate(mdnsd_entry, "mdnsd", (15 * 1024) / sizeof(portSTACK_TYPE), NULL, TASK_PRIORITY_NORMAL, NULL))
    {
        ChipLogProgress(ServiceProvisioning, "Cannot create mdnsd_task");
        error = CHIP_ERROR_INTERNAL;
    }

    initCallback(context, error);

    return error;
}

void ChipDnssdShutdown(void)
{
    ChipLogProgress(ServiceProvisioning, "shutdown mdnsd_task not implemented");
}

static const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strcmp(service->mHostName, "") != 0, CHIP_ERROR_INVALID_ARGUMENT);

    ScopedTXTRecord record;
    ReturnErrorOnFailure(record.Init(service->mTextEntries, service->mTextEntrySize));

    auto regtype     = GetFullTypeWithSubTypes(service);
    auto interfaceId = GetInterfaceId(service->mInterface);
    auto hostname    = GetHostNameWithDomain(service->mHostName);

    return Register(context, callback, interfaceId, regtype.c_str(), service->mName, service->mPort, record, service->mAddressType,
                    hostname.c_str());
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    auto err = MdnsContexts::GetInstance().RemoveAllOfType(ContextType::Register);
    if (CHIP_ERROR_KEY_NOT_FOUND == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

void ChipDNSServiceBrowseReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                               const char * serviceName, const char * regtype, const char * replyDomain, void * context)
{
    DnssdBrowseCallback ChipBrowseHandler = (DnssdBrowseCallback) context;
    DnssdService service;

    ChipLogProgress(ServiceProvisioning, "ChipDNSServiceBrowseReply %s", StringOrNullMarker(serviceName));
    strcpy(service.mName, serviceName);

    ChipBrowseHandler(NULL, &service, 1, true, CHIP_NO_ERROR);
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    DNSServiceErrorType err;
    char ServiceType[kDnssdTypeMaxSize + 10] = { 0 };

    (void) addressType;
    ChipLogProgress(ServiceProvisioning, "ChipDnssdBrowse %s", StringOrNullMarker(type));
    strcpy(ServiceType, type);
    strcat(ServiceType, ".");
    strcat(ServiceType, GetProtocolString(protocol));
    err = DNSServiceBrowse(&BrowseClient, 0, 0, ServiceType, SERVICE_DOMAIN, ChipDNSServiceBrowseReply, (void *) callback);
    ChipLogProgress(ServiceProvisioning, "DNSServiceBrowse %d", (int) err);
    if (err)
    {
        error = CHIP_ERROR_INTERNAL;
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

static void resolve_client_task(void * parameter)
{
    EventBits_t uxBits;
    struct timeval tv;
    int result;
    int fd;
    fd_set readfds;

    while (1)
    {
        int nfds = 0;
        FD_ZERO(&readfds);
        nfds = MdnsContexts::GetInstance().GetSelectFd(&readfds);

        if (nfds == 0)
        {
            uxBits = xEventGroupWaitBits(gResolveTaskWakeEvent, 1, pdTRUE, pdFALSE, portMAX_DELAY);
            continue;
        }

        tv.tv_sec  = 0;
        tv.tv_usec = 100000; // 100ms

        result = select(nfds + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &tv);
        if (result > 0)
        {
            DNSServiceErrorType error = kDNSServiceErr_NoError;

            for (fd = 0; fd <= nfds; ++fd)
            {
                if (FD_ISSET(fd, &readfds))
                {
                    DNSServiceRef resolveClient = NULL;
                    void * context              = MdnsContexts::GetInstance().GetBySockFd(fd);
                    if (context)
                    {
                        auto sdCtx = reinterpret_cast<ResolveContext *>(context);
                        error      = DNSServiceProcessResult(sdCtx->serviceRef);
                        {
                            mdnslogInfo("DNSServiceProcessResult(%d) returned %d\n", fd, error);
                        }
                    } // if context
                }
            } // for
        }
        else
        {
            // remove timeout contexts
            for (fd = 0; fd <= nfds; ++fd)
            {
                GenericContext * context = MdnsContexts::GetInstance().GetBySockFd(fd);
                if (context && context->mSelectCount > 10)
                {
                    context->Finalize(kDNSServiceErr_Timeout);
                }
            }
        }
    } // while

    gResolveTask = NULL;
    vTaskDelete(NULL);
}

static void OnGetAddrInfo(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                          const char * hostname, const struct sockaddr * address, uint32_t ttl, void * context)
{
    ChipLogProgress(ServiceProvisioning, "DNSServiceGetAddrInfo: OnGetAddrInfo");

    auto sdCtx = reinterpret_cast<ResolveContext *>(context);
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));

    if (kDNSServiceErr_NoError == err)
    {
        sdCtx->OnNewAddress(interfaceId, address);
    }

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        VerifyOrReturn(sdCtx->HasAddress(), sdCtx->Finalize(kDNSServiceErr_BadState));
        sdCtx->Finalize();
    }
}

static void GetAddrInfo(ResolveContext * sdCtx)
{
    ChipLogProgress(ServiceProvisioning, "ChipDNSServiceResolveReply: GetAddrInfo");

    int dns_fd;
    auto protocol = sdCtx->protocol;

    for (auto & interface : sdCtx->interfaces)
    {
        DNSServiceRef resolveClient = NULL;
        auto interfaceId            = interface.first;
        auto hostname               = interface.second.fullyQualifiedDomainName.c_str();

        DNSServiceRefDeallocate(sdCtx->serviceRef);

        auto err          = DNSServiceGetAddrInfo(&resolveClient, 0, interfaceId, protocol, hostname, OnGetAddrInfo, sdCtx);
        sdCtx->serviceRef = resolveClient;
        VerifyOrReturn(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
    }
}

void ChipDNSServiceResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                                const char * fullname, const char * hosttarget, uint16_t port,
                                /* In network byte order */ uint16_t txtLen, const unsigned char * txtRecord, void * context)
{
    ChipLogProgress(ServiceProvisioning, "ChipDNSServiceResolveReply");
    if (context)
    {
        auto sdCtx = reinterpret_cast<ResolveContext *>(context);

        sdCtx->OnNewInterface(interfaceIndex, fullname, hosttarget, port, txtLen, txtRecord);
        if (kDNSServiceInterfaceIndexLocalOnly == interfaceIndex)
        {
            sdCtx->OnNewLocalOnlyAddress();
            sdCtx->Finalize();
            return;
        }
        if (!(flags & kDNSServiceFlagsMoreComing))
        {
            VerifyOrReturn(sdCtx->HasInterface(), sdCtx->Finalize(kDNSServiceErr_BadState));
            GetAddrInfo(sdCtx);
        }
    }
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    DNSServiceRef resolveClient = NULL;
    CHIP_ERROR error            = CHIP_NO_ERROR;
    DNSServiceErrorType err;
    char ServiceType[kDnssdTypeMaxSize + 10] = { 0 };
    int dns_fd;
    uint32_t interfaceIndex = GetInterfaceId(interface);

    ChipLogProgress(ServiceProvisioning, "ChipDnssdResolve %s", service->mName);
    strcpy(ServiceType, service->mType);
    strcat(ServiceType, ".");
    strcat(ServiceType, GetProtocolString(service->mProtocol));

    auto sdCtx = chip::Platform::New<ResolveContext>(context, callback, service->mAddressType);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    err = DNSServiceResolve(&resolveClient, 0, interfaceIndex, service->mName, ServiceType, SERVICE_DOMAIN,
                            ChipDNSServiceResolveReply, sdCtx);
    ChipLogProgress(ServiceProvisioning, "DNSServiceResolve %d", (int) err);
    if (err)
    {
        return CHIP_ERROR_INTERNAL;
    }

    sdCtx->serviceRef = resolveClient;
    error             = MdnsContexts::GetInstance().Add(sdCtx, sdCtx->serviceRef);
    if (error == CHIP_NO_ERROR)
    {
        if (gResolveTask == NULL)
        {
            gResolveTaskWakeEvent = xEventGroupCreate();
            if (gResolveTaskWakeEvent == NULL)
            {
                ChipLogProgress(ServiceProvisioning, "Cannot create resolve_task event group");
                return CHIP_ERROR_INTERNAL;
            }

            if (pdPASS !=
                xTaskCreate(resolve_client_task, "mdnsr", (15 * 1024) / sizeof(portSTACK_TYPE), (void *) service,
                            TASK_PRIORITY_NORMAL, &gResolveTask))
            {
                ChipLogProgress(ServiceProvisioning, "Cannot create resolve_task");
                return CHIP_ERROR_INTERNAL;
            }
        }
        else
        {
            // wake up task
            xEventGroupSetBits(gResolveTaskWakeEvent, 1);
        }
    }

    return error;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName) {}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
