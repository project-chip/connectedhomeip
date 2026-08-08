/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/Linux/DnssdInterfaceMonitor.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>

#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Quiet time before DNS-SD is restarted: long enough for a bridge rebuild's burst of messages.
constexpr System::Clock::Timeout kSettleInterval = System::Clock::Milliseconds32(3000);

// Restart no later than this after the first change, however long the burst goes on.
constexpr System::Clock::Timeout kMaxDeferral = System::Clock::Milliseconds32(15000);

/**
 * Whether a change on this interface could affect what mDNS is listening on.
 *
 * A change on an interface the server never binds cannot, and skipping those matters: a Thread
 * border router's wpan0 gains and loses addresses routinely, and every one of them would
 * otherwise tear down and rebuild every socket and put a fresh announcement on the wire.
 *
 * The test deliberately mirrors AddressPolicy_DefaultImpl, including its documented bluntness
 * about names beginning "lo". A build using a policy that does bind those interfaces would want
 * this relaxed. An interface that cannot be named -- it has just been deleted, say -- counts as
 * interesting, since it may well be one that was in use.
 */
bool InterfaceCanAffectListening(unsigned int index)
{
    char name[IF_NAMESIZE];
    VerifyOrReturnValue(index != 0, true);
    VerifyOrReturnValue(if_indextoname(index, name) != nullptr, true);
    return (strncmp(name, "lo", 2) != 0) && (strncmp(name, "wpan", 4) != 0);
}

} // namespace

DnssdInterfaceMonitor::~DnssdInterfaceMonitor()
{
    Shutdown();
}

CHIP_ERROR DnssdInterfaceMonitor::Init()
{
    VerifyOrReturnError(mSocket < 0, CHIP_ERROR_INCORRECT_STATE);

    mSocket = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, NETLINK_ROUTE);
    VerifyOrReturnError(mSocket >= 0, CHIP_ERROR_POSIX(errno));

    struct sockaddr_nl addr = {};
    addr.nl_family          = AF_NETLINK;
    // nl_pid zero: the kernel assigns an id, so the Wi-Fi listener's socket keeps its own.
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV6_IFADDR;
#if INET_CONFIG_ENABLE_IPV4
    addr.nl_groups |= RTMGRP_IPV4_IFADDR;
#endif

    if (bind(mSocket, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        CHIP_ERROR err = CHIP_ERROR_POSIX(errno);
        close(mSocket);
        mSocket = -1;
        return err;
    }

    auto & layer = DeviceLayer::SystemLayerSockets();
    CHIP_ERROR err;
    SuccessOrExit(err = layer.StartWatchingSocket(mSocket, &mWatch));
    mWatching = true;
    SuccessOrExit(err = layer.SetCallback(mWatch, HandleNetlinkReadable, reinterpret_cast<intptr_t>(this)));
    SuccessOrExit(err = layer.RequestCallbackOnPendingRead(mWatch));

    return CHIP_NO_ERROR;

exit:
    Shutdown();
    return err;
}

void DnssdInterfaceMonitor::Shutdown()
{
    if (mChangePending)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleSettled, this);
        mChangePending = false;
    }

    if (mWatching)
    {
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayerSockets().StopWatchingSocket(&mWatch);
        mWatching = false;
    }

    if (mSocket >= 0)
    {
        close(mSocket);
        mSocket = -1;
    }
}

void DnssdInterfaceMonitor::HandleNetlinkReadable(System::SocketEvents, intptr_t data)
{
    reinterpret_cast<DnssdInterfaceMonitor *>(data)->OnNetlinkReadable();
}

void DnssdInterfaceMonitor::HandleSettled(System::Layer *, void * appState)
{
    static_cast<DnssdInterfaceMonitor *>(appState)->OnSettled();
}

void DnssdInterfaceMonitor::OnNetlinkReadable()
{
    // Non-blocking: drain the burst, each message only pushes the settle timer back.
    alignas(NLMSG_ALIGNTO) char buffer[8192];
    bool sawChange = false;

    while (true)
    {
        ssize_t len = recv(mSocket, buffer, sizeof(buffer), MSG_TRUNC);
        if (len < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            if (errno == EINTR)
            {
                continue;
            }
            if (errno == ENOBUFS)
            {
                // Notifications were dropped; what was missed is unknowable, so restart regardless.
                ChipLogError(DeviceLayer, "Netlink notifications were dropped; resynchronising DNS-SD");
                sawChange = true;
                continue;
            }
            // An unrecoverable error would keep the socket readable and spin the loop: stop monitoring.
            ChipLogError(DeviceLayer, "Error reading from the interface netlink socket: %d; stopping interface monitoring", errno);
            Shutdown();
            return;
        }
        if (len == 0)
        {
            break;
        }

        // Truncated (MSG_TRUNC): what was cut off is unknowable, treat it as an overrun.
        if (static_cast<size_t>(len) > sizeof(buffer))
        {
            ChipLogError(DeviceLayer, "Netlink message truncated; resynchronising DNS-SD");
            sawChange = true;
            continue;
        }

        for (struct nlmsghdr * header = reinterpret_cast<struct nlmsghdr *>(buffer);
             NLMSG_OK(header, static_cast<uint32_t>(len)) && header->nlmsg_type != NLMSG_DONE; header = NLMSG_NEXT(header, len))
        {
            unsigned int ifindex = 0;

            switch (header->nlmsg_type)
            {
            case RTM_NEWLINK:
            case RTM_DELLINK:
                if (header->nlmsg_len >= NLMSG_LENGTH(sizeof(struct ifinfomsg)))
                {
                    ifindex = static_cast<unsigned int>(static_cast<struct ifinfomsg *>(NLMSG_DATA(header))->ifi_index);
                }
                break;
            case RTM_NEWADDR:
            case RTM_DELADDR:
                if (header->nlmsg_len >= NLMSG_LENGTH(sizeof(struct ifaddrmsg)))
                {
                    ifindex = static_cast<struct ifaddrmsg *>(NLMSG_DATA(header))->ifa_index;
                }
                break;
            default:
                continue;
            }

            // Too short to name its interface leaves ifindex zero, which counts as interesting.
            if (InterfaceCanAffectListening(ifindex))
            {
                sawChange = true;
            }
        }
    }

    if (sawChange)
    {
        OnChangeSeen();
    }
}

void DnssdInterfaceMonitor::OnChangeSeen()
{
    System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

    if (!mChangePending)
    {
        mFirstChange   = now;
        mChangePending = true;
    }
    else if (now - mFirstChange >= kMaxDeferral)
    {
        // Deferred long enough; let the pending restart fire.
        return;
    }

    // StartTimer replaces a pending timer with the same callback and state.
    VerifyOrReturn(DeviceLayer::SystemLayer().StartTimer(kSettleInterval, HandleSettled, this).Handle([this](CHIP_ERROR err) {
        ChipLogError(DeviceLayer, "Failed to arm the DNS-SD settle timer: %" CHIP_ERROR_FORMAT, err.Format());
        // Better now than never; only reachable if timer allocation is not fatal.
        OnSettled();
    }));
}

void DnssdInterfaceMonitor::OnSettled()
{
    mChangePending = false;

    ChipLogProgress(DeviceLayer, "Network interfaces changed, restarting DNS-SD");

    ChipDeviceEvent event{};
    event.Type = DeviceEventType::kDnssdRestartNeeded;
    PlatformMgr().PostEventOrDie(&event);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
