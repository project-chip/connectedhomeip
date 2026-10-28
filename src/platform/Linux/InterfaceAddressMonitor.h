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

#pragma once

#include <lib/core/CHIPError.h>
#include <platform/Linux/InterfaceAddressChanges.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  Posts a DeviceEventType::kInterfaceIpAddressChanged event whenever
 *  the IP addresses of any of the system's network interfaces change.
 *
 *  The kernel reports address changes on an rtnetlink socket, which the
 *  monitor reads on the Matter thread through the System layer. Each
 *  kind of change (see InterfaceAddressChange) read in one pass over the
 *  socket posts one event, so that a burst of changes posts at most one
 *  event of each kind. An event says only that addresses of one family
 *  were assigned or lost. A consumer reads the addresses it needs from
 *  the system.
 *
 *  This reports changes that no network management implementation
 *  reports, such as the IPv6 link-local address that the kernel assigns
 *  on its own to every interface brought up.
 *
 *  If the kernel drops notifications because the socket's receive
 *  buffer is full, the monitor cannot tell what changed, and posts an
 *  IPv4 and an IPv6 assignment, so that consumers read the addresses
 *  again.
 */
class InterfaceAddressMonitor
{
public:
    InterfaceAddressMonitor() = default;

    InterfaceAddressMonitor(const InterfaceAddressMonitor &)             = delete;
    InterfaceAddressMonitor & operator=(const InterfaceAddressMonitor &) = delete;

    /**
     *  Opens the rtnetlink socket and starts watching it.
     *
     *  Must be called on the Matter thread, or with the Matter stack lock
     *  held, after the System layer is initialized.
     *
     *  @param[in]  systemLayer  The System layer that watches the socket.
     *
     *  @retval  CHIP_NO_ERROR                On success.
     *  @retval  CHIP_ERROR_INCORRECT_STATE   If the monitor is already
     *                                        started.
     *  @retval  CHIP_ERROR_POSIX             If the socket cannot be
     *                                        opened or bound.
     *  @returns Any error from the System layer, if it cannot watch the
     *           socket.
     */
    CHIP_ERROR Init(System::LayerSockets & systemLayer);

    /**
     *  Stops watching the socket and closes it. Does nothing if the
     *  monitor is not started.
     *
     *  Must be called on the Matter thread, or with the Matter stack lock
     *  held, before the System layer is shut down.
     */
    void Shutdown();

private:
    static void HandleSocketEvents(System::SocketEvents events, intptr_t context);
    void ReadChanges();
    static void PostChanges(BitFlags<InterfaceAddressChange> changes);

    System::LayerSockets * mSystemLayer   = nullptr;
    System::SocketWatchToken mSocketWatch = 0;
    int mSocket                           = -1;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
