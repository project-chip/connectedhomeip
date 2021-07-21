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
 *      This file implements WatchableEvents using select().
 */

#include <platform/LockTracker.h>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/SystemSockets.h>

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

void WatchableEventManager::Init(Layer & systemLayer)
{
    mSystemLayer = &systemLayer;
    mMaxFd       = -1;
    FD_ZERO(&mRequest.mReadSet);
    FD_ZERO(&mRequest.mWriteSet);
    FD_ZERO(&mRequest.mErrorSet);
}

void WatchableEventManager::Shutdown()
{
    mSystemLayer = nullptr;
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

void WatchableEventManager::WakeSelect()
{
#if CHIP_SYSTEM_CONFIG_USE_IO_THREAD
    mSystemLayer->WakeIOThread();
#endif // CHIP_SYSTEM_CONFIG_USE_IO_THREAD
}

void WatchableEventManager::Set(int fd, fd_set * fds)
{
    FD_SET(fd, fds);
    if (fd > mMaxFd)
    {
        mMaxFd = fd;
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    WakeSelect();
}

void WatchableEventManager::Clear(int fd, fd_set * fds)
{
    FD_CLR(fd, fds);
    if (fd == mMaxFd)
    {
        MaybeLowerMaxFd();
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    WakeSelect();
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

void WatchableSocket::OnAttach()
{
    mSharedState->Reset(mFD);

    VerifyOrDie(mAttachedNext == nullptr);
    mAttachedNext                  = mSharedState->mAttachedSockets;
    mSharedState->mAttachedSockets = this;
}

void WatchableSocket::OnClose()
{
    VerifyOrDie(mFD >= 0);
    mSharedState->Reset(mFD);

    WatchableSocket ** pp = &mSharedState->mAttachedSockets;
    while (*pp != nullptr)
    {
        if (*pp == this)
        {
            *pp = this->mAttachedNext;
            break;
        }
        pp = &(*pp)->mAttachedNext;
    }

#if CHIP_SYSTEM_CONFIG_USE_IO_THREAD
    // Wake the thread calling select so that it stops selecting on the socket.
    mSharedState->WakeSelect();
#endif // CHIP_SYSTEM_CONFIG_USE_IO_THREAD
}

/**
 *  Sets the bit for the specified file descriptor in the given sets of file descriptors.
 *
 *  @param[out]   nfds      A reference to the range of file descriptors in the set.
 *
 *  @param[in]    readfds   A pointer to the set of readable file descriptors.
 *
 *  @param[in]    writefds  A pointer to the set of writable file descriptors.
 *
 *  @param[in]    exceptfds  A pointer to the set of file descriptors with errors.
 *
 */
void WatchableSocket::SetFDs(int & nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds)
{
    if (mFD >= 0)
    {
        int r = FD_ISSET(mFD, &mSharedState->mRequest.mReadSet);
        int w = FD_ISSET(mFD, &mSharedState->mRequest.mWriteSet);
        int e = FD_ISSET(mFD, &mSharedState->mRequest.mErrorSet);
        if (r)
            FD_SET(mFD, readfds);
        if (w)
            FD_SET(mFD, writefds);
        if (e)
            FD_SET(mFD, exceptfds);
        if ((r || w || e) && mFD >= nfds)
            nfds = mFD + 1;
    }
}

} // namespace System
} // namespace chip
