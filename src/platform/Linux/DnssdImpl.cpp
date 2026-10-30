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

#include <algorithm>
#include <sstream>
#include <string.h>
#include <string>
#include <time.h>
#include <vector>

#include <netinet/in.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemLayer.h>

using chip::Dnssd::DnssdServiceProtocol;
using chip::Dnssd::kDnssdTypeMaxSize;
using chip::Dnssd::TextEntry;
using chip::System::SocketEvents;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

namespace {

AvahiProtocol ToAvahiProtocol(chip::Inet::IPAddressType addressType)
{
#if INET_CONFIG_ENABLE_IPV4
    AvahiProtocol protocol;

    switch (addressType)
    {
    case chip::Inet::IPAddressType::kIPv4:
        protocol = AVAHI_PROTO_INET;
        break;
    case chip::Inet::IPAddressType::kIPv6:
        protocol = AVAHI_PROTO_INET6;
        break;
    default:
        protocol = AVAHI_PROTO_UNSPEC;
        break;
    }

    return protocol;
#else
    // We only support IPV6, never tell AVAHI about INET4 or UNSPEC because
    // UNSPEC may actually return IPv4 data.
    return AVAHI_PROTO_INET6;
#endif
}

chip::Inet::IPAddressType ToAddressType(AvahiProtocol protocol)
{
    chip::Inet::IPAddressType type;

    switch (protocol)
    {
#if INET_CONFIG_ENABLE_IPV4
    case AVAHI_PROTO_INET:
        type = chip::Inet::IPAddressType::kIPv4;
        break;
#endif
    case AVAHI_PROTO_INET6:
        type = chip::Inet::IPAddressType::kIPv6;
        break;
    default:
        type = chip::Inet::IPAddressType::kUnknown;
        break;
    }

    return type;
}

AvahiWatchEvent ToAvahiWatchEvent(SocketEvents events)
{
    return static_cast<AvahiWatchEvent>((events.Has(chip::System::SocketEventFlags::kRead) ? AVAHI_WATCH_IN : 0) |
                                        (events.Has(chip::System::SocketEventFlags::kWrite) ? AVAHI_WATCH_OUT : 0) |
                                        (events.Has(chip::System::SocketEventFlags::kError) ? AVAHI_WATCH_ERR : 0));
}

void AvahiWatchCallbackTrampoline(chip::System::SocketEvents events, intptr_t data)
{
    AvahiWatch * const watch = reinterpret_cast<AvahiWatch *>(data);
    watch->mPendingIO        = ToAvahiWatchEvent(events);
    watch->mCallback(watch, watch->mSocket, watch->mPendingIO, watch->mContext);
}

CHIP_ERROR MakeAvahiStringListFromTextEntries(TextEntry * entries, size_t size, AvahiStringList ** strListOut)
{
    *strListOut = avahi_string_list_new(nullptr, nullptr);

    for (size_t i = 0; i < size; i++)
    {
        uint8_t buf[chip::Dnssd::kDnssdTextMaxSize];
        size_t offset = static_cast<size_t>(snprintf(reinterpret_cast<char *>(buf), sizeof(buf), "%s=", entries[i].mKey));

        if (offset + entries[i].mDataSize > sizeof(buf))
        {
            avahi_string_list_free(*strListOut);
            *strListOut = nullptr;
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        memcpy(&buf[offset], entries[i].mData, entries[i].mDataSize);
        *strListOut = avahi_string_list_add_arbitrary(*strListOut, buf, offset + entries[i].mDataSize);
    }
    return CHIP_NO_ERROR;
}

const char * GetAddressTypeString(chip::Inet::IPAddressType addressType)
{
    switch (addressType)
    {
    case chip::Inet::IPAddressType::kIPv6:
        return "ipv6";
#if INET_CONFIG_ENABLE_IPV4
    case chip::Inet::IPAddressType::kIPv4:
        return "ipv4";
#endif
    default:
        return "any";
    }
}

bool IsGroupLive(AvahiEntryGroup * group)
{
    const int state = avahi_entry_group_get_state(group);

    return (state == AVAHI_ENTRY_GROUP_REGISTERING) || (state == AVAHI_ENTRY_GROUP_ESTABLISHED);
}

const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? "_udp" : "_tcp";
}

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type << "." << GetProtocolString(protocol);
    return typeBuilder.str();
}

} // namespace

namespace chip {
namespace Dnssd {

MdnsAvahi MdnsAvahi::sInstance;

Poller::Poller()
{
    mAvahiPoller.userdata         = this;
    mAvahiPoller.watch_new        = WatchNew;
    mAvahiPoller.watch_update     = WatchUpdate;
    mAvahiPoller.watch_get_events = WatchGetEvents;
    mAvahiPoller.watch_free       = WatchFree;

    mAvahiPoller.timeout_new    = TimeoutNew;
    mAvahiPoller.timeout_update = TimeoutUpdate;
    mAvahiPoller.timeout_free   = TimeoutFree;

    mEarliestTimeout = std::chrono::steady_clock::time_point();
}

AvahiWatch * Poller::WatchNew(const struct AvahiPoll * poller, int fd, AvahiWatchEvent event, AvahiWatchCallback callback,
                              void * context)
{
    return reinterpret_cast<Poller *>(poller->userdata)->WatchNew(fd, event, callback, context);
}

AvahiWatch * Poller::WatchNew(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void * context)
{
    VerifyOrDie(callback != nullptr && fd >= 0);

    auto watch     = std::make_unique<AvahiWatch>();
    watch->mSocket = fd;
    LogErrorOnFailure(DeviceLayer::SystemLayerSockets().StartWatchingSocket(fd, &watch->mSocketWatch));
    LogErrorOnFailure(DeviceLayer::SystemLayerSockets().SetCallback(watch->mSocketWatch, AvahiWatchCallbackTrampoline,
                                                                    reinterpret_cast<intptr_t>(watch.get())));
    WatchUpdate(watch.get(), event);
    watch->mCallback = callback;
    watch->mContext  = context;
    watch->mPoller   = this;
    mWatches.emplace_back(std::move(watch));

    return mWatches.back().get();
}

void Poller::WatchUpdate(AvahiWatch * watch, AvahiWatchEvent event)
{
    if (event & AVAHI_WATCH_IN)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayerSockets().RequestCallbackOnPendingRead(watch->mSocketWatch));
    }
    else
    {
        LogErrorOnFailure(DeviceLayer::SystemLayerSockets().ClearCallbackOnPendingRead(watch->mSocketWatch));
    }
    if (event & AVAHI_WATCH_OUT)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayerSockets().RequestCallbackOnPendingWrite(watch->mSocketWatch));
    }
    else
    {
        LogErrorOnFailure(DeviceLayer::SystemLayerSockets().ClearCallbackOnPendingWrite(watch->mSocketWatch));
    }
}

AvahiWatchEvent Poller::WatchGetEvents(AvahiWatch * watch)
{
    return watch->mPendingIO;
}

void Poller::WatchFree(AvahiWatch * watch)
{
    reinterpret_cast<Poller *>(watch->mPoller)->WatchFree(*watch);
}

void Poller::WatchFree(AvahiWatch & watch)
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayerSockets().StopWatchingSocket(&watch.mSocketWatch);
    mWatches.erase(std::remove_if(mWatches.begin(), mWatches.end(),
                                  [&watch](const std::unique_ptr<AvahiWatch> & aValue) { return aValue.get() == &watch; }),
                   mWatches.end());
}

AvahiTimeout * Poller::TimeoutNew(const AvahiPoll * poller, const struct timeval * timeout, AvahiTimeoutCallback callback,
                                  void * context)
{
    VerifyOrDie(poller != nullptr && callback != nullptr);

    return static_cast<Poller *>(poller->userdata)->TimeoutNew(timeout, callback, context);
}

steady_clock::time_point GetAbsTimeout(const struct timeval * timeout)
{
    steady_clock::time_point now        = steady_clock::now();
    steady_clock::time_point absTimeout = now;

    if (timeout != nullptr)
    {
        absTimeout += seconds(timeout->tv_sec);
        absTimeout += microseconds(timeout->tv_usec);
    }

    return absTimeout;
}

AvahiTimeout * Poller::TimeoutNew(const struct timeval * timeout, AvahiTimeoutCallback callback, void * context)
{
    mTimers.emplace_back(new AvahiTimeout{ GetAbsTimeout(timeout), callback, timeout != nullptr, context, this });
    AvahiTimeout * timer = mTimers.back().get();
    SystemTimerUpdate(timer);
    return timer;
}

void Poller::TimeoutUpdate(AvahiTimeout * timer, const struct timeval * timeout)
{
    if (timeout)
    {
        timer->mAbsTimeout = GetAbsTimeout(timeout);
        timer->mEnabled    = true;
        static_cast<Poller *>(timer->mPoller)->SystemTimerUpdate(timer);
    }
    else
    {
        timer->mEnabled = false;
    }
}

void Poller::TimeoutFree(AvahiTimeout * timer)
{
    static_cast<Poller *>(timer->mPoller)->TimeoutFree(*timer);
}

void Poller::TimeoutFree(AvahiTimeout & timer)
{
    mTimers.erase(std::remove_if(mTimers.begin(), mTimers.end(),
                                 [&timer](const std::unique_ptr<AvahiTimeout> & aValue) { return aValue.get() == &timer; }),
                  mTimers.end());
}

void Poller::SystemTimerCallback(System::Layer * layer, void * data)
{
    static_cast<Poller *>(data)->HandleTimeout();
}

void Poller::HandleTimeout()
{
    mEarliestTimeout             = std::chrono::steady_clock::time_point();
    steady_clock::time_point now = steady_clock::now();

    AvahiTimeout * earliest = nullptr;
    for (auto && timer : mTimers)
    {
        if (!timer->mEnabled)
        {
            continue;
        }
        if (timer->mAbsTimeout <= now)
        {
            timer->mCallback(timer.get(), timer->mContext);
        }
        else
        {
            if ((earliest == nullptr) || (timer->mAbsTimeout < earliest->mAbsTimeout))
            {
                earliest = timer.get();
            }
        }
    }
    if (earliest)
    {
        SystemTimerUpdate(earliest);
    }
}

void Poller::SystemTimerUpdate(AvahiTimeout * timer)
{
    if ((mEarliestTimeout == std::chrono::steady_clock::time_point()) || (timer->mAbsTimeout < mEarliestTimeout))
    {
        mEarliestTimeout = timer->mAbsTimeout;
        auto delay       = std::chrono::duration_cast<chip::System::Clock::Milliseconds32>(steady_clock::now() - mEarliestTimeout);
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(delay, SystemTimerCallback, this);
    }
}

CHIP_ERROR MdnsAvahi::Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int avahiError   = 0;

    Shutdown();

    VerifyOrExit(initCallback != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(errorCallback != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mClient == nullptr && mPublishedGroups.empty() && mHostGroups.empty(), error = CHIP_ERROR_INCORRECT_STATE);
    mInitCallback       = initCallback;
    mErrorCallback      = errorCallback;
    mAsyncReturnContext = context;
    mClient             = avahi_client_new(mPoller.GetAvahiPoll(), AVAHI_CLIENT_NO_FAIL, HandleClientState, this, &avahiError);
    VerifyOrExit(mClient != nullptr, error = CHIP_ERROR_OPEN_FAILED);
    VerifyOrExit(avahiError == 0, error = CHIP_ERROR_OPEN_FAILED);

exit:
    return error;
}

MdnsAvahi::~MdnsAvahi()
{
    // Only the destruction of sInstance at program exit gets here, and the
    // groups are still held only if the process did not call Shutdown().
    // Freeing them now would make a D-Bus call through the client, and
    // through it reach mPoller, which is destroyed before the groups. Leave
    // them to avahi-daemon instead, which frees a client's groups when the
    // client's bus connection closes.

    for (auto & publishedGroup : mPublishedGroups)
    {
        static_cast<void>(publishedGroup.second.mGroup.release());
    }

    for (auto & hostGroup : mHostGroups)
    {
        static_cast<void>(hostGroup.second.mGroup.release());
    }
}

void MdnsAvahi::Shutdown()
{
    TEMPORARY_RETURN_IGNORED StopPublish();
    LogErrorOnFailure(FinalizePublish());
    mCollisionRetryBackOff.Reset();
    if (mClient)
    {
        avahi_client_free(mClient);
        mClient = nullptr;
    }
}

CHIP_ERROR MdnsAvahi::SetHostname(const char * hostname)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(mClient != nullptr, error = CHIP_ERROR_INCORRECT_STATE);
    // Note: we do no longer set the primary hostname here, as other services
    // on the platform might not be happy with the matter mandated hostname.
    // Instead, we'll establish our own hostname when needed (see PublishService())
exit:
    return error;
}

void MdnsAvahi::HandleClientState(AvahiClient * client, AvahiClientState state, void * context)
{
    static_cast<MdnsAvahi *>(context)->HandleClientState(client, state);
}

void MdnsAvahi::HandleClientState(AvahiClient * client, AvahiClientState state)
{
    switch (state)
    {
    case AVAHI_CLIENT_S_RUNNING:
        ChipLogProgress(DeviceLayer, "Avahi client registered");
        mClient = client;
        // no longer create groups here, but on a by-service basis in PublishService()
        mInitCallback(mAsyncReturnContext, CHIP_NO_ERROR);
        break;
    case AVAHI_CLIENT_FAILURE:
        ChipLogError(DeviceLayer, "Avahi client failure");
        CancelCollisionRetry();
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_CLIENT_S_COLLISION:
    case AVAHI_CLIENT_S_REGISTERING:
        ChipLogProgress(DeviceLayer, "Avahi re-register required");
        TEMPORARY_RETURN_IGNORED StopPublish();
        LogErrorOnFailure(FinalizePublish());
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_FORCED_RESET);
        break;
    case AVAHI_CLIENT_CONNECTING:
        ChipLogProgress(DeviceLayer, "Avahi connecting");
        break;
    }
}

void MdnsAvahi::AvahiEntryGroupDeleter::operator()(AvahiEntryGroup * group) const
{
    const int avahiRet = avahi_entry_group_free(group);

    if (avahiRet != AVAHI_OK)
    {
        ChipLogError(DeviceLayer, "Cannot free avahi group: %d: %s", avahiRet, avahi_strerror(avahiRet));
    }
}

void MdnsAvahi::HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state, void * context)
{
    static_cast<MdnsAvahi *>(context)->HandleGroupState(group, state);
}

void MdnsAvahi::HandleGroupState(AvahiEntryGroup * group, AvahiEntryGroupState state)
{
    switch (state)
    {
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
        ChipLogProgress(DeviceLayer, "Avahi group established: %s", GetGroupDescription(group).c_str());
        break;
    case AVAHI_ENTRY_GROUP_COLLISION:
        ChipLogError(DeviceLayer, "Avahi group collision: %s", GetGroupDescription(group).c_str());
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_MDNS_COLLISION);
        ScheduleCollisionRetry();
        break;
    case AVAHI_ENTRY_GROUP_FAILURE:
        ChipLogError(DeviceLayer, "Avahi group internal failure: %s: %s", GetGroupDescription(group).c_str(),
                     avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(group))));
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
    case AVAHI_ENTRY_GROUP_REGISTERING:
        break;
    }
}

void MdnsAvahi::ScheduleCollisionRetry()
{
    // Avahi withdraws an entry group that collides, and nothing else
    // publishes it again: the service stays undiscoverable until the next
    // re-advertisement, which may never come. The Matter SDK has no way
    // to rename an instance on a collision, as RFC 6762, section 9, would
    // otherwise require: the operational instance name is fixed by the
    // compressed fabric and node identifiers, and the commissionable one
    // is chosen once per DNS-SD initialization. Publishing again, once
    // whatever answered with the conflicting record has forgotten it, is
    // the only way to recover.
    //
    // Publish each collided group again, and only those, after a delay
    // that doubles with each consecutive retry, from
    // kCollisionRetryInitialDelay up to kCollisionRetryMaxDelay, and
    // starts over once no retry has been scheduled for twice the longest
    // delay. One pending retry covers every collision until it fires.

    if (mCollisionRetryPending)
    {
        return;
    }

    const System::Clock::Seconds32 delay = mCollisionRetryBackOff.NextDelay(System::SystemClock().GetMonotonicTimestamp());

    ReturnAndLogOnFailure(DeviceLayer::SystemLayer().StartTimer(delay, HandleCollisionRetryTimer, this), DeviceLayer,
                          "Cannot schedule DNS-SD publication retry");

    mCollisionRetryPending = true;

    ChipLogProgress(DeviceLayer, "Publishing collided Avahi group(s) again in %" PRIu32 " s", delay.count());
}

void MdnsAvahi::CancelCollisionRetry()
{
    DeviceLayer::SystemLayer().CancelTimer(HandleCollisionRetryTimer, this);
    mCollisionRetryPending = false;
}

void MdnsAvahi::HandleCollisionRetryTimer(System::Layer *, void * context)
{
    static_cast<MdnsAvahi *>(context)->HandleCollisionRetryTimer();
}

void MdnsAvahi::HandleCollisionRetryTimer()
{
    mCollisionRetryPending = false;

    // Nothing can be published while the client is not running. A client
    // that reconnects re-registers, which frees every group, and its user
    // publishes from scratch.

    if ((mClient == nullptr) || (avahi_client_get_state(mClient) != AVAHI_CLIENT_S_RUNNING))
    {
        ChipLogError(DeviceLayer, "Avahi client not running; not publishing collided Avahi group(s) again");
        return;
    }

    // Only service groups are published again. A host group cannot
    // collide: its records are published with AVAHI_PUBLISH_NO_PROBE, so
    // Avahi moves the group straight from registering to established,
    // and answers a conflicting record by announcing its own again rather
    // than by withdrawing the group. A host group that is not registered
    // for any other reason is published anew at the next
    // re-advertisement (see PublishHostAddresses()).
    //
    // A group that collided is in AVAHI_ENTRY_GROUP_COLLISION; one whose
    // earlier republication failed partway is left uncommitted. Publish
    // either again. If that fails, try again later.

    bool failed = false;

    for (auto & serviceGroup : mPublishedGroups)
    {
        if (IsGroupLive(serviceGroup.second.mGroup.get()))
        {
            continue;
        }

        ChipLogProgress(DeviceLayer, "Publishing Avahi group again: service %s", serviceGroup.first.c_str());

        const CHIP_ERROR error = RepublishServiceGroup(serviceGroup.second);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Cannot publish Avahi group again: service %s: %" CHIP_ERROR_FORMAT,
                         serviceGroup.first.c_str(), error.Format());
            failed = true;
        }
    }

    if (failed)
    {
        ScheduleCollisionRetry();
    }
}

CHIP_ERROR MdnsAvahi::RepublishServiceGroup(const ServiceGroup & serviceGroup)
{
    AvahiEntryGroup * group = serviceGroup.mGroup.get();

    // A collided group keeps nothing: Avahi has withdrawn every record in
    // it. Reset it, add its records again, and commit it again. The reset
    // also clears whatever an earlier, failed republication left in it.

    int avahiRet = avahi_entry_group_reset(group);
    if (avahiRet != AVAHI_OK)
    {
        ChipLogError(DeviceLayer, "Cannot reset avahi group: %d: %s", avahiRet, avahi_strerror(avahiRet));
        return CHIP_ERROR_INTERNAL;
    }

    ReturnErrorOnFailure(AddServiceRecords(group, serviceGroup));

    avahiRet = avahi_entry_group_commit(group);
    if (avahiRet != AVAHI_OK)
    {
        ChipLogError(DeviceLayer, "Cannot commit avahi group: %d: %s", avahiRet, avahi_strerror(avahiRet));
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

std::string MdnsAvahi::GetGroupDescription(const AvahiEntryGroup * group) const
{
    // Every entry group reports its state changes through the one
    // callback. Name the group by its publication key, so that a log
    // says which publication changed state. A group not yet in either
    // map, such as one whose initial state is reported from within
    // avahi_entry_group_new(), is unknown.

    for (const auto & publishedGroup : mPublishedGroups)
    {
        if (publishedGroup.second.mGroup.get() == group)
        {
            return "service " + publishedGroup.first;
        }
    }

    for (const auto & hostGroup : mHostGroups)
    {
        if (hostGroup.second.mGroup.get() == group)
        {
            return "host " + hostGroup.first;
        }
    }

    return "unknown";
}

CHIP_ERROR MdnsAvahi::PublishService(const DnssdService & service, DnssdPublishCallback callback, void * context)
{
    std::ostringstream keyBuilder;
    std::string key;
    std::string type = GetFullType(service.mType, service.mProtocol);
    std::string matterHostname;
    std::string hostGroupKey;
    CHIP_ERROR error          = CHIP_NO_ERROR;
    int avahiRet              = AVAHI_OK;
    AvahiStringList * text    = nullptr;
    const char * mainHostname = nullptr;
    ServiceGroup serviceGroup;
    AvahiIfIndex interface =
        service.mInterface.IsPresent() ? static_cast<AvahiIfIndex>(service.mInterface.GetPlatformInterface()) : AVAHI_IF_UNSPEC;
    AvahiProtocol protocol = ToAvahiProtocol(service.mAddressType);

    keyBuilder << service.mName << "." << type << service.mPort << "." << interface;
    key = keyBuilder.str();
    ChipLogProgress(DeviceLayer, "PublishService %s", key.c_str());
    auto publishedgroups_it = mPublishedGroups.find(key);
    if (publishedgroups_it != mPublishedGroups.end())
    {
        // same service was already published, we need to de-publish it first;
        // avahi_entry_group_free() frees the group even when it fails, so
        // forget the group either way
        avahiRet = avahi_entry_group_free(publishedgroups_it->second.mGroup.release());
        mPublishedGroups.erase(publishedgroups_it);
        if (avahiRet != AVAHI_OK)
        {
            ChipLogError(DeviceLayer, "Cannot remove avahi group: %s", avahi_strerror(avahiRet));
            ExitNow(error = CHIP_ERROR_INTERNAL);
        }
    }

    // establish the host name (separately from avahi's default host name that the platform might have,
    // unless it matches the matter hostname)
    mainHostname = avahi_client_get_host_name(mClient);
    if (strcmp(mainHostname, service.mHostName) == 0)
    {
        // main host name is correct, we can use it
        matterHostname = std::string(mainHostname) + ".local";
    }
    else
    {
        // we need to establish a matter hostname separately from the platform's default hostname
        matterHostname = std::string(service.mHostName) + ".local";
        SuccessOrExit(error = PublishHostAddresses(service, matterHostname, interface, hostGroupKey));
    }

    // Record what the service is published with, so that it can be
    // published again after a collision.

    SuccessOrExit(error = MakeAvahiStringListFromTextEntries(service.mTextEntries, service.mTextEntrySize, &text));

    serviceGroup.mInterface = interface;
    serviceGroup.mProtocol  = protocol;
    serviceGroup.mName      = service.mName;
    serviceGroup.mType      = type;
    serviceGroup.mHost      = matterHostname;
    serviceGroup.mPort      = service.mPort;
    serviceGroup.mText.reset(text);
    text = nullptr;

    for (size_t i = 0; i < service.mSubTypeSize; i++)
    {
        std::ostringstream sstream;

        sstream << service.mSubTypes[i] << "._sub." << type;

        serviceGroup.mSubTypes.push_back(sstream.str());
    }

    // create fresh group; it is freed again, unpublished, if a step below fails
    serviceGroup.mGroup.reset(avahi_entry_group_new(mClient, HandleGroupState, this));
    VerifyOrExit(serviceGroup.mGroup != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = AddServiceRecords(serviceGroup.mGroup.get(), serviceGroup));

    VerifyOrExit(avahi_entry_group_commit(serviceGroup.mGroup.get()) == 0, error = CHIP_ERROR_INTERNAL);

    // group is now published, pass it to the service map
    mPublishedGroups[key] = std::move(serviceGroup);

    // Only a published service claims its host's address records, so that
    // FinalizePublish() frees those of a service whose publication failed.
    if (!hostGroupKey.empty())
    {
        auto hostgroups_it = mHostGroups.find(hostGroupKey);
        if (hostgroups_it != mHostGroups.end())
        {
            hostgroups_it->second.mInUse = true;
        }
    }

exit:
    if (text != nullptr)
    {
        avahi_string_list_free(text);
    }

    // Ideally the callback would be called from `HandleGroupState` when the `AVAHI_ENTRY_GROUP_ESTABLISHED` state
    // is received. But the current code use the `userdata` field to pass a pointer to the current MdnsAvahi instance
    // and this is all comes from MdnsAvahi::Init that does not have any clue about the `type` that *will* be published.
    // The code needs to be updated to support that callback properly.
    if (CHIP_NO_ERROR == error)
    {
        callback(context, type.c_str(), service.mName, CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(DeviceLayer, "PublishService failed: %s",
                     mClient ? avahi_strerror(avahi_client_errno(mClient)) : "no mClient");
        callback(context, nullptr, nullptr, error);
    }

    return error;
}

CHIP_ERROR MdnsAvahi::AddServiceRecords(AvahiEntryGroup * group, const ServiceGroup & serviceGroup)
{
    // create the service
    int avahiRet =
        avahi_entry_group_add_service_strlst(group, serviceGroup.mInterface, serviceGroup.mProtocol, // group, interface, protocol
                                             static_cast<AvahiPublishFlags>(0),                      // publish flags
                                             serviceGroup.mName.c_str(),                             // service name
                                             serviceGroup.mType.c_str(),                             // type
                                             nullptr,                                                // domain
                                             serviceGroup.mHost.c_str(),                             // host
                                             serviceGroup.mPort,                                     // port
                                             serviceGroup.mText.get());                              // TXT records StringList
    if (avahiRet != AVAHI_OK)
    {
        ChipLogError(DeviceLayer, "Cannot add avahi service string list: %d: %s", avahiRet, avahi_strerror(avahiRet));
        return CHIP_ERROR_INTERNAL;
    }

    // add the subtypes
    for (const auto & subType : serviceGroup.mSubTypes)
    {
        avahiRet = avahi_entry_group_add_service_subtype(group, serviceGroup.mInterface, serviceGroup.mProtocol,
                                                         static_cast<AvahiPublishFlags>(0), serviceGroup.mName.c_str(),
                                                         serviceGroup.mType.c_str(), nullptr, subType.c_str());
        if (avahiRet != AVAHI_OK)
        {
            ChipLogError(DeviceLayer, "Cannot add avahi service string subtype: %d: %s", avahiRet, avahi_strerror(avahiRet));
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MdnsAvahi::PublishHostAddresses(const DnssdService & service, const std::string & hostname, AvahiIfIndex interface,
                                           std::string & key)
{
    char b[chip::Inet::IPAddress::kMaxStringLength];
    ReturnErrorOnFailure(service.mInterface.GetInterfaceName(b, sizeof(b)));
    ChipLogDetail(DeviceLayer, "Using addresses from interface id=%d name=%s", service.mInterface.GetPlatformInterface(), b);

    std::vector<HostAddress> addresses;
    ReturnErrorOnFailure(GetHostAddresses(service.mInterface, service.mAddressType, addresses));

    // A dedicated Matter hostname is only resolvable if at least one A/AAAA
    // record backs it. Publishing SRV/PTR/TXT without any address records
    // yields a service that browses but can never be resolved, and nothing
    // downstream would ever detect or repair it. Fail loudly instead so the
    // caller can retry (for example, on a subsequent connectivity-change event).

    if (addresses.empty())
    {
        ChipLogError(DeviceLayer,
                     "No publishable addresses for hostname %s (interface id=%d): "
                     "advertising interface has no usable addresses yet; "
                     "refusing to publish unresolvable service %s",
                     hostname.c_str(), service.mInterface.GetPlatformInterface(), service.mName);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    std::vector<std::string> addressKeys = GetAddressKeys(addresses);

    // A host group publishes the addresses of one address type
    // (service.mAddressType) on one interface restriction, so key it by
    // both: services that differ in either keep groups of their own,
    // rather than replacing each other's address records in turn.

    std::ostringstream keyBuilder;
    keyBuilder << hostname << "." << interface << "." << GetAddressTypeString(service.mAddressType);
    key = keyBuilder.str();

    // The address records already published for this host are exactly
    // these, and are still registered: leave them published, untouched. A
    // group that has collided or failed is published again instead.

    auto hostgroups_it = mHostGroups.find(key);
    if ((hostgroups_it != mHostGroups.end()) && (hostgroups_it->second.mAddresses == addressKeys) &&
        IsGroupLive(hostgroups_it->second.mGroup.get()))
    {
        ChipLogProgress(DeviceLayer, "Address record(s) for %s unchanged (%u)", hostname.c_str(),
                        static_cast<unsigned>(addressKeys.size()));
        return CHIP_NO_ERROR;
    }

    // Publish the address records in an entry group of their own, apart
    // from any service's, and without probing them
    // (AVAHI_PUBLISH_NO_PROBE). Avahi then announces them at once, with
    // the cache-flush bit set, and answers any conflicting response by
    // announcing them again rather than by withdrawing them.
    //
    // Probing the host's address records protects nothing: the Matter
    // hostname is derived from the node's own MAC address, so no other
    // host can rightfully claim it (RFC 6762, section 8.1, skips probing
    // for records known to be unique). Probing them is also harmful: a
    // Multicast DNS gateway or proxy that answers from a stale cache, with
    // an address the node has since lost, makes Avahi withdraw the whole
    // entry group, and with it the service, as a collision that nothing
    // resolves. Matter's own minimal mDNS advertiser does not probe at all.
    //
    // Service records stay in the service's own group and are probed as
    // before; Avahi does not allow AVAHI_PUBLISH_NO_PROBE for them.

    AvahiEntryGroupPtr group(avahi_entry_group_new(mClient, HandleGroupState, this));
    VerifyOrReturnError(group != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(AddHostAddressRecords(group.get(), hostname, addresses));

    const int avahiRet = avahi_entry_group_commit(group.get());
    if (avahiRet != AVAHI_OK)
    {
        ChipLogError(DeviceLayer, "Cannot commit avahi group for %s: %d: %s", hostname.c_str(), avahiRet, avahi_strerror(avahiRet));
        return CHIP_ERROR_INTERNAL;
    }

    // Make before break: replacing the group frees the one with the
    // previous address records only now that the new one is committed.
    // Avahi sends no goodbye for a record that the new group publishes
    // again, so only the addresses no longer present are withdrawn. A
    // replaced group's claim, if any, carries over to the new one.

    HostGroup & hostGroup = mHostGroups[key];

    hostGroup.mGroup     = std::move(group);
    hostGroup.mAddresses = std::move(addressKeys);

    ChipLogProgress(DeviceLayer, "Published %u address record(s) for %s", static_cast<unsigned>(addresses.size()),
                    hostname.c_str());

    return CHIP_NO_ERROR;
}

CHIP_ERROR MdnsAvahi::GetHostAddresses(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType addressType,
                                       std::vector<HostAddress> & addresses)
{
    addresses.clear();

    // find addresses to publish
    for (chip::Inet::InterfaceAddressIterator addr_it; addr_it.HasCurrent(); addr_it.Next())
    {
        // only specific interface?
        if (interfaceId.IsPresent() && addr_it.GetInterfaceId() != interfaceId)
        {
            continue;
        }
        if (!addr_it.IsUp() || addr_it.IsLoopback())
        {
            // do not advertise down or loopback interface addresses
            continue;
        }

        chip::Inet::IPAddress addr;
        if ((addr_it.GetAddress(addr) != CHIP_NO_ERROR) ||
            !((addressType == chip::Inet::IPAddressType::kAny) || (addr.IsIPv6() && addressType == chip::Inet::IPAddressType::kIPv6)
#if INET_CONFIG_ENABLE_IPV4
              || (addr.IsIPv4() && addressType == chip::Inet::IPAddressType::kIPv4)
#endif
                  ))
        {
            continue;
        }

        char b[chip::Inet::IPAddress::kMaxStringLength];
        HostAddress hostAddress;

        VerifyOrReturnError(addr.ToString(b) != nullptr, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(avahi_address_parse(b, AVAHI_PROTO_UNSPEC, &hostAddress.mAddress) != nullptr, CHIP_ERROR_INTERNAL);

        hostAddress.mInterface = static_cast<AvahiIfIndex>(addr_it.GetInterfaceId().GetPlatformInterface());
        hostAddress.mProtocol  = ToAvahiProtocol(addr.Type());
        hostAddress.mText      = b;

        addresses.push_back(hostAddress);
    }

    return CHIP_NO_ERROR;
}

std::vector<std::string> MdnsAvahi::GetAddressKeys(const std::vector<HostAddress> & addresses)
{
    std::vector<std::string> addressKeys;

    for (const auto & hostAddress : addresses)
    {
        addressKeys.push_back(std::to_string(hostAddress.mInterface) + "/" + hostAddress.mText);
    }

    std::sort(addressKeys.begin(), addressKeys.end());

    return addressKeys;
}

CHIP_ERROR MdnsAvahi::AddHostAddressRecords(AvahiEntryGroup * group, const std::string & hostname,
                                            const std::vector<HostAddress> & addresses)
{
    for (const auto & hostAddress : addresses)
    {
        // Note: NO_REVERSE publish flag is needed because otherwise we can't have more than one hostname
        //   for reverse resolving IP addresses back to hostnames
        const int avahiRet = avahi_entry_group_add_address(group,                                                    // group
                                                           hostAddress.mInterface,                                   // interface
                                                           hostAddress.mProtocol,                                    // protocol
                                                           static_cast<AvahiPublishFlags>(AVAHI_PUBLISH_NO_REVERSE | // publish
                                                                                          AVAHI_PUBLISH_NO_PROBE),   //   flags
                                                           hostname.c_str(),                                         // hostname
                                                           &hostAddress.mAddress);                                   // address
        if (avahiRet != AVAHI_OK)
        {
            ChipLogError(DeviceLayer, "Cannot add avahi group address for %s (%d) for %s: %d: %s", hostAddress.mText.c_str(),
                         static_cast<int>(hostAddress.mInterface), hostname.c_str(), avahiRet, avahi_strerror(avahiRet));
            return CHIP_ERROR_INTERNAL;
        }

        ChipLogDetail(DeviceLayer, "Added address %s (if=%d) for %s", hostAddress.mText.c_str(),
                      static_cast<int>(hostAddress.mInterface), hostname.c_str());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MdnsAvahi::StopPublish()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    for (auto & group : mPublishedGroups)
    {
        if (group.second.mGroup)
        {
            int avahiRet = avahi_entry_group_free(group.second.mGroup.release());
            if (avahiRet != AVAHI_OK)
            {
                ChipLogError(DeviceLayer, "Error freeing avahi group: %s", avahi_strerror(avahiRet));
                error = CHIP_ERROR_INTERNAL;
            }
        }
    }
    mPublishedGroups.clear();

    // Advertising is being removed, or redone from scratch: either way,
    // a pending collision retry is superseded.

    CancelCollisionRetry();

    // Keep the host address groups published across a service update:
    // mark them unclaimed, so that PublishService() can leave unchanged
    // address records alone, and FinalizePublish() frees those that no
    // service claims again.

    for (auto & hostGroup : mHostGroups)
    {
        hostGroup.second.mInUse = false;
    }

    return error;
}

CHIP_ERROR MdnsAvahi::FinalizePublish()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    for (auto hostgroups_it = mHostGroups.begin(); hostgroups_it != mHostGroups.end();)
    {
        if (hostgroups_it->second.mInUse)
        {
            ++hostgroups_it;
            continue;
        }

        int avahiRet = avahi_entry_group_free(hostgroups_it->second.mGroup.release());
        if (avahiRet != AVAHI_OK)
        {
            ChipLogError(DeviceLayer, "Error freeing avahi group: %s", avahi_strerror(avahiRet));
            error = CHIP_ERROR_INTERNAL;
        }

        hostgroups_it = mHostGroups.erase(hostgroups_it);
    }

    return error;
}

CHIP_ERROR MdnsAvahi::Browse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                             chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                             intptr_t * browseIdentifier)
{
    AvahiServiceBrowser * browser;
    BrowseContext * browseContext = chip::Platform::New<BrowseContext>();
    AvahiIfIndex avahiInterface   = static_cast<AvahiIfIndex>(interface.GetPlatformInterface());

    browseContext->mInstance    = this;
    browseContext->mContext     = context;
    browseContext->mCallback    = callback;
    browseContext->mAddressType = addressType;
    if (!interface.IsPresent())
    {
        avahiInterface = AVAHI_IF_UNSPEC;
    }
    browseContext->mInterface         = avahiInterface;
    browseContext->mProtocol          = GetFullType(type, protocol);
    browseContext->mReceivedAllCached = false;
    browseContext->mStopped.store(false);

    browser = avahi_service_browser_new(mClient, avahiInterface, AVAHI_PROTO_UNSPEC, browseContext->mProtocol.c_str(), nullptr,
                                        static_cast<AvahiLookupFlags>(0), HandleBrowse, browseContext);
    // Otherwise the browser will be freed in the callback
    if (browser == nullptr)
    {
        chip::Platform::Delete(browseContext);
        *browseIdentifier = reinterpret_cast<intptr_t>(nullptr);
    }
    else
    {
        *browseIdentifier = reinterpret_cast<intptr_t>(browseContext);
    }

    return browser == nullptr ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

CHIP_ERROR MdnsAvahi::StopBrowse(intptr_t browseIdentifier)
{
    BrowseContext * browseContext = reinterpret_cast<BrowseContext *>(browseIdentifier);
    if (browseContext == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    // Any running timers here will check mStopped before rescheduling. Leave the timer running
    // so we don't race on deletion of the browse context.
    browseContext->mStopped.store(true);
    return CHIP_NO_ERROR;
}

DnssdServiceProtocol GetProtocolInType(const char * type)
{
    const char * deliminator = strrchr(type, '.');

    if (deliminator == nullptr)
    {
        ChipLogError(Discovery, "Failed to find protocol in type: %s", StringOrNullMarker(type));
        return DnssdServiceProtocol::kDnssdProtocolUnknown;
    }

    if (strcmp("._tcp", deliminator) == 0)
    {
        return DnssdServiceProtocol::kDnssdProtocolTcp;
    }
    if (strcmp("._udp", deliminator) == 0)
    {
        return DnssdServiceProtocol::kDnssdProtocolUdp;
    }

    ChipLogError(Discovery, "Unknown protocol in type: %s", StringOrNullMarker(type));
    return DnssdServiceProtocol::kDnssdProtocolUnknown;
}

/// Copies the type from a string containing both type and protocol
///
/// e.g. if input is "foo.bar", output is "foo", input is 'a.b._tcp", output is "a.b"
template <size_t N>
void CopyTypeWithoutProtocol(char (&dest)[N], const char * typeAndProtocol)
{
    const char * dotPos          = strrchr(typeAndProtocol, '.');
    size_t lengthWithoutProtocol = (dotPos != nullptr) ? static_cast<size_t>(dotPos - typeAndProtocol) : N;

    Platform::CopyString(dest, typeAndProtocol);

    /// above copied everything including the protocol. Truncate the protocol away.
    if (lengthWithoutProtocol < N)
    {
        dest[lengthWithoutProtocol] = 0;
    }
}

void MdnsAvahi::InvokeDelegateOrCleanUp(BrowseContext * context, AvahiServiceBrowser * browser)
{
    // If we were already asked to stop, no need to send a callback - no one is listening.
    if (!context->mStopped.load())
    {
        // since this is continuous browse, finalBrowse will always be false.
        context->mCallback(context->mContext, context->mServices.data(), context->mServices.size(), false, CHIP_NO_ERROR);

        // Clearing records/services already passed to application through delegate. Keeping it may cause
        // duplicates in next query / retry attempt as currently found will also come again from cache.
        context->mServices.clear();
    }
    else
    {
        // browse is stopped, so free browse handle and context
        avahi_service_browser_free(browser);
        chip::Platform::Delete(context);
    }
}

void MdnsAvahi::HandleBrowse(AvahiServiceBrowser * browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
                             const char * name, const char * type, const char * domain, AvahiLookupResultFlags /*flags*/,
                             void * userdata)
{
    BrowseContext * context = static_cast<BrowseContext *>(userdata);

    switch (event)
    {
    case AVAHI_BROWSER_FAILURE:
        context->mCallback(context->mContext, nullptr, 0, true, CHIP_ERROR_INTERNAL);
        avahi_service_browser_free(browser);
        chip::Platform::Delete(context);
        break;
    case AVAHI_BROWSER_NEW:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache new");
        if (context->mStopped.load())
        {
            // browse is stopped, so free browse handle and context
            avahi_service_browser_free(browser);
            chip::Platform::Delete(context);
            break;
        }
        if (strcmp("local", domain) == 0)
        {
            DnssdService service = {};

            Platform::CopyString(service.mName, name);
            CopyTypeWithoutProtocol(service.mType, type);
            service.mProtocol      = GetProtocolInType(type);
            service.mAddressType   = context->mAddressType;
            service.mTransportType = ToAddressType(protocol);
            service.mInterface     = Inet::InterfaceId::Null();
            if (interface != AVAHI_IF_UNSPEC)
            {
                service.mInterface = static_cast<chip::Inet::InterfaceId>(interface);
            }
            service.mType[kDnssdTypeMaxSize] = 0;
            context->mServices.push_back(service);
            if (context->mReceivedAllCached)
            {
                InvokeDelegateOrCleanUp(context, browser);
            }
        }
        break;
    case AVAHI_BROWSER_ALL_FOR_NOW: {
        ChipLogProgress(DeviceLayer, "Avahi browse: all for now");
        context->mReceivedAllCached = true;

        InvokeDelegateOrCleanUp(context, browser);
        break;
    }
    case AVAHI_BROWSER_REMOVE:
        ChipLogProgress(DeviceLayer, "Avahi browse: remove");
        if (strcmp("local", domain) == 0)
        {
            context->mServices.erase(std::remove_if(context->mServices.begin(), context->mServices.end(),
                                                    [name, type](const DnssdService & service) {
                                                        return strcmp(name, service.mName) == 0 &&
                                                            type == GetFullType(service.mType, service.mProtocol);
                                                    }),
                                     context->mServices.end());

            if (context->mReceivedAllCached)
            {
                DnssdService service = {};

                Platform::CopyString(service.mName, name);
                CopyTypeWithoutProtocol(service.mType, type);
                service.mProtocol      = GetProtocolInType(type);
                service.mAddressType   = context->mAddressType;
                service.mTransportType = ToAddressType(protocol);
                service.mInterface     = Inet::InterfaceId::Null();
                if (interface != AVAHI_IF_UNSPEC)
                {
                    service.mInterface = static_cast<chip::Inet::InterfaceId>(interface);
                }
                service.mTtlSeconds = 0;

                context->mServices.push_back(service);
                InvokeDelegateOrCleanUp(context, browser);
            }
        }

        break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache exhausted");
        break;
    }
}

MdnsAvahi::ResolveContext * MdnsAvahi::AllocateResolveContext()
{
    ResolveContext * context = chip::Platform::New<ResolveContext>();
    if (context == nullptr)
    {
        return nullptr;
    }

    context->mNumber = mResolveCount++;
    mAllocatedResolves.push_back(context);

    return context;
}

MdnsAvahi::ResolveContext * MdnsAvahi::ResolveContextForHandle(size_t handle)
{
    for (auto it : mAllocatedResolves)
    {
        if (it->mNumber == handle)
        {
            return it;
        }
    }
    return nullptr;
}

void MdnsAvahi::FreeResolveContext(size_t handle)
{
    for (auto it = mAllocatedResolves.begin(); it != mAllocatedResolves.end(); it++)
    {
        if ((*it)->mNumber == handle)
        {
            chip::Platform::Delete(*it);
            mAllocatedResolves.erase(it);
            return;
        }
    }
}

void MdnsAvahi::StopResolve(const char * name)
{
    // Cancel and delete any pending resolves for this instance name.
    //
    // Note: callbacks may re-enter into dnssd logic, so first detach matching contexts from
    // `mAllocatedResolves`, then invoke callbacks and delete outside of the list iteration.
    std::vector<ResolveContext *> contexts;
    mAllocatedResolves.remove_if([&contexts, name](ResolveContext * ctx) {
        if (strcmp(ctx->mName, name) == 0)
        {
            contexts.push_back(ctx);
            return true;
        }
        return false;
    });

    for (auto * context : contexts)
    {
        context->mCallback(context->mContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_CANCELLED);
        chip::Platform::Delete(context);
    }
}

CHIP_ERROR MdnsAvahi::Resolve(const char * name, const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                              Inet::IPAddressType transportType, Inet::InterfaceId interface, DnssdResolveCallback callback,
                              void * context)
{
    AvahiIfIndex avahiInterface     = static_cast<AvahiIfIndex>(interface.GetPlatformInterface());
    ResolveContext * resolveContext = AllocateResolveContext();
    if (resolveContext == nullptr)
    {
        ChipLogError(Discovery, "Failed to allocate resolve context");
        return CHIP_ERROR_NO_MEMORY;
    }
    CHIP_ERROR error          = CHIP_NO_ERROR;
    resolveContext->mInstance = this;
    resolveContext->mCallback = callback;
    resolveContext->mContext  = context;

    if (!interface.IsPresent())
    {
        avahiInterface = AVAHI_IF_UNSPEC;
    }

    Platform::CopyString(resolveContext->mName, name);
    resolveContext->mInterface   = avahiInterface;
    resolveContext->mTransport   = ToAvahiProtocol(transportType);
    resolveContext->mAddressType = ToAvahiProtocol(addressType);
    resolveContext->mFullType    = GetFullType(type, protocol);

    resolveContext->mResolver =
        avahi_service_resolver_new(mClient, avahiInterface, resolveContext->mTransport, name, resolveContext->mFullType.c_str(),
                                   nullptr, resolveContext->mAddressType, static_cast<AvahiLookupFlags>(0), HandleResolve,
                                   reinterpret_cast<void *>(resolveContext->mNumber));
    // Otherwise the resolver will be freed in the callback
    if (resolveContext->mResolver == nullptr)
    {
        error = CHIP_ERROR_INTERNAL;
        FreeResolveContext(resolveContext->mNumber);
    }

    return error;
}

void MdnsAvahi::HandleResolve(AvahiServiceResolver * resolver, AvahiIfIndex interface, AvahiProtocol protocol,
                              AvahiResolverEvent event, const char * name, const char * type, const char * /*domain*/,
                              const char * host_name, const AvahiAddress * address, uint16_t port, AvahiStringList * txt,
                              AvahiLookupResultFlags flags, void * userdata)
{
    size_t handle            = reinterpret_cast<size_t>(userdata);
    ResolveContext * context = sInstance.ResolveContextForHandle(handle);
    std::vector<TextEntry> textEntries;

    if (context == nullptr)
    {
        ChipLogError(Discovery, "Invalid context for handling resolves: %ld", static_cast<long>(handle));
        return;
    }

    switch (event)
    {
    case AVAHI_RESOLVER_FAILURE:
        if (context->mAttempts++ < 3)
        {
            ChipLogProgress(DeviceLayer, "Re-trying resolve");
            avahi_service_resolver_free(resolver);
            context->mResolver = avahi_service_resolver_new(
                context->mInstance->mClient, context->mInterface, context->mTransport, context->mName, context->mFullType.c_str(),
                nullptr, context->mAddressType, static_cast<AvahiLookupFlags>(0), HandleResolve, userdata);
            if (context->mResolver == nullptr)
            {
                ChipLogError(DeviceLayer, "Avahi resolve failed on retry");
                context->mCallback(context->mContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_INTERNAL);
                sInstance.FreeResolveContext(handle);
            }
            return;
        }
        ChipLogError(DeviceLayer, "Avahi resolve failed");
        context->mCallback(context->mContext, nullptr, Span<Inet::IPAddress>(), CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_RESOLVER_FOUND:
        DnssdService result = {};

        ChipLogProgress(DeviceLayer, "Avahi resolve found");

        Platform::CopyString(result.mName, name);
        CopyTypeWithoutProtocol(result.mType, type);
        result.mProtocol    = GetProtocolInType(type);
        result.mPort        = port;
        result.mAddressType = ToAddressType(protocol);
        result.mInterface   = Inet::InterfaceId::Null();
        // It's not clear if we can get the actual value from avahi, so just assume default.
        result.mTtlSeconds = AVAHI_DEFAULT_TTL_HOST_NAME;
        if (interface != AVAHI_IF_UNSPEC)
        {
            result.mInterface = static_cast<chip::Inet::InterfaceId>(interface);
        }
        Platform::CopyString(result.mHostName, host_name);
        // Returned value is full QName, want only host part.
        char * dot = strchr(result.mHostName, '.');
        if (dot != nullptr)
        {
            *dot = '\0';
        }

        CHIP_ERROR result_err = CHIP_ERROR_INVALID_ADDRESS;
        chip::Inet::IPAddress ipAddress; // Will be set of result_err is set to CHIP_NO_ERROR
        if (address)
        {
            switch (address->proto)
            {
            case AVAHI_PROTO_INET:
#if INET_CONFIG_ENABLE_IPV4
                struct in_addr addr4;

                memcpy(&addr4, &(address->data.ipv4), sizeof(addr4));
                ipAddress  = chip::Inet::IPAddress(addr4);
                result_err = CHIP_NO_ERROR;
#else
                ChipLogError(Discovery, "Ignoring IPv4 mDNS address.");
#endif
                break;
            case AVAHI_PROTO_INET6:
                struct in6_addr addr6;

                memcpy(&addr6, &(address->data.ipv6), sizeof(addr6));
                ipAddress  = chip::Inet::IPAddress(addr6);
                result_err = CHIP_NO_ERROR;
                break;
            default:
                break;
            }
        }

        while (txt != nullptr)
        {
            for (size_t i = 0; i < txt->size; i++)
            {
                if (txt->text[i] == '=')
                {
                    txt->text[i] = '\0';
                    textEntries.push_back(TextEntry{ reinterpret_cast<char *>(txt->text), &txt->text[i + 1], txt->size - i - 1 });
                    break;
                }
            }
            txt = txt->next;
        }

        if (!textEntries.empty())
        {
            result.mTextEntries = textEntries.data();
        }
        result.mTextEntrySize = textEntries.size();

        if (result_err == CHIP_NO_ERROR)
        {
            context->mCallback(context->mContext, &result, Span<Inet::IPAddress>(&ipAddress, 1), CHIP_NO_ERROR);
        }
        else
        {
            context->mCallback(context->mContext, nullptr, Span<Inet::IPAddress>(), result_err);
        }
        break;
    }

    sInstance.FreeResolveContext(handle);
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    return MdnsAvahi::GetInstance().Init(initCallback, errorCallback, context);
}

void ChipDnssdShutdown()
{
    MdnsAvahi::GetInstance().Shutdown();
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (strcmp(service->mHostName, "") != 0)
    {
        ReturnErrorOnFailure(MdnsAvahi::GetInstance().SetHostname(service->mHostName));
    }

    return MdnsAvahi::GetInstance().PublishService(*service, callback, context);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    return MdnsAvahi::GetInstance().StopPublish();
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return MdnsAvahi::GetInstance().FinalizePublish();
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    return MdnsAvahi::GetInstance().Browse(type, protocol, addressType, interface, callback, context, browseIdentifier);
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return MdnsAvahi::GetInstance().StopBrowse(browseIdentifier);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)

{
    VerifyOrReturnError(browseResult != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return MdnsAvahi::GetInstance().Resolve(browseResult->mName, browseResult->mType, browseResult->mProtocol,
                                            browseResult->mAddressType, Inet::IPAddressType::kAny, interface, callback, context);
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName)
{
    MdnsAvahi::GetInstance().StopResolve(instanceName);
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
