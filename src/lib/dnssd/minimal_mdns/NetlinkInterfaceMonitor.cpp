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

#include <lib/dnssd/minimal_mdns/NetlinkInterfaceMonitor.h>

#include <inet/InetConfig.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mdns {
namespace Minimal {

namespace {

// Quiet time before listening again, so that a burst of changes takes one pass.
constexpr chip::System::Clock::Timeout kSettleInterval = chip::System::Clock::Milliseconds32(1000);

// Listen again no later than this after the first change, however long the burst goes on.
constexpr chip::System::Clock::Timeout kMaxDeferral = chip::System::Clock::Milliseconds32(5000);

} // namespace

NetlinkInterfaceMonitor::~NetlinkInterfaceMonitor()
{
    // A static instance outlives the system layer, so only the socket is closed here.
    if (mSocket >= 0)
    {
        close(mSocket);
    }
}

CHIP_ERROR NetlinkInterfaceMonitor::Init(chip::System::LayerSockets & systemLayer, Callback callback, void * context)
{
    VerifyOrReturnError(mSocket < 0, CHIP_ERROR_INCORRECT_STATE);

    mSocket = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, NETLINK_ROUTE);
    VerifyOrReturnError(mSocket >= 0, CHIP_ERROR_POSIX(errno));

    struct sockaddr_nl addr = {};
    addr.nl_family          = AF_NETLINK;
    // nl_pid zero: the kernel assigns an id, so other netlink sockets of the process keep theirs.
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

    mSystemLayer = &systemLayer;
    mCallback    = callback;
    mContext     = context;

    CHIP_ERROR err;
    SuccessOrExit(err = systemLayer.StartWatchingSocket(mSocket, &mWatch));
    mWatching = true;
    SuccessOrExit(err = systemLayer.SetCallback(mWatch, HandleReadable, reinterpret_cast<intptr_t>(this)));
    SuccessOrExit(err = systemLayer.RequestCallbackOnPendingRead(mWatch));

    return CHIP_NO_ERROR;

exit:
    Shutdown();
    return err;
}

void NetlinkInterfaceMonitor::Shutdown()
{
    if (mChangePending)
    {
        mSystemLayer->CancelTimer(HandleSettled, this);
        mChangePending = false;
    }

    if (mWatching)
    {
        LogErrorOnFailure(mSystemLayer->StopWatchingSocket(&mWatch));
        mWatching = false;
    }

    if (mSocket >= 0)
    {
        close(mSocket);
        mSocket = -1;
    }
}

void NetlinkInterfaceMonitor::HandleReadable(chip::System::SocketEvents, intptr_t data)
{
    reinterpret_cast<NetlinkInterfaceMonitor *>(data)->OnReadable();
}

void NetlinkInterfaceMonitor::HandleSettled(chip::System::Layer *, void * appState)
{
    auto * self          = static_cast<NetlinkInterfaceMonitor *>(appState);
    self->mChangePending = false;
    self->mCallback(self->mContext);
}

void NetlinkInterfaceMonitor::OnReadable()
{
    // Only link and address messages arrive here, and what they say does not matter: listening
    // again finds what changed. Drain the burst; each message only pushes the settle timer back.
    char buffer[4096];
    bool sawChange = false;

    while (true)
    {
        ssize_t len = recv(mSocket, buffer, sizeof(buffer), 0);
        if (len >= 0)
        {
            sawChange = true;
            continue;
        }
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
            // Notifications were dropped; listening again finds whatever they were about.
            sawChange = true;
            continue;
        }
        // An unrecoverable error would keep the socket readable and spin the loop: stop monitoring.
        ChipLogError(Discovery, "Error reading from the interface netlink socket: %d; no longer following interface changes",
                     errno);
        Shutdown();
        return;
    }

    if (sawChange)
    {
        OnChangeSeen();
    }
}

void NetlinkInterfaceMonitor::OnChangeSeen()
{
    chip::System::Clock::Timestamp now = chip::System::SystemClock().GetMonotonicTimestamp();

    if (!mChangePending)
    {
        mFirstChange   = now;
        mChangePending = true;
    }
    else if (now - mFirstChange >= kMaxDeferral)
    {
        // Deferred long enough; let the pending timer fire.
        return;
    }

    // StartTimer replaces a pending timer with the same callback and state.
    CHIP_ERROR err = mSystemLayer->StartTimer(kSettleInterval, HandleSettled, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to arm the interface settle timer: %" CHIP_ERROR_FORMAT, err.Format());
        // Better now than never.
        HandleSettled(mSystemLayer, this);
    }
}

} // namespace Minimal
} // namespace mdns
