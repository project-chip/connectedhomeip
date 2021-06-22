/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file declares an implementation of WatchableEvents using select().
 */

#pragma once

#include <sys/select.h>

#include <support/BitFlags.h>

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE
#error "This file should only be included from <system/SystemSockets.h>"
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE

namespace chip {

namespace System {

class WatchableEventManager
{
public:
    void Init(System::Layer & systemLayer);
    void Shutdown();

    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

    // TODO(#5556): Some unit tests supply a timeout at low level, due to originally using select(); these should a proper timer.
    void PrepareEventsWithTimeout(timeval & nextTimeout);

    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

protected:
    friend class WatchableSocket;

    void Set(int fd, fd_set * fds);
    void Clear(int fd, fd_set * fds);

    Layer * mSystemLayer               = nullptr;
    WatchableSocket * mAttachedSockets = nullptr;

    // TODO(#5556): Integrate timer platform details with WatchableEventManager.
    struct timeval mNextTimeout;

    // Members for select loop
    struct SelectSets
    {
        fd_set mReadSet;
        fd_set mWriteSet;
        fd_set mErrorSet;
    };
    SelectSets mRequest;
    SelectSets mSelected;
    int mMaxFd;
    int mSelectResult; ///< return value from select()

private:
    bool HasAny(int fd);
    void MaybeLowerMaxFd();
    void Reset(int fd);
    void WakeSelect();
};

class WatchableSocket : public WatchableSocketBasis<WatchableSocket>
{
public:
    void OnInit() { mAttachedNext = nullptr; }
    void OnAttach();
    void OnClose();

    void OnRequestCallbackOnPendingRead() { mSharedState->Set(mFD, &mSharedState->mRequest.mReadSet); }
    void OnRequestCallbackOnPendingWrite() { mSharedState->Set(mFD, &mSharedState->mRequest.mWriteSet); }
    void OnClearCallbackOnPendingRead() { mSharedState->Clear(mFD, &mSharedState->mRequest.mReadSet); }
    void OnClearCallbackOnPendingWrite() { mSharedState->Clear(mFD, &mSharedState->mRequest.mWriteSet); }

    void SetPendingIO(SocketEvents events) { mPendingIO = events; }
    void SetFDs(int & nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds);

private:
    friend class WatchableEventManager;

    WatchableSocket * mAttachedNext; ///< Next element in the list of sockets attached to the WatchableEventManager.
};

} // namespace System
} // namespace chip
