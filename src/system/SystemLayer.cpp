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

Layer::Layer() : mLayerState(kLayerState_NotInitialized) {}

CHIP_ERROR Layer::Init()
{
    VerifyOrReturnError(State() == kLayerState_NotInitialized, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mWatchableEventsManager.Init(*this));
    this->mLayerState = kLayerState_Initialized;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Layer::Shutdown()
{
    VerifyOrReturnError(State() == kLayerState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mWatchableEventsManager.Shutdown());
    this->mLayerState = kLayerState_NotInitialized;
    return CHIP_NO_ERROR;
}

/**
 * @brief
 *   This method starts a one-shot timer.
 *
 *   @note
 *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
 *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
 *       the currently-running timer will first be cancelled.
 *
 *   @param[in]  aDelayMilliseconds Time in milliseconds before this timer fires.
 *   @param[in]  aComplete          A pointer to the function called when timer expires.
 *   @param[in]  aAppState          A pointer to the application state object used when timer expires.
 *
 *   @return CHIP_NO_ERROR On success.
 *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
 *   @return Other Value indicating timer failed to start.
 *
 */
CHIP_ERROR Layer::StartTimer(uint32_t aDelayMilliseconds, Timers::OnCompleteFunct aComplete, void * aAppState)
{
    VerifyOrReturnError(State() == kLayerState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.StartTimer(aDelayMilliseconds, aComplete, aAppState);
}

/**
 * @brief
 *   This method cancels a one-shot timer, started earlier through @p StartTimer().
 *
 *   @note
 *       The cancellation could fail silently in two different ways. If the timer specified by the combination of the callback
 *       function and application state object couldn't be found, cancellation could fail. If the timer has fired, but not yet
 *       removed from memory, cancellation could also fail.
 *
 *   @param[in]  aOnComplete   A pointer to the callback function used in calling @p StartTimer().
 *   @param[in]  aAppState     A pointer to the application state object used in calling @p StartTimer().
 *
 */
void Layer::CancelTimer(Timers::OnCompleteFunct aOnComplete, void * aAppState)
{
    VerifyOrReturn(this->State() == kLayerState_Initialized);
    return mWatchableEventsManager.CancelTimer(aOnComplete, aAppState);
}

/**
 * @brief
 *   Schedules a function with a signature identical to
 *   `OnCompleteFunct` to be run as soon as possible on the CHIP
 *   thread.
 *
 * @note
 *   This function could, in principle, be implemented as
 *   `StartTimer`.  The specification for
 *   `SystemTimer` however permits certain optimizations that might
 *   make that implementation impossible. Specifically, `SystemTimer`
 *   API may only be called from the thread owning the particular
 *   `SystemLayer`, whereas the `ScheduleWork` may be called from
 *   any thread.  Additionally, whereas the `SystemTimer` API permits
 *   the invocation of the already expired handler in line,
 *   `ScheduleWork` guarantees that the handler function will be
 *   called only after the current CHIP event completes.
 *
 * @param[in] aComplete A pointer to a callback function to be called
 *                      when this timer fires.
 *
 * @param[in] aAppState A pointer to an application state object to be
 *                      passed to the callback function as argument.
 *
 * @retval CHIP_ERROR_INCORRECT_STATE If the SystemLayer has
 *                      not been initialized.
 *
 * @retval CHIP_ERROR_NO_MEMORY If the SystemLayer cannot
 *                      allocate a new timer.
 *
 * @retval CHIP_NO_ERROR On success.
 */
CHIP_ERROR Layer::ScheduleWork(Timers::OnCompleteFunct aComplete, void * aAppState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(State() == kLayerState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    return mWatchableEventsManager.ScheduleWork(aComplete, aAppState);
}

} // namespace System
} // namespace chip
