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

namespace mdns {
namespace Minimal {

/**
 * Watches RTNETLINK for link and address changes and calls back once they settle, so that the
 * server can listen again (see ServerBase::Listen). Changes arrive in bursts, so a settle timer
 * coalesces them, capped so that a long stream cannot defer the callback forever.
 */
class NetlinkInterfaceMonitor
{
public:
    using Callback = void (*)(void * context);

    NetlinkInterfaceMonitor() = default;
    ~NetlinkInterfaceMonitor();
    NetlinkInterfaceMonitor(const NetlinkInterfaceMonitor &)             = delete;
    NetlinkInterfaceMonitor & operator=(const NetlinkInterfaceMonitor &) = delete;

    /// Opens the socket and watches it on the CHIP event loop. CHIP thread, after the system layer is up.
    CHIP_ERROR Init(chip::System::LayerSockets & systemLayer, Callback callback, void * context);

    /// Stops watching and closes the socket. Before the system layer is shut down; idempotent.
    void Shutdown();

    bool IsRunning() const { return mSocket >= 0; }

private:
    static void HandleReadable(chip::System::SocketEvents events, intptr_t data);
    static void HandleSettled(chip::System::Layer * layer, void * appState);

    void OnReadable();
    void OnChangeSeen();

    chip::System::LayerSockets * mSystemLayer = nullptr;
    Callback mCallback                        = nullptr;
    void * mContext                           = nullptr;
    int mSocket                               = -1;
    chip::System::SocketWatchToken mWatch     = 0;
    bool mWatching                            = false;
    bool mChangePending                       = false;
    chip::System::Clock::Timestamp mFirstChange{ 0 };
};

} // namespace Minimal
} // namespace mdns
