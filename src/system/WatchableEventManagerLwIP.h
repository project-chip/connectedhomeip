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
 *      This file declares an implementation of WatchableEventManager using LwIP.
 */

#pragma once

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#error "This file should only be included from <system/WatchableEventManager.h>"
#include <system/WatchableEventManager.h>
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE

namespace chip {
namespace System {

class LwIPEventHandlerDelegate
{
    friend class WatchableEventManager;

public:
    typedef CHIP_ERROR (*LwIPEventHandlerFunction)(Object & aTarget, EventType aEventType, uintptr_t aArgument);

    bool IsInitialized(void) const;
    void Init(LwIPEventHandlerFunction aFunction);
    void Prepend(const LwIPEventHandlerDelegate *& aDelegateList);

private:
    LwIPEventHandlerFunction mFunction;
    const LwIPEventHandlerDelegate * mNextDelegate;
};

class WatchableEventManager
{
private:
    // Transitionally, ensure that these ‘overrides’ can only be called via the System::Layer equivalents.
    friend class Layer;

    // Core ‘overrides’.
    WatchableEventManager();
    CHIP_ERROR Init(System::Layer & systemLayer);
    CHIP_ERROR Shutdown();

    // Timer ‘overrides’.
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState);
    void CancelTimer(TimerCompleteCallback onComplete, void * appState);
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState);

    // LwIP-specific ‘overrides’.
    CHIP_ERROR AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate);
    CHIP_ERROR PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);

public:
    // Platform implementation.
    CHIP_ERROR DispatchEvents(void); // XXX called only in a test → PlatformEventing::DispatchEvents → PlatformMgr().RunEventLoop()
    CHIP_ERROR HandleEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);
    CHIP_ERROR HandlePlatformTimer(void);

private:
    friend class PlatformEventing;

    static CHIP_ERROR HandleSystemLayerEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);

    CHIP_ERROR DispatchEvent(Event aEvent);
    CHIP_ERROR StartPlatformTimer(uint32_t aDelayMilliseconds);

    static LwIPEventHandlerDelegate sSystemEventHandlerDelegate;

    Layer * mSystemLayer = nullptr;
    Timer::MutexedList mTimerList;
    bool mHandlingTimerComplete; // true while handling any timer completion
    const LwIPEventHandlerDelegate * mEventDelegateList;
};

} // namespace System
} // namespace chip
