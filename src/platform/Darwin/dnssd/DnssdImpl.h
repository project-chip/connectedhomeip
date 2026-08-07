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

#pragma once

#include <dns_sd.h>
#include <dnssd/dnssd_config.h>
#include <lib/core/Global.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <platform/CHIPDeviceLayer.h>

#include "DnssdHostNameRegistrar.h"

#include <map>
#include <string>
#include <vector>

namespace chip {
namespace Dnssd {

enum class ContextType
{
    Register,
    Browse,
    BrowseWithDelegate,
    Resolve,
};

struct GenericContext
{
    ContextType type;
    void * context;
    // When using a GenericContext, if a DNSServiceRef is created successfully
    // API consumers must ensure that it gets set as serviceRef on the context
    // immediately, before any other operations that might fail can happen.
    //
    // In all cases, once a context has been created, Finalize() must be called
    // on it to clean it up properly.
    DNSServiceRef serviceRef = nullptr;

    virtual ~GenericContext() {}

    CHIP_ERROR Finalize(CHIP_ERROR err);
    CHIP_ERROR Finalize(DNSServiceErrorType err = kDNSServiceErr_NoError);

    virtual void DispatchFailure(const char * errorStr, CHIP_ERROR err) = 0;
    virtual void DispatchSuccess()                                      = 0;

private:
    CHIP_ERROR FinalizeInternal(const char * errorStr, CHIP_ERROR err);
};

struct BrowseWithDelegateContext;
struct RegisterContext;
struct ResolveContext;

class MdnsContexts
{
public:
    MdnsContexts(const MdnsContexts &)             = delete;
    MdnsContexts & operator=(const MdnsContexts &) = delete;
    ~MdnsContexts();
    static MdnsContexts & GetInstance() { return sInstance.get(); }

    // The context being added is expected to have a valid serviceRef.
    CHIP_ERROR Add(GenericContext * context);
    CHIP_ERROR Remove(GenericContext * context);
    CHIP_ERROR RemoveAllOfType(ContextType type);
    CHIP_ERROR Has(GenericContext * context);
#if DNSSD_VERBOSE_CONTEXT_PRINT
    void Print() const;
#endif

    /**
     * @brief
     *   Returns a pointer to a RegisterContext that has previously been registered
     *   with a given type.
     *
     * @param[in]  type     A service type. Service type are composed of
     *                      of the service name, the service protocol, and the PTR records.
     *                      Example:
     *                        _matterc._udp,_V65521,_S15,_L3840,_CM
     *                        _matter._tcp,_I4CEEAD044CC35B63
     * @param[in]  name     The instance name for the service.
     * @param[out] context  A reference to the context previously registered
     *
     * @return     On success, the context parameter will point to the previously
     *             registered context.
     */
    CHIP_ERROR GetRegisterContextOfTypeAndName(const char * type, const char * name, RegisterContext ** context);

    /**
     * Return a pointer to an existing ResolveContext for the given
     * instanceName, if any.  Returns nullptr if there are none.
     */
    ResolveContext * GetExistingResolveForInstanceName(const char * instanceName);

    /**
     * Return a pointer to an existing BrowserWithDelegateContext for the given
     * delegate, if any.  Returns nullptr if there are none.
     */
    BrowseWithDelegateContext * GetExistingBrowseForDelegate(DnssdBrowseDelegate * delegate);

    /**
     * Remove context from the list, if it's present in the list.  Return
     * whether it was present.
     */
    bool RemoveWithoutDeleting(GenericContext * context);

    void Delete(GenericContext * context);

    /**
     * Fill the provided vector with all contexts for which the given predicate
     * returns true.
     */
    template <typename F>
    void FindAllMatchingPredicate(F predicate, std::vector<GenericContext *> & results)
    {
        results.clear();
        for (auto & ctx : mContexts)
        {
            if (predicate(ctx))
            {
                results.push_back(ctx);
            }
        }
    }

private:
    MdnsContexts() = default;
    friend Global<MdnsContexts>;
    static Global<MdnsContexts> sInstance;

    std::vector<GenericContext *> mContexts;
};

struct RegisterContext : public GenericContext
{
    DnssdPublishCallback callback;
    std::string mType;
    std::string mInstanceName;
    HostNameRegistrar mHostNameRegistrar;

    RegisterContext(const char * sType, const char * instanceName, DnssdPublishCallback cb, void * cbContext);
    virtual ~RegisterContext() { mHostNameRegistrar.Unregister(); }

    void DispatchFailure(const char * errorStr, CHIP_ERROR err) override;
    void DispatchSuccess() override;

    bool matches(const char * type, const char * name) { return mType == type && mInstanceName == name; }
};

struct BrowseHandler : public GenericContext
{
    virtual ~BrowseHandler() {}

    DnssdServiceProtocol protocol;

    virtual void OnBrowse(DNSServiceFlags flags, const char * name, const char * type, const char * domain,
                          uint32_t interfaceId)                                                                  = 0;
    virtual void OnBrowseAdd(const char * name, const char * type, const char * domain, uint32_t interfaceId)    = 0;
    virtual void OnBrowseRemove(const char * name, const char * type, const char * domain, uint32_t interfaceId) = 0;
};

struct BrowseContext : public BrowseHandler
{
    DnssdBrowseCallback callback;
    std::vector<std::pair<DnssdService, std::string>> services;
    bool dispatchedSuccessOnce = false;

    BrowseContext(void * cbContext, DnssdBrowseCallback cb, DnssdServiceProtocol cbContextProtocol);

    void DispatchFailure(const char * errorStr, CHIP_ERROR err) override;
    void DispatchSuccess() override;

    // Dispatch what we have found so far, but don't stop browsing.
    void DispatchPartialSuccess();

    void OnBrowse(DNSServiceFlags flags, const char * name, const char * type, const char * domain, uint32_t interfaceId) override;
    void OnBrowseAdd(const char * name, const char * type, const char * domain, uint32_t interfaceId) override;
    void OnBrowseRemove(const char * name, const char * type, const char * domain, uint32_t interfaceId) override;

    // While we are dispatching partial success, sContextDispatchingSuccess will
    // be set to the BrowseContext doing the dispatch.  This allows resolves
    // triggered by the browse dispatch to be associated with the browse.  This
    // relies on our consumer starting the resolves synchronously from the
    // partial success callback.
    //
    // The other option would be to do the resolve ourselves before signaling
    // browse success, but that would only allow us to pass in one ip per
    // discovered hostname, and we want to pass in all the IPs we resolve.
    //
    // TODO: Consider fixing the higher-level APIs to make it possible to pass
    // in multiple IPs for a successful browse result.
    static BrowseContext * sContextDispatchingSuccess;
    static std::vector<DnssdService> * sDispatchedServices;
};

struct BrowseWithDelegateContext : public BrowseHandler
{
    BrowseWithDelegateContext(DnssdBrowseDelegate * delegate, DnssdServiceProtocol cbContextProtocol);

    void DispatchFailure(const char * errorStr, CHIP_ERROR err) override;
    void DispatchSuccess() override;

    void OnBrowse(DNSServiceFlags flags, const char * name, const char * type, const char * domain, uint32_t interfaceId) override;
    void OnBrowseAdd(const char * name, const char * type, const char * domain, uint32_t interfaceId) override;
    void OnBrowseRemove(const char * name, const char * type, const char * domain, uint32_t interfaceId) override;

    bool Matches(DnssdBrowseDelegate * otherDelegate) const { return context == otherDelegate; }
};

struct InterfaceInfo
{
    InterfaceInfo();
    InterfaceInfo(InterfaceInfo && other);
    // Copying is not safe, because DnssdService bits need to be
    // copied/deallocated properly.
    InterfaceInfo(const InterfaceInfo & other) = delete;
    ~InterfaceInfo();

    DnssdService service;
    std::vector<Inet::IPAddress> addresses;
    std::string fullyQualifiedDomainName;
    bool isDNSLookUpRequested = false;
    bool HasAddresses() const { return addresses.size() != 0; };
};

struct InterfaceKey
{
    InterfaceKey()  = default;
    ~InterfaceKey() = default;
    inline bool operator<(const InterfaceKey & other) const
    {
        return (this->interfaceId < other.interfaceId) ||
            ((this->interfaceId == other.interfaceId) && (this->hostname < other.hostname)) ||
            ((this->interfaceId == other.interfaceId) && (this->hostname == other.hostname) &&
             (this->isSRPResult < other.isSRPResult));
    }

    inline bool operator==(const InterfaceKey & other) const
    {
        return this->interfaceId == other.interfaceId && this->hostname == other.hostname && this->isSRPResult == other.isSRPResult;
    }

    uint32_t interfaceId;
    std::string hostname;
    bool isSRPResult = false;
};

struct ResolveContextWithType
{
    ResolveContextWithType()  = delete;
    ~ResolveContextWithType() = default;

    ResolveContext * const context;
    const bool isSRPResolve;
};

struct ResolveContext : public GenericContext
{
    DnssdResolveCallback callback;
    std::map<InterfaceKey, InterfaceInfo> interfaces;
    DNSServiceProtocol protocol;
    std::string instanceName;
    std::shared_ptr<uint32_t> consumerCounter;
    BrowseContext * const browseThatCausedResolve; // Can be null

    // The interfaceId / service type / domain the consumer originally asked
    // for. Persisted so the rescue path in Resolve() can refuse to coalesce a
    // new request whose interface, service type, or domain doesn't match the
    // existing context (which would silently inherit the previous caller's
    // subscription scope, or worse hand the new caller results from a
    // DNSServiceResolve started for a different service type). Named
    // 'requested...' to avoid shadowing the local 'interfaceId' params/locals
    // that ResolveContext member functions already use.
    uint32_t requestedInterfaceId{ kDNSServiceInterfaceIndexAny };
    std::string requestedType;
    std::string requestedDomain;

    // Set to true while a deferred teardown timer is scheduled against this
    // context (i.e. consumerCounter dropped to 0 but we have not yet torn the
    // context/serviceRef down). A new ChipDnssdResolve for the same instance
    // name during this window will cancel the timer and reuse the context
    // rather than open a fresh DNSServiceCreateConnection. See
    // ChipDnssdResolveNoLongerNeeded for the rationale.
    bool deferredTeardownScheduled = false;

    ResolveContextWithType resolveContextWithSRPType    = { this, true };
    ResolveContextWithType resolveContextWithNonSRPType = { this, false };

    // browseCausingResolve can be null.
    ResolveContext(void * cbContext, DnssdResolveCallback cb, chip::Inet::IPAddressType cbAddressType,
                   const char * instanceNameToResolve, BrowseContext * browseCausingResolve,
                   std::shared_ptr<uint32_t> && consumerCounterToUse);
    ResolveContext(DiscoverNodeDelegate * delegate, chip::Inet::IPAddressType cbAddressType, const char * instanceNameToResolve,
                   std::shared_ptr<uint32_t> && consumerCounterToUse);

    void DispatchFailure(const char * errorStr, CHIP_ERROR err) override;
    void DispatchSuccess() override;

    CHIP_ERROR OnNewAddress(const InterfaceKey & interfaceKey, const struct sockaddr * address);
    bool HasAddress();

    void OnNewInterface(uint32_t interfaceId, const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen,
                        const unsigned char * txtRecord, bool isSRPResult);
    bool HasInterface();
    bool Matches(const char * otherInstanceName) const { return instanceName == otherInstanceName; }
};

// Helper used by DnssdContexts.cpp to cancel any pending deferred-teardown
// timer when a ResolveContext is finalized via the regular dispatch path. It
// is the narrow surface DnssdContexts needs from the deferred-teardown timer
// machinery; the timer callback itself stays internal to DnssdImpl.cpp.
void CancelDeferredTeardownIfScheduled(ResolveContext * rctx);

#if CHIP_CONFIG_TEST
// Returns the deferred-teardown delay used by ChipDnssdResolveNoLongerNeeded.
// Tests may override this via SetResolveDeferredTeardownDelay to make timing
// observable without waiting the full default window.
chip::System::Clock::Milliseconds32 GetResolveDeferredTeardownDelay();
void SetResolveDeferredTeardownDelay(chip::System::Clock::Milliseconds32 delay);

// Timer callback for the deferred-teardown window. Exposed under
// CHIP_CONFIG_TEST so tests can synthesize stale-timer-fire scenarios; not
// part of the production symbol surface.
void OnResolveDeferredTeardown(chip::System::Layer * aLayer, void * aAppState);
#endif // CHIP_CONFIG_TEST

} // namespace Dnssd
} // namespace chip
