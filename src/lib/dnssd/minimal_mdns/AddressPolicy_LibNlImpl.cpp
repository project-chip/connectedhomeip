/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "AddressPolicy_LibNlImpl.h"

#include <netlink/route/addr.h>
#include <netlink/socket.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {
namespace LibNl {

namespace {

using namespace chip::Inet;
using namespace chip::Platform;

class AllListenIterator : public mdns::Minimal::ListenIterator
{
public:
    ~AllListenIterator() override;

    bool Next(InterfaceId * id, IPAddressType * type) override;

private:
    /// Move mCurrentLink to the next valid element.
    /// Opens up nl_sockets and caches as needed.
    void Advance();

    /// Validate if the current interface is usable (up, not loopback etc.)
    bool IsCurrentLinkUsable();

    nl_sock * mNlSocket            = nullptr;
    nl_cache * mNlCache            = nullptr;
    nl_object * mCurrentLink       = nullptr;
    IPAddressType mCurrentLinkType = IPAddressType::kUnknown;

    rtnl_link * CurrentLink() { return reinterpret_cast<rtnl_link *>(mCurrentLink); }
};

class ValidIpAddressIterator : public mdns::Minimal::IpAddressIterator
{
public:
    ValidIpAddressIterator(InterfaceId id, IPAddressType type) : mInterfaceIdFilter(id), mAddrType(type) {}
    ~ValidIpAddressIterator() override;

    bool Next(IPAddress & dest) override;

private:
    /// Move mCurrentAddress to the next valid element.
    /// Opens up nl_sockets and caches as needed.
    void Advance();

    /// Validate if the current address is usable:
    /// - valid interface id
    /// - not temporary/deprecated/...
    bool IsCurrentAddressUsable();

    /// Attempt to decode the address pointed by mCurrentAddress into
    /// a CHIP-specific IP address.
    CHIP_ERROR DecodeCurrentAddress(IPAddress & dest);

    const InterfaceId mInterfaceIdFilter;
    const IPAddressType mAddrType;

    nl_sock * mNlSocket         = nullptr;
    nl_cache * mNlCache         = nullptr;
    nl_object * mCurrentAddress = nullptr;

    rtnl_addr * CurrentAddress() { return reinterpret_cast<rtnl_addr *>(mCurrentAddress); }
};

AllListenIterator::~AllListenIterator()
{
    if (mNlCache != nullptr)
    {
        nl_cache_free(mNlCache);
        mNlCache = nullptr;
    }

    if (mNlSocket != nullptr)
    {
        nl_socket_free(mNlSocket);
        mNlSocket = nullptr;
    }
}

void AllListenIterator::Advance()
{
    // If finding a new link, assume IPv6 is supported on that link
    // this assumption may be wrong, however current MinMdns code just asks
    // on what interfaces to try listening and actual IP address detection
    // (if needed) will be done later anyway.
    mCurrentLinkType = IPAddressType::kIPv6;

    if (mNlCache != nullptr)
    {
        if (mCurrentLink != nullptr)
        {
            mCurrentLink = nl_cache_get_next(mCurrentLink);
        }
        return;
    }

    if (mNlSocket == nullptr)
    {
        mNlSocket = nl_socket_alloc();

        if (mNlSocket == nullptr)
        {
            ChipLogError(Inet, "Failed to allocate nl socket");
            return;
        }

        int result = nl_connect(mNlSocket, NETLINK_ROUTE);
        if (result != 0)
        {
            ChipLogError(Inet, "Failed to connect NL socket: %s", nl_geterror(result));
            return;
        }
    }

    int result = rtnl_link_alloc_cache(mNlSocket, AF_UNSPEC, &mNlCache);
    if (result != 0)
    {
        ChipLogError(Inet, "Failed to cache links");
        return;
    }

    mCurrentLink = nl_cache_get_first(mNlCache);
}

bool AllListenIterator::Next(InterfaceId * id, IPAddressType * type)
{
    while (true)
    {
#if INET_CONFIG_ENABLE_IPV4
        // FOR IPv4, report all interfaces as 'try IPv4 here as well'
        if (mCurrentLinkType == IPAddressType::kIPv6)
        {
            mCurrentLinkType = IPAddressType::kIPv4;
        }
        else
        {
            mCurrentLinkType = IPAddressType::kIPv6;
            Advance();
        }
#else
        Advance();
#endif

        if (mCurrentLink == nullptr)
        {
            return false;
        }

        if (!IsCurrentLinkUsable())
        {
            // move to the next interface
            continue;
        }

        int idx = rtnl_link_get_ifindex(CurrentLink());
        if (idx == 0)
        {
            // Invalid index, move to the next interface
            continue;
        }

        *id   = InterfaceId(idx);
        *type = mCurrentLinkType; // advancing should have set this
        return true;
    }
}

bool AllListenIterator::IsCurrentLinkUsable()
{
    VerifyOrReturnError(mCurrentLink != nullptr, false);

    unsigned int flags = rtnl_link_get_flags(CurrentLink());

    if ((flags & IFF_UP) == 0)
    {
        // only consider interfaces that are actually up
        return false;
    }

    if ((flags & IFF_LOOPBACK) != 0)
    {
        // skip local loopback
        return false;
    }

    if ((flags & (IFF_BROADCAST | IFF_MULTICAST)) == 0)
    {
        // minmdns requires broadcast/multicast
        return false;
    }

    return true;
}

ValidIpAddressIterator::~ValidIpAddressIterator()
{
    if (mNlCache != nullptr)
    {
        nl_cache_free(mNlCache);
        mNlCache = nullptr;
    }

    if (mNlSocket != nullptr)
    {
        nl_socket_free(mNlSocket);
        mNlSocket = nullptr;
    }
}

void ValidIpAddressIterator::Advance()
{
    if (mNlCache != nullptr)
    {
        if (mCurrentAddress != nullptr)
        {
            mCurrentAddress = nl_cache_get_next(mCurrentAddress);
        }
        return;
    }

    if (mNlSocket == nullptr)
    {
        mNlSocket = nl_socket_alloc();

        if (mNlSocket == nullptr)
        {
            ChipLogError(Inet, "Failed to allocate nl socket");
            return;
        }

        int result = nl_connect(mNlSocket, NETLINK_ROUTE);
        if (result != 0)
        {
            ChipLogError(Inet, "Failed to connect NL socket: %s", nl_geterror(result));
            return;
        }
    }

    int result = rtnl_addr_alloc_cache(mNlSocket, &mNlCache);
    if (result != 0)
    {
        ChipLogError(Inet, "Failed to cache addresses");
        return;
    }

    mCurrentAddress = nl_cache_get_first(mNlCache);
}

bool ValidIpAddressIterator::Next(IPAddress & dest)
{
    while (true)
    {
        Advance();

        if (mCurrentAddress == nullptr)
        {
            return false;
        }

        if (!IsCurrentAddressUsable())
        {
            continue;
        }

        if (DecodeCurrentAddress(dest) == CHIP_NO_ERROR)
        {
            return true;
        }
    }
}

bool ValidIpAddressIterator::IsCurrentAddressUsable()
{
    VerifyOrReturnError(mCurrentAddress != nullptr, false);

    if (mInterfaceIdFilter != InterfaceId(rtnl_addr_get_ifindex(CurrentAddress())))
    {
        // Not for the correct interface id
        return false;
    }

    // Check if flags seem ok for this to be a good address to use/report
    int ifa_flags = rtnl_addr_get_flags(CurrentAddress());

    return (ifa_flags &
            (
                // Still going through  duplicate address detection (DAD), should only
                // be used once DAD completed
                IFA_F_OPTIMISTIC | IFA_F_DADFAILED |
                IFA_F_TENTATIVE
                // Linux deprecated - should not be used anymore. We skip this
                // from the list of used/reported addresses
                | IFA_F_DEPRECATED)) == 0;
}

CHIP_ERROR ValidIpAddressIterator::DecodeCurrentAddress(IPAddress & dest)
{
    VerifyOrReturnError(mCurrentAddress != nullptr, CHIP_ERROR_INCORRECT_STATE);

    nl_addr * local = rtnl_addr_get_local(CurrentAddress());
    if (local == nullptr)
    {
        ChipLogError(Inet, "Failed to get local IP address");
        return CHIP_ERROR_INTERNAL;
    }

    switch (nl_addr_get_family(local))
    {
    case AF_INET6: {
        if ((mAddrType != IPAddressType::kAny) && (mAddrType != IPAddressType::kIPv6))
        {
            return CHIP_ERROR_INVALID_ADDRESS;
        }

        struct sockaddr_in6 sa;
        memset(&sa, 0, sizeof(sa));
        socklen_t len = sizeof(sa);

        int result = nl_addr_fill_sockaddr(local, reinterpret_cast<sockaddr *>(&sa), &len);
        if (result != 0)
        {
            ChipLogError(Inet, "Failed to process address: %s", nl_geterror(result));
            return CHIP_ERROR_INVALID_ADDRESS;
        }

        dest = IPAddress::FromSockAddr(sa);
        return CHIP_NO_ERROR;
    }
    case AF_INET: {
#if INET_CONFIG_ENABLE_IPV4
        if ((mAddrType != IPAddressType::kAny) && (mAddrType != IPAddressType::kIPv4))
        {
            return CHIP_ERROR_INVALID_ADDRESS;
        }
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        socklen_t len = sizeof(sa);

        int result = nl_addr_fill_sockaddr(local, reinterpret_cast<sockaddr *>(&sa), &len);
        if (result != 0)
        {
            ChipLogError(Inet, "Failed to process address: %s", nl_geterror(result));
            return CHIP_ERROR_INVALID_ADDRESS;
        }

        dest = IPAddress::FromSockAddr(sa);
        return CHIP_NO_ERROR;
#else
        return CHIP_ERROR_INVALID_ADDRESS;
#endif
    }
    default:
        ChipLogError(Inet, "Unsupported/unknown local address: %d", nl_addr_get_family(local));
        return CHIP_ERROR_INVALID_ADDRESS;
    }

    return CHIP_NO_ERROR;
}

LibNl_AddressPolicy gAddressPolicy;

} // namespace

UniquePtr<ListenIterator> LibNl_AddressPolicy::GetListenEndpoints()
{
    return UniquePtr<ListenIterator>(chip::Platform::New<AllListenIterator>());
}

UniquePtr<IpAddressIterator> LibNl_AddressPolicy::GetIpAddressesForEndpoint(InterfaceId interfaceId, IPAddressType type)
{
    return UniquePtr<IpAddressIterator>(chip::Platform::New<ValidIpAddressIterator>(interfaceId, type));
}

void SetAddressPolicy()
{
    SetAddressPolicy(&gAddressPolicy);
}

} // namespace LibNl
} // namespace Minimal
} // namespace mdns
