/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <device/types/closure-panel/ClosurePanel.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/// One panel of a LoggingClosure, behaving the same way: it reaches its commanded position, latch
/// and speed after a fixed delay, without passing through intermediate positions. A Step lands on
/// the end position in one go rather than travelling step by step. A new command supersedes one
/// still in flight, and the owning closure abandons the movement when it handles Stop.
class LoggingClosurePanel : public Clusters::ClosureDimension::ClosureDimensionClusterDelegate,
                            public ClosurePanel,
                            public TimerContext
{
public:
    explicit LoggingClosurePanel(Config config, TimerDelegate & delegate);
    ~LoggingClosurePanel() override;

    Protocols::InteractionModel::Status HandleSetTarget(const Optional<Percent100ths> & position, const Optional<bool> & latch,
                                                        const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed) override;
    Protocols::InteractionModel::Status HandleStep(const Clusters::ClosureDimension::StepDirectionEnum & direction,
                                                   const uint16_t & numberOfSteps,
                                                   const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed) override;
    /// Applies the target the cluster committed. ClosureDimensionCluster stores TargetState only
    /// after the delegate returns Success, so the target cannot be read from the handlers above.
    void TimerFired() override;
    void CancelTimer() { mTimerDelegate.CancelTimer(this); }

private:
    /// Stands in for how long the panel would take to reach its target. Kept short so tests do not
    /// have to wait on it.
    static constexpr uint32_t kMotionDurationSec = 1;
    TimerDelegate & mTimerDelegate;
};

} // namespace chip::app
