/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl_POSIX<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_POSIX_IPP
#define GENERIC_PLATFORM_MANAGER_IMPL_POSIX_IPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_POSIX<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.ipp>

#include <system/SystemLayer.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <sys/select.h>
#include <unistd.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mChipStackLock = PTHREAD_MUTEX_INITIALIZER;

    // Initialize the Configuration Manager object.
    err = ConfigurationMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Configuration Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

    mShouldRunEventLoop.store(true, std::memory_order_relaxed);

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_LockChipStack(void)
{
    assert(pthread_mutex_lock(&mChipStackLock) == 0);
}

template <class ImplClass>
bool GenericPlatformManagerImpl_POSIX<ImplClass>::_TryLockChipStack(void)
{
    return pthread_mutex_trylock(&mChipStackLock) == 0;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_UnlockChipStack(void)
{
    assert(pthread_mutex_unlock(&mChipStackLock) == 0);
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartChipTimer(uint32_t aMilliseconds)
{
    // Let SystemLayer.PrepareSelect() handle timers.
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    mChipEventQueue.push(*event); // Thread safe due to ChipStackLock taken by App thread
    SysOnEventSignal(this);       // Trigger wake select on CHIP thread
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::ProcessDeviceEvents()
{
    while (!mChipEventQueue.empty())
    {
        Impl()->DispatchEvent(&mChipEventQueue.front());
        mChipEventQueue.pop();
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysOnEventSignal(void * arg)
{
    SystemLayer.WakeSelect();
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysUpdate()
{
    FD_ZERO(&mReadSet);
    FD_ZERO(&mWriteSet);
    FD_ZERO(&mErrorSet);
    mMaxFd = 0;

    // Max out this duration and let CHIP set it appropriately.
    mNextTimeout.tv_sec  = DEFAULT_MIN_SLEEP_PERIOD;
    mNextTimeout.tv_usec = 0;

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }

#if !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }
#endif // !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysProcess()
{
    int selectRes;
    uint32_t nextTimeoutMs;

    nextTimeoutMs = mNextTimeout.tv_sec * 1000 + mNextTimeout.tv_usec / 1000;
    ChipLogDetail(DeviceLayer, "Timer: %ld", nextTimeoutMs);
    _StartChipTimer(nextTimeoutMs);

    Impl()->UnlockChipStack();
    selectRes = select(mMaxFd + 1, &mReadSet, &mWriteSet, &mErrorSet, &mNextTimeout);
    Impl()->LockChipStack();

    if (selectRes < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

#if !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }
#endif // !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)

    ProcessDeviceEvents();
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_RunEventLoop(void)
{
    Impl()->LockChipStack();

    do
    {
        SysUpdate();
        SysProcess();
    } while (mShouldRunEventLoop.load(std::memory_order_relaxed));

    Impl()->UnlockChipStack();
}

template <class ImplClass>
void * GenericPlatformManagerImpl_POSIX<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");

    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(arg)->Impl()->RunEventLoop();
    return NULL;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartEventLoopTask(void)
{
    int err;
    err = pthread_attr_init(&mChipTaskAttr);
    SuccessOrExit(err);
    err = pthread_attr_getschedparam(&mChipTaskAttr, &mChipTaskSchedParam);
    SuccessOrExit(err);
    err = pthread_attr_setschedpolicy(&mChipTaskAttr, SCHED_RR);
    SuccessOrExit(err);
    err = pthread_create(&mChipTask, &mChipTaskAttr, EventLoopTaskMain, this);
    SuccessOrExit(err);
exit:
    return System::MapErrorPOSIX(err);
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_Shutdown(void)
{
    int err = 0;
    mShouldRunEventLoop.store(false, std::memory_order_relaxed);
    if (mChipTask)
    {
        SuccessOrExit(err = pthread_join(mChipTask, NULL));
    }

exit:
    return System::MapErrorPOSIX(err);
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_POSIX_IPP
