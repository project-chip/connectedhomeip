/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include "DnssdType.h"
#include "MdnsError.h"

#include <cstdio>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Dnssd;
using namespace chip::Dnssd::Internal;

namespace {

constexpr char kLocalDot[] = "local.";

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

std::string GetHostNameWithDomain(const char * hostname)
{
    return std::string(hostname) + '.' + kLocalDot;
}

void LogOnFailure(const char * name, DNSServiceErrorType err)
{
    if (kDNSServiceErr_NoError != err)
    {
        ChipLogError(Discovery, "%s (%s)", StringOrNullMarker(name), Error::ToString(err));
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

std::shared_ptr<uint32_t> GetCounterHolder(const char * name)
{
    // This is a little silly, in that resolves for the same name, type, etc get
    // coalesced by the underlying mDNSResponder anyway.  But we need to keep
    // track of our context/callback/etc, (even though in practice it's always
    // exactly the same) and the interface id (which might actually be different
    // for different Resolve calls). So for now just keep using a
    // ResolveContext to track all that.
    if (auto existingCtx = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(name))
    {
        return existingCtx->consumerCounter;
    }
    return std::make_shared<uint32_t>(0);
}

} // namespace

namespace chip {
namespace Dnssd {

Global<MdnsContexts> MdnsContexts::sInstance;

namespace {

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    ChipLogProgress(Discovery, "Mdns: %s name: %s, type: %s, domain: %s, flags: %d", __func__, StringOrNullMarker(name),
                    StringOrNullMarker(type), StringOrNullMarker(domain), flags);

    auto sdCtx = reinterpret_cast<RegisterContext *>(context);
    sdCtx->Finalize(err);
};

CHIP_ERROR Register(void * context, DnssdPublishCallback callback, uint32_t interfaceId, const char * type, const char * name,
                    uint16_t port, ScopedTXTRecord & record, Inet::IPAddressType addressType, const char * hostname)
{
    ChipLogProgress(Discovery, "Registering service %s on host %s with port %u and type: %s on interface id: %" PRIu32,
                    StringOrNullMarker(name), StringOrNullMarker(hostname), port, StringOrNullMarker(type), interfaceId);

    RegisterContext * sdCtx = nullptr;
    if (CHIP_NO_ERROR == MdnsContexts::GetInstance().GetRegisterContextOfTypeAndName(type, name, &sdCtx))
    {
        auto err = DNSServiceUpdateRecord(sdCtx->serviceRef, nullptr, kRegisterFlags, record.size(), record.data(), 0 /* ttl */);
        VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
        return CHIP_NO_ERROR;
    }

    sdCtx = chip::Platform::New<RegisterContext>(type, name, callback, context);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    auto err = sdCtx->mHostNameRegistrar.Init(hostname, addressType, interfaceId);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    DNSServiceRef sdRef;
    err = DNSServiceRegister(&sdRef, kRegisterFlags, interfaceId, name, type, kLocalDot, hostname, htons(port), record.size(),
                             record.data(), OnRegister, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

static void OnBrowse(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err, const char * name,
                     const char * type, const char * domain, void * context)
{
    auto sdCtx = reinterpret_cast<BrowseHandler *>(context);
    VerifyOrReturn(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
    sdCtx->OnBrowse(flags, name, type, domain, interfaceId);
}

CHIP_ERROR Browse(BrowseHandler * sdCtx, uint32_t interfaceId, const char * type)
{
    ChipLogProgress(Discovery, "Browsing for: %s", StringOrNullMarker(type));
    DNSServiceRef sdRef;
    auto err = DNSServiceBrowse(&sdRef, kBrowseFlags, interfaceId, type, kLocalDot, OnBrowse, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

CHIP_ERROR Browse(void * context, DnssdBrowseCallback callback, uint32_t interfaceId, const char * type,
                  DnssdServiceProtocol protocol, intptr_t * browseIdentifier)
{
    auto sdCtx = chip::Platform::New<BrowseContext>(context, callback, protocol);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(Browse(sdCtx, interfaceId, type));
    *browseIdentifier = reinterpret_cast<intptr_t>(sdCtx);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Browse(DnssdBrowseDelegate * delegate, uint32_t interfaceId, const char * type, DnssdServiceProtocol protocol)
{
    auto sdCtx = chip::Platform::New<BrowseWithDelegateContext>(delegate, protocol);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    return Browse(sdCtx, interfaceId, type);
}

static void OnGetAddrInfo(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                          const char * hostname, const struct sockaddr * address, uint32_t ttl, void * context)
{
    ChipLogProgress(Discovery, "Mdns: %s flags: %d, interface: %u, hostname: %s", __func__, flags, (unsigned) interfaceId,
                    StringOrNullMarker(hostname));
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
    ChipLogProgress(Discovery, "Mdns: %s flags: %d, interface: %u, fullname: %s, hostname: %s, port: %u", __func__, flags,
                    (unsigned) interfaceId, StringOrNullMarker(fullname), StringOrNullMarker(hostname), ntohs(port));
    auto sdCtx = reinterpret_cast<ResolveContext *>(context);
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
    LogOnFailure(__func__, err);

    if (kDNSServiceErr_NoError == err)
    {
        sdCtx->OnNewInterface(interfaceId, fullname, hostname, port, txtLen, txtRecord);
    }

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        VerifyOrReturn(sdCtx->HasInterface(), sdCtx->Finalize(kDNSServiceErr_BadState));
        GetAddrInfo(sdCtx);
    }
}

static CHIP_ERROR Resolve(ResolveContext * sdCtx, uint32_t interfaceId, chip::Inet::IPAddressType addressType, const char * type,
                          const char * name)
{
    ChipLogProgress(Discovery, "Resolve type=%s name=%s interface=%" PRIu32, StringOrNullMarker(type), StringOrNullMarker(name),
                    interfaceId);

    auto err = DNSServiceCreateConnection(&sdCtx->serviceRef);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    auto sdRefCopy = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection
    err            = DNSServiceResolve(&sdRefCopy, kResolveFlags, interfaceId, name, type, kLocalDot, OnResolve, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    auto retval = MdnsContexts::GetInstance().Add(sdCtx, sdCtx->serviceRef);
    if (retval == CHIP_NO_ERROR)
    {
        (*(sdCtx->consumerCounter))++;
    }
    return retval;
}

static CHIP_ERROR Resolve(void * context, DnssdResolveCallback callback, uint32_t interfaceId,
                          chip::Inet::IPAddressType addressType, const char * type, const char * name)
{
    auto counterHolder = GetCounterHolder(name);
    auto sdCtx         = chip::Platform::New<ResolveContext>(context, callback, addressType, name,
                                                     BrowseContext::sContextDispatchingSuccess, std::move(counterHolder));
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    return Resolve(sdCtx, interfaceId, addressType, type, name);
}

static CHIP_ERROR Resolve(CommissioningResolveDelegate * delegate, uint32_t interfaceId, chip::Inet::IPAddressType addressType,
                          const char * type, const char * name)
{
    auto counterHolder = GetCounterHolder(name);
    auto sdCtx         = chip::Platform::New<ResolveContext>(delegate, addressType, name, std::move(counterHolder));
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    return Resolve(sdCtx, interfaceId, addressType, type, name);
}

} // namespace

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback successCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(successCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    successCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown()
{
    // Drop our existing advertisements now, so they don't stick around while we
    // are not actually in a responsive state.
    ChipDnssdRemoveServices();
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
    assertChipStackLockedByCurrentThread();

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

    // We have been canceled.  Don't bother delivering the partial
    // results we have queued up in the BrowseContext, if any.  In practice
    // there shouldn't be anything there long-term anyway.
    //
    // Make sure to cancel all the resolves first, before we cancel the
    // browse (just to avoid dangling pointers in the resolves, even though we
    // only use them for equality compares).
    std::vector<GenericContext *> resolves;
    MdnsContexts::GetInstance().FindAllMatchingPredicate(
        [ctx](GenericContext * item) {
            return item->type == ContextType::Resolve && static_cast<ResolveContext *>(item)->browseThatCausedResolve == ctx;
        },
        resolves);

    for (auto & resolve : resolves)
    {
        resolve->Finalize(CHIP_ERROR_CANCELLED);
    }

    ctx->Finalize(CHIP_ERROR_CANCELLED);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseDelegate * delegate)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);

    auto regtype     = GetFullTypeWithSubTypes(type, protocol);
    auto interfaceId = GetInterfaceId(interface);
    return Browse(delegate, interfaceId, regtype.c_str(), protocol);
}

CHIP_ERROR ChipDnssdStopBrowse(DnssdBrowseDelegate * delegate)
{
    auto existingCtx = MdnsContexts::GetInstance().GetExistingBrowseForDelegate(delegate);
    if (existingCtx == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return existingCtx->Finalize(kDNSServiceErr_NoError);
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

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, CommissioningResolveDelegate * delegate)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    auto regtype     = GetFullType(service);
    auto interfaceId = GetInterfaceId(interface);
    return Resolve(delegate, interfaceId, service->mAddressType, regtype.c_str(), service->mName);
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName)
{
    ChipLogProgress(Discovery, "No longer need resolve for %s", instanceName);
    auto existingCtx = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(instanceName);
    VerifyOrReturn(existingCtx != nullptr);
    VerifyOrReturn(*existingCtx->consumerCounter != 0);

    (*existingCtx->consumerCounter)--;

    if (*existingCtx->consumerCounter == 0)
    {
        // No more consumers; clear out all of these resolves so they don't
        // stick around.  Dispatch a "cancelled" failure on all of them to make
        // sure whatever kicked them off cleans up resources as needed.
        do
        {
            existingCtx->Finalize(CHIP_ERROR_CANCELLED);
            existingCtx = MdnsContexts::GetInstance().GetExistingResolveForInstanceName(instanceName);
        } while (existingCtx != nullptr);
    }
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
