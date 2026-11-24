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

#include "SimulatedClosurePanel.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

SimulatedClosurePanel::SimulatedClosurePanel(Config config, TimerDelegate & delegate) :
    ClosurePanel(*this, config), mTimerDelegate(delegate)
{}

SimulatedClosurePanel::~SimulatedClosurePanel()
{
    mTimerDelegate.CancelTimer(this);
}

Protocols::InteractionModel::Status SimulatedClosurePanel::HandleSetTarget(const Optional<Percent100ths> & position,
                                                                           const Optional<bool> & latch,
                                                                           const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(DeviceLayer, "SimulatedClosurePanel::HandleSetTarget() -> position=%u latch=%d speed=%u", position.ValueOr(0),
                    latch.ValueOr(false), to_underlying(speed.ValueOr(Globals::ThreeLevelAutoEnum::kAuto)));

    mTimerDelegate.CancelTimer(this);
    VerifyOrReturnValue(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kMotionDurationSec)).Handle([](CHIP_ERROR err) {
        ChipLogError(DeviceLayer, "SimulatedClosurePanel: failed to start move timer: %" CHIP_ERROR_FORMAT, err.Format());
    }),
                        Protocols::InteractionModel::Status::Failure);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedClosurePanel::HandleStep(const ClosureDimension::StepDirectionEnum & direction,
                                                                      const uint16_t & numberOfSteps,
                                                                      const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(DeviceLayer, "SimulatedClosurePanel::HandleStep() -> direction=%u numberOfSteps=%u speed=%u",
                    to_underlying(direction), numberOfSteps, to_underlying(speed.ValueOr(Globals::ThreeLevelAutoEnum::kAuto)));
    mTimerDelegate.CancelTimer(this);
    VerifyOrReturnValue(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kMotionDurationSec)).Handle([](CHIP_ERROR err) {
        ChipLogError(DeviceLayer, "SimulatedClosurePanel: failed to start step timer: %" CHIP_ERROR_FORMAT, err.Format());
    }),
                        Protocols::InteractionModel::Status::Failure);

    return Protocols::InteractionModel::Status::Success;
}

void SimulatedClosurePanel::TimerFired()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosurePanel::TimerFired()");
    auto target = ClosureDimensionCluster().GetTargetState();
    VerifyOrReturn(!target.IsNull());

    auto current = ClosureDimensionCluster().GetCurrentState();
    auto next    = current.IsNull() ? Clusters::ClosureDimension::GenericDimensionStateStruct{} : current.Value();
    if (target.Value().position.HasValue())
    {
        next.position = target.Value().position;
    }
    if (target.Value().latch.HasValue())
    {
        next.latch = target.Value().latch;
    }
    if (target.Value().speed.HasValue())
    {
        next.speed = target.Value().speed;
    }
    LogErrorOnFailure(ClosureDimensionCluster().SetCurrentState(DataModel::MakeNullable(next)));
}

} // namespace chip::app
