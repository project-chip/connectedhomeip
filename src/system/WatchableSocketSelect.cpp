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
 *      This file implements WatchableSocket using select().
 */

#include <platform/LockTracker.h>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/WatchableEventManager.h>
#include <system/WatchableSocket.h>

#include <errno.h>

namespace chip {
namespace System {

CHIP_ERROR WatchableSocket::OnAttach()
{
    mSharedState->ResetRequests(mFD);

    VerifyOrReturnError(mAttachedNext == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mAttachedNext                  = mSharedState->mAttachedSockets;
    mSharedState->mAttachedSockets = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableSocket::OnRelease()
{
    VerifyOrReturnError(mFD >= 0, CHIP_ERROR_INCORRECT_STATE);
    mSharedState->ResetRequests(mFD);

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

    // Wake the thread calling select so that it stops selecting on the socket.
    mSharedState->Signal();
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableSocket::OnRequestCallbackOnPendingRead()
{
    return mSharedState->SetRequest(mFD, &mSharedState->mRequest.mReadSet);
}

CHIP_ERROR WatchableSocket::OnRequestCallbackOnPendingWrite()
{
    return mSharedState->SetRequest(mFD, &mSharedState->mRequest.mWriteSet);
}

CHIP_ERROR WatchableSocket::OnClearCallbackOnPendingRead()
{
    return mSharedState->ClearRequest(mFD, &mSharedState->mRequest.mReadSet);
}

CHIP_ERROR WatchableSocket::OnClearCallbackOnPendingWrite()
{
    return mSharedState->ClearRequest(mFD, &mSharedState->mRequest.mWriteSet);
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
