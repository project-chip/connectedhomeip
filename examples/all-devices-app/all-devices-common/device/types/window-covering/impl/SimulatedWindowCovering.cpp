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

constexpr System::Clock::Milliseconds32 kTransitionInterval  = System::Clock::Milliseconds32(500);
constexpr System::Clock::Milliseconds32 kCalibrationDuration = System::Clock::Milliseconds32(3000);
// Delay before deciding whether the device still needs its initial calibration; see Register().
constexpr System::Clock::Milliseconds32 kInitialCalibrationCheckDelay = System::Clock::Milliseconds32(250);
constexpr Percent100ths kPositionStep                                 = 500; // 5% step

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
                           .optionalAttributes =
                               []() {
                                   Clusters::WindowCovering::OptionalAttributeSet optionals;
                                   optionals.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
                                       .Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
                                       .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
                                       .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
                                       .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
                                   return optionals;
                               }(),
                           .type           = Clusters::WindowCovering::Type::kTiltBlindLiftAndTilt,
                           .endProductType = Clusters::WindowCovering::EndProductType::kSheerShade })
{}

SimulatedWindowCovering::~SimulatedWindowCovering()
{
    mContext.timerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedWindowCovering::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                             EndpointComposition composition)
{
    ReturnErrorOnFailure(WindowCovering::Register(endpoint, provider, composition));

    // This simulated device starts uncalibrated: both current positions are null (targets stay
    // null until a client commands a movement) until a calibration completes. Device registration
    // happens before the cluster's Startup(), which only then loads any persisted positions from
    // NVS on later boots, so whether an initial calibration is still needed can only be decided
    // after that load - hence the deferred check below rather than a check at register time.
    //
    // If the positions are still unknown once Startup() is done (fresh install, nothing
    // persisted), the device enters calibration mode by itself instead of waiting for a client to
    // request it, and resolves to a known position shortly after startup on its own (see
    // TimerFired()). A client can also enter calibration mode at any time by writing
    // Mode.CalibrationMode. No movement commands are accepted by the cluster while calibrating
    // (GetMotionLockStatus()), and any movement or calibration transition cancels this pending
    // check timer, so the two paths cannot interfere.
    return mContext.timerDelegate.StartTimer(this, kInitialCalibrationCheckDelay);
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

    auto & cluster = WindowCoveringCluster();

    // No need to freeze targets here: WindowCoveringCluster::HandleStopMotion() already sets
    // target = current for us right after this returns CHIP_NO_ERROR.

    auto currentLift               = cluster.GetCurrentPositionLiftPercent100ths();
    auto currentTilt               = cluster.GetCurrentPositionTiltPercent100ths();
    [[maybe_unused]] auto opStatus = cluster.GetOperationalStatus();

    // Positions may be null while the device is uncalibrated (a fresh install before its initial
    // calibration completes), so only log the values when they are known.
    if (currentLift.IsNull() || currentTilt.IsNull())
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: Halted while uncalibrated, positions unknown | OpStatus raw=0x%02X",
                        opStatus.Raw());
    }
    else
    {
        ChipLogProgress(DeviceLayer,
                        "WindowCovering: Halted. Frozen State -> Lift: %" PRIu16 ", Tilt: %" PRIu16 " | OpStatus raw=0x%02X",
                        currentLift.Value(), currentTilt.Value(), opStatus.Raw());
    }

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

void SimulatedWindowCovering::OnModeChanged(chip::BitMask<Mode> newMode)
{
    // Per spec 9.3.6.14.2, calibration mode is entered as soon as the bit is set (movement
    // commands are rejected by WindowCoveringCluster's own motion lock while calibrating, so
    // that can't be the trigger point - see HandleMovement()/GetMotionLockStatus()).
    if (!newMode.Has(Mode::kCalibrationMode) || mCalibrating)
    {
        return;
    }

    ChipLogProgress(DeviceLayer, "WindowCovering: Starting fake calibration (%" PRIu32 " ms)", kCalibrationDuration.count());
    mCalibrating = true;
    // No movement can be in progress while calibrating (motion is locked), so drop any stale
    // moving flags; a client must send a fresh movement command after calibration completes.
    mMovingLift    = false;
    mMovingTilt    = false;
    auto & cluster = WindowCoveringCluster();
    cluster.SetCurrentPositionLiftPercent100ths(DataModel::Nullable<Percent100ths>());
    cluster.SetCurrentPositionTiltPercent100ths(DataModel::Nullable<Percent100ths>());
    mContext.timerDelegate.CancelTimer(this);
    LogErrorOnFailure(mContext.timerDelegate.StartTimer(this, kCalibrationDuration));
}

void SimulatedWindowCovering::TimerFired()
{
    auto & cluster = WindowCoveringCluster();

    // The deferred initial-calibration check from Register(): if there is still no known position
    // once Startup() has loaded persisted state (fresh install), run the initial calibration by
    // entering calibration mode; OnModeChanged() takes it from here. Any client-initiated
    // calibration or movement would have cancelled this timer before it could fire, so this cannot
    // interrupt a transition already in progress.
    if (!mCalibrating && !mMovingLift && !mMovingTilt &&
        (cluster.GetCurrentPositionLiftPercent100ths().IsNull() || cluster.GetCurrentPositionTiltPercent100ths().IsNull()))
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: No known position after startup, starting initial calibration");
        chip::BitMask<Mode> mode = cluster.GetMode();
        mode.Set(Mode::kCalibrationMode);
        cluster.SetMode(mode);
        return;
    }

    if (mCalibrating)
    {
        // Fake calibration finished: resolve to a known position and leave calibration mode by
        // completing the routine, per spec 9.3.6.14.2. No move was ever accepted by the cluster
        // while calibrating (see OnModeChanged()), so there's nothing deferred to resume here -
        // a client must send a fresh movement command now that the device is operational again.
        mCalibrating = false;
        cluster.SetCurrentPositionLiftPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));
        cluster.SetCurrentPositionTiltPercent100ths(
            DataModel::Nullable<Percent100ths>(Clusters::WindowCovering::kWcPercent100thsMinOpen));

        // NOTE: WindowCoveringCluster::WriteAttribute() does not currently reject a client write
        // that clears CalibrationMode directly (spec 9.3.6.14.2 requires FAILURE in that case;
        // see the disabled check there), so a client can bypass this simulated calibration by
        // writing Mode=0 directly instead of waiting for it to complete.
        auto mode = cluster.GetMode();
        mode.Clear(Mode::kCalibrationMode);
        cluster.SetMode(mode);

        ChipLogProgress(DeviceLayer, "WindowCovering: Fake calibration complete, positions resolved");
        return;
    }

    if (mMovingLift)
    {
        // Current positions are always non-null while moving (calibration locks motion), so read
        // them directly; the target is null unless a client commanded a movement, in which case it
        // equals the position we are stepping towards.
        Percent100ths currentVal = cluster.GetCurrentPositionLiftPercent100ths().Value();
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
        Percent100ths currentVal = cluster.GetCurrentPositionTiltPercent100ths().Value();
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
