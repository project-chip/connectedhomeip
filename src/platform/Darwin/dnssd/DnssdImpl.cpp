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
#include "DnssdError.h"
#include "DnssdType.h"

#include <cstdio>

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/UserDefaults.h>

#include <atomic>
#include <string>

using namespace chip::Dnssd;
using namespace chip::Dnssd::Internal;
using namespace chip::Platform;

namespace {

constexpr char kLocalDot[] = "local.";

constexpr char kSRPDot[] = "default.service.arpa.";

// Default delay between consumerCounter dropping to 0 and the actual teardown
// of the underlying DNSServiceRef. This window exists so that any inbound
// resolve result that has already been written to the dnssd socket but not yet
// dispatched up to us is still consumed before DNSServiceRefDeallocate is
// called: per the dnssd contract, DNSServiceRefDeallocate discards any
// already-queued events on that connection. Without this window we hit a race
// where an upper-layer consumer cancels the query in the gap between a result
// being written to the socket and the read indicator firing, and the good
// resolve result is lost. ~500ms is well beyond the largest gap observed in
// the field while remaining short enough that an unused ResolveContext does
// not linger.
constexpr uint32_t kDefaultResolveDeferredTeardownDelayMs = 500;

// Stored as a raw millisecond count in a std::atomic<uint32_t> so the test
// thread can update the delay without racing reads on the Matter event-loop
// thread (TSAN would otherwise flag a data race on a plain non-atomic global).
std::atomic<uint32_t> gResolveDeferredTeardownDelayMs{ kDefaultResolveDeferredTeardownDelayMs };

bool IsSupportedProtocol(DnssdServiceProtocol protocol)
{
    return (protocol == DnssdServiceProtocol::kDnssdProtocolUdp) || (protocol == DnssdServiceProtocol::kDnssdProtocolTcp);
}

uint32_t GetInterfaceId(chip::Inet::InterfaceId interfaceId)
{
    return interfaceId.IsPresent() ? interfaceId.GetPlatformInterface() : kDNSServiceInterfaceIndexAny;
}

// Mirrors the GetProtocol() helper in DnssdContexts.cpp. Kept as a local
// helper here (rather than shared via the header) so the two files don't
// have to be refactored to expose what is otherwise a one-line policy.
DNSServiceProtocol GetDNSServiceProtocolFromAddressType(chip::Inet::IPAddressType addressType)
{
#if INET_CONFIG_ENABLE_IPV4
    if (addressType == chip::Inet::IPAddressType::kIPv4)
    {
        return kDNSServiceProtocol_IPv4;
    }
    if (addressType == chip::Inet::IPAddressType::kIPv6)
    {
        return kDNSServiceProtocol_IPv6;
    }
    return kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6;
#else
    return kDNSServiceProtocol_IPv6;
#endif
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

// Forward declaration so the rescue paths in Resolve() can reference the
// timer callback without it being part of the public symbol surface.
void OnResolveDeferredTeardownImpl(chip::System::Layer * aLayer, void * aAppState);

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    ChipLogProgress(Discovery, "Mdns: %s name: %s, type: %s, domain: %s, flags: %d, err: %" PRIi32 " (%s)", __func__,
                    StringOrNullMarker(name), StringOrNullMarker(type), StringOrNullMarker(domain), flags, err,
                    Error::ToString(err));

    auto sdCtx = reinterpret_cast<RegisterContext *>(context);
    VerifyOrReturn(kDNSServiceErr_NoError == err, TEMPORARY_RETURN_IGNORED sdCtx->Finalize(err));

    // Once a service has been properly published it is normally unreachable because the hostname has not yet been
    // registered against the dns daemon. Register the records mapping the hostname to our IP.
    auto error = sdCtx->mHostNameRegistrar.Register(^(DNSServiceErrorType registerRecordError) {
        ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
        TEMPORARY_RETURN_IGNORED sdCtx->Finalize(registerRecordError);
    });
    ReturnOnFailure(error, TEMPORARY_RETURN_IGNORED sdCtx->Finalize(error));
};

CHIP_ERROR Register(void * context, DnssdPublishCallback callback, uint32_t interfaceId, const char * type, const char * name,
                    uint16_t port, ScopedTXTRecord & record, Inet::IPAddressType addressType, const char * hostname)
{
    ChipLogProgress(Discovery, "Registering service %s on host %s with port %u and type: %s on interface id: %" PRIu32,
                    StringOrNullMarker(name), StringOrNullMarker(hostname), port, StringOrNullMarker(type), interfaceId);

    constexpr DNSServiceFlags registerFlags = kDNSServiceFlagsNoAutoRename;
    RegisterContext * sdCtx                 = nullptr;
    if (CHIP_NO_ERROR == MdnsContexts::GetInstance().GetRegisterContextOfTypeAndName(type, name, &sdCtx))
    {
        auto err = DNSServiceUpdateRecord(sdCtx->serviceRef, nullptr, registerFlags, record.size(), record.data(), 0 /* ttl */);
        VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
        return CHIP_NO_ERROR;
    }

    sdCtx = chip::Platform::New<RegisterContext>(type, name, callback, context);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    auto err = sdCtx->mHostNameRegistrar.Init(hostname, addressType, interfaceId);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));

    err = DNSServiceRegister(&sdCtx->serviceRef, registerFlags, interfaceId, name, type, kLocalDot, hostname, htons(port),
                             record.size(), record.data(), OnRegister, sdCtx);
    if (err != kDNSServiceErr_NoError)
    {
        // Just in case DNSServiceCreateConnection put garbage in the outparam
        // on failure.
        sdCtx->serviceRef = nullptr;
        return sdCtx->Finalize(err);
    }

    return MdnsContexts::GetInstance().Add(sdCtx);
}

static void OnBrowse(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err, const char * name,
                     const char * type, const char * domain, void * context)
{
    auto sdCtx = reinterpret_cast<BrowseHandler *>(context);
    VerifyOrReturn(kDNSServiceErr_NoError == err, TEMPORARY_RETURN_IGNORED sdCtx->Finalize(err));
    sdCtx->OnBrowse(flags, name, type, domain, interfaceId);
}

CHIP_ERROR BrowseOnDomain(BrowseHandler * sdCtx, uint32_t interfaceId, const char * type, const char * domain)
{
    auto sdRef = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection

    auto err = DNSServiceBrowse(&sdRef, kDNSServiceFlagsShareConnection, interfaceId, type, domain, OnBrowse, sdCtx);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Browse(BrowseHandler * sdCtx, uint32_t interfaceId, const char * type)
{
    auto err = DNSServiceCreateConnection(&sdCtx->serviceRef);
    if (err != kDNSServiceErr_NoError)
    {
        // Just in case DNSServiceCreateConnection put garbage in the outparam
        // on failure.
        sdCtx->serviceRef = nullptr;
        return sdCtx->Finalize(err);
    }

    // We will browse on both the local domain and the SRP domain.
    // BrowseOnDomain guarantees it will Finalize() on failure.
    ChipLogProgress(Discovery, "Browsing for: %s on local domain", StringOrNullMarker(type));
    ReturnErrorOnFailure(BrowseOnDomain(sdCtx, interfaceId, type, kLocalDot));

    ChipLogProgress(Discovery, "Browsing for: %s on %s domain", StringOrNullMarker(type), kSRPDot);
    ReturnErrorOnFailure(BrowseOnDomain(sdCtx, interfaceId, type, kSRPDot));

    return MdnsContexts::GetInstance().Add(sdCtx);
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
    ChipLogProgress(Discovery, "Mdns: %s flags: %d, interface: %u, hostname: %s, err: %" PRIi32 " (%s)", __func__, flags,
                    (unsigned) interfaceId, StringOrNullMarker(hostname), err, Error::ToString(err));

    auto contextWithType = reinterpret_cast<ResolveContextWithType *>(context);
    VerifyOrReturn(contextWithType != nullptr, ChipLogError(Discovery, "ResolveContextWithType is null"));

    auto sdCtx = contextWithType->context;
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
    LogOnFailure(__func__, err);

    if (kDNSServiceErr_NoError == err)
    {
        InterfaceKey interfaceKey = { interfaceId, hostname, contextWithType->isSRPResolve };
        TEMPORARY_RETURN_IGNORED sdCtx->OnNewAddress(interfaceKey, address);
    }

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        VerifyOrReturn(sdCtx->HasAddress(), TEMPORARY_RETURN_IGNORED sdCtx->Finalize(kDNSServiceErr_BadState));
        TEMPORARY_RETURN_IGNORED sdCtx->Finalize();
    }
}

static void GetAddrInfo(ResolveContext * sdCtx)
{
    auto protocol = sdCtx->protocol;

    for (auto & interface : sdCtx->interfaces)
    {
        if (interface.second.isDNSLookUpRequested)
        {
            continue;
        }

        auto interfaceId = interface.first.interfaceId;
        auto hostname    = interface.second.fullyQualifiedDomainName.c_str();
        auto sdRefCopy   = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection

        ResolveContextWithType * contextWithType =
            (interface.first.isSRPResult) ? &sdCtx->resolveContextWithSRPType : &sdCtx->resolveContextWithNonSRPType;
        auto err = DNSServiceGetAddrInfo(&sdRefCopy, kDNSServiceFlagsShareConnection, interfaceId, protocol, hostname,
                                         OnGetAddrInfo, contextWithType);
        VerifyOrReturn(kDNSServiceErr_NoError == err, TEMPORARY_RETURN_IGNORED sdCtx->Finalize(err));
        interface.second.isDNSLookUpRequested = true;
    }
}

static void OnResolve(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                      const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen, const unsigned char * txtRecord,
                      void * context)
{
    ChipLogProgress(Discovery, "Mdns: %s flags: %d, interface: %u, fullname: %s, hostname: %s, port: %u, err: %" PRIi32 " (%s)",
                    __func__, flags, (unsigned) interfaceId, StringOrNullMarker(fullname), StringOrNullMarker(hostname),
                    ntohs(port), err, Error::ToString(err));

    auto contextWithType = reinterpret_cast<ResolveContextWithType *>(context);
    VerifyOrReturn(contextWithType != nullptr, ChipLogError(Discovery, "ResolveContextWithType is null"));

    auto sdCtx = contextWithType->context;
    ReturnOnFailure(MdnsContexts::GetInstance().Has(sdCtx));
    LogOnFailure(__func__, err);

    if (kDNSServiceErr_NoError == err)
    {
        sdCtx->OnNewInterface(interfaceId, fullname, hostname, port, txtLen, txtRecord, contextWithType->isSRPResolve);
    }

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        VerifyOrReturn(sdCtx->HasInterface(), TEMPORARY_RETURN_IGNORED sdCtx->Finalize(kDNSServiceErr_BadState));
        GetAddrInfo(sdCtx);
    }
}

static CHIP_ERROR ResolveWithContext(ResolveContext * sdCtx, uint32_t interfaceId, const char * type, const char * name,
                                     const char * domain, ResolveContextWithType * contextWithType)
{
    auto sdRef = sdCtx->serviceRef; // Mandatory copy because of kDNSServiceFlagsShareConnection

    auto err =
        DNSServiceResolve(&sdRef, kDNSServiceFlagsShareConnection, interfaceId, name, type, domain, OnResolve, contextWithType);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, sdCtx->Finalize(err));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR Resolve(ResolveContext * sdCtx, uint32_t interfaceId, chip::Inet::IPAddressType addressType, const char * type,
                          const char * name, const char * domain)
{
    ChipLogProgress(Discovery, "Resolve type=%s name=%s domain=%s interface=%" PRIu32, StringOrNullMarker(type),
                    StringOrNullMarker(name), StringOrNullMarker(domain), interfaceId);

    // Persist the requested scope so the rescue path in the public Resolve()
    // overloads can refuse to coalesce a future request with a different
    // interface, service type, or domain.
    sdCtx->requestedInterfaceId = interfaceId;
    sdCtx->requestedType        = (type != nullptr) ? std::string(type) : std::string();
    sdCtx->requestedDomain      = (domain != nullptr) ? std::string(domain) : std::string();

    auto err = DNSServiceCreateConnection(&sdCtx->serviceRef);
    if (err != kDNSServiceErr_NoError)
    {
        // Just in case DNSServiceCreateConnection put garbage in the outparam
        // on failure.
        sdCtx->serviceRef = nullptr;
        return sdCtx->Finalize(err);
    }

    // If we have a single domain from a browse, we will use that for the Resolve.
    // Otherwise we will try to resolve using both the local domain and the SRP domain.
    // ResolveWithContext guarantees it will Finalize() on failure.
    if (domain != nullptr)
    {
        ReturnErrorOnFailure(ResolveWithContext(sdCtx, interfaceId, type, name, domain, &sdCtx->resolveContextWithNonSRPType));
    }
    else
    {
        ReturnErrorOnFailure(ResolveWithContext(sdCtx, interfaceId, type, name, kLocalDot, &sdCtx->resolveContextWithNonSRPType));
        ReturnErrorOnFailure(ResolveWithContext(sdCtx, interfaceId, type, name, kSRPDot, &sdCtx->resolveContextWithNonSRPType));
    }

    auto retval = MdnsContexts::GetInstance().Add(sdCtx);
    if (retval == CHIP_NO_ERROR)
    {
        (*(sdCtx->consumerCounter))++;
    }
    return retval;
}

static CHIP_ERROR Resolve(void * context, DnssdResolveCallback callback, uint32_t interfaceId,
                          chip::Inet::IPAddressType addressType, const char * type, const char * name, const char * domain)
{
    // If there's an existing ResolveContext for this instance name that is
    // currently in a deferred-teardown window, cancel the timer and reuse it
    // rather than tearing down its DNSServiceRef and opening a new one. This
    // is the coalescing half of the deferred-teardown fix: avoid the cancel-
    // then-restart pattern that drops still-queued resolve results.
    //
    // ChipDnssdResolveNoLongerNeeded arms a deferred-teardown timer on EVERY
    // ResolveContext bound to this instance name (multi-interface scans and
    // prior-caller leftovers can produce siblings). All sibling contexts for
    // a given instance name share the same std::shared_ptr<uint32_t>
    // consumerCounter (see GetCounterHolder), so we must cancel every
    // sibling's timer and clear its deferredTeardownScheduled flag BEFORE
    // bumping the shared counter -- otherwise the first iteration would flip
    // the *consumerCounter == 0 gate, leaving every subsequent sibling
    // un-rescued. Those un-rescued siblings' timers would still fire and
    // OnResolveDeferredTeardown would observe a non-zero counter, returning
    // without finalizing and leaving stale ResolveContexts and DNSServiceRefs
    // alive (resource leak). Bump the counter exactly once for this logical
    // Resolve call.
    // Only coalesce with an existing ResolveContext that was constructed via
    // the same callback-based overload (callback != nullptr). The two ctors
    // are mutually exclusive discriminants in DispatchSuccess/DispatchFailure
    // (callback == nullptr means dispatch via DiscoverNodeDelegate; non-null
    // means invoke callback(context, ...)). Reusing a delegate-based context
    // here and assigning a callback into it -- or vice versa in the other
    // overload -- would silently route results to the wrong consumer and, in
    // the worst case, type-confuse a DiscoverNodeDelegate* into a void*
    // user-context arg (UAF / memory corruption). When the kind doesn't
    // match, leave the existing context alone (its deferred-teardown timer
    // will fire normally) and fall through to create a fresh context.
    // Walk every sibling ResolveContext that already exists for this name,
    // looking for one we can rescue (counter==0, deferred-teardown timer
    // armed, kind matches). Non-primary siblings that ALSO satisfy the
    // rescue eligibility predicate must be Finalize()d here -- not merely
    // had their timer cancelled and flag cleared -- because the new caller
    // only rebinds callback/context onto a single primary sibling.
    // Leaving non-primary siblings around with stale callback/context
    // pointers is a use-after-free hazard the moment any of them later
    // dispatches (success or the eventual cancel via this code path's own
    // future ChipDnssdResolveNoLongerNeeded).
    std::vector<GenericContext *> existingResolves;
    MdnsContexts::GetInstance().FindAllMatchingPredicate(
        [name](GenericContext * item) {
            return item->type == ContextType::Resolve && static_cast<ResolveContext *>(item)->Matches(name);
        },
        existingResolves);
    ResolveContext * primaryCtx    = nullptr;
    DNSServiceProtocol newProtocol = GetDNSServiceProtocolFromAddressType(addressType);
    std::string newType            = (type != nullptr) ? std::string(type) : std::string();
    std::string newDomain          = (domain != nullptr) ? std::string(domain) : std::string();
    std::vector<ResolveContext *> nonPrimaryToFinalize;
    for (auto * item : existingResolves)
    {
        auto * existingCtx = static_cast<ResolveContext *>(item);
        if (!existingCtx->deferredTeardownScheduled || *existingCtx->consumerCounter != 0)
        {
            continue;
        }
        // Kind mismatch: existing was constructed delegate-based.
        if (existingCtx->callback == nullptr)
        {
            continue;
        }
        // Scope mismatch: a future StopBrowse on the original browse would
        // spuriously cancel an unrelated standalone consumer (or vice versa);
        // and a different interface/protocol/service-type/domain means the
        // rescued context would silently inherit the previous caller's
        // subscription scope (or, in the service-type case, hand the new
        // caller results from a DNSServiceResolve started for the wrong
        // service type). Refuse to coalesce in any of these cases and fall
        // through to fresh allocation. The existing context's own deferred-
        // teardown timer will fire normally.
        if (existingCtx->browseThatCausedResolve != nullptr)
        {
            continue;
        }
        if (existingCtx->requestedInterfaceId != interfaceId)
        {
            continue;
        }
        if (existingCtx->protocol != newProtocol)
        {
            continue;
        }
        if (existingCtx->requestedType != newType)
        {
            continue;
        }
        if (existingCtx->requestedDomain != newDomain)
        {
            continue;
        }
        chip::DeviceLayer::SystemLayer().CancelTimer(OnResolveDeferredTeardownImpl, existingCtx);
        existingCtx->deferredTeardownScheduled = false;
        if (primaryCtx == nullptr)
        {
            primaryCtx = existingCtx;
        }
        else
        {
            // Non-primary sibling: schedule Finalize after the rescue commit.
            // We can't Finalize here because Finalize calls
            // RemoveWithoutDeleting which would invalidate the iterator on
            // existingResolves. Defer to a second pass below.
            nonPrimaryToFinalize.push_back(existingCtx);
        }
    }
    if (primaryCtx != nullptr)
    {
        // The previous consumer count dropped to zero, so the original
        // callback/context pointers may now refer to objects that have been
        // destroyed. Rebind the dispatch target to the new caller's values
        // before bumping the counter to avoid invoking a dangling callback or
        // dereferencing a freed context (UAF).
        if (primaryCtx->callback != callback || primaryCtx->context != context)
        {
            ChipLogDetail(Discovery, "Mdns: Reusing deferred-teardown ResolveContext for %s with new callback/context",
                          StringOrNullMarker(name));
            primaryCtx->callback = callback;
            primaryCtx->context  = context;
        }
        // Finalize non-primary siblings with CHIP_ERROR_CANCELLED. They share
        // the consumerCounter shared_ptr with primaryCtx; Finalize routes
        // through DispatchFailure which calls back the (possibly-stale) old
        // callback/context for the SIBLING -- but those are exactly the
        // already-departed consumer pointers ChipDnssdResolveNoLongerNeeded
        // is racing to clear, so the dispatch is best-effort and matches the
        // pre-rescue contract (one CHIP_ERROR_CANCELLED per cancelled
        // consumer). The single-canonical-context invariant is restored
        // before we bump the shared counter for the new logical Resolve.
        for (auto * sibling : nonPrimaryToFinalize)
        {
            TEMPORARY_RETURN_IGNORED sibling->Finalize(CHIP_ERROR_CANCELLED);
        }
        // Bump the shared consumer counter exactly once for this logical
        // Resolve call.
        (*primaryCtx->consumerCounter)++;
        return CHIP_NO_ERROR;
    }

    auto counterHolder = GetCounterHolder(name);
    auto sdCtx         = chip::Platform::New<ResolveContext>(context, callback, addressType, name,
                                                     BrowseContext::sContextDispatchingSuccess, std::move(counterHolder));
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    return Resolve(sdCtx, interfaceId, addressType, type, name, domain);
}

static CHIP_ERROR Resolve(DiscoverNodeDelegate * delegate, uint32_t interfaceId, chip::Inet::IPAddressType addressType,
                          const char * type, const char * name)
{
    // Same coalescing path as the callback overload above, including the
    // kind-match guard: only coalesce with an existing ResolveContext that
    // was constructed via the delegate-based ctor (callback == nullptr). See
    // the callback overload for why kind mismatches are unsafe.
    std::vector<GenericContext *> existingResolves;
    MdnsContexts::GetInstance().FindAllMatchingPredicate(
        [name](GenericContext * item) {
            return item->type == ContextType::Resolve && static_cast<ResolveContext *>(item)->Matches(name);
        },
        existingResolves);
    ResolveContext * primaryCtx    = nullptr;
    DNSServiceProtocol newProtocol = GetDNSServiceProtocolFromAddressType(addressType);
    std::string newType            = (type != nullptr) ? std::string(type) : std::string();
    std::vector<ResolveContext *> nonPrimaryToFinalize;
    for (auto * item : existingResolves)
    {
        auto * existingCtx = static_cast<ResolveContext *>(item);
        if (!existingCtx->deferredTeardownScheduled || *existingCtx->consumerCounter != 0)
        {
            continue;
        }
        // Kind mismatch: existing was constructed callback-based.
        if (existingCtx->callback != nullptr)
        {
            continue;
        }
        // Scope mismatch checks: see callback overload for rationale.
        if (existingCtx->browseThatCausedResolve != nullptr)
        {
            continue;
        }
        if (existingCtx->requestedInterfaceId != interfaceId)
        {
            continue;
        }
        if (existingCtx->protocol != newProtocol)
        {
            continue;
        }
        if (existingCtx->requestedType != newType)
        {
            continue;
        }
        // The delegate overload doesn't take a domain argument; it always
        // resolves on both kLocalDot and kSRPDot. Rescued contexts created
        // via the same overload will have an empty domain (we record domain
        // as empty when the inner Resolve helper is called with domain ==
        // nullptr). Refuse to coalesce if the existing context has a non-
        // empty domain (i.e. it was created via a browse-driven path).
        if (!existingCtx->requestedDomain.empty())
        {
            continue;
        }
        chip::DeviceLayer::SystemLayer().CancelTimer(OnResolveDeferredTeardownImpl, existingCtx);
        existingCtx->deferredTeardownScheduled = false;
        if (primaryCtx == nullptr)
        {
            primaryCtx = existingCtx;
        }
        else
        {
            nonPrimaryToFinalize.push_back(existingCtx);
        }
    }
    if (primaryCtx != nullptr)
    {
        // Counter was zero, so the previous delegate may be gone; rebind.
        if (primaryCtx->context != delegate)
        {
            ChipLogDetail(Discovery, "Mdns: Reusing deferred-teardown ResolveContext for %s with a new delegate",
                          StringOrNullMarker(name));
            primaryCtx->context = delegate;
        }
        for (auto * sibling : nonPrimaryToFinalize)
        {
            TEMPORARY_RETURN_IGNORED sibling->Finalize(CHIP_ERROR_CANCELLED);
        }
        (*primaryCtx->consumerCounter)++;
        return CHIP_NO_ERROR;
    }

    auto counterHolder = GetCounterHolder(name);
    auto sdCtx         = chip::Platform::New<ResolveContext>(delegate, addressType, name, std::move(counterHolder));
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    return Resolve(sdCtx, interfaceId, addressType, type, name, nullptr);
}

} // namespace

#if CHIP_CONFIG_TEST
chip::System::Clock::Milliseconds32 GetResolveDeferredTeardownDelay()
{
    return chip::System::Clock::Milliseconds32(gResolveDeferredTeardownDelayMs.load(std::memory_order_relaxed));
}

void SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32 delay)
{
    gResolveDeferredTeardownDelayMs.store(static_cast<uint32_t>(delay.count()), std::memory_order_relaxed);
}
#endif // CHIP_CONFIG_TEST

namespace {

// Fires when the deferred-teardown timer scheduled by
// ChipDnssdResolveNoLongerNeeded elapses without a new ChipDnssdResolve
// arriving for the same instance name. At this point we genuinely have no
// consumers, so dispatch CHIP_ERROR_CANCELLED to flush callback contracts and
// tear down the underlying DNSServiceRef.
void OnResolveDeferredTeardownImpl(chip::System::Layer * /* aLayer */, void * aAppState)
{
    auto * sdCtx = static_cast<ResolveContext *>(aAppState);
    // Make sure the context is still tracked (it could have been finalized
    // out from under us by, e.g., a successful resolve completing in the
    // meantime).
    if (MdnsContexts::GetInstance().Has(sdCtx) != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Mdns: Deferred teardown timer fired for context already finalized; ignoring");
        return;
    }
    if (!sdCtx->deferredTeardownScheduled)
    {
        // Someone re-armed the context (a new Resolve came in and reused it).
        ChipLogDetail(Discovery, "Mdns: Deferred teardown timer fired for %s but flag was cleared; ignoring",
                      sdCtx->instanceName.c_str());
        return;
    }
    if (*sdCtx->consumerCounter != 0)
    {
        // A consumer reattached without going through the cancel path. This
        // is unexpected and likely indicates an upstream state-machine bug
        // because the only sanctioned re-attach path is the coalescing branch
        // in Resolve(), which clears deferredTeardownScheduled before
        // bumping the counter.
        ChipLogError(Discovery,
                     "Mdns: Deferred teardown timer fired for %s with non-zero consumerCounter (%u); "
                     "consumer reattached without going through coalescing path",
                     sdCtx->instanceName.c_str(), *sdCtx->consumerCounter);
        sdCtx->deferredTeardownScheduled = false;
        return;
    }
    sdCtx->deferredTeardownScheduled = false;
    TEMPORARY_RETURN_IGNORED sdCtx->Finalize(CHIP_ERROR_CANCELLED);
}

// Schedule a deferred-teardown timer against every still-tracked ResolveContext
// in `resolves`. Contexts whose StartTimer call fails fall back to immediate
// Finalize(CHIP_ERROR_CANCELLED) so they don't leak. Shared by the two cancel
// entry points -- ChipDnssdResolveNoLongerNeeded and ChipDnssdStopBrowse's
// browse-triggered-resolve cleanup -- so both honor the same "don't drop
// in-flight results queued on the socket" contract.
//
// Delegate-based ResolveContexts (callback == nullptr) are deliberately NOT
// deferred. The MTRCommissionableBrowser flow churns OnBrowseAdd/OnBrowseRemove
// for the same instance name on the order of microseconds while the device is
// being discovered. Holding the underlying DNSServiceRef alive across that
// churn (instead of synchronously tearing it down and starting fresh on the
// next OnBrowseAdd) starves DNSServiceGetAddrInfo of a chance to deliver its
// callback before the next remove arrives, and the browse delegate never sees
// the device as "fully resolved" (observed in
// MTRCommissionableBrowserTests/test005 under TSAN). The deferred-teardown
// coalescing was added to fix the NodeID-resolve cancel/restart latency in
// the callback-based Resolve path; that path uses callback != nullptr and is
// unaffected by this carve-out.
void ScheduleDeferredTeardownForResolves(const std::vector<GenericContext *> & resolves, const char * instanceNameForLog)
{
    // Track contexts whose StartTimer call failed so we can immediately
    // Finalize them rather than leaking. Gating the fallback on a global
    // anyScheduled flag would silently drop the failed-to-schedule
    // contexts whenever ANY of their siblings successfully scheduled.
    std::vector<ResolveContext *> failedToSchedule;
    std::vector<ResolveContext *> delegateBased;
    for (auto * item : resolves)
    {
        auto * rctx = static_cast<ResolveContext *>(item);
        // Never arm a deferred-teardown timer on a context whose shared
        // counter is still non-zero (i.e. has a live consumer). Doing so
        // would corrupt the "deferredTeardownScheduled => no consumers"
        // invariant the timer callback and the rescue path rely on, and
        // could cause the rescue path to silently hijack the live
        // consumer's callback.
        if (*rctx->consumerCounter != 0)
        {
            continue;
        }
        if (rctx->deferredTeardownScheduled)
        {
            continue;
        }
        if (rctx->callback == nullptr)
        {
            // Delegate-based: tear down synchronously (see comment above).
            delegateBased.push_back(rctx);
            continue;
        }
        rctx->deferredTeardownScheduled = true;
        auto err                        = chip::DeviceLayer::SystemLayer().StartTimer(
            chip::System::Clock::Milliseconds32(gResolveDeferredTeardownDelayMs.load(std::memory_order_relaxed)),
            OnResolveDeferredTeardownImpl, rctx);
        if (err != CHIP_NO_ERROR)
        {
            // SystemLayer::StartTimer returns CriticalFailure, which has no Format(); log
            // without the error code (the failure mode here is timer-pool exhaustion, which
            // is the only failure StartTimer can return).
            ChipLogError(Discovery,
                         "Mdns: StartTimer for deferred teardown of %s failed; "
                         "falling back to immediate teardown for this context",
                         StringOrNullMarker(instanceNameForLog));
            rctx->deferredTeardownScheduled = false;
            failedToSchedule.push_back(rctx);
        }
    }

    // Finalize any contexts that couldn't have a deferred teardown
    // scheduled, so they don't leak with no consumer and no timer.
    for (auto * rctx : failedToSchedule)
    {
        TEMPORARY_RETURN_IGNORED rctx->Finalize(CHIP_ERROR_CANCELLED);
    }

    // Delegate-based contexts always tear down synchronously.
    for (auto * rctx : delegateBased)
    {
        TEMPORARY_RETURN_IGNORED rctx->Finalize(CHIP_ERROR_CANCELLED);
    }
}

} // namespace

void CancelDeferredTeardownIfScheduled(ResolveContext * rctx)
{
    if (rctx == nullptr)
    {
        return;
    }
    if (rctx->deferredTeardownScheduled)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnResolveDeferredTeardownImpl, rctx);
        rctx->deferredTeardownScheduled = false;
    }
}

#if CHIP_CONFIG_TEST
// Test-only public wrapper for the internal timer callback. Tests synthesize
// stale-fire scenarios; production code must never call this directly --
// the timer is armed and disarmed exclusively through
// ScheduleDeferredTeardownForResolves / CancelDeferredTeardownIfScheduled.
void OnResolveDeferredTeardown(chip::System::Layer * aLayer, void * aAppState)
{
    OnResolveDeferredTeardownImpl(aLayer, aAppState);
}
#endif // CHIP_CONFIG_TEST

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
    TEMPORARY_RETURN_IGNORED ChipDnssdRemoveServices();
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

    // Decrement the shared consumerCounter for each browse-triggered resolve
    // we're cancelling (each one bumped the counter by exactly 1 when it was
    // created in Resolve()). Finalize browse-triggered resolves synchronously
    // -- StopBrowse callers (per the long-standing pre-PR-72273 contract)
    // expect the cancel callback to fire before they return so they can
    // safely free the resolve callback's user-context object.
    //
    // The deferred-teardown rescue window only benefits the
    // ChipDnssdResolveNoLongerNeeded (user-initiated) path, where an
    // in-flight result already queued on the socket might still be useful to
    // the same caller (who would re-issue Resolve). A browse cancellation is
    // a different semantic: the browse-driven consumer is going away, so
    // immediate teardown is correct.
    std::vector<ResolveContext *> toFinalize;
    for (auto * item : resolves)
    {
        auto * rctx = static_cast<ResolveContext *>(item);
        if (*rctx->consumerCounter == 0)
        {
            // Already in the deferred-teardown window (counter previously hit
            // zero via ChipDnssdResolveNoLongerNeeded). Nothing to decrement;
            // the existing timer will run to completion.
            continue;
        }
        (*rctx->consumerCounter)--;
        if (*rctx->consumerCounter == 0)
        {
            toFinalize.push_back(rctx);
        }
    }

    for (auto * rctx : toFinalize)
    {
        TEMPORARY_RETURN_IGNORED rctx->Finalize(CHIP_ERROR_CANCELLED);
    }

    TEMPORARY_RETURN_IGNORED ctx->Finalize(CHIP_ERROR_CANCELLED);
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

    auto regtype        = GetFullType(service);
    auto interfaceId    = GetInterfaceId(interface);
    const char * domain = nullptr;

    if (BrowseContext::sContextDispatchingSuccess != nullptr)
    {
        for (size_t i = 0; i < BrowseContext::sDispatchedServices->size(); ++i)
        {
            if (service == &BrowseContext::sDispatchedServices->at(i))
            {
                domain = BrowseContext::sContextDispatchingSuccess->services[i].second.c_str();
                break;
            }
        }
    }

    return Resolve(context, callback, interfaceId, service->mAddressType, regtype.c_str(), service->mName, domain);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DiscoverNodeDelegate * delegate)
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
        // No more consumers. Per the dnssd contract, calling
        // DNSServiceRefDeallocate immediately would discard any resolve
        // result already written to the socket but not yet consumed by us.
        // Instead, schedule a deferred teardown for every still-tracked
        // ResolveContext bound to this instance name so that a still-queued
        // read indicator gets a chance to fire and dispatch the result
        // through DispatchSuccess (which finalizes the context naturally).
        // If a new ChipDnssdResolve arrives for the same instance name during
        // this window, it will reuse the existing context and cancel the
        // deferred teardown rather than open a fresh
        // DNSServiceCreateConnection -- per guidance from the mDNSResponder
        // maintainers that "starting and stopping queries doesn't query
        // harder".
        std::vector<GenericContext *> resolves;
        MdnsContexts::GetInstance().FindAllMatchingPredicate(
            [instanceName](GenericContext * item) {
                return item->type == ContextType::Resolve && static_cast<ResolveContext *>(item)->Matches(instanceName);
            },
            resolves);

        ScheduleDeferredTeardownForResolves(resolves, instanceName);
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

    auto error = DNSServiceReconfirmRecord(0 /* DNSServiceFlags */, interfaceId, fullname.c_str(), rrtype, rrclass, rdlen, rdata);
    LogOnFailure(__func__, error);

    return Error::ToChipError(error);
}

} // namespace Dnssd
} // namespace chip
