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
    return interfaceId.IsPresent() ? interfaceId.GetPlatformInterface() : kDNSServiceInterfaceIndexAny;
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

MdnsContexts MdnsContexts::sInstance;

namespace {

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    ChipLogDetail(Discovery, "Mdns: %s name: %s, type: %s, domain: %s, flags: %d", __func__, name, type, domain, flags);

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

    sdCtx->mHostNameRegistrar.Init(hostname, addressType, interfaceId);

    DNSServiceRef sdRef;
    auto err = DNSServiceRegister(&sdRef, kRegisterFlags, interfaceId, name, type, kLocalDot, hostname, ntohs(port), record.size(),
                                  record.data(), OnRegister, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

void OnBrowseAdd(BrowseContext * context, const char * name, const char * type, const char * domain, uint32_t interfaceId)
{
    ChipLogDetail(Discovery, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %" PRIu32, __func__, name, type, domain,
                  interfaceId);

    VerifyOrReturn(strcmp(kLocalDot, domain) == 0);

    DnssdService service = {};
    service.mInterface   = Inet::InterfaceId(interfaceId);
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

void OnBrowseRemove(BrowseContext * context, const char * name, const char * type, const char * domain, uint32_t interfaceId)
{
    ChipLogDetail(Discovery, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %" PRIu32, __func__, name, type, domain,
                  interfaceId);

    VerifyOrReturn(strcmp(kLocalDot, domain) == 0);

    context->services.erase(std::remove_if(context->services.begin(), context->services.end(),
                                           [name, type, interfaceId](const DnssdService & service) {
                                               return strcmp(name, service.mName) == 0 && type == GetFullType(&service) &&
                                                   service.mInterface == Inet::InterfaceId(interfaceId);
                                           }),
                            context->services.end());
}

static void OnBrowse(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err, const char * name,
                     const char * type, const char * domain, void * context)
{
    auto sdCtx = reinterpret_cast<BrowseContext *>(context);
    VerifyOrReturn(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    (flags & kDNSServiceFlagsAdd) ? OnBrowseAdd(sdCtx, name, type, domain, interfaceId)
                                  : OnBrowseRemove(sdCtx, name, type, domain, interfaceId);

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        sdCtx->DispatchPartialSuccess();
    }
}

CHIP_ERROR Browse(void * context, DnssdBrowseCallback callback, uint32_t interfaceId, const char * type,
                  DnssdServiceProtocol protocol, intptr_t * browseIdentifier)
{
    auto sdCtx = chip::Platform::New<BrowseContext>(context, callback, protocol);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    ChipLogProgress(Discovery, "Browsing for: %s", type);
    DNSServiceRef sdRef;
    auto err = DNSServiceBrowse(&sdRef, kBrowseFlags, interfaceId, type, kLocalDot, OnBrowse, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    ReturnErrorOnFailure(MdnsContexts::GetInstance().Add(sdCtx, sdRef));
    *browseIdentifier = reinterpret_cast<intptr_t>(sdCtx);
    return CHIP_NO_ERROR;
}

static void OnGetAddrInfo(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                          const char * hostname, const struct sockaddr * address, uint32_t ttl, void * context)
{
    auto sdCtx = reinterpret_cast<ResolveContext *>(context);
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
    LogOnFailure(__func__, err);

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
    auto protocol = sdCtx->protocol;

    for (auto & interface : sdCtx->interfaces)
    {
        auto interfaceId = interface.first;
        auto hostname    = interface.second.fullyQualifiedDomainName.c_str();
        auto sdRefCopy   = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection
        auto err = DNSServiceGetAddrInfo(&sdRefCopy, kGetAddrInfoFlags, interfaceId, protocol, hostname, OnGetAddrInfo, sdCtx);
        VerifyOrReturn(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
    }
}

static void OnResolve(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                      const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen, const unsigned char * txtRecord,
                      void * context)
{
    auto sdCtx = reinterpret_cast<ResolveContext *>(context);
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
    LogOnFailure(__func__, err);

    if (kDNSServiceErr_NoError == err)
    {
        sdCtx->OnNewInterface(interfaceId, fullname, hostname, port, txtLen, txtRecord);
        if (kDNSServiceInterfaceIndexLocalOnly == interfaceId)
        {
            sdCtx->OnNewLocalOnlyAddress();
            sdCtx->Finalize();
            return;
        }
    }

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        VerifyOrReturn(sdCtx->HasInterface(), sdCtx->Finalize(kDNSServiceErr_BadState));
        GetAddrInfo(sdCtx);
    }
}

static CHIP_ERROR Resolve(void * context, DnssdResolveCallback callback, uint32_t interfaceId,
                          chip::Inet::IPAddressType addressType, const char * type, const char * name)
{
    ChipLogDetail(Discovery, "Resolve type=%s name=%s interface=%" PRIu32, type, name, interfaceId);

    auto sdCtx = chip::Platform::New<ResolveContext>(context, callback, addressType);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    auto err = DNSServiceCreateConnection(&sdCtx->serviceRef);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    auto sdRefCopy = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection
    err            = DNSServiceResolve(&sdRefCopy, kResolveFlags, interfaceId, name, type, kLocalDot, OnResolve, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    return MdnsContexts::GetInstance().Add(sdCtx, sdCtx->serviceRef);
}

} // namespace

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback successCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(successCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    successCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown() {}

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

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);

    auto regtype     = GetFullTypeWithSubTypes(type, protocol);
    auto interfaceId = GetInterfaceId(interface);
    return Browse(context, callback, interfaceId, regtype.c_str(), protocol, browseIdentifier);
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    auto ctx = reinterpret_cast<GenericContext *>(browseIdentifier);
    if (MdnsContexts::GetInstance().Has(ctx) != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // We know this is an actual context now, so can check the type.
    if (ctx->type != ContextType::Browse)
    {
        // stale pointer that got reallocated.
        return CHIP_ERROR_NOT_FOUND;
    }

    // Just treat this as a timeout error.  Don't bother delivering the partial
    // results we have queued up in the BrowseContext, if any.  In practice
    // there shouldn't be anything there long-term anyway.
    auto browseCtx = static_cast<BrowseContext *>(ctx);
    browseCtx->Finalize(kDNSServiceErr_Timeout);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    auto regtype     = GetFullType(service);
    auto interfaceId = GetInterfaceId(interface);
    return Resolve(context, callback, interfaceId, service->mAddressType, regtype.c_str(), service->mName);
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    VerifyOrReturnError(hostname != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto interfaceId = interface.GetPlatformInterface();
    auto rrclass     = kDNSServiceClass_IN;
    auto fullname    = GetHostNameWithDomain(hostname);

    uint16_t rrtype;
    uint16_t rdlen;
    const void * rdata;

    in6_addr ipv6;
#if INET_CONFIG_ENABLE_IPV4
    in_addr ipv4;
#endif // INET_CONFIG_ENABLE_IPV4

    if (address.IsIPv6())
    {
        ipv6   = address.ToIPv6();
        rrtype = kDNSServiceType_AAAA;
        rdlen  = static_cast<uint16_t>(sizeof(in6_addr));
        rdata  = &ipv6;
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (address.IsIPv4())
    {
        ipv4   = address.ToIPv4();
        rrtype = kDNSServiceType_A;
        rdlen  = static_cast<uint16_t>(sizeof(in_addr));
        rdata  = &ipv4;
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto error = DNSServiceReconfirmRecord(kReconfirmRecordFlags, interfaceId, fullname.c_str(), rrtype, rrclass, rdlen, rdata);
    LogOnFailure(__func__, error);

    return Error::ToChipError(error);
}

} // namespace Dnssd
} // namespace chip
