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

#include "MdnsImpl.h"

#include <algorithm>
#include <sstream>
#include <string.h>
#include <time.h>
#include <vector>

#include <netinet/in.h>

#include "platform/Mdns.h"
#include "support/CodeUtils.h"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

namespace chip {
namespace DeviceLayer {

MdnsAvahi MdnsAvahi::sInstance;

constexpr uint64_t kUsPerSec = 1000 * 1000;

Poller::Poller(void)
{
    mAvahiPoller.userdata         = this;
    mAvahiPoller.watch_new        = WatchNew;
    mAvahiPoller.watch_update     = WatchUpdate;
    mAvahiPoller.watch_get_events = WatchGetEvents;
    mAvahiPoller.watch_free       = WatchFree;

    mAvahiPoller.timeout_new    = TimeoutNew;
    mAvahiPoller.timeout_update = TimeoutUpdate;
    mAvahiPoller.timeout_free   = TimeoutFree;
}

AvahiWatch * Poller::WatchNew(const struct AvahiPoll * poller, int fd, AvahiWatchEvent event, AvahiWatchCallback callback,
                              void * context)
{
    return reinterpret_cast<Poller *>(poller->userdata)->WatchNew(fd, event, callback, context);
}

AvahiWatch * Poller::WatchNew(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void * context)
{
    VerifyOrDie(callback != nullptr && fd >= 0);

    mWatches.emplace_back(new AvahiWatch{ fd, event, 0, callback, context, this });

    return mWatches.back().get();
}

void Poller::WatchUpdate(AvahiWatch * watch, AvahiWatchEvent event)
{
    watch->mWatchEvents = event;
}

AvahiWatchEvent Poller::WatchGetEvents(AvahiWatch * watch)
{
    return static_cast<AvahiWatchEvent>(watch->mHappenedEvents);
}

void Poller::WatchFree(AvahiWatch * watch)
{
    reinterpret_cast<Poller *>(watch->mPoller)->WatchFree(*watch);
}

void Poller::WatchFree(AvahiWatch & watch)
{
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
    return mTimers.back().get();
}

void Poller::TimeoutUpdate(AvahiTimeout * timer, const struct timeval * timeout)
{
    if (timeout)
    {
        timer->mAbsTimeout = GetAbsTimeout(timeout);
        timer->mEnabled    = true;
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

void Poller::UpdateFdSet(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & aMaxFd, timeval & timeout)
{
    microseconds timeoutVal = seconds(timeout.tv_sec) + microseconds(timeout.tv_usec);

    for (auto && watch : mWatches)
    {
        int fd                 = watch->mFd;
        AvahiWatchEvent events = watch->mWatchEvents;

        if (AVAHI_WATCH_IN & events)
        {
            FD_SET(fd, &readFdSet);
        }

        if (AVAHI_WATCH_OUT & events)
        {
            FD_SET(fd, &writeFdSet);
        }

        if (AVAHI_WATCH_ERR & events)
        {
            FD_SET(fd, &errorFdSet);
        }

        if (aMaxFd < fd)
        {
            aMaxFd = fd;
        }

        watch->mHappenedEvents = 0;
    }

    for (auto && timer : mTimers)
    {
        steady_clock::time_point absTimeout = timer->mAbsTimeout;
        steady_clock::time_point now        = steady_clock::now();

        if (!timer->mEnabled)
        {
            continue;
        }
        if (absTimeout < now)
        {
            timeoutVal = microseconds(0);
            break;
        }
        else
        {
            timeoutVal = std::min(timeoutVal, duration_cast<microseconds>(absTimeout - now));
        }
    }

    timeout.tv_sec  = timeoutVal.count() / kUsPerSec;
    timeout.tv_usec = timeoutVal.count() % kUsPerSec;
}

void Poller::Process(const fd_set & readFdSet, const fd_set & writeFdSet, const fd_set & errorFdSet)
{
    steady_clock::time_point now = steady_clock::now();

    for (auto && watch : mWatches)
    {
        int fd                 = watch->mFd;
        AvahiWatchEvent events = watch->mWatchEvents;

        watch->mHappenedEvents = 0;

        if ((AVAHI_WATCH_IN & events) && FD_ISSET(fd, &readFdSet))
        {
            watch->mHappenedEvents |= AVAHI_WATCH_IN;
        }

        if ((AVAHI_WATCH_OUT & events) && FD_ISSET(fd, &writeFdSet))
        {
            watch->mHappenedEvents |= AVAHI_WATCH_OUT;
        }

        if ((AVAHI_WATCH_ERR & events) && FD_ISSET(fd, &errorFdSet))
        {
            watch->mHappenedEvents |= AVAHI_WATCH_ERR;
        }

        if (watch->mHappenedEvents)
        {
            watch->mCallback(watch.get(), watch->mFd, static_cast<AvahiWatchEvent>(watch->mHappenedEvents), watch->mContext);
        }
    }

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
    }
}

CHIP_ERROR MdnsAvahi::Init(MdnsAsnycReturnCallback initCallback, MdnsAsnycReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int avahiError   = 0;

    VerifyOrExit(initCallback != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(errorCallback != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mClient == nullptr && mGroup == nullptr, error = CHIP_ERROR_INCORRECT_STATE);
    mInitCallback       = initCallback;
    mErrorCallback      = errorCallback;
    mAsyncReturnContext = context;
    mClient             = avahi_client_new(mPoller.GetAvahiPoll(), AVAHI_CLIENT_NO_FAIL, HandleClientState, this, &avahiError);
    VerifyOrExit(mClient != nullptr, error = CHIP_ERROR_OPEN_FAILED);
    VerifyOrExit(avahiError == 0, error = CHIP_ERROR_OPEN_FAILED);

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
        mGroup  = avahi_entry_group_new(client, HandleGroupState, this);
        if (mGroup == nullptr)
        {
            ChipLogError(DeviceLayer, "Failed to create avahi group: %s", avahi_strerror(avahi_client_errno(client)));
            mInitCallback(mAsyncReturnContext, CHIP_ERROR_OPEN_FAILED);
        }
        else
        {
            mInitCallback(mAsyncReturnContext, CHIP_NO_ERROR);
        }
        break;
    case AVAHI_CLIENT_FAILURE:
        ChipLogError(DeviceLayer, "Avahi client failure");
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_CLIENT_S_COLLISION:
    case AVAHI_CLIENT_S_REGISTERING:
        ChipLogProgress(DeviceLayer, "Avahi re-register required");
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_MDNS_COLLISSION);
        if (mGroup != nullptr)
        {
            avahi_entry_group_reset(mGroup);
        }
        break;
    case AVAHI_CLIENT_CONNECTING:
        ChipLogProgress(DeviceLayer, "Avahi connecting");
        break;
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
        ChipLogProgress(DeviceLayer, "Avahi group established");
        break;
    case AVAHI_ENTRY_GROUP_COLLISION:
        ChipLogError(DeviceLayer, "Avahi group collission");
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_MDNS_COLLISSION);
        break;
    case AVAHI_ENTRY_GROUP_FAILURE:
        ChipLogError(DeviceLayer, "Avahi group internal failure %s",
                     avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(mGroup))));
        mErrorCallback(mAsyncReturnContext, CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
    case AVAHI_ENTRY_GROUP_REGISTERING:
        break;
    }
}

static const char * GetProtocolString(MdnsServiceProtocol protocol)
{
    return protocol == MdnsServiceProtocol::kMdnsProtocolUdp ? "_udp" : "_tcp";
}

static std::string GetFullType(const char * type, MdnsServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type << "." << GetProtocolString(protocol);
    return typeBuilder.str();
}

static AvahiStringList * MakeAvahiStringListFromTextEntries(TextEntry * entries, size_t size)
{
    AvahiStringList * strList = avahi_string_list_new(nullptr, nullptr);

    for (size_t i = 0; i < size; i++)
    {
        uint8_t buf[kMdnsTypeMaxSize];
        size_t keySize   = strnlen(entries[i].mKey, kMdnsTypeMaxSize);
        size_t valueSize = 0;

        memcpy(buf, entries[i].mKey, keySize);
        if (size < sizeof(buf))
        {
            valueSize    = std::min(entries[i].mDataSize, sizeof(buf) - keySize - 1);
            buf[keySize] = static_cast<uint8_t>('=');
            memcpy(&buf[keySize + 1], entries[i].mData, valueSize);
        }
        strList = avahi_string_list_add_arbitrary(strList, buf, keySize + valueSize + 1);
    }

    return strList;
}

CHIP_ERROR MdnsAvahi::PublishService(const MdnsService & service)
{
    std::ostringstream keyBuilder;
    std::string key;
    std::string type       = GetFullType(service.mType, service.mProtocol);
    CHIP_ERROR error       = CHIP_NO_ERROR;
    AvahiStringList * text = nullptr;
    AvahiIfIndex interface = service.interface == INET_NULL_INTERFACEID ? AVAHI_IF_UNSPEC : service.interface;

    keyBuilder << service.mName << "." << type << service.mPort << "." << interface;
    key = keyBuilder.str();
    if (mPublishedServices.find(key) == mPublishedServices.end())
    {
        text = MakeAvahiStringListFromTextEntries(service.mTextEntryies, service.mTextEntrySize);

        mPublishedServices.emplace(key);
        VerifyOrExit(avahi_entry_group_add_service_strlst(mGroup, interface, AVAHI_PROTO_UNSPEC, static_cast<AvahiPublishFlags>(0),
                                                          service.mName, type.c_str(), nullptr, nullptr, service.mPort, text) == 0,
                     error = CHIP_ERROR_INTERNAL);
    }
    else
    {
        text = MakeAvahiStringListFromTextEntries(service.mTextEntryies, service.mTextEntrySize);

        VerifyOrExit(avahi_entry_group_update_service_txt_strlst(mGroup, interface, AVAHI_PROTO_UNSPEC,
                                                                 static_cast<AvahiPublishFlags>(0), service.mName, type.c_str(),
                                                                 nullptr, text) == 0,
                     error = CHIP_ERROR_INTERNAL);
    }
    VerifyOrExit(avahi_entry_group_commit(mGroup) == 0, error = CHIP_ERROR_INTERNAL);

exit:
    if (text != nullptr)
    {
        avahi_string_list_free(text);
    }
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Avahi publish service failed: %d", static_cast<int>(error));
    }

    return error;
}

CHIP_ERROR MdnsAvahi::StopPublish()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(avahi_entry_group_reset(mGroup) == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(avahi_entry_group_commit(mGroup) == 0, error = CHIP_ERROR_INTERNAL);
exit:
    return error;
}

CHIP_ERROR MdnsAvahi::Browse(const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                             MdnsBrowseCallback callback, void * context)
{
    AvahiServiceBrowser * browser;
    std::shared_ptr<BrowseContext> browseContext = std::make_shared<BrowseContext>();

    browseContext->mInstance = this;
    browseContext->mContext  = context;
    browseContext->mCallback = callback;
    if (interface == INET_NULL_INTERFACEID)
    {
        interface = AVAHI_IF_UNSPEC;
    }
    browser = avahi_service_browser_new(mClient, interface, AVAHI_PROTO_UNSPEC, GetFullType(type, protocol).c_str(), nullptr,
                                        static_cast<AvahiLookupFlags>(0), HandleBrowse, browseContext.get());
    if (browser != nullptr)
    {
        mBrowseContexts[browseContext.get()] = browseContext;
    }

    return browser == nullptr ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

MdnsServiceProtocol TruncateProtocolInType(char * type)
{
    char * deliminator           = strrchr(type, '.');
    MdnsServiceProtocol protocol = MdnsServiceProtocol::kMdnsProtocolUnknown;

    if (deliminator != NULL)
    {
        if (strcmp("._tcp", deliminator) == 0)
        {
            protocol     = MdnsServiceProtocol::kMdnsProtocolTcp;
            *deliminator = 0;
        }
        else if (strcmp("._udp", deliminator) == 0)
        {
            protocol     = MdnsServiceProtocol::kMdnsProtocolUdp;
            *deliminator = 0;
        }
    }
    return protocol;
}

void MdnsAvahi::HandleBrowse(AvahiServiceBrowser * browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
                             const char * name, const char * type, const char * domain, AvahiLookupResultFlags flags,
                             void * userdata)
{
    (void) flags;
    BrowseContext * context = static_cast<BrowseContext *>(userdata);

    switch (event)
    {
    case AVAHI_BROWSER_FAILURE:
        context->mCallback(context->mContext, nullptr, 0, CHIP_ERROR_INTERNAL);
        avahi_service_browser_free(browser);
        context->mInstance->mBrowseContexts.erase(context);
        break;
    case AVAHI_BROWSER_NEW:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache new");
        if (strcmp("local", domain) == 0)
        {
            MdnsService service;

            strncpy(service.mName, name, sizeof(service.mName));
            strncpy(service.mType, type, sizeof(service.mType));
            service.mName[kMdnsNameMaxSize] = 0;
            service.mType[kMdnsTypeMaxSize] = 0;
            service.mProtocol               = TruncateProtocolInType(service.mType);
            context->mServices.push_back(service);
            break;
        }
    case AVAHI_BROWSER_ALL_FOR_NOW:
        ChipLogProgress(DeviceLayer, "Avahi browse: all for now");
        context->mCallback(context->mContext, context->mServices.data(), context->mServices.size(), CHIP_NO_ERROR);
        avahi_service_browser_free(browser);
        context->mInstance->mBrowseContexts.erase(context);
        break;
    case AVAHI_BROWSER_REMOVE:
        ChipLogProgress(DeviceLayer, "Avahi browse: remove");
        if (strcmp("local", domain) == 0)
        {
            std::remove_if(context->mServices.begin(), context->mServices.end(), [name, type](const MdnsService service) {
                return name == std::string(service.mName) && type == GetFullType(service.mType, service.mProtocol);
            });
        }
        break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache exhausted");
        break;
    }
}

CHIP_ERROR MdnsAvahi::Resolve(const char * name, const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                              MdnsResolveCallback callback, void * context)
{
    AvahiServiceResolver * resolver;
    std::shared_ptr<ResolveContext> resolveContext = std::make_shared<ResolveContext>();
    CHIP_ERROR error                               = CHIP_NO_ERROR;

    resolveContext->mInstance = this;
    resolveContext->mCallback = callback;
    resolveContext->mContext  = context;
    if (interface == INET_NULL_INTERFACEID)
    {
        interface = AVAHI_IF_UNSPEC;
    }
    resolver =
        avahi_service_resolver_new(mClient, interface, AVAHI_PROTO_UNSPEC, name, GetFullType(type, protocol).c_str(), nullptr,
                                   AVAHI_PROTO_UNSPEC, static_cast<AvahiLookupFlags>(0), HandleResolve, resolveContext.get());
    VerifyOrExit(resolver != nullptr, error = CHIP_ERROR_INTERNAL);
    mResolveContexts[resolveContext.get()] = resolveContext;

exit:
    return error;
}

void MdnsAvahi::HandleResolve(AvahiServiceResolver * resolver, AvahiIfIndex interface, AvahiProtocol protocol,
                              AvahiResolverEvent event, const char * name, const char * type, const char * domain,
                              const char * host_name, const AvahiAddress * address, uint16_t port, AvahiStringList * txt,
                              AvahiLookupResultFlags flags, void * userdata)
{
    (void) host_name;
    (void) domain;
    ResolveContext * context = reinterpret_cast<ResolveContext *>(userdata);
    std::vector<TextEntry> textEntries;

    switch (event)
    {
    case AVAHI_RESOLVER_FAILURE:
        ChipLogError(DeviceLayer, "Avahi resolve failed");
        context->mCallback(context->mContext, nullptr, CHIP_ERROR_INTERNAL);
    case AVAHI_RESOLVER_FOUND:
        MdnsResolveResult result;

        ChipLogError(DeviceLayer, "Avahi resolve found");
        strncpy(result.mService.mName, name, sizeof(result.mService.mName));
        strncpy(result.mService.mType, type, sizeof(result.mService.mType));
        result.mService.mName[kMdnsNameMaxSize] = 0;
        result.mService.mType[kMdnsTypeMaxSize] = 0;
        result.mService.mProtocol               = TruncateProtocolInType(result.mService.mType);
        result.mService.mPort                   = port;

        if (address)
        {
            switch (address->proto)
            {
            case AVAHI_PROTO_INET:
                struct in_addr addr4;

                memcpy(&addr4, &(address->data.ipv4), sizeof(addr4));
                result.mAddress = chip::Inet::IPAddress::FromIPv4(addr4);
            case AVAHI_PROTO_INET6:
                struct in6_addr addr6;

                memcpy(&addr6, &(address->data.ipv6), sizeof(addr6));
                result.mAddress = chip::Inet::IPAddress::FromIPv6(addr6);
            default:
                break;
            }
        }

        while (txt != nullptr)
        {
            TextEntry entry;

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
            result.mService.mTextEntryies = textEntries.data();
        }
        result.mService.mTextEntrySize = textEntries.size();

        context->mCallback(context->mContext, &result, CHIP_NO_ERROR);
    }

    avahi_service_resolver_free(resolver);
}

MdnsAvahi::~MdnsAvahi()
{
    if (mGroup)
    {
        avahi_entry_group_free(mGroup);
    }
    if (mClient)
    {
        avahi_client_free(mClient);
    }
}

void UpdateMdnsDataset(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout)
{
    MdnsAvahi::GetInstance().GetPoller().UpdateFdSet(readFdSet, writeFdSet, errorFdSet, maxFd, timeout);
}

void ProcessMdns(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet)
{
    MdnsAvahi::GetInstance().GetPoller().Process(readFdSet, writeFdSet, errorFdSet);
}

CHIP_ERROR ChipMdnsInit(MdnsAsnycReturnCallback initCallback, MdnsAsnycReturnCallback errorCallback, void * context)
{
    return MdnsAvahi::GetInstance().Init(initCallback, errorCallback, context);
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    return MdnsAvahi::GetInstance().PublishService(*service);
}

CHIP_ERROR ChipMdnsStopPublish()
{
    return MdnsAvahi::GetInstance().StopPublish();
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                          MdnsBrowseCallback callback, void * context)
{
    return MdnsAvahi::GetInstance().Browse(type, protocol, interface, callback, context);
}

CHIP_ERROR ChipMdnsResolve(const char * name, const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                           MdnsResolveCallback callback, void * context)
{
    return MdnsAvahi::GetInstance().Resolve(name, type, protocol, interface, callback, context);
}

} // namespace DeviceLayer
} // namespace chip
