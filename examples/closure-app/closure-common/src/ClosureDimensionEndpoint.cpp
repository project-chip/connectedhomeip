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
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init start");
    GenericCurrentStateStruct current;
    current.position.SetValue(0);
    current.latching.SetValue(false);
    current.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    mLogic.SetCurrentState(current);

    GenericTargetStruct target;
    target.position.SetValue(0);
    target.latch.SetValue(false);
    target.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    mLogic.SetTarget(target);

    Structs::RangePercent100thsStruct::Type limitRange;
    limitRange.min = LIMIT_RANGE_MIN;
    limitRange.max = LIMIT_RANGE_MAX;
    mLogic.SetLimitRange(limitRange);

    mLogic.SetStepValue(STEP);
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init done");
    return CHIP_NO_ERROR;
}

void PrintOnlyDelegate::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

static void MotionTimerEventHandler(System::Layer * systemLayer, void * data)
{
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
    delegate->SetDeviceMoving(false);
    GenericTargetStruct target;
    GenericCurrentStateStruct current;

    delegate->getLogic()->GetTarget(target);
    current.position.SetValue(static_cast<uint16_t>(target.position.Value()));
    if (target.latch.HasValue())
    {
        if (target.latch.Value() == TargetLatchEnum::kUnlatch)
        {
            current.latching.SetValue(LatchingEnum::kNotLatched);
        }
        else
        {
            current.latching.SetValue(LatchingEnum::kLatchedAndSecured);
        }
    }

    if (target.speed.HasValue())
    {
        current.speed.SetValue(target.speed.Value());
    }
    delegate->getLogic()->SetCurrentState(current);

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
    GenericTargetStruct target;
    Status status = Status::Success;

    ClusterState state             = GetLogic()->GetState();
    ClusterConformance conformance = GetLogic()->GetConformance();

    if (pos.HasValue() && (pos.Value() != state.currentState.position.Value()))
    {
        motionNeeded    = true;
        target.position = pos;
    }

    if (latch.HasValue())
    {
        latchNeeded  = true;
        target.latch = latch;
    }

    if (speed.HasValue())
    {
        if (!state.currentState.speed.HasValue() || (state.currentState.speed.Value() != speed.Value()))
        {
            motionNeeded = true;
            target.speed = speed;
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
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
    VerifyOrReturn(delegate != nullptr, void());

    ClusterState state = delegate->GetLogic()->GetState();

    StepDirectionEnum direction = delegate->GetTargetDirection();

    uint32_t newPos;
    if (direction == StepDirectionEnum::kDecrease)
    {
        newPos = std::max((state.target.position.Value() + 0), (state.currentState.position.Value() - state.stepValue));
    }
    else
    {
        newPos = std::min((state.target.position.Value() + 0), (state.currentState.position.Value() + state.stepValue));
    }
    state.currentState.Value().position.SetValue(newPos);
    delegate->GetLogic()->SetCurrentState(state.currentState);
    if (state.target.Value().position.Value() == state.currentState.position.Value())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleStepMotion, delegate);
        if (delegate->mActionCompleted_CB)
        {
            delegate->mActionCompleted_CB(delegate->GetAction());
        }
    }
    else
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(delegate->kExampleStepCountDown),
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
    
    ClusterConformance conformance;
    conformance.FeatureMap() = 255;
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kOverflow);
    
    ClusterInitParameters clusterInitParameters;
    clusterInitParameters.modulationType = ModulationTypeEnum::kVentilation;
    clusterInitParameters.rotationAxis = RotationAxisEnum::kTop;
    clusterInitParameters.translationDirection = TranslationDirectionEnum::kDownward;
     
    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ReturnErrorOnFailure(mInterface.Init());
    ReturnErrorOnFailure(mDelegate.Init());
      
    ChipLogProgress(AppServer, "ClosureDimensionEndpoint::Init end");
      
    return CHIP_NO_ERROR;
}