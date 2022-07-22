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

#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/domain.h>
#include <avahi-common/error.h>
#include <avahi-common/watch.h>

#include "lib/dnssd/platform/Dnssd.h"

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
    MdnsAvahi(const MdnsAvahi &) = delete;
    MdnsAvahi & operator=(const MdnsAvahi &) = delete;

    CHIP_ERROR Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);
    void Shutdown();
    CHIP_ERROR SetHostname(const char * hostname);
    CHIP_ERROR PublishService(const DnssdService & service, DnssdPublishCallback callback, void * context);
    CHIP_ERROR StopPublish();
    CHIP_ERROR Browse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                      chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context);
    CHIP_ERROR Resolve(const char * name, const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                       chip::Inet::IPAddressType transportType, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                       void * context);

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
    };

    struct ResolveContext
    {
        MdnsAvahi * mInstance;
        DnssdResolveCallback mCallback;
        void * mContext;
        char mName[Common::kInstanceNameMaxLength + 1];
        AvahiIfIndex mInterface;
        AvahiProtocol mTransport;
        AvahiProtocol mAddressType;
        std::string mFullType;
        uint8_t mAttempts = 0;
    };

    MdnsAvahi() : mClient(nullptr), mGroup(nullptr) {}
    static MdnsAvahi sInstance;

    static void HandleClientState(AvahiClient * client, AvahiClientState state, void * context);
    void HandleClientState(AvahiClient * client, AvahiClientState state);

    static void HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state, void * context);
    void HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state);

    static void HandleBrowse(AvahiServiceBrowser * broswer, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
                             const char * name, const char * type, const char * domain, AvahiLookupResultFlags flags,
                             void * userdata);
    static void HandleResolve(AvahiServiceResolver * resolver, AvahiIfIndex interface, AvahiProtocol protocol,
                              AvahiResolverEvent event, const char * name, const char * type, const char * domain,
                              const char * host_name, const AvahiAddress * address, uint16_t port, AvahiStringList * txt,
                              AvahiLookupResultFlags flags, void * userdata);

    DnssdAsyncReturnCallback mInitCallback;
    DnssdAsyncReturnCallback mErrorCallback;
    void * mAsyncReturnContext;

    std::set<std::string> mPublishedServices;
    AvahiClient * mClient;
    AvahiEntryGroup * mGroup;
    Poller mPoller;
};

} // namespace Dnssd
} // namespace chip
