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
 *      This file declares an implementation of LayerImplLwIP using LwIP.
 */

#pragma once

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>

namespace chip {
namespace System {

class LayerImplLwIP : public LayerLwIP
{
public:
    LayerImplLwIP();
    ~LayerImplLwIP() { mLayerState.Destroy(); }

    // Layer overrides.
    CHIP_ERROR Init() override;
    CHIP_ERROR Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

    // LayerLwIP overrides.
    CHIP_ERROR AddEventHandlerDelegate(EventHandlerDelegate & aDelegate);
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

    static EventHandlerDelegate sSystemEventHandlerDelegate;

    Timer::MutexedList mTimerList;
    bool mHandlingTimerComplete; // true while handling any timer completion
    const EventHandlerDelegate * mEventDelegateList;
    ObjectLifeCycle mLayerState;
};

using LayerImpl = LayerImplLwIP;

} // namespace System
} // namespace chip
