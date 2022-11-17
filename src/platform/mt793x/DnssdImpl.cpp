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
#include "mDNSDebug.h"
#include "task.h"
#include "task_def.h"
#include <errno.h>

#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

extern "C" {
extern void mDNSPlatformWriteLogRedirect(void (*)(const char *, const char *));
}

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Dnssd {

#define SERVICE_DOMAIN ("local")

MdnsContexts MdnsContexts::sInstance;
static DNSServiceRef client = NULL;
static TXTRecordRef PublishTxtRecord;

void ChipDnssdMdnsLog(const char * level, const char * msg)
{
    ChipLogProgress(ServiceProvisioning, "%s %s", StringOrNullMarker(level), StringOrNullMarker(msg));
}

/**
 * @brief     mDNS Daemon Task entry
 * @param[in] void *not_used:Not used
 * @return    None
 */
static void mdnsd_entry(void * not_used)
{
    ChipLogProgress(ServiceProvisioning, "mdnsd_entry start");
    mdnsd_start();
    ChipLogProgress(ServiceProvisioning, "mdnsd_entry return");
    client = NULL;
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

#if 0
#define MDNS_STACK_SIZE ((64 * 1024) / sizeof(portSTACK_TYPE))
    static StackType_t          xMDnsStack[ MDNS_STACK_SIZE ];
    static StaticTask_t         xMDnsTask;

    // xTaskHandle create mDNS daemon task
    if ( NULL != xTaskCreateStatic( mdnsd_entry,
                                    "mdnsd",
                                    MDNS_STACK_SIZE,
                                    NULL,
                                    TASK_PRIORITY_NORMAL,
                                    &xMDnsStack[0],
                                    &xMDnsTask ) )
#else
    // xTaskHandle create mDNS daemon task
    if (pdPASS != xTaskCreate(mdnsd_entry, "mdnsd", (15 * 1024) / sizeof(portSTACK_TYPE), NULL, TASK_PRIORITY_NORMAL, NULL))
#endif
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
    CHIP_ERROR error = CHIP_NO_ERROR;
    DNSServiceErrorType err;
    DNSServiceFlags flags                    = 0;
    char ServiceType[kDnssdTypeMaxSize + 10] = { 0 };

    (void) callback;
    (void) context;

    ChipLogProgress(ServiceProvisioning, "ChipDnssdPublishService");

    strcpy(ServiceType, service->mType);
    strcat(ServiceType, ".");
    strcat(ServiceType, GetProtocolString(service->mProtocol));

    ChipLogProgress(ServiceProvisioning, "ServiceName:   %s", service->mName);
    ChipLogProgress(ServiceProvisioning, "ServiceType:   %s", ServiceType);
    ChipLogProgress(ServiceProvisioning, "ServiceDomain: %s", SERVICE_DOMAIN);
    ChipLogProgress(ServiceProvisioning, "Hostname:      %s", service->mHostName);
    ChipLogProgress(ServiceProvisioning, "ServicePort:   %d", (int) service->mPort);

    VerifyOrExit(service->mTextEntrySize <= UINT8_MAX, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (service->mTextEntries)
    {
        // Create TXT Record
        TXTRecordCreate(&PublishTxtRecord, 0, NULL);
        for (size_t i = 0; i < service->mTextEntrySize; i++)
        {
            ChipLogProgress(ServiceProvisioning, "service: key %s size %d data %s", service->mTextEntries[i].mKey,
                            service->mTextEntries[i].mDataSize, service->mTextEntries[i].mData);

            err = TXTRecordSetValue(&PublishTxtRecord, service->mTextEntries[i].mKey, service->mTextEntries[i].mDataSize,
                                    service->mTextEntries[i].mData);
            VerifyOrExit(err == 0, error = CHIP_ERROR_INTERNAL);
        }
    }

    if (client != NULL)
    {
        // ChipLogProgress(ServiceProvisioning, "ChipDnssdPublishService - DNSServiceRefDeallocate");
        // DNSServiceRefDeallocate(client);
        // client = NULL;
    }

    ChipLogProgress(ServiceProvisioning, "ChipDnssdPublishService - client %p", client);

    ChipLogProgress(ServiceProvisioning, "ChipDnssdPublishService - DNSServiceRegister");

    // Register Bonjour Service
    err = DNSServiceRegister(&client,                      // DNSServiceRef
                             flags,                        // DNSServiceFlags
                             kDNSServiceInterfaceIndexAny, // interface index
                             service->mName,               // service name
                             ServiceType,                  // service type
                             SERVICE_DOMAIN,               // domain
                             NULL,                         // host
                             // service->mHostName,                        // host
                             htons(service->mPort),                   // port
                             TXTRecordGetLength(&PublishTxtRecord),   // txt record length
                             TXTRecordGetBytesPtr(&PublishTxtRecord), // txt record pointer
                             NULL,                                    // callback
                             NULL);                                   // context
    VerifyOrExit(err == 0, error = CHIP_ERROR_INTERNAL);

exit:
    // PublishTxtRecord is static global, no need to free

    return error;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    ChipLogProgress(ServiceProvisioning, "ChipDnssdRemoveServices");
    TXTRecordDeallocate(&PublishTxtRecord);
    DNSServiceRefDeallocate(client);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

static DNSServiceRef BrowseClient = NULL;

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

static TaskHandle_t gResolveTask = NULL;
static EventGroupHandle_t gResolveTaskWakeEvent;

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

uint32_t GetInterfaceId(chip::Inet::InterfaceId interfaceId)
{
    return interfaceId.IsPresent() ? (uint32_t)(void *) interfaceId.GetPlatformInterface() : kDNSServiceInterfaceIndexAny;
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

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
