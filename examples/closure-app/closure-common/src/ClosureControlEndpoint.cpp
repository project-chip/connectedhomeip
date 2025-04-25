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

using namespace chip;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

void PrintOnlyDelegate::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    PrintOnlyDelegate * delegate = reinterpret_cast<PrintOnlyDelegate *>(data);

    MainStateEnum state;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    delegate->GetLogic()->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    auto countdown_time = delegate->GetRemainingTime();

    if (countdown_time.IsNull() || (!countdown_time.IsNull() && countdown_time.Value() > 0))
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

Status PrintOnlyDelegate::HandleCalibrateCommand()
{
    ChipLogProgress(AppServer, "HandleCalibrateCommand");
    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleCalibrateCountDown));

    if (mActionInitiated_CB)
    {
        mActionInitiated_CB(CALIBRATE_ACTION);
    }
    
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    return Status::Success;
}

PositioningEnum PrintOnlyDelegate::GetStatePositionFromTarget(TargetPositionEnum targetPosition)
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

Status PrintOnlyDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & tag, const Optional<bool> & latch,
                                              const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleMoveToCommand");
    VerifyOrReturnValue(tag.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    bool motionNeeded = false;
    bool latchNeeded  = false;

    ClusterLogic * logic = GetLogic();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    MainStateEnum state;
    chip::app::DataModel::Nullable<GenericOverallTarget> overallTarget;
    chip::app::DataModel::Nullable<GenericOverallState> overallState;
    logic->GetMainState(state);
    logic->GetOverallTarget(overallTarget);
    logic->GetOverallState(overallState);
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

    // // If device is already at TargetState ,no Action is required will give Status::Success
    // VerifyOrReturnValue(motionNeeded || latchNeeded, Status::Success);

    // chip::DeviceLayer::PlatformMgr().LockChipStack();
    // logic->SetOverallTarget(overallTarget);
    // chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // if (IsDeviceReadytoMove())
    // {
    //     if (state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion)
    //     {
    //         return HandleMotion();
    //     }
    //     else
    //     {
    //         chip::DeviceLayer::PlatformMgr().LockChipStack();
    //         err = logic->SetMainState(MainStateEnum::kMoving);
    //         chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    //         VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);

    //         return HandleMotion();
    //     }
    // }
    // else
    // {
    //     chip::DeviceLayer::PlatformMgr().LockChipStack();
    //     err = logic->SetMainState(MainStateEnum::kWaitingForMotion);
    //     mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleWaitforMotionCountDown));
    //     err = logic->SetCountdownTimeFromDelegate();
    //     chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    //     VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);

    //     (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    // }
    
    return Status::Success;
}

bool PrintOnlyDelegate::IsDeviceReadytoMove()
{
    // Check if device needs some action before movement.(manufacture specific)
    return true;
}

bool PrintOnlyDelegate::IsPreStageComplete()
{
    return true;
}

chip::app::DataModel::Nullable<ElapsedS> PrintOnlyDelegate::GetRemainingTime()
{
    
    if (mCountDownTime.IsNull())
        return chip::app::DataModel::NullNullable;

    return chip::app::DataModel::MakeNullable((mCountDownTime.Value() - static_cast<ElapsedS>(mMovingTime + mWaitingTime + mCalibratingTime)));
}

void PrintOnlyDelegate::HandleCountdownTimeExpired()
{
    ClusterLogic * logic = GetLogic();
    MainStateEnum state;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    logic->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (state == MainStateEnum::kCalibrating)
    {
        mCalibratingTime = 0;
    }

    if (state == MainStateEnum::kWaitingForMotion)
    {
        mWaitingTime = 0;
        
        if (PrintOnlyDelegate::IsPreStageComplete())
        {
            logic->SetMainState(MainStateEnum::kMoving);
            mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleMotionCountDown));
            HandleMotion();
        }
    }

    if (state == MainStateEnum::kMoving)
    {
        mMovingTime = 0;
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        logic->PostMovementCompletedEvent();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(MOVE_ACTION);
        }
    }

    mCountDownTime.SetNonNull(0);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    logic->SetMainState(MainStateEnum::kStopped);
    logic->SetCountdownTimeFromDelegate(mCountDownTime);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

Protocols::InteractionModel::Status PrintOnlyDelegate::HandleMotion()
{
    Action_t action = INVALID_ACTION;
    chip::app::DataModel::Nullable<GenericOverallTarget> target;
    GetLogic()->GetOverallTarget(target);
    // Cancel timer if any motion is in progress
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);

    if (target.Value().latch.HasValue() && target.Value().latch.Value())
    {
        action = MOVE_AND_LATCH_ACTION;
    }
    else
    {
        action = MOVE_ACTION;
    }

    mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleMotionCountDown));

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    GetLogic()->SetCountdownTimeFromDelegate(GetRemainingTime());
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (mActionInitiated_CB)
    {
        mActionInitiated_CB(action);
    }

    return Status::Success;
}

Status PrintOnlyDelegate::HandleStopCommand()
{
    ChipLogProgress(AppServer, "HandleStopCommand");
    MainStateEnum state;
    ClusterLogic * logic = GetLogic();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);
    logic->GetMainState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (state)
    {
        case MainStateEnum::kCalibrating:
            mCalibratingTime = 0;
            break;
        case MainStateEnum::kMoving:
            mMovingTime = 0;
            break;
        case MainStateEnum::kWaitingForMotion:
            mWaitingTime = 0;
            break;
        default:
            // No action needed for other states
            break;
    }

    if (mActionCompleted_CB)
    {
        mActionCompleted_CB(STOP_ACTION);
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    err = logic->PostMovementCompletedEvent();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);

    mCountDownTime.SetNull();
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    logic->SetCountdownTimeFromDelegate(mCountDownTime);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return Status::Success;
}

bool PrintOnlyDelegate::IsManualLatchingNeeded()
{
    ChipLogProgress(AppServer, "IsManualLatchingNeeded");
    // Add the IsManualLatchingNeeded  logic here
    return false;
}

bool PrintOnlyDelegate::IsReadyToMove()
{
    ChipLogProgress(AppServer, "IsReadyToMove");
    // Add the IsReadyToMove logic here
    return false;
}

ElapsedS PrintOnlyDelegate::GetMovingCountdownTime()
{
    ChipLogProgress(AppServer, "GetMovingCountdownTime");
    // Add the GetMovingCountdownTime logic here
    return static_cast<ElapsedS>(kExampleMotionCountDown);
}

ElapsedS PrintOnlyDelegate::GetWaitingForMotionCountdownTime()
{
    ChipLogProgress(AppServer, "GetWaitingForMotionCountdownTime");
    // Add the GetWaitingForMotionCountdownTime logic here
    return static_cast<ElapsedS>(kExampleWaitforMotionCountDown);
}

CHIP_ERROR PrintOnlyDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR PrintOnlyDelegate::SetCurrentErrorInList(const ClosureErrorEnum & closureError)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ChipLogProgress(AppServer, "ClosureControlEndpoint::Init start");
    
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed).Set(Feature::kVentilation)
                            .Set(Feature::kPedestrian).Set(Feature::kCalibration).Set(Feature::kProtection).Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);
    
    ClusterInitParameters clusterInitParameters;
    clusterInitParameters.mMainState = MainStateEnum::kStopped;
    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ReturnErrorOnFailure(mInterface.Init());
    
    ChipLogProgress(AppServer, "ClosureControlEndpoint::Init end");
     
    return CHIP_NO_ERROR;
 }