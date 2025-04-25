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

#include <ClosureDimensionEndpoint.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>
#include <app/clusters/closure-dimension-server/closure-dimension-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <cmsis_os2.h>

namespace {
    constexpr chip::Percent100ths LIMIT_RANGE_MIN = 0;
    constexpr chip::Percent100ths LIMIT_RANGE_MAX = 10000;
    constexpr chip::Percent100ths STEP            = 1000;
    const uint32_t kExampleMotionCountDown = 5;
    const uint32_t kExampleStepCountDown   = 3000;
} // namespace
using namespace chip;
using namespace chip::app::Clusters::ClosureDimension;
 
using Protocols::InteractionModel::Status;
 
CHIP_ERROR PrintOnlyDelegate::Init()
{
    CHIP_ERROR err;
    GenericCurrentStateStruct currentState{ Optional<Percent100ths>(0), Optional<bool>(false),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    err = mLogic->SetCurrentState(chip::app::DataModel::MakeNullable(currentState));
    // ChipLogProgress(AppServer, "SetCurrentState p1");
    // osDelay(1000);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    GenericTargetStruct targetState{ Optional<Percent100ths>(0), Optional<bool>(false),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    // chip::app::DataModel::Nullable<GenericTargetStruct> target;
    // target.Value().position.SetValue(0);
    // target.Value().latch.SetValue(false);
    // target.Value().speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    err = mLogic->SetTarget(chip::app::DataModel::MakeNullable(targetState));
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);


    Structs::RangePercent100thsStruct::Type limitRange;
    limitRange.min = LIMIT_RANGE_MIN;
    limitRange.max = LIMIT_RANGE_MAX;
    err = mLogic->SetLimitRange(limitRange);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);


    err = mLogic->SetStepValue(STEP);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    return CHIP_NO_ERROR;
}

void PrintOnlyDelegate::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

static void MotionTimerEventHandler(System::Layer * systemLayer, void * data)
{
    PrintOnlyDelegate * delegate = reinterpret_cast<PrintOnlyDelegate *>(data);
    delegate->SetDeviceMoving(false);
    chip::app::DataModel::Nullable<GenericTargetStruct> target;
    chip::app::DataModel::Nullable<GenericCurrentStateStruct> current;

    delegate->GetLogic()->GetTarget(target);
    current.Value().position.SetValue(static_cast<uint16_t>(target.Value().position.Value()));
    if (target.Value().latch.HasValue())
    {
        current.Value().latch.SetValue(target.Value().latch.Value());
    }

    if (target.Value().speed.HasValue())
    {
        current.Value().speed.SetValue(target.Value().speed.Value());
    }
    delegate->GetLogic()->SetCurrentState(current);

    if (delegate->mActionCompleted_CB)
    {
        delegate->mActionCompleted_CB(delegate->GetAction());
    }
}

Status PrintOnlyDelegate::HandleMotion(bool latchNeeded, bool motionNeeded, bool newTarget)
{
    Action_t action = INVALID_ACTION;

    // Target changes when target is in motion
    if (newTarget)
    {
        SetAction(action);
        (void) DeviceLayer::SystemLayer().CancelTimer(MotionTimerEventHandler, this);
        action = TARGET_CHANGE_ACTION;
    }
    else
    {
        if (latchNeeded)
        {
            action = MOVE_AND_LATCH_ACTION;
        }
        else
        {
            action = MOVE_ACTION;
        }
    }
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(kExampleMotionCountDown), MotionTimerEventHandler, this);

    if (mActionInitiated_CB)
    {
        isMoving = true;
        SetAction(action);
        mActionInitiated_CB(action);
    }
    return Status::Success;
}

Status PrintOnlyDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleSetTarget");
    bool motionNeeded = false;
    bool latchNeeded  = false;
    chip::app::DataModel::Nullable<GenericTargetStruct> target;
    Status status = Status::Success;

    ClusterState state = GetLogic()->GetState();

    if (pos.HasValue() && (pos.Value() != state.currentState.Value().position.Value()))
    {
        motionNeeded    = true;
        target.Value().position = pos;
    }

    if (latch.HasValue())
    {
        latchNeeded  = true;
        target.Value().latch = latch;
    }

    if (speed.HasValue())
    {
        if (!state.currentState.Value().speed.HasValue() || (state.currentState.Value().speed.Value() != speed.Value()))
        {
            motionNeeded = true;
            target.Value().speed = speed;
        }
    }
    // If device is already at TargetState ,no Action is required will give Status::Success
    VerifyOrReturnValue(motionNeeded || latchNeeded, Status::Success);

    if (isMoving)
    {
        status = HandleMotion(latchNeeded, motionNeeded, true);
    }
    else
    {
        status = HandleMotion(latchNeeded, motionNeeded, false);
    }
    return status;
}

static void HandleStepMotion(System::Layer * systemLayer, void * data)
{
    PrintOnlyDelegate * delegate = reinterpret_cast<PrintOnlyDelegate *>(data);
    VerifyOrReturn(delegate != nullptr, void());

    ClusterState state = delegate->GetLogic()->GetState();

    StepDirectionEnum direction = delegate->GetTargetDirection();

    uint32_t newPos;
    if (direction == StepDirectionEnum::kDecrease)
    {
        newPos = std::max((state.target.Value().position.Value() + 0), (state.currentState.Value().position.Value() - state.stepValue));
    }
    else
    {
        newPos = std::min((state.target.Value().position.Value() + 0), (state.currentState.Value().position.Value() + state.stepValue));
    }
    state.currentState.Value().position.SetValue(newPos);
    delegate->GetLogic()->SetCurrentState(state.currentState);
    if (state.target.Value().position.Value() == state.currentState.Value().position.Value())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleStepMotion, delegate);
        if (delegate->mActionCompleted_CB)
        {
            delegate->mActionCompleted_CB(delegate->GetAction());
        }
    }
    else
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown),
                                                     HandleStepMotion, delegate);
    }

    if (delegate->mActionInitiated_CB)
    {
        delegate->SetDeviceMoving(true);
        delegate->mActionInitiated_CB(delegate->GetAction());
    }
}

Status PrintOnlyDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleStep");
    mAction          = STEP_ACTION;
    mTargetDirection = direction;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown), HandleStepMotion, this);
    return Status::Success;
    
}

bool PrintOnlyDelegate::IsManualLatchingNeeded()
{
    ChipLogProgress(AppServer, "IsManualLatchingNeeded");
    // Add the IsManualLatchingNeeded  logic here
    return false;
}
 
CHIP_ERROR ClosureDimensionEndpoint::Init()
{
    ChipLogProgress(AppServer, "ClosureDimensionEndpoint::Init start");
    osDelay(1000);
    
    ClusterConformance conformance;
    conformance.FeatureMap() = 255;
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kOverflow);
    
    ClusterInitParameters clusterInitParameters;
    clusterInitParameters.modulationType = ModulationTypeEnum::kVentilation;
    clusterInitParameters.rotationAxis = RotationAxisEnum::kTop;
    clusterInitParameters.translationDirection = TranslationDirectionEnum::kDownward;
     
    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ChipLogProgress(AppServer, "mLogicInit done");
    osDelay(1000);
    ReturnErrorOnFailure(mInterface.Init());
    ChipLogProgress(AppServer, "interfaceInit done");
    osDelay(1000);
    ReturnErrorOnFailure(mDelegate.Init());
      
    ChipLogProgress(AppServer, "ClosureDimensionEndpoint::Init end");
    osDelay(1000);
    return CHIP_NO_ERROR;
}