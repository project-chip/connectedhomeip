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

// Note: CONFIG_BUILD_FOR_HOST_UNIT_TEST
//
// Certain unit tests are executed without a main dispatch queue, relying instead on a mock clock
// to manually trigger timer callbacks at specific times.
// Under normal conditions, the absence of a dispatch queue would cause the tests to fail. However,
// when CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined, this constraint is relaxed, allowing such tests
// to run successfully.
//
// In these scenarios, timer and sockets events are explicitly triggered via calls to
// HandleDispatchQueueEvents(), using the mock clock provided by the test environment.
// Creating a dispatch queue in test mode does not work because dispatch_source timers always follow
// the real system clock, not our mock clock, so we must rely on mTimerList + HandleDispatchQueueEvents() instead.

namespace chip {
namespace System {
    CHIP_ERROR LayerImplDispatch::StartWatchingSocket(int fd, SocketWatchToken * tokenOut)
    {
        SocketWatch * watch = nullptr;

        ReturnErrorOnFailure(mSocketWatchPool.Allocate(fd, watch));
        *tokenOut = reinterpret_cast<SocketWatchToken>(watch);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::StopWatchingSocket(SocketWatchToken * tokenInOut)
    {
        __auto_type * watch = reinterpret_cast<SocketWatch *>(*tokenInOut);

        ReturnErrorOnFailure(mSocketWatchPool.Release(watch));
        *tokenInOut = InvalidSocketWatchToken();

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
        __auto_type dispatchQueue = GetDispatchQueue();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        // Note: if no dispatch queue is available, callbacks most probably will not work, unless,
        //       as in some tests from a test-specific local loop, the select based event handling is invoked.
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

#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void LayerImplDispatch::HandleSocketsAndTimerEvents(Clock::Timeout timeout) {};
    bool LayerImplDispatch::SocketWatchPool::PrepareEvents(SelectSets & sets, timeval timeout) const { return false; };
    void LayerImplDispatch::SocketWatchPool::HandleEvents(const SelectSets & sets) const {};
    void LayerImplDispatch::SocketWatch::PrepareEvents(SelectSets & sets, int & maxFd) const {};
    void LayerImplDispatch::SocketWatch::HandleEvents(const SelectSets & sets) const {};
#else
    void LayerImplDispatch::HandleSocketsAndTimerEvents(Clock::Timeout timeout)
    {
        const Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();
        Clock::Timestamp awakenTime = currentTime + timeout;

        TimerList::Node * timer = mTimerList.Earliest();
        if (timer) {
            awakenTime = std::min(awakenTime, timer->AwakenTime());
        }

        const Clock::Timestamp sleepTime = (awakenTime > currentTime) ? (awakenTime - currentTime) : Clock::kZero;
        timeval nextTimeout;
        Clock::ToTimeval(sleepTime, nextTimeout);

        SelectSets sets;
        bool hasEvents = mSocketWatchPool.PrepareEvents(sets, nextTimeout);

        HandleTimerEvents(timeout);

        if (hasEvents) {
            mSocketWatchPool.HandleEvents(sets);
        }
    }

    bool LayerImplDispatch::SocketWatchPool::PrepareEvents(SelectSets & sets, timeval timeout) const
    {
        // NOLINTBEGIN(clang-analyzer-security.insecureAPI.bzero)
        //
        // NOTE: darwin uses bzero to clear out FD sets. This is not a security concern.
        FD_ZERO(&sets.mReadSet);
        FD_ZERO(&sets.mWriteSet);
        FD_ZERO(&sets.mErrorSet);
        // NOLINTEND(clang-analyzer-security.insecureAPI.bzero)

        int maxFd = kInvalidFd;
        for (auto & w : mPool) {
            if (w.IsActive()) {
                w.PrepareEvents(sets, maxFd);
            }
        }
        VerifyOrReturnValue(kInvalidFd != maxFd, false);

        int selectResult = select(maxFd + 1, &sets.mReadSet, &sets.mWriteSet, &sets.mErrorSet, &timeout);
        VerifyOrReturnValue(selectResult > 0, false);

        return true;
    }

    void LayerImplDispatch::SocketWatchPool::HandleEvents(const SelectSets & sets) const
    {
        for (auto & w : mPool) {
            if (w.IsActive()) {
                w.HandleEvents(sets);
            }
        }
    }

    void LayerImplDispatch::SocketWatch::PrepareEvents(SelectSets & sets, int & maxFd) const
    {
        if (mFD > maxFd) {
            maxFd = mFD;
        }

        if (mPendingIO.Has(SocketEventFlags::kRead)) {
            FD_SET(mFD, &sets.mReadSet);
        }

        if (mPendingIO.Has(SocketEventFlags::kWrite)) {
            FD_SET(mFD, &sets.mWriteSet);
        }
    }

    void LayerImplDispatch::SocketWatch::HandleEvents(const SelectSets & sets) const
    {
        VerifyOrReturn(nullptr != mCallback);

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
    }
#endif

    CHIP_ERROR LayerImplDispatch::SocketWatchPool::Allocate(int fd, SocketWatch *& outWatch)
    {
        // 1) See if we already have one for this fd.
        for (auto & w : mPool) {
            if (w.mFD == fd) {
                outWatch = &w;
                return CHIP_NO_ERROR;
            }
        }

        // 2) Otherwise find a free slot.
        for (auto & w : mPool) {
            if (!w.IsActive()) {
                w.mFD = fd;
                outWatch = &w;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    CHIP_ERROR LayerImplDispatch::SocketWatchPool::Release(SocketWatch * watch)
    {
        VerifyOrReturnError(nullptr != watch, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(watch->IsActive(), CHIP_ERROR_INVALID_ARGUMENT);

        watch->Clear();
        return CHIP_NO_ERROR;
    }

    void LayerImplDispatch::SocketWatchPool::Clear()
    {
        for (auto & w : mPool) {
            w.Clear();
        }
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
