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

#include "InterfaceAddressMonitor.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Large enough for any one rtnetlink address notification, which the
// kernel sends one to a datagram.
constexpr size_t kReceiveBufferSize = 8192;

} // namespace

CHIP_ERROR InterfaceAddressMonitor::Init(System::LayerSockets & systemLayer)
{
    VerifyOrReturnError(mSocket < 0, CHIP_ERROR_INCORRECT_STATE);

    const int fd = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, NETLINK_ROUTE);
    VerifyOrReturnError(fd >= 0, CHIP_ERROR_POSIX(errno));

    struct sockaddr_nl address = {};

    address.nl_family = AF_NETLINK;
    address.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

    if (bind(fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) != 0)
    {
        const CHIP_ERROR error = CHIP_ERROR_POSIX(errno);

        close(fd);

        return error;
    }

    System::SocketWatchToken watch = systemLayer.InvalidSocketWatchToken();
    CHIP_ERROR error               = systemLayer.StartWatchingSocket(fd, &watch);

    if (error == CHIP_NO_ERROR)
    {
        error = systemLayer.SetCallback(watch, HandleSocketEvents, reinterpret_cast<intptr_t>(this));
    }

    if (error == CHIP_NO_ERROR)
    {
        error = systemLayer.RequestCallbackOnPendingRead(watch);
    }

    if (error != CHIP_NO_ERROR)
    {
        if (watch != systemLayer.InvalidSocketWatchToken())
        {
            LogErrorOnFailure(systemLayer.StopWatchingSocket(&watch));
        }

        close(fd);

        return error;
    }

    mSystemLayer = &systemLayer;
    mSocketWatch = watch;
    mSocket      = fd;

    return CHIP_NO_ERROR;
}

void InterfaceAddressMonitor::Shutdown()
{
    VerifyOrReturn(mSocket >= 0);

    LogErrorOnFailure(mSystemLayer->StopWatchingSocket(&mSocketWatch));

    close(mSocket);

    mSystemLayer = nullptr;
    mSocket      = -1;
}

void InterfaceAddressMonitor::HandleSocketEvents(System::SocketEvents events, intptr_t context)
{
    VerifyOrReturn(events.Has(System::SocketEventFlags::kRead));

    reinterpret_cast<InterfaceAddressMonitor *>(context)->ReadChanges();
}

void InterfaceAddressMonitor::ReadChanges()
{
    alignas(struct nlmsghdr) uint8_t buffer[kReceiveBufferSize];
    BitFlags<InterfaceAddressChange> changes;

    while (true)
    {
        // MSG_TRUNC returns the datagram's full length, so that a
        // truncated datagram can be told from a complete one.

        const ssize_t received = recv(mSocket, buffer, sizeof(buffer), MSG_TRUNC);

        if (received < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            if (errno == ENOBUFS)
            {
                ChipLogError(DeviceLayer, "Interface address notifications were dropped; reading all addresses again");
                changes.Set(InterfaceAddressChange::kIPv4Assigned).Set(InterfaceAddressChange::kIPv6Assigned);
                continue;
            }

            if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
            {
                ChipLogError(DeviceLayer, "Cannot read interface address notifications: %s", strerror(errno));
            }

            break;
        }

        if (static_cast<size_t>(received) > sizeof(buffer))
        {
            ChipLogError(DeviceLayer, "Interface address notification truncated; reading all addresses again");
            changes.Set(InterfaceAddressChange::kIPv4Assigned).Set(InterfaceAddressChange::kIPv6Assigned);
            continue;
        }

        changes.Set(GetInterfaceAddressChanges(ByteSpan(buffer, static_cast<size_t>(received))));
    }

    PostChanges(changes);
}

void InterfaceAddressMonitor::PostChanges(BitFlags<InterfaceAddressChange> changes)
{
    static constexpr struct
    {
        InterfaceAddressChange mChange;
        InterfaceIpChangeType mType;
        const char * mDescription;
    } kEvents[] = {
        { InterfaceAddressChange::kIPv4Assigned, InterfaceIpChangeType::kIpV4_Assigned, "IPv4 address assigned" },
        { InterfaceAddressChange::kIPv4Lost, InterfaceIpChangeType::kIpV4_Lost, "IPv4 address lost" },
        { InterfaceAddressChange::kIPv6Assigned, InterfaceIpChangeType::kIpV6_Assigned, "IPv6 address assigned" },
        { InterfaceAddressChange::kIPv6Lost, InterfaceIpChangeType::kIpV6_Lost, "IPv6 address lost" },
    };

    for (const auto & entry : kEvents)
    {
        if (!changes.Has(entry.mChange))
        {
            continue;
        }

        ChipLogDetail(DeviceLayer, "Interface address change: %s", entry.mDescription);

        ChipDeviceEvent event{};

        event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
        event.InterfaceIpAddressChanged.Type = entry.mType;

        LogErrorOnFailure(PlatformMgr().PostEvent(&event));
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
