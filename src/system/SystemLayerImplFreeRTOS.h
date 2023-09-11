/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file declares an implementation of LayerImplFreeRTOS using LwIP.
 */

#pragma once

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

namespace chip {
namespace System {

class LayerImplFreeRTOS : public LayerFreeRTOS
{
public:
    LayerImplFreeRTOS();
    ~LayerImplFreeRTOS() { VerifyOrDie(mLayerState.Destroy()); }

    // Layer overrides.
    CHIP_ERROR Init() override;
    void Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

public:
    // Platform implementation.
    CHIP_ERROR HandlePlatformTimer(void);

private:
    friend class PlatformEventing;

    CHIP_ERROR StartPlatformTimer(System::Clock::Timeout aDelay);

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    bool mHandlingTimerComplete; // true while handling any timer completion
    ObjectLifeCycle mLayerState;
};

using LayerImpl = LayerImplFreeRTOS;

} // namespace System
} // namespace chip
