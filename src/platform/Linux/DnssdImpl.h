/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <sys/select.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/domain.h>
#include <avahi-common/error.h>
#include <avahi-common/watch.h>

#include <lib/dnssd/platform/Dnssd.h>
#include <platform/Linux/ExponentialBackOff.h>
#include <system/SystemLayer.h>

struct AvahiWatch
{
    int mSocket;
    chip::System::SocketWatchToken mSocketWatch;
    AvahiWatchCallback mCallback; ///< The function to be called when interested events happened on mFd.
    AvahiWatchEvent mPendingIO;   ///< The pending events from the currently active or most recent callback.
    void * mContext;              ///< A pointer to application-specific context.
    void * mPoller;               ///< The poller created this watch.
};

struct AvahiTimeout
{
    std::chrono::steady_clock::time_point mAbsTimeout; ///< Absolute time when this timer timeout.
    AvahiTimeoutCallback mCallback;                    ///< The function to be called when timeout.
    bool mEnabled;                                     ///< Whether the timeout is enabled.
    void * mContext;                                   ///< The pointer to application-specific context.
    void * mPoller;                                    ///< The poller created this timer.
};

namespace chip {
namespace Dnssd {

class Poller
{
public:
    Poller(void);

    void HandleTimeout();

    const AvahiPoll * GetAvahiPoll(void) const { return &mAvahiPoller; }

private:
    static AvahiWatch * WatchNew(const struct AvahiPoll * poller, int fd, AvahiWatchEvent event, AvahiWatchCallback callback,
                                 void * context);
    AvahiWatch * WatchNew(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void * context);

    static void WatchUpdate(AvahiWatch * watch, AvahiWatchEvent event);

    static AvahiWatchEvent WatchGetEvents(AvahiWatch * watch);

    static void WatchFree(AvahiWatch * watch);
    void WatchFree(AvahiWatch & watch);

    static AvahiTimeout * TimeoutNew(const AvahiPoll * poller, const struct timeval * timeout, AvahiTimeoutCallback callback,
                                     void * context);
    AvahiTimeout * TimeoutNew(const struct timeval * timeout, AvahiTimeoutCallback callback, void * context);

    static void TimeoutUpdate(AvahiTimeout * timer, const struct timeval * timeout);

    static void TimeoutFree(AvahiTimeout * timer);
    void TimeoutFree(AvahiTimeout & timer);

    void SystemTimerUpdate(AvahiTimeout * timer);
    static void SystemTimerCallback(System::Layer * layer, void * data);

    std::vector<std::unique_ptr<AvahiWatch>> mWatches;
    std::vector<std::unique_ptr<AvahiTimeout>> mTimers;
    std::chrono::steady_clock::time_point mEarliestTimeout;

    AvahiPoll mAvahiPoller;
};

class MdnsAvahi
{
public:
    MdnsAvahi(const MdnsAvahi &)             = delete;
    MdnsAvahi & operator=(const MdnsAvahi &) = delete;

    CHIP_ERROR Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);
    void Shutdown();
    CHIP_ERROR SetHostname(const char * hostname);
    CHIP_ERROR PublishService(const DnssdService & service, DnssdPublishCallback callback, void * context);
    CHIP_ERROR StopPublish();
    CHIP_ERROR FinalizePublish();
    CHIP_ERROR Browse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                      chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context, intptr_t * browseIdentifier);
    CHIP_ERROR StopBrowse(intptr_t browseIdentifier);
    CHIP_ERROR Resolve(const char * name, const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                       chip::Inet::IPAddressType transportType, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                       void * context);
    void StopResolve(const char * name);

    Poller & GetPoller() { return mPoller; }

    static MdnsAvahi & GetInstance() { return sInstance; }

private:
    struct BrowseContext
    {
        MdnsAvahi * mInstance;
        DnssdBrowseCallback mCallback;
        void * mContext;
        Inet::IPAddressType mAddressType;
        std::vector<DnssdService> mServices;
        bool mReceivedAllCached;
        AvahiIfIndex mInterface;
        std::string mProtocol;
        std::atomic_bool mStopped{ false };
        AvahiServiceBrowser * mBrowser;
    };

    struct ResolveContext
    {
        size_t mNumber; // unique number for this context
        MdnsAvahi * mInstance;
        DnssdResolveCallback mCallback;
        void * mContext;
        char mName[Common::kInstanceNameMaxLength + 1];
        AvahiIfIndex mInterface;
        AvahiProtocol mTransport;
        AvahiProtocol mAddressType;
        std::string mFullType;
        uint8_t mAttempts                = 0;
        AvahiServiceResolver * mResolver = nullptr;

        ~ResolveContext()
        {
            if (mResolver != nullptr)
            {
                avahi_service_resolver_free(mResolver);
                mResolver = nullptr;
            }
        }
    };

    /// One address published for a Matter hostname: the Avahi interface
    /// and protocol it is published on, the address, and its text form.
    struct HostAddress
    {
        AvahiIfIndex mInterface;
        AvahiProtocol mProtocol;
        AvahiAddress mAddress;
        std::string mText;
    };

    /// Frees an Avahi entry group, withdrawing whatever it published.
    struct AvahiEntryGroupDeleter
    {
        void operator()(AvahiEntryGroup * group) const;
    };

    using AvahiEntryGroupPtr = std::unique_ptr<AvahiEntryGroup, AvahiEntryGroupDeleter>;

    /// The address records of a Matter hostname, published in an entry
    /// group of their own, apart from the services that use the hostname.
    struct HostGroup
    {
        AvahiEntryGroupPtr mGroup;
        std::vector<std::string> mAddresses; ///< "<interface>/<address>" of each record, sorted.
        bool mInUse = false;                 ///< Claimed by a published service since the last StopPublish().
    };

    struct AvahiStringListDeleter
    {
        void operator()(AvahiStringList * list) const { avahi_string_list_free(list); }
    };

    /// A published service: its entry group, and what it was published
    /// with, so that it can be published again after a collision.
    struct ServiceGroup
    {
        AvahiEntryGroupPtr mGroup;
        AvahiIfIndex mInterface = AVAHI_IF_UNSPEC;
        AvahiProtocol mProtocol = AVAHI_PROTO_UNSPEC;
        std::string mName;
        std::string mType;
        std::string mHost;
        uint16_t mPort = 0;
        std::unique_ptr<AvahiStringList, AvahiStringListDeleter> mText;
        std::vector<std::string> mSubTypes; ///< Full subtype names, "<subtype>._sub.<type>".
    };

    MdnsAvahi() : mClient(nullptr) {}
    ~MdnsAvahi();
    static MdnsAvahi sInstance;

    /// Allocates a new resolve context with a unique `mNumber`
    ResolveContext * AllocateResolveContext();

    ResolveContext * ResolveContextForHandle(size_t handle);
    void FreeResolveContext(size_t handle);
    void FreeResolveContext(const char * name);

    static void HandleClientState(AvahiClient * client, AvahiClientState state, void * context);
    void HandleClientState(AvahiClient * client, AvahiClientState state);

    static void HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state, void * context);
    void HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state);
    std::string GetGroupDescription(const AvahiEntryGroup * group) const;
    CHIP_ERROR PublishHostAddresses(const DnssdService & service, const std::string & hostname, AvahiIfIndex interface,
                                    std::string & key);
    static CHIP_ERROR GetHostAddresses(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType addressType,
                                       std::vector<HostAddress> & addresses);
    static std::vector<std::string> GetAddressKeys(const std::vector<HostAddress> & addresses);
    static CHIP_ERROR AddHostAddressRecords(AvahiEntryGroup * group, const std::string & hostname,
                                            const std::vector<HostAddress> & addresses);
    static CHIP_ERROR AddServiceRecords(AvahiEntryGroup * group, const ServiceGroup & serviceGroup);

    void ScheduleCollisionRetry();
    void CancelCollisionRetry();
    static void HandleCollisionRetryTimer(System::Layer * layer, void * context);
    void HandleCollisionRetryTimer();
    static CHIP_ERROR RepublishServiceGroup(const ServiceGroup & serviceGroup);

    static void HandleBrowse(AvahiServiceBrowser * broswer, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
                             const char * name, const char * type, const char * domain, AvahiLookupResultFlags flags,
                             void * userdata);
    static void InvokeDelegateOrCleanUp(BrowseContext * context, AvahiServiceBrowser * browser);
    static void HandleResolve(AvahiServiceResolver * resolver, AvahiIfIndex interface, AvahiProtocol protocol,
                              AvahiResolverEvent event, const char * name, const char * type, const char * domain,
                              const char * host_name, const AvahiAddress * address, uint16_t port, AvahiStringList * txt,
                              AvahiLookupResultFlags flags, void * userdata);

    DnssdAsyncReturnCallback mInitCallback;
    DnssdAsyncReturnCallback mErrorCallback;
    void * mAsyncReturnContext;

    AvahiClient * mClient;
    std::map<std::string, ServiceGroup> mPublishedGroups;
    std::map<std::string, HostGroup> mHostGroups;

    // Retrying DNS-SD advertising after an entry group collision.
    static constexpr System::Clock::Seconds32 kCollisionRetryInitialDelay = System::Clock::Seconds32(1);
    static constexpr System::Clock::Seconds32 kCollisionRetryMaxDelay     = System::Clock::Seconds32(32);
    DeviceLayer::Internal::ExponentialBackOff mCollisionRetryBackOff{ kCollisionRetryInitialDelay, kCollisionRetryMaxDelay,
                                                                      2 * kCollisionRetryMaxDelay };
    bool mCollisionRetryPending = false;
    Poller mPoller;
    static constexpr size_t kMaxBrowseRetries = 4;

    // Handling of allocated resolves
    size_t mResolveCount = 0;
    std::list<ResolveContext *> mAllocatedResolves;
};

} // namespace Dnssd
} // namespace chip
