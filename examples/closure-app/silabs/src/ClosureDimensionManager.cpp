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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosurDimesnion;

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
    GenericTargetStruct target {};
    Status status = Status::Success;
    
    ClosureDimensionDelegate * closureManager = reinterpret_cast<ClosureDimensionDelegate *>(this);
    
    ClusterState state = closureManager->gLogic->GetState();
    ClusterConformance conformance = closureManager->gLogic->GetConformance();
        
    VerifyOrReturnValue(pos.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (pos.HasValue())
    {
        VerifyOrReturnValue(pos.Value() >= 0 && pos.Value() <= 10000, Status::ConstraintError);
        VerifyOrReturnValue(conformance.HasFeature(Feature::kPositioning),Status::Success);
        //VerifyOrReturnValue(IsPositionValid(), Status::InvalidInState); - manufacture specific

        if (pos.Value() != state.currentState.position.Value())
        {
                motionNeeded  = true;
                target.position = pos;
        }
    }

    if (latch.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(latch.Value()) != TargetLatchEnum::kUnknownEnumValue, Status::ConstraintError);
        VerifyOrReturnValue(conformance.HasFeature(Feature::kMotionLatching),Status::Success);
        VerifyOrReturnValue(isManualLatch, Status::InvalidAction);
            //if (state.currentState.latching.Value() != latch.Value())
            {
                latchNeeded  = true;
                target.latch = latch;
            }
    }

    if (speed.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            Status::ConstraintError);
        if (conformance.HasFeature(Feature::kSpeed))
        {
            if(state.currentState.speed.Value() != speed.Value())
            {
                motionNeeded  = true;
                target.speed = speed;
            }
        }
    }
    return PositioningEnum::kUnknownEnumValue;
}

void ClosureDimensionManager::SetClosureDimensionInstance(ClosureDimension::Instance & instance)
{
    mpClosureDimensionInstance = &instance;
}

ClosureDimensionManager ClosureDimensionManager::sClosureCtrlMgr;

/*********************************************************************************
 *
 * Methods implementing the ClosureDimension::Delegate interace
 *
 *********************************************************************************/

// Return default value, will add timers and attribute handling in next phase
DataModel::Nullable<uint32_t> ClosureDimensionManager::GetCountdownTime()
{
    // TODO: handle countdown timer
    return DataModel::NullNullable;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureDimensionManager::StartCurrentErrorListRead()
{
    // Notify device that errorlist is being read and data should locked
    return CHIP_NO_ERROR;
}

// TODO: Return emualted error list, will add event handling along with Events
CHIP_ERROR ClosureDimensionManager::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    VerifyOrReturnError(Index < MATTER_ARRAY_SIZE(kCurrentErrorList), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    closureError = kCurrentErrorList[Index];
    return CHIP_NO_ERROR;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureDimensionManager::EndCurrentErrorListRead()
{
    // Notify device that errorlist is being read completed and lock on data is removed
    return CHIP_NO_ERROR;
}

    // If device is already at TargetState ,no Action is required will give Status::Success
    VerifyOrReturnValue(motionNeeded || latchNeeded, Status::Success);

    closureManager->gLogic->SetTarget(target);

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
    
    auto newPos = std::min((state.target.position.Value() + 0), (state.currentState.position.Value() + state.stepValue));
    
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
    ClosureDimensionDelegate * closureManager = reinterpret_cast<ClosureDimensionDelegate *>(this);
    VerifyOrReturnError(closureManager != nullptr, Status::InvalidInState);
    
    VerifyOrReturnError((Clusters::EnsureKnownEnumValue(direction) != StepDirectionEnum::kUnknownEnumValue), Status::ConstraintError);
    VerifyOrReturnError((Clusters::EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue), Status::ConstraintError);
    VerifyOrReturnValue(numberOfSteps > 0, Status::ConstraintError);

    ClusterState state = closureManager->gLogic->GetState();
    GenericTargetStruct target = state.target;
    
    // Check if the current position is valid
    VerifyOrReturnValue(state.currentState.position.HasValue(), Status::InvalidInState);
    uint16_t currentPos = static_cast<uint16_t>(state.currentState.position.Value());
    
    // Convert step to position delta
    int32_t delta    = numberOfSteps * state.stepValue;
    int32_t newPos   = 0;
    
    bool limitSupported = closureManager->gLogic->GetConformance().HasFeature(Feature::kLimitation) ? true : false;

    switch (direction)
    {
        case StepDirectionEnum::kDecrease:
            newPos = currentPos - delta;
            newPos = limitSupported ? std::max(newPos, static_cast<int32_t>(state.limitRange.min)) : std::max(newPos, (int32_t)0);
            target.position.SetValue(newPos);
            break;
        case StepDirectionEnum::kIncrease:
            newPos = currentPos + delta;
            newPos = limitSupported ? std::min(newPos, static_cast<int32_t>(state.limitRange.min)) : std::min(newPos, (int32_t)10000);
            target.position.SetValue(newPos);
            break;
        default:
            // Should never reach here due to earlier VerifyOrReturnError check
            ChipLogError(AppServer, "Unhandled StepDirectionEnum value");
            return Status::ConstraintError;
    }
    
    if (speed.HasValue())
    {
        target.speed = speed;
        ChipLogProgress(NotSpecified, "Speed mode: %d", static_cast<uint8_t>(speed.Value()));
    }
    
    mAction = STEP_ACTION;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown), HandleStepMotion, this);
    
    VerifyOrReturnValue(closureManager->gLogic->SetTarget(target) == CHIP_NO_ERROR, Status::Failure);
    
    return Status::Success;
}

static void MotionTimerEventHandler(System::Layer * systemLayer, void * data)
{
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
    
    
    delegate->SetDeviceMoving(false);
    GenericTargetStruct target {};
    GenericCurrentStateStruct current {};
    delegate->getLogic()->GetTarget(target);
    current.position.SetValue(target.position.Value()); 
    //current.latching.SetValue(target.latch.Value());
    current.speed.SetValue(target.speed.Value());
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
