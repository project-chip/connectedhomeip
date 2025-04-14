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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;

using Protocols::InteractionModel::Status;

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

    if (pos.HasValue())
    {
        if (conformance.HasFeature(Feature::kPositioning))
        {
            if (static_cast<uint16_t>(pos.Value()) != static_cast<uint16_t>(state.currentState.position.Value()))
            {
                    motionNeeded  = true;
                    target.position = pos;
            }
        }
    }

    if (latch.HasValue())
    {
        if (conformance.HasFeature(Feature::kMotionLatching))
        {
            latchNeeded  = true;
            target.latch = latch;
        }
    }

    if (speed.HasValue())
    {
        if (conformance.HasFeature(Feature::kSpeed))
        {
            if(static_cast<uint16_t>(state.currentState.speed.Value()) != static_cast<uint16_t>(speed.Value()))
            {
                motionNeeded  = true;
                target.speed = speed;
            }
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
    ChipLogDetail(Zcl, "state.target.position.Value() = %d, state.currentState.position.Value() = %d ", state.target.position.Value(),state.currentState.position.Value());
    int32_t newPos = std::min((state.target.position.Value() + 0), (state.currentState.position.Value() + state.stepValue));
    ChipLogDetail(Zcl,"Handle newpos : %ld", newPos);
    osDelay(2000);
    state.currentState.position.SetValue(newPos);
    delegate->getLogic()->SetCurrentState(state.currentState);
    ChipLogDetail(Zcl,"after SetCurrentState");
    osDelay(2000);
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
        ChipLogDetail(Zcl,"Timer restart");
        osDelay(2000);
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
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown), HandleStepMotion, this);

    return Status::Success;
}

static void MotionTimerEventHandler(System::Layer * systemLayer, void * data)
{
    ChipLogDetail(Zcl, "Inside MotionTimerEventHandler");
    osDelay(2000);
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
    delegate->SetDeviceMoving(false);
    ChipLogDetail(Zcl,"After setdevice");
    osDelay(2000);
    GenericTargetStruct target;
    GenericCurrentStateStruct current;
    
    delegate->getLogic()->GetTarget(target);

    ChipLogDetail(Zcl, "After GetTarget()");
    osDelay(2000);

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
    osDelay(2000);
    ChipLogDetail(Zcl, "SetCurrentState done");
    
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
    // chip::DeviceLayer::PlatformMgr().LockChipStack();
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(kExampleMotionCountDown), MotionTimerEventHandler, this);
    // chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    if (mActionInitiated_CB)
    {   
        isMoving = true;
        SetAction(action);  
        mActionInitiated_CB(action);
    }
    return Status::Success;
}
