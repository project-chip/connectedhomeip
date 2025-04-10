/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements Layer using dispatch and sockets.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <system/SystemLayerImplDispatch.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace System {
    void LayerImplDispatch::HandleSocketsAndTimerEvents(Clock::Timeout timeout)
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        SelectSets selected;
        int maxFd = -1;

        // NOLINTBEGIN(clang-analyzer-security.insecureAPI.bzero)
        //
        // NOTE: darwin uses bzero to clear out FD sets. This is not a security concern.
        FD_ZERO(&selected.mReadSet);
        FD_ZERO(&selected.mWriteSet);
        FD_ZERO(&selected.mErrorSet);
        // NOLINTEND(clang-analyzer-security.insecureAPI.bzero)

        for (auto & w : mSocketWatchPool) {
            w.PrepareEvents(selected, maxFd);
        }

        Clock::ToTimeval(timeout, mNextTimeout);
        const int selectResult = select(maxFd + 1, &selected.mReadSet, &selected.mWriteSet, &selected.mErrorSet, &mNextTimeout);

        HandleTimerEvents(timeout);

        // Process socket events, if any
        if (selectResult > 0) {
            for (auto & w : mSocketWatchPool) {
                w.HandleEvents(selected);
            }
        }
#endif
    }

    CHIP_ERROR LayerImplDispatch::StartWatchingSocket(int fd, SocketWatchToken * tokenOut)
    {
        // Find a free slot.
        SocketWatch * watch = nullptr;
        for (auto & w : mSocketWatchPool) {
            if (w.mFD == fd) {
                // Already registered, return the existing token
                *tokenOut = reinterpret_cast<SocketWatchToken>(&w);
                return CHIP_NO_ERROR;
            }
            if ((w.mFD == kInvalidFd) && (watch == nullptr)) {
                watch = &w;
            }
        }
        VerifyOrReturnError(watch != nullptr, CHIP_ERROR_ENDPOINT_POOL_FULL);

        watch->mFD = fd;

        *tokenOut = reinterpret_cast<SocketWatchToken>(watch);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data)
    {
        __auto_type * watch = reinterpret_cast<SocketWatch *>(token);
        VerifyOrReturnError(nullptr != watch, CHIP_ERROR_INVALID_ARGUMENT);

        watch->mCallback = callback;
        watch->mCallbackData = data;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::RequestCallback(SocketWatchToken token, SocketEventFlags flag)
    {
        __auto_type * watch = reinterpret_cast<SocketWatch *>(token);
        VerifyOrReturnError(nullptr != watch, CHIP_ERROR_INVALID_ARGUMENT);

        watch->mPendingIO.Set(flag);

        __auto_type & source = (flag == SocketEventFlags::kRead) ? watch->mRdSource : watch->mWrSource;
        __auto_type sourceType = (flag == SocketEventFlags::kRead) ? DISPATCH_SOURCE_TYPE_READ : DISPATCH_SOURCE_TYPE_WRITE;

        VerifyOrReturnError(nullptr == source, CHIP_NO_ERROR);

        // First time requesting callback for those events: install a dispatch source
        dispatch_queue_t dispatchQueue = GetDispatchQueue();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        // Note: if no dispatch queue is available, callbacks most probably will not work,
        //       unless, as in some tests from a test-specific local loop,
        //       the select based event handling is invoked.
        VerifyOrReturnError(nullptr != dispatchQueue, CHIP_NO_ERROR);
#else
        VerifyOrDie(nullptr != dispatchQueue);
#endif

        source = dispatch_source_create(sourceType, static_cast<uintptr_t>(watch->mFD), 0, dispatchQueue);
        VerifyOrReturnError(nullptr != source, CHIP_ERROR_NO_MEMORY);

        dispatch_source_set_event_handler(source, ^{
            if (watch->mPendingIO.Has(flag) && watch->mCallback != nullptr) {
                SocketEvents events;
                events.Set(flag);
                watch->mCallback(events, watch->mCallbackData);
            }
        });
        // only now we are sure the source exists and can become active
        dispatch_activate(source);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::ClearCallback(SocketWatchToken token, SocketEventFlags flag)
    {
        __auto_type * watch = reinterpret_cast<SocketWatch *>(token);
        VerifyOrReturnError(nullptr != watch, CHIP_ERROR_INVALID_ARGUMENT);

        watch->mPendingIO.Clear(flag);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::RequestCallbackOnPendingRead(SocketWatchToken token)
    {
        return RequestCallback(token, SocketEventFlags::kRead);
    }

    CHIP_ERROR LayerImplDispatch::RequestCallbackOnPendingWrite(SocketWatchToken token)
    {
        return RequestCallback(token, SocketEventFlags::kWrite);
    }

    CHIP_ERROR LayerImplDispatch::ClearCallbackOnPendingRead(SocketWatchToken token)
    {
        return ClearCallback(token, SocketEventFlags::kRead);
    }

    CHIP_ERROR LayerImplDispatch::ClearCallbackOnPendingWrite(SocketWatchToken token)
    {
        return ClearCallback(token, SocketEventFlags::kWrite);
    }

    CHIP_ERROR LayerImplDispatch::StopWatchingSocket(SocketWatchToken * tokenInOut)
    {
        __auto_type * watch = reinterpret_cast<SocketWatch *>(*tokenInOut);
        *tokenInOut = InvalidSocketWatchToken();

        VerifyOrReturnError(nullptr != watch, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(watch->mFD >= 0, CHIP_ERROR_INCORRECT_STATE);

        watch->Clear();

        return CHIP_NO_ERROR;
    }

    void LayerImplDispatch::ClearSocketWatchPool()
    {
        for (auto & w : mSocketWatchPool) {
            w.Clear();
        }
    }

    void LayerImplDispatch::SocketWatch::PrepareEvents(SelectSets & sets, int & maxFd) const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrReturn(IsActive());

        if (mFD > maxFd) {
            maxFd = mFD;
        }

        if (mPendingIO.Has(SocketEventFlags::kRead)) {
            FD_SET(mFD, &sets.mReadSet);
        }

        if (mPendingIO.Has(SocketEventFlags::kWrite)) {
            FD_SET(mFD, &sets.mWriteSet);
        }
#endif
    }

    void LayerImplDispatch::SocketWatch::HandleEvents(const SelectSets & sets) const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrReturn(IsActive());
        VerifyOrReturn(nullptr != mCallback);
        VerifyOrReturn(mFD >= 0);

        SocketEvents events;

        // POSIX does not define the fd_set parameter of FD_ISSET() as const, even though it isn't modified.
        if (FD_ISSET(mFD, const_cast<fd_set *>(&sets.mReadSet))) {
            events.Set(SocketEventFlags::kRead);
        }

        if (FD_ISSET(mFD, const_cast<fd_set *>(&sets.mWriteSet))) {
            events.Set(SocketEventFlags::kWrite);
        }

        if (FD_ISSET(mFD, const_cast<fd_set *>(&sets.mErrorSet))) {
            events.Set(SocketEventFlags::kExcept);
        }

        VerifyOrReturn(events.HasAny());

        mCallback(events, mCallbackData);
#endif
    }

    void LayerImplDispatch::SocketWatch::Clear()
    {
        mFD = kInvalidFd;
        mPendingIO.ClearAll();
        mCallback = nullptr;
        mCallbackData = 0;

        if (mRdSource) {
            dispatch_source_cancel(mRdSource);
            mRdSource = nullptr;
        }

        if (mWrSource) {
            dispatch_source_cancel(mWrSource);
            mWrSource = nullptr;
        }
    }

} // namespace System
} // namespace chip
