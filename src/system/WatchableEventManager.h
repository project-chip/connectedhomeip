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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {

namespace System {

class Layer;

/**
 * @class WatchableEventManager
 *
 * An instance of this type is contained in System::Layer. Its purpose is to hold socket-event system state
 * or methods available to every associated instance of WatchableSocket.
 *
 * It MUST provide at least three methods:
 *
 * - CHIP_ERROR Init(System::Layer & systemLayer) -- called from System::Layer::Init()
 * - CHIP_ERROR Shutdown()                        -- called from System::Layer::Shutdown()
 * - CHIP_ERROR Signal()                          -- called to indicate that event monitoring may need to be refreshed or resumed.
 *
 * Other contents depend on the contract between socket-event implementation and platform layer implementation.
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
class WatchableEventManager;

} // namespace System
} // namespace chip

#define INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE 1
#ifdef CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#include CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#else // CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#include <system/WatchableEventManagerSelect.h>
#endif // CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#undef INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE

#endif // CHIP_SYSTEM_CONFIG_USE_EVENT_MANAGERS
