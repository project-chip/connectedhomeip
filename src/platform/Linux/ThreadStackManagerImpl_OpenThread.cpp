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

#include <openthread-select.h>

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

#if CHIP_ENABLE_OTNS
extern "C" {
extern int gSockFd;
uint64_t platformAlarmGetNext(void);
bool platformRadioIsTransmitPending(void);
bool platformRadioIsBusy(void);
}

namespace {
bool sOtnsNeedsSleepEvent        = true;
uint64_t sOtnsLastSleepNextAlarm = UINT64_MAX;

void DrainOtnsPendingWork(otInstance * instance)
{
    fd_set emptyFds;
    FD_ZERO(&emptyFds);
    while (otTaskletsArePending(instance) || platformAlarmGetNext() == 0 ||
           (platformRadioIsTransmitPending() && !platformRadioIsBusy()))
    {
        otTaskletsProcess(instance);
        otSysProcessEvents(instance, &emptyFds, &emptyFds, &emptyFds);
    }
}
} // namespace
#endif // CHIP_ENABLE_OTNS

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
#if CHIP_ENABLE_OTNS
    // Drain all pending OpenThread tasklets, zero-delay virtual alarms, and
    // unstarted radio transmissions before reporting a SleepEvent to the OTNS
    // dispatcher. Also preserve the host select() wall-clock timeout rather
    // than shrinking it to the simulated microsecond nextAlarm delta, and
    // avoid sending duplicate SleepEvents if woken by a non-OTNS fd when
    // nextAlarm has not changed.
    DrainOtnsPendingWork(OTInstance());

    uint64_t nextAlarm = platformAlarmGetNext();
    if (sOtnsNeedsSleepEvent || nextAlarm != sOtnsLastSleepNextAlarm)
    {
        struct timeval ignoredOtnsTimeout = timeout;
        otSysUpdateEvents(OTInstance(), &maxfd, &readfds, &writefds, &exceptfds, &ignoredOtnsTimeout);
        sOtnsNeedsSleepEvent    = false;
        sOtnsLastSleepNextAlarm = nextAlarm;
    }
    else
    {
        FD_SET(gSockFd, &readfds);
        if (gSockFd > maxfd)
        {
            maxfd = gSockFd;
        }
    }
#else
    struct timeval timeoutOpenThread = timeout;

    otTaskletsProcess(OTInstance());
    otSysUpdateEvents(OTInstance(), &maxfd, &readfds, &writefds, &exceptfds, &timeoutOpenThread);

    if (timeoutOpenThread.tv_sec < timeout.tv_sec ||
        (timeoutOpenThread.tv_sec == timeout.tv_sec && timeoutOpenThread.tv_usec < timeout.tv_usec))
    {
        timeout = timeoutOpenThread;
    }
#endif // CHIP_ENABLE_OTNS
}

void ThreadStackManagerImpl::ProcessEvents(const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds)
{
#if CHIP_ENABLE_OTNS
    if (FD_ISSET(gSockFd, &readfds))
    {
        sOtnsNeedsSleepEvent = true;
    }
    otSysProcessEvents(OTInstance(), &readfds, &writefds, &exceptfds);
    DrainOtnsPendingWork(OTInstance());
#else
    otSysProcessEvents(OTInstance(), &readfds, &writefds, &exceptfds);
#endif // CHIP_ENABLE_OTNS
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
