/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file implements WatchableEventManager using select().
 */

#include <platform/LockTracker.h>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/WatchableEventManager.h>
#include <system/WatchableSocket.h>

#include <errno.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

namespace chip {
namespace Mdns {
void GetMdnsTimeout(timeval & timeout);
void HandleMdnsTimeout();
} // namespace Mdns
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

namespace chip {
namespace System {

CHIP_ERROR WatchableEventManager::Init(Layer & systemLayer)
{
    mSystemLayer = &systemLayer;
    mMaxFd       = -1;
    FD_ZERO(&mRequest.mReadSet);
    FD_ZERO(&mRequest.mWriteSet);
    FD_ZERO(&mRequest.mErrorSet);

    // Create an event to allow an arbitrary thread to wake the thread in the select loop.
    return mWakeEvent.Open(*this);
}

CHIP_ERROR WatchableEventManager::Shutdown()
{
    CHIP_ERROR error = mWakeEvent.Close();
    mSystemLayer     = nullptr;
    return error;
}

CHIP_ERROR WatchableEventManager::Signal()
{
    /*
     * Wake up the I/O thread by writing a single byte to the wake pipe.
     *
     * If p WakeIOThread() is being called from within an I/O event callback, then writing to the wake pipe can be skipped,
     * since the I/O thread is already awake.
     *
     * Furthermore, we don't care if this write fails as the only reasonably likely failure is that the pipe is full, in which
     * case the select calling thread is going to wake up anyway.
     */
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    if (pthread_equal(mSystemLayer->mHandleSelectThread, pthread_self()))
    {
        return CHIP_NO_ERROR;
    }
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Send notification to wake up the select call.
    CHIP_ERROR status = mWakeEvent.Notify();
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "System wake event notify failed: %" CHIP_ERROR_FORMAT, ChipError::FormatError(status));
    }
    return status;
}

/**
 *  Set the read, write or exception bit flags for the specified socket based on its status in
 *  the corresponding file descriptor sets.
 *
 *  @param[in]    socket    The file descriptor for which the bit flags are being set.
 *
 *  @param[in]    readfds   A pointer to the set of readable file descriptors.
 *
 *  @param[in]    writefds  A pointer to the set of writable file descriptors.
 *
 *  @param[in]    exceptfds  A pointer to the set of file descriptors with errors.
 */
SocketEvents WatchableEventManager::SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds,
                                                        const fd_set & exceptfds)
{
    SocketEvents res;

    if (socket >= 0)
    {
        // POSIX does not define the fd_set parameter of FD_ISSET() as const, even though it isn't modified.
        if (FD_ISSET(socket, const_cast<fd_set *>(&readfds)))
            res.Set(SocketEventFlags::kRead);
        if (FD_ISSET(socket, const_cast<fd_set *>(&writefds)))
            res.Set(SocketEventFlags::kWrite);
        if (FD_ISSET(socket, const_cast<fd_set *>(&exceptfds)))
            res.Set(SocketEventFlags::kExcept);
    }

    return res;
}

bool WatchableEventManager::HasAny(int fd)
{
    return FD_ISSET(fd, &mRequest.mReadSet) || FD_ISSET(fd, &mRequest.mWriteSet) || FD_ISSET(fd, &mRequest.mErrorSet);
}

CHIP_ERROR WatchableEventManager::Set(int fd, fd_set * fds)
{
    FD_SET(fd, fds);
    if (fd > mMaxFd)
    {
        mMaxFd = fd;
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    return Signal();
}

CHIP_ERROR WatchableEventManager::Clear(int fd, fd_set * fds)
{
    FD_CLR(fd, fds);
    if (fd == mMaxFd)
    {
        MaybeLowerMaxFd();
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    return Signal();
}

void WatchableEventManager::Reset(int fd)
{
    FD_CLR(fd, &mRequest.mReadSet);
    FD_CLR(fd, &mRequest.mWriteSet);
    FD_CLR(fd, &mRequest.mErrorSet);
    if (fd == mMaxFd)
    {
        MaybeLowerMaxFd();
    }
}

void WatchableEventManager::MaybeLowerMaxFd()
{
    int fd;
    for (fd = mMaxFd; fd >= 0; --fd)
    {
        if (HasAny(fd))
        {
            break;
        }
    }
    mMaxFd = fd;
}

void WatchableEventManager::PrepareEvents()
{
    assertChipStackLockedByCurrentThread();

    // Max out this duration and let CHIP set it appropriately.
    mNextTimeout.tv_sec  = DEFAULT_MIN_SLEEP_PERIOD;
    mNextTimeout.tv_usec = 0;
    PrepareEventsWithTimeout(mNextTimeout);
}

void WatchableEventManager::PrepareEventsWithTimeout(struct timeval & nextTimeout)
{
    // TODO(#5556): Integrate timer platform details with WatchableEventManager.
    mSystemLayer->GetTimeout(nextTimeout);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    chip::Mdns::GetMdnsTimeout(nextTimeout);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

    mSelected = mRequest;
}

void WatchableEventManager::WaitForEvents()
{
    mSelectResult = select(mMaxFd + 1, &mSelected.mReadSet, &mSelected.mWriteSet, &mSelected.mErrorSet, &mNextTimeout);
}

void WatchableEventManager::HandleEvents()
{
    assertChipStackLockedByCurrentThread();

    if (mSelectResult < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    VerifyOrDie(mSystemLayer != nullptr);
    mSystemLayer->HandleTimeout();

    for (WatchableSocket * watchable = mAttachedSockets; watchable != nullptr; watchable = watchable->mAttachedNext)
    {
        watchable->SetPendingIO(
            SocketEventsFromFDs(watchable->GetFD(), mSelected.mReadSet, mSelected.mWriteSet, mSelected.mErrorSet));
    }
    for (WatchableSocket * watchable = mAttachedSockets; watchable != nullptr; watchable = watchable->mAttachedNext)
    {
        if (watchable->mPendingIO.HasAny())
        {
            watchable->InvokeCallback();
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    chip::Mdns::HandleMdnsTimeout();
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
}

} // namespace System
} // namespace chip
