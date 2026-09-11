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
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::WindowCovering;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Milliseconds32 kTransitionInterval  = System::Clock::Milliseconds32(500);
constexpr System::Clock::Milliseconds32 kCalibrationDuration = System::Clock::Milliseconds32(30000);
constexpr Percent100ths kPositionStep                        = 500; // 5% step

// Returns the next position, moved at most kPositionStep towards target.
Percent100ths ComputeStepToTarget(Percent100ths current, Percent100ths target)
{
    if (current < target)
    {
        return static_cast<Percent100ths>((target - current > kPositionStep) ? (current + kPositionStep) : target);
    }
    if (current > target)
    {
        return static_cast<Percent100ths>((current - target > kPositionStep) ? (current - kPositionStep) : target);
    }
    return current;
}

} // namespace

SimulatedWindowCovering::SimulatedWindowCovering(const Context & context) :
    WindowCovering(context, Delegates{ .windowCovering = *this, .identify = *this },
                   Config{ .features = BitFlags<Clusters::WindowCovering::Feature>(
                               Clusters::WindowCovering::Feature::kLift, Clusters::WindowCovering::Feature::kPositionAwareLift,
                               Clusters::WindowCovering::Feature::kTilt, Clusters::WindowCovering::Feature::kPositionAwareTilt),
                           .optionalAttributes = []() {
                               Clusters::WindowCovering::OptionalAttributeSet optionals;
                               optionals.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
                                   .Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
                                   .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
                                   .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
                                   .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
                               return optionals;
                           }() })
{}

SimulatedWindowCovering::~SimulatedWindowCovering()
{
    mContext.timerDelegate.CancelTimer(this);
}

void SimulatedWindowCovering::Unregister(CodeDrivenDataModelProvider & provider)
{
    mContext.timerDelegate.CancelTimer(this);
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

    if (cluster.GetMode().Has(Mode::kCalibrationMode))
    {
        // Per spec 5.3.6.14.2, a movement command while in calibration mode triggers a
        // self-calibration before the command executes. Defer this move until the fake
        // calibration timer (started below) completes in TimerFired().
        if (!mCalibrating)
        {
            mCalibrating = true;
            cluster.SetCurrentPositionLiftPercent100ths(DataModel::Nullable<Percent100ths>());
            cluster.SetCurrentPositionTiltPercent100ths(DataModel::Nullable<Percent100ths>());
            mContext.timerDelegate.CancelTimer(this);
            ReturnErrorOnFailure(mContext.timerDelegate.StartTimer(this, kCalibrationDuration));
        }
        return CHIP_NO_ERROR;
    }

    if (type == Clusters::WindowCovering::WindowCoveringType::Lift)
    {
        auto target  = cluster.GetTargetPositionLiftPercent100ths();
        auto current = cluster.GetCurrentPositionLiftPercent100ths();
        if (!target.IsNull() && !current.IsNull() && target != current)
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
        mContext.timerDelegate.CancelTimer(this);
        ReturnErrorOnFailure(mContext.timerDelegate.StartTimer(this, kTransitionInterval));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedWindowCovering::HandleStopMotion()
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleStopMotion");

    mContext.timerDelegate.CancelTimer(this);
    mMovingLift = false;
    mMovingTilt = false;

    // Abort an in-progress fake calibration. Per spec 5.3.6.14.2, the only way to leave
    // calibration mode is to complete the calibration routine, so we leave Mode's
    // CalibrationMode bit set and positions null; the next movement command retries calibration.
    mCalibrating = false;

    auto & cluster = WindowCoveringCluster();

    // No need to freeze targets here: WindowCoveringCluster::HandleStopMotion() already sets
    // target = current for us right after this returns CHIP_NO_ERROR.

    auto currentLift               = cluster.GetCurrentPositionLiftPercent100ths();
    auto currentTilt               = cluster.GetCurrentPositionTiltPercent100ths();
    [[maybe_unused]] auto opStatus = cluster.GetOperationalStatus();

    // Longest content is "65535\0" (max uint16_t), so 6 bytes covers either that or "NULL\0".
    StringBuilder<6> liftStr;
    currentLift.IsNull() ? liftStr.Add("NULL") : liftStr.AddFormat("%u", currentLift.Value());
    StringBuilder<6> tiltStr;
    currentTilt.IsNull() ? tiltStr.Add("NULL") : tiltStr.AddFormat("%u", currentTilt.Value());

    ChipLogProgress(DeviceLayer, "WindowCovering: Halted. Frozen State -> Lift: %s, Tilt: %s | OpStatus raw=0x%02X",
                    liftStr.c_str(), tiltStr.c_str(), opStatus.Raw());

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

    if (mCalibrating)
    {
        // Fake calibration finished: resolve to a known position, per spec 5.3.6.14.2 leave
        // calibration mode by completing the routine, then run the deferred move for each axis.
        mCalibrating = false;
        cluster.SetCurrentPositionLiftPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));
        cluster.SetCurrentPositionTiltPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));

        auto mode = cluster.GetMode();
        mode.Clear(Mode::kCalibrationMode);
        cluster.SetMode(mode);

        LogErrorOnFailure(HandleMovement(Clusters::WindowCovering::WindowCoveringType::Lift));
        LogErrorOnFailure(HandleMovement(Clusters::WindowCovering::WindowCoveringType::Tilt));
        return;
    }

    if (mMovingLift)
    {
        Percent100ths currentVal = cluster.GetCurrentPositionLiftPercent100ths().ValueOr(0);
        Percent100ths targetVal  = cluster.GetTargetPositionLiftPercent100ths().ValueOr(currentVal);
        Percent100ths nextVal    = ComputeStepToTarget(currentVal, targetVal);

        cluster.SetCurrentPositionLiftPercent100ths(DataModel::Nullable<Percent100ths>(nextVal));
        [[maybe_unused]] BitMask<OperationalStatus> opStatus = cluster.GetOperationalStatus();
        ChipLogProgress(DeviceLayer,
                        "WindowCovering: Simulating Lift -> %" PRIu16 " / Target %" PRIu16
                        " | OpStatus raw=0x%02X (global=%u, lift=%u)",
                        nextVal, targetVal, opStatus.Raw(), opStatus.GetField(OperationalStatus::kGlobal),
                        opStatus.GetField(OperationalStatus::kLift));

        mMovingLift = (nextVal != targetVal);
    }

    if (mMovingTilt)
    {
        Percent100ths currentVal = cluster.GetCurrentPositionTiltPercent100ths().ValueOr(0);
        Percent100ths targetVal  = cluster.GetTargetPositionTiltPercent100ths().ValueOr(currentVal);
        Percent100ths nextVal    = ComputeStepToTarget(currentVal, targetVal);

        cluster.SetCurrentPositionTiltPercent100ths(DataModel::Nullable<Percent100ths>(nextVal));
        [[maybe_unused]] BitMask<OperationalStatus> opStatus = cluster.GetOperationalStatus();
        ChipLogProgress(DeviceLayer,
                        "WindowCovering: Simulating Tilt -> %" PRIu16 " / Target %" PRIu16
                        " | OpStatus raw=0x%02X (global=%u, tilt=%u)",
                        nextVal, targetVal, opStatus.Raw(), opStatus.GetField(OperationalStatus::kGlobal),
                        opStatus.GetField(OperationalStatus::kTilt));

        mMovingTilt = (nextVal != targetVal);
    }

    if (mMovingLift || mMovingTilt)
    {
        LogErrorOnFailure(mContext.timerDelegate.StartTimer(this, kTransitionInterval));
    }
}

CHIP_ERROR SimulatedWindowCovering::RegisterOptionalClusters(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    // Groups is optional (Active, O) per the Window Covering device type, but we implement it
    // here since it lets the device respond to groupcast commands like other simulated devices.
    mGroupsCluster.Create(endpoint,
                          Clusters::GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = nullptr, // Window Covering does not implement Scenes
                              .identifyIntegration = &IdentifyCluster(),
                          });
    return provider.AddCluster(mGroupsCluster.Registration());
}

void SimulatedWindowCovering::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
