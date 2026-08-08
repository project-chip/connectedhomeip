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
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Watches the kernel for interface and address changes and asks DNS-SD to restart when they
 * settle.
 *
 * minimal-mDNS binds a UDP endpoint per interface and enumerates the interfaces once, when the
 * advertiser starts, so an interface that appears afterwards is never advertised on and one that
 * goes away leaves a dead endpoint behind. Restarting DNS-SD re-runs that enumeration.
 *
 * Changes arrive in bursts -- bringing one interface up produces a link message and one message
 * per address -- so restarting on each of them would announce many times over for a single event.
 * Instead a change starts a settle timer, further changes push it back, and a hard cap keeps a
 * long stream of changes from deferring the restart forever.
 */
class DnssdInterfaceMonitor
{
public:
    DnssdInterfaceMonitor() = default;
    ~DnssdInterfaceMonitor();
    DnssdInterfaceMonitor(const DnssdInterfaceMonitor &)             = delete;
    DnssdInterfaceMonitor & operator=(const DnssdInterfaceMonitor &) = delete;

    /**
     * Opens an RTNETLINK socket and watches it on the CHIP event loop. Must be called on the CHIP
     * thread, after the system layer is up.
     */
    CHIP_ERROR Init();

    /**
     * Stops watching and closes the socket. Must be called before the system layer is shut down.
     * Idempotent.
     */
    void Shutdown();

private:
    static void HandleNetlinkReadable(System::SocketEvents events, intptr_t data);
    static void HandleSettled(System::Layer * layer, void * appState);

    void OnNetlinkReadable();
    void OnChangeSeen();
    void OnSettled();

    int mSocket                     = -1;
    System::SocketWatchToken mWatch = 0;
    bool mWatching                  = false;
    bool mChangePending             = false;
    System::Clock::Timestamp mFirstChange{ 0 };
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
