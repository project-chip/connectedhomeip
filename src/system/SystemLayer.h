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
 *      This file contains declarations of the
 *      chip::System::Layer class and its related types, data and
 *      functions.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

#include <core/CHIPCallback.h>

#include <support/DLLUtil.h>
#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemEvent.h>
#include <system/SystemObject.h>
#include <system/SystemTimer.h>
#include <system/WatchableEventManager.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

namespace chip {
namespace System {

class Layer;
class Timer;
class Object;

class PlatformEventing
{
public:
    static CHIP_ERROR PostEvent(System::Layer & aLayer, Object & aTarget, EventType aType, uintptr_t aArgument);
    static CHIP_ERROR DispatchEvents(System::Layer & aLayer);
    static CHIP_ERROR DispatchEvent(System::Layer & aLayer, Event aEvent);
    static CHIP_ERROR StartTimer(System::Layer & aLayer, uint32_t aMilliseconds);
};

/**
 *  @enum LayerState
 *
 *  The state of a Layer object.
 */
enum LayerState
{
    kLayerState_NotInitialized = 0, /**< Not initialized state. */
    kLayerState_Initialized    = 1  /**< Initialized state. */
};

/**
 * This provides access to timers according to the configured event handling model.
 */
class DLL_EXPORT Layer
{
public:
    Layer();

    CHIP_ERROR Init();

    // Some other layers hold pointers to System::Layer, so care must be taken
    // to ensure that they are not used after calling Shutdown().
    CHIP_ERROR Shutdown();

    LayerState State() const { return mLayerState; }

    CHIP_ERROR StartTimer(uint32_t aMilliseconds, Timers::OnCompleteFunct aComplete, void * aAppState);
    void CancelTimer(Timers::OnCompleteFunct aOnComplete, void * aAppState);
    CHIP_ERROR ScheduleWork(Timers::OnCompleteFunct aComplete, void * aAppState);
    WatchableEventManager & WatchableEventsManager() { return mWatchableEventsManager; }

    Clock & GetClock() { return mClock; }

private:
    LayerState mLayerState;
    WatchableEventManager mWatchableEventsManager;
    Clock mClock;

    // Copy and assignment NOT DEFINED
    Layer(const Layer &) = delete;
    Layer & operator=(const Layer &) = delete;
};

} // namespace System
} // namespace chip
