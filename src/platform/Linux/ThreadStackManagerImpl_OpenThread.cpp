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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          OpenThread platform.
 *
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "ThreadStackManagerImpl_OpenThread.h"

// CHIP_DEVICE_CONFIG_THREAD_OT_POSIX_MAINLOOP (defined in CHIPDevicePlatformConfig.h,
// pulled in by CHIPDeviceConfig.h) selects the posix otSysMainloop* event API over
// the simulation platform's otSysUpdateEvents/otSysProcessEvents.
#include <platform/CHIPDeviceConfig.h>
#if CHIP_DEVICE_CONFIG_THREAD_OT_POSIX_MAINLOOP
#include <openthread/openthread-system.h>
#else
#include <openthread-select.h>
#endif

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>
#include <system/SystemLayerImpl.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl & ThreadStackManagerImpl::Instance()
{
    static ThreadStackManagerImpl instance;
    return instance;
}

ThreadStackManagerImpl::~ThreadStackManagerImpl()
{
    static_cast<chip::System::LayerImpl &>(SystemLayer()).EventSourceRemove(this);
}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    return GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(nullptr);
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    static_cast<chip::System::LayerImpl &>(SystemLayer()).EventSourceAdd(this);
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::PrepareEvents(int & maxfd, fd_set & readfds, fd_set & writefds, fd_set & exceptfds,
                                           struct timeval & timeout)
{
    struct timeval timeoutOpenThread = timeout;

    otTaskletsProcess(OTInstance());
#if CHIP_DEVICE_CONFIG_THREAD_OT_POSIX_MAINLOOP
    // The POSIX platform bundles the fd sets, max fd and timeout into a single
    // otSysMainloopContext. otSysMainloopUpdate only *adds* descriptors and
    // shortens the timeout, so seed the context with CHIP's aggregates and copy
    // the merged results back into timeoutOpenThread for the shared min below.
    otSysMainloopContext mainloop{};
    mainloop.mMaxFd      = maxfd;
    mainloop.mReadFdSet  = readfds;
    mainloop.mWriteFdSet = writefds;
    mainloop.mErrorFdSet = exceptfds;
    mainloop.mTimeout    = timeoutOpenThread;
    otSysMainloopUpdate(OTInstance(), &mainloop);
    maxfd             = mainloop.mMaxFd;
    readfds           = mainloop.mReadFdSet;
    writefds          = mainloop.mWriteFdSet;
    exceptfds         = mainloop.mErrorFdSet;
    timeoutOpenThread = mainloop.mTimeout;
#else
    otSysUpdateEvents(OTInstance(), &maxfd, &readfds, &writefds, &exceptfds, &timeoutOpenThread);
#endif

    if (timeoutOpenThread.tv_sec < timeout.tv_sec ||
        (timeoutOpenThread.tv_sec == timeout.tv_sec && timeoutOpenThread.tv_usec < timeout.tv_usec))
    {
        timeout = timeoutOpenThread;
    }
}

void ThreadStackManagerImpl::ProcessEvents(const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds)
{
#if CHIP_DEVICE_CONFIG_THREAD_OT_POSIX_MAINLOOP
    // mMaxFd is unused on the process path (otSysMainloopProcess dispatches by
    // FD_ISSET, never iterating up to mMaxFd), so only the fd sets are seeded.
    otSysMainloopContext mainloop{};
    mainloop.mReadFdSet  = readfds;
    mainloop.mWriteFdSet = writefds;
    mainloop.mErrorFdSet = exceptfds;
    otSysMainloopProcess(OTInstance(), &mainloop);
#else
    otSysProcessEvents(OTInstance(), &readfds, &writefds, &exceptfds);
#endif
}

void ThreadStackManagerImpl::_LockThreadStack()
{
    // Matter stack is supposed to be the only user of OpenThread Stack and
    // OpenThread Stack is integrated into Matter mainloop.
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    // Matter stack is supposed to be the only user of OpenThread Stack and
    // OpenThread Stack is integrated into Matter mainloop.
    return true;
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
    // Matter stack is supposed to be the only user of OpenThread Stack and
    // OpenThread Stack is integrated into Matter mainloop.
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
void ThreadStackManagerImpl::_WaitOnSrpClearAllComplete() {}

void ThreadStackManagerImpl::_NotifySrpClearAllComplete() {}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

ThreadStackManager & ThreadStackMgr()
{
    return ThreadStackManagerImpl::Instance();
}

ThreadStackManagerImpl & ThreadStackMgrImpl()
{
    return ThreadStackManagerImpl::Instance();
}

} // namespace DeviceLayer
} // namespace chip
