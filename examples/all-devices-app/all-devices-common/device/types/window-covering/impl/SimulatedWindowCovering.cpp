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
#include <device/types/window-covering/impl/SimulatedWindowCovering.h>
#include <inttypes.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::WindowCovering;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Milliseconds32 kTransitionInterval = System::Clock::Milliseconds32(500);
constexpr Percent100ths kPositionStep                       = 500; // 5% step

} // namespace

SimulatedWindowCovering::SimulatedWindowCovering(const Context & context) :
    WindowCovering(*this, *this, context, []() {
        Clusters::WindowCovering::OptionalAttributeSet optionals;
        optionals.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
            .Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
            .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
            .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
            .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
        return optionals;
    }())
{}

SimulatedWindowCovering::~SimulatedWindowCovering()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedWindowCovering::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                             EndpointComposition composition)
{
    ReturnErrorOnFailure(WindowCovering::Register(endpoint, provider, composition));

    auto & cluster = WindowCoveringCluster();
    if (cluster.GetCurrentPositionLiftPercent100ths().IsNull())
    {
        cluster.SetCurrentPositionLiftPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));
    }
    if (cluster.GetCurrentPositionTiltPercent100ths().IsNull())
    {
        cluster.SetCurrentPositionTiltPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));
    }

    return CHIP_NO_ERROR;
}

void SimulatedWindowCovering::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    WindowCovering::Unregister(provider);
}

void SimulatedWindowCovering::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnIdentifyStart");
}

void SimulatedWindowCovering::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnIdentifyStop");
}

void SimulatedWindowCovering::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnTriggerEffect");
}

CHIP_ERROR SimulatedWindowCovering::HandleMovement(Clusters::WindowCovering::WindowCoveringType type)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleMovement type=%" PRIu16, static_cast<uint16_t>(type));
    auto & cluster = WindowCoveringCluster();

    if (type == Clusters::WindowCovering::WindowCoveringType::Lift)
    {
        auto target  = cluster.GetTargetPositionLiftPercent100ths();
        auto current = cluster.GetCurrentPositionLiftPercent100ths();
        if (!target.IsNull() && !current.IsNull() && target.Value() != current.Value())
        {
            mMovingLift = true;
        }
    }
    else if (type == Clusters::WindowCovering::WindowCoveringType::Tilt)
    {
        auto target  = cluster.GetTargetPositionTiltPercent100ths();
        auto current = cluster.GetCurrentPositionTiltPercent100ths();
        if (!target.IsNull() && !current.IsNull() && target.Value() != current.Value())
        {
            mMovingTilt = true;
        }
    }

    if (mMovingLift || mMovingTilt)
    {
        ReturnErrorOnFailure(mTimerDelegate.StartTimer(this, kTransitionInterval));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWindowCovering::HandleStopMotion()
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleStopMotion");

    mTimerDelegate.CancelTimer(this);
    mMovingLift = false;
    mMovingTilt = false;

    auto & cluster = WindowCoveringCluster();

    // Freeze the covering exactly where it is by updating targets to match current values
    cluster.SetTargetPositionLiftPercent100ths(cluster.GetCurrentPositionLiftPercent100ths());
    cluster.SetTargetPositionTiltPercent100ths(cluster.GetCurrentPositionTiltPercent100ths());

    [[maybe_unused]] auto currentLift = cluster.GetCurrentPositionLiftPercent100ths();
    [[maybe_unused]] auto currentTilt = cluster.GetCurrentPositionTiltPercent100ths();
    [[maybe_unused]] auto opStatus    = cluster.GetOperationalStatus();

    ChipLogProgress(DeviceLayer,
                    "WindowCovering: Halted. Frozen State -> Lift: %" PRIu16 ", Tilt: %" PRIu16 " | OpStatus raw=0x%02X",
                    currentLift.IsNull() ? static_cast<uint16_t>(0) : currentLift.Value(),
                    currentTilt.IsNull() ? static_cast<uint16_t>(0) : currentTilt.Value(), opStatus.Raw());

    return CHIP_NO_ERROR;
}

void SimulatedWindowCovering::OnTargetPositionLiftChanged(DataModel::Nullable<Percent100ths> newTargetLift)
{
    if (newTargetLift.IsNull())
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionLiftChanged -> NULL");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionLiftChanged -> %" PRIu16, newTargetLift.Value());
    }
}

void SimulatedWindowCovering::OnTargetPositionTiltChanged(DataModel::Nullable<Percent100ths> newTargetTilt)
{
    if (newTargetTilt.IsNull())
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionTiltChanged -> NULL");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionTiltChanged -> %" PRIu16, newTargetTilt.Value());
    }
}

void SimulatedWindowCovering::TimerFired()
{
    auto & cluster = WindowCoveringCluster();

    if (mMovingLift)
    {
        auto targetVal  = cluster.GetTargetPositionLiftPercent100ths().Value();
        auto currentVal = cluster.GetCurrentPositionLiftPercent100ths().Value();

        if (currentVal < targetVal)
        {
            currentVal = static_cast<Percent100ths>(currentVal + kPositionStep);
            if (currentVal > targetVal)
                currentVal = targetVal;
        }
        else if (currentVal > targetVal)
        {
            currentVal = static_cast<Percent100ths>(currentVal - kPositionStep);
            if (currentVal < targetVal)
                currentVal = targetVal;
        }

        cluster.SetCurrentPositionLiftPercent100ths(DataModel::Nullable<Percent100ths>(currentVal));
        [[maybe_unused]] auto opStatus = cluster.GetOperationalStatus();
        ChipLogProgress(DeviceLayer,
                        "WindowCovering: Simulating Lift -> %" PRIu16 " / Target %" PRIu16
                        " | OpStatus raw=0x%02X (global=%u, lift=%u)",
                        currentVal, targetVal, opStatus.Raw(), opStatus.GetField(OperationalStatus::kGlobal),
                        opStatus.GetField(OperationalStatus::kLift));

        if (currentVal == targetVal)
        {
            mMovingLift = false;
        }
    }

    if (mMovingTilt)
    {
        auto targetVal  = cluster.GetTargetPositionTiltPercent100ths().Value();
        auto currentVal = cluster.GetCurrentPositionTiltPercent100ths().Value();

        if (currentVal < targetVal)
        {
            currentVal = static_cast<Percent100ths>(currentVal + kPositionStep);
            if (currentVal > targetVal)
                currentVal = targetVal;
        }
        else if (currentVal > targetVal)
        {
            currentVal = static_cast<Percent100ths>(currentVal - kPositionStep);
            if (currentVal < targetVal)
                currentVal = targetVal;
        }

        cluster.SetCurrentPositionTiltPercent100ths(DataModel::Nullable<Percent100ths>(currentVal));
        [[maybe_unused]] auto opStatus = cluster.GetOperationalStatus();
        ChipLogProgress(DeviceLayer,
                        "WindowCovering: Simulating Tilt -> %" PRIu16 " / Target %" PRIu16
                        " | OpStatus raw=0x%02X (global=%u, tilt=%u)",
                        currentVal, targetVal, opStatus.Raw(), opStatus.GetField(OperationalStatus::kGlobal),
                        opStatus.GetField(OperationalStatus::kTilt));

        if (currentVal == targetVal)
        {
            mMovingTilt = false;
        }
    }

    if (mMovingLift || mMovingTilt)
    {
        LogErrorOnFailure(mTimerDelegate.StartTimer(this, kTransitionInterval));
    }
}

} // namespace app
} // namespace chip
