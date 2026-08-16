/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#include "TermHandling.h"

#include <csignal>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

namespace chip::app {

/// Set the given function to be called when user requests
/// application stop (generally SIGINT and/or SIGTERM)
void SetTerminateHandler(void (*handler)(int))
{
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    auto & platformMgr = chip::DeviceLayer::PlatformMgrImpl();
    platformMgr.RegisterSignalHandler(SIGINT, ^{
        platformMgr.UnregisterAllSignalHandlers();
        handler(SIGINT);
    });

    platformMgr.RegisterSignalHandler(SIGTERM, ^{
        platformMgr.UnregisterAllSignalHandlers();
        handler(SIGTERM);
    });
#else
    // NOTE: For some reason, on Darwin, the signal handler is not called if the signal is
    //       registered with sigaction() call and TSAN is enabled. The problem seems to be
    //       related with the dispatch_semaphore_wait() function in the RunEventLoop() method.
    //       If this call is commented out, the signal handler is called as expected...
    // NOLINTBEGIN(bugprone-signal-handler)
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    // NOLINTEND(bugprone-signal-handler)
#endif
}

} // namespace chip::app
