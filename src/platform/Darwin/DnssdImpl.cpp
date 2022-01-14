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
#include "DnssdImpl.h"

#include "MdnsError.h"

#include <cstdio>
#include <sstream>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Dnssd;

namespace {

constexpr const char * kLocalDot    = "local.";
constexpr const char * kProtocolTcp = "._tcp";
constexpr const char * kProtocolUdp = "._udp";
constexpr uint8_t kDnssdKeyMaxSize  = 32;

bool IsSupportedProtocol(DnssdServiceProtocol protocol)
{
    return (protocol == DnssdServiceProtocol::kDnssdProtocolUdp) || (protocol == DnssdServiceProtocol::kDnssdProtocolTcp);
}

uint32_t GetInterfaceId(chip::Inet::InterfaceId interfaceId)
{
    return interfaceId.IsPresent() ? interfaceId.GetPlatformInterface() : kDNSServiceInterfaceIndexAny;
}

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kProtocolUdp : kProtocolTcp);
    return typeBuilder.str();
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

} // namespace

namespace chip {
namespace Dnssd {

MdnsContexts MdnsContexts::sInstance;

void MdnsContexts::Delete(GenericContext * context)
{
    if (context->type == ContextType::GetAddrInfo)
    {
        GetAddrInfoContext * addrInfoContext = reinterpret_cast<GetAddrInfoContext *>(context);
        std::vector<TextEntry>::iterator textEntry;
        for (textEntry = addrInfoContext->textEntries.begin(); textEntry != addrInfoContext->textEntries.end(); textEntry++)
        {
            free(const_cast<char *>(textEntry->mKey));
            free(const_cast<uint8_t *>(textEntry->mData));
        }
    }

    if (context->serviceRef != nullptr)
    {
        DNSServiceRefDeallocate(context->serviceRef);
    }
    chip::Platform::Delete(context);
}

MdnsContexts::~MdnsContexts()
{
    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        Delete(*iter);
        mContexts.erase(iter);
    }
}

CHIP_ERROR MdnsContexts::Add(GenericContext * context, DNSServiceRef sdRef)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sdRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    context->serviceRef = sdRef;
    mContexts.push_back(context);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MdnsContexts::Remove(GenericContext * context)
{
    bool found = false;

    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if (*iter != context)
        {
            iter++;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
        break;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::Removes(ContextType type)
{
    bool found = false;

    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if ((*iter)->type != type)
        {
            iter++;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::Get(ContextType type, GenericContext ** context)
{
    bool found = false;
    std::vector<GenericContext *>::iterator iter;

    for (iter = mContexts.begin(); iter != mContexts.end(); iter++)
    {
        if ((*iter)->type == type)
        {
            *context = *iter;
            found    = true;
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::GetRegisterType(const char * type, GenericContext ** context)
{
    bool found = false;
    std::vector<GenericContext *>::iterator iter;

    for (iter = mContexts.begin(); iter != mContexts.end(); iter++)
    {
        if ((*iter)->type == ContextType::Register && ((RegisterContext *) (*iter))->matches(type))
        {
            *context = *iter;
            found    = true;
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

void MdnsContexts::PrepareSelect(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout) {}

void MdnsContexts::HandleSelectResult(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet) {}

CHIP_ERROR PopulateTextRecord(TXTRecordRef * record, char * buffer, uint16_t bufferLen, TextEntry * textEntries,
                              size_t textEntrySize)
{
    VerifyOrReturnError(textEntrySize <= kDnssdTextMaxSize, CHIP_ERROR_INVALID_ARGUMENT);

    DNSServiceErrorType err;
    TXTRecordCreate(record, bufferLen, buffer);

    for (size_t i = 0; i < textEntrySize; i++)
    {
        TextEntry entry = textEntries[i];
        VerifyOrReturnError(chip::CanCastTo<uint8_t>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);

        err = TXTRecordSetValue(record, entry.mKey, static_cast<uint8_t>(entry.mDataSize), entry.mData);
        VerifyOrReturnError(err == kDNSServiceErr_NoError, CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CHIP_NO_ERROR;
}

bool CheckForSuccess(GenericContext * context, const char * name, DNSServiceErrorType err, bool useCallback = false)
{
    if (context == nullptr)
    {
        ChipLogError(DeviceLayer, "%s (%s)", name, "Mdns context is null.");
        return false;
    }

    if (kDNSServiceErr_NoError != err)
    {
        ChipLogError(DeviceLayer, "%s (%s)", name, Error::ToString(err));

        if (useCallback)
        {
            switch (context->type)
            {
            case ContextType::Register:
                // Nothing special to do. Maybe ChipDnssdPublishService should take a callback ?
                break;
            case ContextType::Browse: {
                BrowseContext * browseContext = reinterpret_cast<BrowseContext *>(context);
                browseContext->callback(browseContext->context, nullptr, 0, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::Resolve: {
                ResolveContext * resolveContext = reinterpret_cast<ResolveContext *>(context);
                resolveContext->callback(resolveContext->context, nullptr, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::GetAddrInfo: {
                GetAddrInfoContext * resolveContext = reinterpret_cast<GetAddrInfoContext *>(context);
                resolveContext->callback(resolveContext->context, nullptr, CHIP_ERROR_INTERNAL);
                break;
            }
            }
        }

        if (CHIP_ERROR_KEY_NOT_FOUND == MdnsContexts::GetInstance().Remove(context))
        {
            chip::Platform::Delete(context);
        }

        return false;
    }

    return true;
}

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    RegisterContext * sdCtx = reinterpret_cast<RegisterContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err));

    ChipLogDetail(DeviceLayer, "Mdns: %s name: %s, type: %s, domain: %s, flags: %d", __func__, name, type, domain, flags);
};

CHIP_ERROR Register(uint32_t interfaceId, const char * type, const char * name, uint16_t port, TXTRecordRef * recordRef)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    GenericContext * sdCtx = nullptr;

    uint16_t recordLen          = TXTRecordGetLength(recordRef);
    const void * recordBytesPtr = TXTRecordGetBytesPtr(recordRef);

    if (CHIP_NO_ERROR == MdnsContexts::GetInstance().GetRegisterType(type, &sdCtx))
    {
        err = DNSServiceUpdateRecord(sdCtx->serviceRef, NULL, 0 /* flags */, recordLen, recordBytesPtr, 0 /* ttl */);
        TXTRecordDeallocate(recordRef);
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);
        return CHIP_NO_ERROR;
    }

    sdCtx = chip::Platform::New<RegisterContext>(type, nullptr);
    err   = DNSServiceRegister(&sdRef, 0 /* flags */, interfaceId, name, type, kLocalDot, NULL, ntohs(port), recordLen,
                             recordBytesPtr, OnRegister, sdCtx);
    TXTRecordDeallocate(recordRef);

    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    err = DNSServiceSetDispatchQueue(sdRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

void OnBrowseAdd(BrowseContext * context, const char * name, const char * type, const char * domain,
                 chip::Inet::InterfaceId interfaceId)
{
    ChipLogDetail(DeviceLayer, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %d", __func__, name, type, domain,
                  interfaceId.GetPlatformInterface());

    VerifyOrReturn(strcmp(kLocalDot, domain) == 0);

    DnssdService service = {};
    service.mInterface   = interfaceId;
    service.mProtocol    = context->protocol;

    Platform::CopyString(service.mName, name);
    Platform::CopyString(service.mType, type);

    // only the first token after '.' should be included in the type
    for (char * p = service.mType; *p != '\0'; p++)
    {
        if (*p == '.')
        {
            *p = '\0';
            break;
        }
    }

    context->services.push_back(service);
}

void OnBrowseRemove(BrowseContext * context, const char * name, const char * type, const char * domain,
                    chip::Inet::InterfaceId interfaceId)
{
    ChipLogDetail(DeviceLayer, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %d", __func__, name, type, domain,
                  interfaceId.GetPlatformInterface());

    VerifyOrReturn(strcmp(kLocalDot, domain) == 0);

    std::remove_if(context->services.begin(), context->services.end(), [name, type, interfaceId](const DnssdService & service) {
        return strcmp(name, service.mName) == 0 && type == GetFullType(service.mType, service.mProtocol) &&
            service.mInterface == interfaceId;
    });
}

static void OnBrowse(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err, const char * name,
                     const char * type, const char * domain, void * context)
{
    BrowseContext * sdCtx = reinterpret_cast<BrowseContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    (flags & kDNSServiceFlagsAdd) ? OnBrowseAdd(sdCtx, name, type, domain, Inet::InterfaceId(interfaceId))
                                  : OnBrowseRemove(sdCtx, name, type, domain, Inet::InterfaceId(interfaceId));

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        sdCtx->callback(sdCtx->context, sdCtx->services.data(), sdCtx->services.size(), CHIP_NO_ERROR);
        MdnsContexts::GetInstance().Remove(sdCtx);
    }
}

CHIP_ERROR Browse(void * context, DnssdBrowseCallback callback, uint32_t interfaceId, const char * type,
                  DnssdServiceProtocol protocol)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    BrowseContext * sdCtx;

    std::string regtype(type);
    std::string subtypeDelimiter = "._sub.";
    size_t position              = regtype.find(subtypeDelimiter);
    if (position != std::string::npos)
    {
        regtype = regtype.substr(position + subtypeDelimiter.size()) + "," + regtype.substr(0, position);
    }

    sdCtx = chip::Platform::New<BrowseContext>(context, callback, protocol);
    err   = DNSServiceBrowse(&sdRef, 0 /* flags */, interfaceId, regtype.c_str(), kLocalDot, OnBrowse, sdCtx);
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    err = DNSServiceSetDispatchQueue(sdRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

static void OnGetAddrInfo(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                          const char * hostname, const struct sockaddr * address, uint32_t ttl, void * context)
{
    GetAddrInfoContext * sdCtx = reinterpret_cast<GetAddrInfoContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    ChipLogDetail(DeviceLayer, "Mdns: %s hostname:%s", __func__, hostname);

    DnssdService service   = {};
    service.mPort          = sdCtx->port;
    service.mTextEntries   = sdCtx->textEntries.empty() ? nullptr : sdCtx->textEntries.data();
    service.mTextEntrySize = sdCtx->textEntries.empty() ? 0 : sdCtx->textEntries.size();
    chip::Inet::IPAddress ip;
    CHIP_ERROR status = chip::Inet::IPAddress::GetIPAddressFromSockAddr(*address, ip);
    service.mAddress.SetValue(ip);
    Platform::CopyString(service.mName, sdCtx->name);
    Platform::CopyString(service.mHostName, hostname);
    service.mInterface = Inet::InterfaceId(sdCtx->interfaceId);

    sdCtx->callback(sdCtx->context, &service, status);
    MdnsContexts::GetInstance().Remove(sdCtx);
}

static CHIP_ERROR GetAddrInfo(void * context, DnssdResolveCallback callback, uint32_t interfaceId,
                              chip::Inet::IPAddressType addressType, const char * name, const char * hostname, uint16_t port,
                              uint16_t txtLen, const unsigned char * txtRecord)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    GetAddrInfoContext * sdCtx;

    sdCtx = chip::Platform::New<GetAddrInfoContext>(context, callback, name, interfaceId, port);

    char key[kDnssdKeyMaxSize];
    char value[kDnssdTextMaxSize];
    uint8_t valueLen;
    const void * valuePtr;

    uint16_t recordCount = TXTRecordGetCount(txtLen, txtRecord);
    for (uint16_t i = 0; i < recordCount; i++)
    {
        err = TXTRecordGetItemAtIndex(txtLen, txtRecord, i, kDnssdKeyMaxSize, key, &valueLen, &valuePtr);
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

        if (valueLen >= sizeof(value))
        {
            // Truncation, but nothing better we can do
            valueLen = sizeof(value) - 1;
        }
        memcpy(value, valuePtr, valueLen);
        value[valueLen] = 0;

        sdCtx->textEntries.push_back(TextEntry{ strdup(key), reinterpret_cast<const uint8_t *>(strdup(value)), valueLen });
    }

    DNSServiceProtocol protocol;

#if INET_CONFIG_ENABLE_IPV4
    if (addressType == chip::Inet::IPAddressType::kIPv4)
    {
        protocol = kDNSServiceProtocol_IPv4;
    }
    else if (addressType == chip::Inet::IPAddressType::kIPv6)
    {
        protocol = kDNSServiceProtocol_IPv6;
    }
    else
    {
        protocol = kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6;
    }
#else
    // without IPv4, IPv6 is the only option
    protocol = kDNSServiceProtocol_IPv6;
#endif

    if (interfaceId != kDNSServiceInterfaceIndexLocalOnly)
    {
        // -1 is the local only interface. If we're not on that interface, we need to get the address for the given hostname.
        err = DNSServiceGetAddrInfo(&sdRef, 0 /* flags */, interfaceId, protocol, hostname, OnGetAddrInfo, sdCtx);
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

        err = DNSServiceSetDispatchQueue(sdRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

        return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
    }
    else
    {
        sockaddr_in6 sockaddr;
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin6_len    = sizeof(sockaddr);
        sockaddr.sin6_family = AF_INET6;
        sockaddr.sin6_addr   = in6addr_loopback;
        sockaddr.sin6_port   = htons((unsigned short) port);
        uint32_t ttl         = 120; // default TTL for records with hostnames is 120 seconds
        uint32_t interface   = 0;   // Set interface to ANY (0) - network stack can decide how to route this.
        OnGetAddrInfo(nullptr, 0 /* flags */, interface, kDNSServiceErr_NoError, hostname,
                      reinterpret_cast<struct sockaddr *>(&sockaddr), ttl, sdCtx);

        // Don't leak memory.
        sdCtx->serviceRef = nullptr;
        MdnsContexts::GetInstance().Delete(sdCtx);
        return CHIP_NO_ERROR;
    }
}

static void OnResolve(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                      const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen, const unsigned char * txtRecord,
                      void * context)
{
    ChipLogDetail(DeviceLayer, "Resolved interface id: %u", interfaceId);

    ResolveContext * sdCtx = reinterpret_cast<ResolveContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    GetAddrInfo(sdCtx->context, sdCtx->callback, interfaceId, sdCtx->addressType, sdCtx->name, hostname, ntohs(port), txtLen,
                txtRecord);
    MdnsContexts::GetInstance().Remove(sdCtx);
}

static CHIP_ERROR Resolve(void * context, DnssdResolveCallback callback, uint32_t interfaceId,
                          chip::Inet::IPAddressType addressType, const char * type, const char * name)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    ResolveContext * sdCtx;

    sdCtx = chip::Platform::New<ResolveContext>(context, callback, name, addressType);
    err   = DNSServiceResolve(&sdRef, 0 /* flags */, interfaceId, name, type, kLocalDot, OnResolve, sdCtx);
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    err = DNSServiceSetDispatchQueue(sdRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback successCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(successCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    successCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdShutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(service->mHostName, "") != 0)
    {
        MdnsContexts::GetInstance().SetHostname(service->mHostName);
    }
    std::string regtype  = GetFullTypeWithSubTypes(service->mType, service->mProtocol, service->mSubTypes, service->mSubTypeSize);
    uint32_t interfaceId = GetInterfaceId(service->mInterface);

    TXTRecordRef record;
    char buffer[kDnssdTextMaxSize];
    ReturnErrorOnFailure(PopulateTextRecord(&record, buffer, sizeof(buffer), service->mTextEntries, service->mTextEntrySize));

    ChipLogProgress(DeviceLayer, "Publishing service %s on port %u with type: %s on interface id: %" PRIu32, service->mName,
                    service->mPort, regtype.c_str(), interfaceId);
    return Register(interfaceId, regtype.c_str(), service->mName, service->mPort, &record);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    GenericContext * sdCtx = nullptr;
    if (CHIP_ERROR_KEY_NOT_FOUND == MdnsContexts::GetInstance().Get(ContextType::Register, &sdCtx))
    {
        return CHIP_NO_ERROR;
    }

    return MdnsContexts::GetInstance().Removes(ContextType::Register);
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype  = GetFullType(type, protocol);
    uint32_t interfaceId = GetInterfaceId(interface);

    return Browse(context, callback, interfaceId, regtype.c_str(), protocol);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype  = GetFullType(service->mType, service->mProtocol);
    uint32_t interfaceId = GetInterfaceId(interface);

    return Resolve(context, callback, interfaceId, service->mAddressType, regtype.c_str(), service->mName);
}

} // namespace Dnssd
} // namespace chip
