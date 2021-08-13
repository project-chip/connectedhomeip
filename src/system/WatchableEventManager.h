/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file declares the abstraction for managing a collection of socket (file descriptor) events.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

namespace chip {

namespace System {

class Layer;

/**
 * @class WatchableEventManager
 *
 * An instance of this type is contained in System::Layer, to provide an implementation of event handling.
 */
class WatchableEventManager;

/**
 * @fn CHIP_ERROR WatchableEventManager::Init(System::Layer & systemLayer)
 *
 * Initialize the WatchableEventManager. Called from System::Layer::Init().
 */

/**
 * @fn CHIP_ERROR WatchableEventManager::Shutdown()
 *
 * Shut down the WatchableEventManager. Called from System::Layer::Shutdown()
 */

/**
 * @fn void WatchableEventManager::Signal()
 *
 * Called to indicate that event monitoring may need to be refreshed or resumed.
 */

/**
 * @fn CHIP_ERROR WatchableEventManager::StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void *
 * appState)
 *
 * This method starts a one-shot timer.
 *
 * Only a single timer is allowed to be started with the same @a onComplete and @a appState
 * arguments. If called with @a onComplete and @a appState identical to an existing timer,
 * the currently-running timer will first be cancelled.
 *
 * @param[in]  delayMilliseconds    Expiration time in milliseconds.
 * @param[in]  onComplete           A pointer to the function called when timer expires.
 * @param[in]  appState             A pointer to the application state object used when timer expires.
 *
 * @return CHIP_NO_ERROR            On success.
 * @return CHIP_ERROR_NO_MEMORY     If a timer cannot be allocated.
 * @return Other value indicating timer failed to start.
 */

/**
 * @fn void WatchableEventManager::CancelTimer(TimerCompleteCallback onComplete, void * appState)
 *
 * This method cancels a one-shot timer, started earlier through @p StartTimer().
 *
 * The cancellation could fail silently in two different ways. If the timer specified by the combination of the callback
 * function and application state object couldn't be found, cancellation could fail. If the timer has fired, but not yet
 * removed from memory, cancellation could also fail.
 *
 * @param[in]  onComplete   A pointer to the callback function used in calling @p StartTimer().
 * @param[in]  appState     A pointer to the application state object used in calling @p StartTimer().
 */

/**
 * @fn CHIP_ERROR WatchableEventManager::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
 *
 * Schedules a function to be run as soon as possible on the CHIP thread.
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
 * @param[in] onComplete A pointer to a callback function to be called
 *                       when this timer fires.
 *
 * @param[in] appState A pointer to an application state object to be
 *                     passed to the callback function as argument.
 *
 * @retval CHIP_ERROR_INCORRECT_STATE If the SystemLayer has
 *                      not been initialized.
 *
 * @retval CHIP_ERROR_NO_MEMORY If the SystemLayer cannot
 *                      allocate a new timer.
 *
 * @retval CHIP_NO_ERROR On success.
 */

/*
 * Other contents depend on the contract between event implementation and platform layer implementation.
 * For POSIX-like platforms, WatchableEventManager provides a set of functions called from the event loop:
 *
 * - void EventLoopBegins()  -- Called before the first iterations of the event loop.
 * - void PrepareEvents()    -- Called at the start of each iteration of the event loop.
 * - void WaitForEvents()    -- Called on each iteration of the event loop, between PrepareEvents() and HandleEvents().
 *                              Uniquely, this method gets called with the CHIP stack NOT locked, so it can block.
 *                              For example, the select()-based implementation calls select() here.
 * - void HandleEvents()     -- Called at the end of each iteration of the event loop.
 * - void EventLoopEnds()    -- Called after the last iteration of the event loop.
 */

} // namespace System
} // namespace chip

#define INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE 1
#ifdef CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#include CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#else // CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#include <system/WatchableEventManagerSelect.h>
#endif // CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#undef INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
