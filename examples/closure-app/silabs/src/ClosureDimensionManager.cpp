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
#include <ClosureDimensionManager.h>
#include <app/clusters/closure-dimension-server/closure-dimension-server.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>
#include <platform/CHIPDeviceLayer.h>
#include <cmsis_os2.h>
namespace {
    constexpr chip::Percent100ths LIMIT_RANGE_MIN = 0;
    constexpr chip::Percent100ths LIMIT_RANGE_MAX = 10000;
    constexpr chip::Percent100ths STEP = 1000;
}
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;

using Protocols::InteractionModel::Status;

CHIP_ERROR ClosureDimensionDelegate::Init()
{
    ChipLogProgress(AppServer, "ClosureDimensionDelegate::Init start");
    GenericCurrentStateStruct current;
    current.position.SetValue(0);
    current.latching.SetValue(LatchingEnum::kNotLatched);
    current.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    getLogic()->SetCurrentState(current);

    GenericTargetStruct target;
    target.position.SetValue(0);
    target.latch.SetValue(TargetLatchEnum::kUnlatch);
    target.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
    getLogic()->SetTarget(target);

    Structs::RangePercent100thsStruct::Type limitRange;
    limitRange.min = LIMIT_RANGE_MIN;
    limitRange.max = LIMIT_RANGE_MAX;
    getLogic()->SetLimitRange(limitRange);
    
    getLogic()->SetStepValue(STEP);
    ChipLogProgress(AppServer, "ClosureDimensionDelegate::Init done");
    return CHIP_NO_ERROR;
}

void ClosureDimensionDelegate::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

Status ClosureDimensionDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                                                     const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    bool motionNeeded = false;
    bool latchNeeded  = false;
    GenericTargetStruct target;
    Status status = Status::Success;
        
    ClusterState state = getLogic()->GetState();
    ClusterConformance conformance = getLogic()->GetConformance();

    if (conformance.HasFeature(Feature::kPositioning) && pos.HasValue())
    {
        if (static_cast<uint16_t>(pos.Value()) != static_cast<uint16_t>(state.currentState.position.Value()))
        {
            motionNeeded  = true;
            target.position = pos;
        }
    }

    if (conformance.HasFeature(Feature::kMotionLatching) && latch.HasValue())
    {
        latchNeeded  = true;
        target.latch = latch;
    }

    if (conformance.HasFeature(Feature::kSpeed) && speed.HasValue())
    {
        if(!state.currentState.speed.HasValue())
        {
            motionNeeded  = true;
            target.speed = speed;
        }
        else if(static_cast<uint16_t>(state.currentState.speed.Value()) != static_cast<uint16_t>(speed.Value()))
        {
            motionNeeded  = true;
            target.speed = speed;
        }
    }
    // If device is already at TargetState ,no Action is required will give Status::Success
    VerifyOrReturnValue(motionNeeded || latchNeeded, Status::Success);
 
    if (isMoving)
    {  
        status = HandleMotion(latchNeeded, motionNeeded, true);
    } else
    {
        status = HandleMotion(latchNeeded, motionNeeded, false);
    }
    return status;
 
}

static void HandleStepMotion(System::Layer * systemLayer, void * data)
{
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
    VerifyOrReturn(delegate != nullptr, void());
    
    ClusterState state = delegate->getLogic()->GetState();

    StepDirectionEnum direction = delegate->GetTargetDirection();
    
    int32_t newPos;
    if (direction == StepDirectionEnum::kDecrease)
    {
        newPos = std::max((state.target.position.Value() + 0), (state.currentState.position.Value() - state.stepValue));
    }
    else
    {
        newPos = std::min((state.target.position.Value() + 0), (state.currentState.position.Value() + state.stepValue));
    }
    state.currentState.position.SetValue(newPos);
    delegate->getLogic()->SetCurrentState(state.currentState);
    if (state.target.position.Value() == state.currentState.position.Value())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleStepMotion, delegate);
        if (delegate->mActionCompleted_CB)
        {    
            delegate->mActionCompleted_CB(delegate->GetAction());
        }
        
    } 
    else
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(delegate->kExampleStepCountDown), HandleStepMotion, delegate);
    }
    
    if (delegate->mActionInitiated_CB)
    {   
        delegate->SetDeviceMoving(true);  
        delegate->mActionInitiated_CB(delegate->GetAction());
    }
    
}

Status ClosureDimensionDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                const Optional<Globals::ThreeLevelAutoEnum> & speed)
{    
    mAction = STEP_ACTION;
    mTargetDirection = direction;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown), HandleStepMotion, this);
    return Status::Success;
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

Status ClosureDimensionDelegate::HandleMotion(bool latchNeeded, bool motionNeeded, bool newTarget)
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
