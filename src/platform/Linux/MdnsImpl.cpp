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

#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>
#include <support/CodeUtils.h>

using chip::Mdns::kMdnsTypeMaxSize;
using chip::Mdns::MdnsServiceProtocol;
using chip::Mdns::TextEntry;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

namespace {

AvahiProtocol ToAvahiProtocol(chip::Inet::IPAddressType addressType)
{
    AvahiProtocol protocol;

    switch (addressType)
    {
    case chip::Inet::IPAddressType::kIPAddressType_IPv4:
        protocol = AVAHI_PROTO_INET;
        break;
    case chip::Inet::IPAddressType::kIPAddressType_IPv6:
        protocol = AVAHI_PROTO_INET6;
        break;
    default:
        protocol = AVAHI_PROTO_UNSPEC;
        break;
    }

    return protocol;
}

chip::Inet::IPAddressType ToAddressType(AvahiProtocol protocol)
{
    chip::Inet::IPAddressType type;

    switch (protocol)
    {
    case AVAHI_PROTO_INET:
        type = chip::Inet::IPAddressType::kIPAddressType_IPv4;
        break;
    case AVAHI_PROTO_INET6:
        type = chip::Inet::IPAddressType::kIPAddressType_IPv6;
        break;
    default:
        type = chip::Inet::IPAddressType::kIPAddressType_Unknown;
        break;
    }

    return type;
}

CHIP_ERROR MakeAvahiStringListFromTextEntries(TextEntry * entries, size_t size, AvahiStringList ** strListOut)
{
    *strListOut = avahi_string_list_new(nullptr, nullptr);

    for (size_t i = 0; i < size; i++)
    {
        uint8_t buf[chip::Mdns::kMdnsTextMaxSize];
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

const char * GetProtocolString(MdnsServiceProtocol protocol)
{
    return protocol == MdnsServiceProtocol::kMdnsProtocolUdp ? "_udp" : "_tcp";
}

std::string GetFullType(const char * type, MdnsServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type << "." << GetProtocolString(protocol);
    return typeBuilder.str();
}

} // namespace

namespace chip {
namespace Mdns {

MdnsAvahi MdnsAvahi::sInstance;

constexpr uint64_t kUsPerSec = 1000 * 1000;

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

    timeout.tv_sec  = static_cast<uint64_t>(timeoutVal.count()) / kUsPerSec;
    timeout.tv_usec = static_cast<uint64_t>(timeoutVal.count()) % kUsPerSec;
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

CHIP_ERROR MdnsAvahi::Init(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
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

CHIP_ERROR MdnsAvahi::SetHostname(const char * hostname)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int avahiRet;

    VerifyOrExit(mClient != nullptr, error = CHIP_ERROR_INCORRECT_STATE);
    avahiRet = avahi_client_set_host_name(mClient, hostname);
    if (avahiRet == AVAHI_ERR_ACCESS_DENIED)
    {
        ChipLogError(DeviceLayer, "Cannot set hostname on this system, continue anyway...");
    }
    else if (avahiRet != AVAHI_OK && avahiRet != AVAHI_ERR_NO_CHANGE)
    {
        error = CHIP_ERROR_INTERNAL;
    }

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
        if (mGroup != nullptr)
        {
            avahi_entry_group_reset(mGroup);
            avahi_entry_group_free(mGroup);
        }
        mGroup = avahi_entry_group_new(client, HandleGroupState, this);
        mPublishedServices.clear();
        if (mGroup == nullptr)
        {
            ChipLogError(DeviceLayer, "Failed to create avahi group: %s", avahi_strerror(avahi_client_errno(client)));
            mErrorCallback(mAsyncReturnContext, CHIP_ERROR_OPEN_FAILED);
        }
        else
        {
            mErrorCallback(mAsyncReturnContext, CHIP_ERROR_FORCED_RESET);
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

CHIP_ERROR MdnsAvahi::PublishService(const MdnsService & service)
{
    std::ostringstream keyBuilder;
    std::string key;
    std::string type       = GetFullType(service.mType, service.mProtocol);
    CHIP_ERROR error       = CHIP_NO_ERROR;
    AvahiStringList * text = nullptr;
    AvahiIfIndex interface =
        service.mInterface == INET_NULL_INTERFACEID ? AVAHI_IF_UNSPEC : static_cast<AvahiIfIndex>(service.mInterface);

    keyBuilder << service.mName << "." << type << service.mPort << "." << interface;
    key = keyBuilder.str();
    ChipLogProgress(DeviceLayer, "PublishService %s", key.c_str());

    if (mPublishedServices.find(key) == mPublishedServices.end())
    {
        SuccessOrExit(error = MakeAvahiStringListFromTextEntries(service.mTextEntries, service.mTextEntrySize, &text));

        mPublishedServices.emplace(key);
        VerifyOrExit(avahi_entry_group_add_service_strlst(mGroup, interface, ToAvahiProtocol(service.mAddressType),
                                                          static_cast<AvahiPublishFlags>(0), service.mName, type.c_str(), nullptr,
                                                          nullptr, service.mPort, text) == 0,
                     error = CHIP_ERROR_INTERNAL);
        for (size_t i = 0; i < service.mSubTypeSize; i++)
        {
            std::ostringstream sstream;

            sstream << service.mSubTypes[i] << "._sub." << type;

            VerifyOrExit(avahi_entry_group_add_service_subtype(mGroup, interface, ToAvahiProtocol(service.mAddressType),
                                                               static_cast<AvahiPublishFlags>(0), service.mName, type.c_str(),
                                                               nullptr, sstream.str().c_str()) == 0,
                         error = CHIP_ERROR_INTERNAL);
        }
    }
    else
    {
        SuccessOrExit(error = MakeAvahiStringListFromTextEntries(service.mTextEntries, service.mTextEntrySize, &text));

        VerifyOrExit(avahi_entry_group_update_service_txt_strlst(mGroup, interface, ToAvahiProtocol(service.mAddressType),
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
exit:
    return error;
}

CHIP_ERROR MdnsAvahi::Browse(const char * type, MdnsServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                             chip::Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
    AvahiServiceBrowser * browser;
    BrowseContext * browseContext = chip::Platform::New<BrowseContext>();
    AvahiIfIndex avahiInterface   = static_cast<AvahiIfIndex>(interface);

    browseContext->mInstance = this;
    browseContext->mContext  = context;
    browseContext->mCallback = callback;
    if (interface == INET_NULL_INTERFACEID)
    {
        avahiInterface = AVAHI_IF_UNSPEC;
    }

    browser = avahi_service_browser_new(mClient, avahiInterface, ToAvahiProtocol(addressType), GetFullType(type, protocol).c_str(),
                                        nullptr, static_cast<AvahiLookupFlags>(0), HandleBrowse, browseContext);
    // Otherwise the browser will be freed in the callback
    if (browser == nullptr)
    {
        chip::Platform::Delete(browseContext);
    }

    return browser == nullptr ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

MdnsServiceProtocol GetProtocolInType(const char * type)
{
    const char * deliminator = strrchr(type, '.');

    if (deliminator == NULL)
    {
        ChipLogError(Discovery, "Failed to find protocol in type: %s", type);
        return MdnsServiceProtocol::kMdnsProtocolUnknown;
    }

    if (strcmp("._tcp", deliminator) == 0)
    {
        return MdnsServiceProtocol::kMdnsProtocolTcp;
    }
    if (strcmp("._udp", deliminator) == 0)
    {
        return MdnsServiceProtocol::kMdnsProtocolUdp;
    }

    ChipLogError(Discovery, "Unknown protocol in type: %s", type);
    return MdnsServiceProtocol::kMdnsProtocolUnknown;
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

void MdnsAvahi::HandleBrowse(AvahiServiceBrowser * browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
                             const char * name, const char * type, const char * domain, AvahiLookupResultFlags /*flags*/,
                             void * userdata)
{
    BrowseContext * context = static_cast<BrowseContext *>(userdata);

    switch (event)
    {
    case AVAHI_BROWSER_FAILURE:
        context->mCallback(context->mContext, nullptr, 0, CHIP_ERROR_INTERNAL);
        avahi_service_browser_free(browser);
        chip::Platform::Delete(context);
        break;
    case AVAHI_BROWSER_NEW:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache new");
        if (strcmp("local", domain) == 0)
        {
            MdnsService service = {};

            Platform::CopyString(service.mName, name);
            CopyTypeWithoutProtocol(service.mType, type);
            service.mProtocol               = GetProtocolInType(type);
            service.mAddressType            = ToAddressType(protocol);
            service.mType[kMdnsTypeMaxSize] = 0;
            context->mServices.push_back(service);
        }
        break;
    case AVAHI_BROWSER_ALL_FOR_NOW:
        ChipLogProgress(DeviceLayer, "Avahi browse: all for now");
        context->mCallback(context->mContext, context->mServices.data(), context->mServices.size(), CHIP_NO_ERROR);
        avahi_service_browser_free(browser);
        chip::Platform::Delete(context);
        break;
    case AVAHI_BROWSER_REMOVE:
        ChipLogProgress(DeviceLayer, "Avahi browse: remove");
        if (strcmp("local", domain) == 0)
        {
            std::remove_if(context->mServices.begin(), context->mServices.end(), [name, type](const MdnsService & service) {
                return strcmp(name, service.mName) == 0 && type == GetFullType(service.mType, service.mProtocol);
            });
        }
        break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        ChipLogProgress(DeviceLayer, "Avahi browse: cache exhausted");
        break;
    }
}

CHIP_ERROR MdnsAvahi::Resolve(const char * name, const char * type, MdnsServiceProtocol protocol,
                              chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface,
                              MdnsResolveCallback callback, void * context)
{
    AvahiServiceResolver * resolver;
    AvahiIfIndex avahiInterface     = static_cast<AvahiIfIndex>(interface);
    ResolveContext * resolveContext = chip::Platform::New<ResolveContext>();
    CHIP_ERROR error                = CHIP_NO_ERROR;

    resolveContext->mInstance = this;
    resolveContext->mCallback = callback;
    resolveContext->mContext  = context;
    if (interface == INET_NULL_INTERFACEID)
    {
        avahiInterface = AVAHI_IF_UNSPEC;
    }
    resolver = avahi_service_resolver_new(mClient, avahiInterface, ToAvahiProtocol(addressType), name,
                                          GetFullType(type, protocol).c_str(), nullptr, ToAvahiProtocol(addressType),
                                          static_cast<AvahiLookupFlags>(0), HandleResolve, resolveContext);
    // Otherwise the resolver will be freed in the callback
    if (resolver == nullptr)
    {
        error = CHIP_ERROR_INTERNAL;
        chip::Platform::Delete(resolveContext);
    }

    return error;
}

void MdnsAvahi::HandleResolve(AvahiServiceResolver * resolver, AvahiIfIndex interface, AvahiProtocol protocol,
                              AvahiResolverEvent event, const char * name, const char * type, const char * /*domain*/,
                              const char * host_name, const AvahiAddress * address, uint16_t port, AvahiStringList * txt,
                              AvahiLookupResultFlags flags, void * userdata)
{
    ResolveContext * context = reinterpret_cast<ResolveContext *>(userdata);
    std::vector<TextEntry> textEntries;

    switch (event)
    {
    case AVAHI_RESOLVER_FAILURE:
        ChipLogError(DeviceLayer, "Avahi resolve failed");
        context->mCallback(context->mContext, nullptr, CHIP_ERROR_INTERNAL);
        break;
    case AVAHI_RESOLVER_FOUND:
        MdnsService result = {};

        result.mAddress.SetValue(chip::Inet::IPAddress());
        ChipLogError(DeviceLayer, "Avahi resolve found");

        Platform::CopyString(result.mName, name);
        CopyTypeWithoutProtocol(result.mType, type);
        result.mProtocol    = GetProtocolInType(type);
        result.mPort        = port;
        result.mAddressType = ToAddressType(protocol);
        Platform::CopyString(result.mHostName, host_name);
        // Returned value is full QName, want only host part.
        char * dot = strchr(result.mHostName, '.');
        if (dot != nullptr)
        {
            *dot = '\0';
        }

        if (address)
        {
            switch (address->proto)
            {
            case AVAHI_PROTO_INET:
                struct in_addr addr4;

                memcpy(&addr4, &(address->data.ipv4), sizeof(addr4));
                result.mAddress.SetValue(chip::Inet::IPAddress::FromIPv4(addr4));
                break;
            case AVAHI_PROTO_INET6:
                struct in6_addr addr6;

                memcpy(&addr6, &(address->data.ipv6), sizeof(addr6));
                result.mAddress.SetValue(chip::Inet::IPAddress::FromIPv6(addr6));
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

        context->mCallback(context->mContext, &result, CHIP_NO_ERROR);
        break;
    }

    avahi_service_resolver_free(resolver);
    chip::Platform::Delete(context);
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

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    return MdnsAvahi::GetInstance().Init(initCallback, errorCallback, context);
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    if (strcmp(service->mHostName, "") != 0)
    {
        ReturnErrorOnFailure(MdnsAvahi::GetInstance().SetHostname(service->mHostName));
    }
    return MdnsAvahi::GetInstance().PublishService(*service);
}

CHIP_ERROR ChipMdnsStopPublish()
{
    return MdnsAvahi::GetInstance().StopPublish();
}

CHIP_ERROR ChipMdnsStopPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
    return MdnsAvahi::GetInstance().Browse(type, protocol, addressType, interface, callback, context);
}

CHIP_ERROR ChipMdnsResolve(MdnsService * browseResult, chip::Inet::InterfaceId interface, MdnsResolveCallback callback,
                           void * context)

{
    CHIP_ERROR error;

    if (browseResult != nullptr)
    {
        error = MdnsAvahi::GetInstance().Resolve(browseResult->mName, browseResult->mType, browseResult->mProtocol,
                                                 browseResult->mAddressType, interface, callback, context);
    }
    else
    {
        error = CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

} // namespace Mdns
} // namespace chip
