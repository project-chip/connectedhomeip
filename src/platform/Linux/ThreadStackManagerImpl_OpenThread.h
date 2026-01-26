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

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/ThreadStackManager.h>
#include <system/SystemLayerImplSelect.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ThreadStackManager singleton object for Linux platform using OpenThread Endpoint.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public System::LayerImplSelect::EventSource,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>
{
    friend ThreadStackManager;

public:
    CHIP_ERROR _InitThreadStack();

    void PrepareEvents(int & maxfd, fd_set & readfds, fd_set & writefds, fd_set & exceptfds, struct timeval & timeout) override;
    void ProcessEvents(const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds) override;

    static ThreadStackManagerImpl & Instance();

protected:
    CHIP_ERROR _StartThreadTask();
    void _LockThreadStack();
    bool _TryLockThreadStack();
    void _UnlockThreadStack();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    void _WaitOnSrpClearAllComplete();
    void _NotifySrpClearAllComplete();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    void _ProcessThreadActivity() {}

private:
    ThreadStackManagerImpl() = default;
};

} // namespace DeviceLayer
} // namespace chip
