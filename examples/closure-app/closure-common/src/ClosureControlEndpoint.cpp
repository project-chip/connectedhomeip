/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <ClosureControlEndpoint.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <protocols/interaction_model/StatusCode.h>

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <platform/CHIPDeviceLayer.h>

namespace {
    constexpr uint32_t kExampleCalibrateCountDown     = 10;
    constexpr uint32_t kExampleMotionCountDown        = 15;
    constexpr uint32_t kExampleWaitforMotionCountDown = 15;
} // namespace

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

namespace {
constexpr ElapsedS kDefaultCountdownTime = 30;
} // namespace

PositioningEnum ClosureControlDelegate::GetStatePositionFromTarget(TargetPositionEnum targetPosition)
{
    switch (targetPosition)
    {
        case TargetPositionEnum::kCloseInFull:
            return PositioningEnum::kFullyClosed;

        case TargetPositionEnum::kOpenInFull:
            return PositioningEnum::kFullyOpened;

        case TargetPositionEnum::kPedestrian:
            return PositioningEnum::kOpenedForPedestrian;

        case TargetPositionEnum::kVentilation:
            return PositioningEnum::kOpenedForVentilation;

        case TargetPositionEnum::kSignature:
            return PositioningEnum::kOpenedAtSignature;

        default:
            return PositioningEnum::kUnknownEnumValue;
    }
}

DataModel::Nullable<ElapsedS> ClosureControlDelegate::GetRemainingTime()
{

    if (mCountDownTime.IsNull())
        return DataModel::NullNullable;

    return DataModel::MakeNullable((mCountDownTime.Value() - static_cast<ElapsedS>(mMovingTime + mWaitingTime + mCalibratingTime)));
}

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    ClosureControlDelegate * delegate = reinterpret_cast<ClosureControlDelegate *>(data);

    MainStateEnum state;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    delegate->GetLogic()->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    DataModel::Nullable<ElapsedS> countdown_time = delegate->GetRemainingTime();

    // if (countdown_time.IsNull() || (!countdown_time.IsNull() && countdown_time.Value() > 0))
    if (countdown_time.IsNull() || countdown_time.Value() > 0)
    {
        if (state == MainStateEnum::kMoving)
        {
            delegate->mMovingTime++;
        }
        if (state == MainStateEnum::kCalibrating)
        {
            delegate->mCalibratingTime++;
        }
        if (state == MainStateEnum::kWaitingForMotion)
        {
            delegate->mWaitingTime++;
            if (delegate->IsPreStageComplete())
            {
                delegate->HandleMotion();
            }
        }
    }
    else if (!countdown_time.IsNull() && countdown_time.Value() <= 0)
    {
        delegate->HandleCountdownTimeExpired();
    }

    if (state == MainStateEnum::kMoving || state == MainStateEnum::kCalibrating || state == MainStateEnum::kWaitingForMotion)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, delegate);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    else
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}

void ClosureControlDelegate::HandleCountdownTimeExpired()
{
    ClusterLogic * logic = GetLogic();
    MainStateEnum state;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mCountDownTime.SetNonNull(0);
    mCalibratingTime = 0;
    mWaitingTime = 0;
    mMovingTime = 0;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    logic->SetCountdownTimeFromDelegate(mCountDownTime);
    logic->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (state == MainStateEnum::kWaitingForMotion &&  ClosureControlDelegate::IsPreStageComplete())
    {
            logic->SetMainState(MainStateEnum::kMoving);
            HandleMotion();
    }

    if (state == MainStateEnum::kMoving)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        // Update OverallState object
        DataModel::Nullable<GenericOverallTarget> target;
        GetLogic()->GetOverallTarget(target);
        GenericOverallState current {};
        // Copy position if present
        if (target.Value().position.HasValue())
        {
            current.positioning.SetValue(DataModel::MakeNullable(GetStatePositionFromTarget(target.Value().position.Value())));
        }
        // Copy latch if present
        if (target.Value().latch.HasValue())
        {
            current.latch.SetValue(DataModel::MakeNullable(target.Value().latch.Value()));
        }
        // Copy speed if present
        if (target.Value().speed.HasValue())
        {
            current.speed.SetValue(DataModel::MakeNullable(target.Value().speed.Value()));
        }
        GetLogic()->SetOverallState(DataModel::MakeNullable(current));
        logic->GenerateMovementCompletedEvent();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    logic->SetMainState(MainStateEnum::kStopped);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

Status ClosureControlDelegate::HandleCalibrateCommand()
{
    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleCalibrateCountDown));

    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);

    //Trigger Calibrate Action.

    return Status::Success;
}

Status ClosureControlDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & tag, const Optional<bool> & latch,
                                              const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    MainStateEnum state;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    GetLogic()->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    VerifyOrReturnError(state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion, Status::Failure);

    if (state == MainStateEnum::kMoving)
    {
        return HandleMotion();
    }

    else if (state == MainStateEnum::kWaitingForMotion)
    {
        mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleWaitforMotionCountDown));
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
        return Status::Success;
    }

    return Status::Success;
}

Status ClosureControlDelegate::HandleStopCommand()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mCalibratingTime = 0;
    mMovingTime = 0;
    mWaitingTime = 0;

    mCountDownTime.SetNonNull(0);
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    GetLogic()->SetCountdownTimeFromDelegate(mCountDownTime);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    //Trigger Stop Action.

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    VerifyOrReturnError(GetLogic()->GenerateMovementCompletedEvent() == CHIP_NO_ERROR, Status::Failure);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlDelegate::HandleMotion()
{
    // Cancel timer if any motion is in progress
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);

    mWaitingTime = 0;
    mMovingTime = 0;

    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleMotionCountDown));

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    //Trigger Motion Action
    return Status::Success;
}

CHIP_ERROR ClosureControlDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    // TODO: Add  error list handling
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

bool ClosureControlDelegate::IsManualLatchingNeeded()
{
     // Check if closure needs manual latching.(manufacture specific)
    return false;
}

bool ClosureControlDelegate::IsReadyToMove()
{
    // Check if closure needs some action before movement.(manufacture specific)
    return true;
}

bool ClosureControlDelegate::IsPreStageComplete()
{
    // Check if closure has completed the PreStage required before the motion
    return true;
}

ElapsedS ClosureControlDelegate::GetCalibrationCountdownTime()
{
    // Return the CountdownTime needed for closure calibration
    return static_cast<ElapsedS>(kExampleCalibrateCountDown);
}

ElapsedS ClosureControlDelegate::GetMovingCountdownTime()
{
    // Return the CountdownTime needed for closure motion
    return static_cast<ElapsedS>(kExampleMotionCountDown);
}

ElapsedS ClosureControlDelegate::GetWaitingForMotionCountdownTime()
{
    // Return the CountdownTime needed for closure to complete prestages before motion.
    return static_cast<ElapsedS>(kExampleWaitforMotionCountDown);
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed).Set(Feature::kVentilation)
                            .Set(Feature::kPedestrian).Set(Feature::kCalibration).Set(Feature::kProtection).Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);

    ClusterInitParameters clusterInitParameters;
    clusterInitParameters.mMainState = MainStateEnum::kStopped;
    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ReturnErrorOnFailure(mInterface.Init());

    return CHIP_NO_ERROR;
 }
