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

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DBusError dbusError;

    dbus_error_init(&dbusError);
    mDBusConnection = UniqueDBusConnection(dbus_bus_get(DBUS_BUS_SYSTEM, &dbusError));
    VerifyOrExit(mDBusConnection != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(dbus_bus_register(mDBusConnection.get(), &dbusError) == true, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(
        dbus_connection_set_watch_functions(mDBusConnection.get(), AddDBusWatch, RemoveDBusWatch, ToggleDBusWatch, this, NULL), err = CHIP_ERROR_INTERNAL);

    mChipStackLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    dbus_error_free(&dbusError);
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

    UpdateDBusFdSet();
    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }

    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysProcess()
{
    int selectRes;
    uint32_t nextTimeoutMs;

    nextTimeoutMs = mNextTimeout.tv_sec * 1000 + mNextTimeout.tv_usec / 1000;
    _StartChipTimer(nextTimeoutMs);

    Impl()->UnlockChipStack();
    selectRes = select(mMaxFd + 1, &mReadSet, &mWriteSet, &mErrorSet, &mNextTimeout);
    Impl()->LockChipStack();

    if (selectRes < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    ProcessDBus();

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

    ProcessDeviceEvents();
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_RunEventLoop(void)
{
    Impl()->LockChipStack();

    // TODO(#742): add exit condition
    while (true)
    {
        SysUpdate();
        SysProcess();
    }

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
DBusConnection & GenericPlatformManagerImpl_POSIX<ImplClass>::GetSystemDBusConnection(void)
{
    return *mDBusConnection.get();
}

template <class ImplClass>
dbus_bool_t GenericPlatformManagerImpl_POSIX<ImplClass>::AddDBusWatch(struct DBusWatch * aWatch, void * aContext)
{
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(aContext)->mWatches[aWatch] = true;
    return TRUE;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::RemoveDBusWatch(struct DBusWatch * aWatch, void * aContext)
{
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(aContext)->mWatches.erase(aWatch);
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::ToggleDBusWatch(struct DBusWatch * aWatch, void * aContext)
{
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(aContext)->mWatches[aWatch] =
        (dbus_watch_get_enabled(aWatch) ? true : false);
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::UpdateDBusFdSet()
{
    if (dbus_connection_get_dispatch_status(mDBusConnection.get()) == DBUS_DISPATCH_DATA_REMAINS)
    {
        mNextTimeout = { 0, 0 };
    }

    for (const auto & p : mWatches)
    {
        DBusWatch * watch = NULL;
        unsigned int flags;
        int fd;

        if (!p.second)
        {
            continue;
        }

        watch = p.first;
        flags = dbus_watch_get_flags(watch);
        fd    = dbus_watch_get_unix_fd(watch);

        if (fd < 0)
        {
            continue;
        }

        if (flags & DBUS_WATCH_READABLE)
        {
            FD_SET(fd, &mReadSet);
        }

        if ((flags & DBUS_WATCH_WRITABLE) && dbus_connection_has_messages_to_send(mDBusConnection.get()))
        {
            FD_SET(fd, &mWriteSet);
        }

        FD_SET(fd, &mErrorSet);

        if (fd > mMaxFd)
        {
            mMaxFd = fd;
        }
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::ProcessDBus()
{
    for (const auto & p : mWatches)
    {
        DBusWatch * watch = NULL;
        unsigned int flags;
        int fd;

        if (!p.second)
        {
            continue;
        }

        watch = p.first;
        flags = dbus_watch_get_flags(watch);
        fd    = dbus_watch_get_unix_fd(watch);

        if (fd < 0)
        {
            continue;
        }

        if ((flags & DBUS_WATCH_READABLE) && !FD_ISSET(fd, &mReadSet))
        {
            flags &= static_cast<unsigned int>(~DBUS_WATCH_READABLE);
        }

        if ((flags & DBUS_WATCH_WRITABLE) && !FD_ISSET(fd, &mWriteSet))
        {
            flags &= static_cast<unsigned int>(~DBUS_WATCH_WRITABLE);
        }

        if (FD_ISSET(fd, &mErrorSet))
        {
            flags |= DBUS_WATCH_ERROR;
        }

        dbus_watch_handle(watch, flags);
    }

    while (DBUS_DISPATCH_DATA_REMAINS == dbus_connection_get_dispatch_status(mDBusConnection.get()) &&
           dbus_connection_read_write_dispatch(mDBusConnection.get(), 0))
        ;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_POSIX_IPP
