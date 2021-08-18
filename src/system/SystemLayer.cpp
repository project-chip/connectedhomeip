/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file contains definitions of the chip::System::Layer
 *      class methods and related data and functions.
 */

#include <system/SystemLayer.h>

#include <platform/LockTracker.h>
#include <support/CodeUtils.h>

namespace chip {
namespace System {

Layer::Layer() : mLayerState(LayerState::kUninitialized) {}

CHIP_ERROR Layer::Init()
{
    VerifyOrReturnError(State() == LayerState::kUninitialized, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mWatchableEventsManager.Init(*this));
    this->mLayerState = LayerState::kInitialized;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Layer::Shutdown()
{
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mWatchableEventsManager.Shutdown());
    this->mLayerState = LayerState::kUninitialized;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Layer::StartTimer(uint32_t aDelayMilliseconds, TimerCompleteCallback aComplete, void * aAppState)
{
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.StartTimer(aDelayMilliseconds, aComplete, aAppState);
}

void Layer::CancelTimer(TimerCompleteCallback aOnComplete, void * aAppState)
{
    VerifyOrReturn(this->State() == LayerState::kInitialized);
    return mWatchableEventsManager.CancelTimer(aOnComplete, aAppState);
}

CHIP_ERROR Layer::ScheduleWork(TimerCompleteCallback aComplete, void * aAppState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.ScheduleWork(aComplete, aAppState);
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

CHIP_ERROR Layer::StartWatchingSocket(int fd, SocketWatchToken * tokenOut)
{
    return mWatchableEventsManager.StartWatchingSocket(fd, tokenOut);
}

CHIP_ERROR Layer::SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data)
{
    return mWatchableEventsManager.SetCallback(token, callback, data);
}

CHIP_ERROR Layer::RequestCallbackOnPendingRead(SocketWatchToken token)
{
    return mWatchableEventsManager.RequestCallbackOnPendingRead(token);
}

CHIP_ERROR Layer::RequestCallbackOnPendingWrite(SocketWatchToken token)
{
    return mWatchableEventsManager.RequestCallbackOnPendingWrite(token);
}

CHIP_ERROR Layer::ClearCallbackOnPendingRead(SocketWatchToken token)
{
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.ClearCallbackOnPendingRead(token);
}

CHIP_ERROR Layer::ClearCallbackOnPendingWrite(SocketWatchToken token)
{
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.ClearCallbackOnPendingWrite(token);
}

CHIP_ERROR Layer::StopWatchingSocket(SocketWatchToken * tokenInOut)
{
    VerifyOrReturnError(State() == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.StopWatchingSocket(tokenInOut);
}

SocketWatchToken Layer::InvalidSocketWatchToken()
{
    return mWatchableEventsManager.InvalidSocketWatchToken();
}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH

void Layer::SetDispatchQueue(dispatch_queue_t dispatchQueue)
{
    mWatchableEventsManager.SetDispatchQueue(dispatchQueue);
}

dispatch_queue_t Layer::GetDispatchQueue()
{
    return mWatchableEventsManager.GetDispatchQueue();
}

#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP

CHIP_ERROR Layer::AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate)
{
    VerifyOrReturnError(mLayerState == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.AddEventHandlerDelegate(aDelegate);
}

CHIP_ERROR Layer::PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    VerifyOrReturnError(mLayerState == LayerState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.PostEvent(aTarget, aEventType, aArgument);
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace chip
