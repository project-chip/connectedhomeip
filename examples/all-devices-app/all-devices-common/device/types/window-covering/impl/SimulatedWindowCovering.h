/*
 *
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

#include <device/types/window-covering/WindowCovering.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip {
namespace app {

class SimulatedWindowCovering : public WindowCovering,
                                public Clusters::IdentifyDelegate,
                                public Clusters::WindowCovering::WindowCoveringDelegate,
                                public TimerContext
{
public:
    explicit SimulatedWindowCovering(const Context & context);
    ~SimulatedWindowCovering() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // IdentifyDelegate implementation
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

    // WindowCoveringDelegate implementation
    CHIP_ERROR HandleMovement(Clusters::WindowCovering::WindowCoveringType type) override;
    CHIP_ERROR HandleStopMotion() override;
    void OnTargetPositionLiftChanged(DataModel::Nullable<Percent100ths> newTargetLift) override;
    void OnTargetPositionTiltChanged(DataModel::Nullable<Percent100ths> newTargetTilt) override;

    // TimerContext
    void TimerFired() override;

private:
    bool mMovingLift = false;
    bool mMovingTilt = false;
};

} // namespace app
} // namespace chip
