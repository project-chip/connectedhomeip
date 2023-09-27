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
 *      This file declares an implementation of LayerImplRTThread using RT-Thread.
 */

#pragma once

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

namespace chip {
namespace System {

class LayerImplRTThread : public LayerRTThread
{
public:
    LayerImplRTThread();
    ~LayerImplRTThread() { VerifyOrDie(mLayerState.Destroy()); }    

    // Layer overrides.
    CHIP_ERROR Init() override;
    void Shutdown() override;
    bool IsInitialized() const override;
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

    // Platform-specific function for handling timers.
    CHIP_ERROR HandlePlatformTimer(void);

private:
    friend class PlatformEventing;
    
    CHIP_ERROR StartPlatformTimer(System::Clock::Timeout aDelay);

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    bool mHandlingTimerComplete; // true while handling any timer completion
    ObjectLifeCycle mLayerState;
};

using LayerImpl = LayerImplRTThread;

} // namespace System
} // namespace chip