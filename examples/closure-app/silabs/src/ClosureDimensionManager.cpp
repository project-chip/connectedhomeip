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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosurDimesnion;

using Protocols::InteractionModel::Status;

ClosureDimensionManager ClosureDimensionManager::sManager();

void ClosureDimensionManager::MoveToPosition(uint16_t position)
{
    // Send command to actuator or update internal state
    ChipLogProgress(AppServer, "Moving to position: %u", position);

    GenericCurrentStateStruct currState;
    currState.position.SetValue(position);

    // TODO : Update Cluster attribute
    ClosureDimensionManager::GetManagerInstance().getLogic().SetCurrentState(currState);

    mCurrentPosition = position;
    // setcallback()
}

// Update speed in internal state or send to motor controller
void ClosureDimensionManager::SetSpeed(Globals::ThreeLevelAutoEnum speedMode)
{
    // TODO : Update Cluster attribute as well
    mCurrentSpeed = speedMode;
}


CHIP_ERROR ClosureDimensionDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                                                     const Optional<Globals::ThreeLevelAutoEnum> & speed)
{

    if (pos.HasValue())
    {
        uint16_t position = pos.Value(); // 0 - 10000

        // TODO: Convert to hardware-specific range and move actuator using position

        // Optionally handle latch and speed
        if (latch.HasValue())
        {
            ChipLogProgress(NotSpecified, "Target latch mode: %d", static_cast<uint8_t>(latch.Value()));
        }

        if (speed.HasValue())
        {
            ChipLogProgress(NotSpecified, "Speed mode: %d", static_cast<uint8_t>(speed.Value()));
        }

        return CHIP_NO_ERROR;
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

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ClosureDimensionDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    // Check if Direction is valid
    if (direction != StepDirectionEnum::kIncrease && direction != StepDirectionEnum::kDecrease)
    {
        ChipLogError(AppServer, "Invalid direction value");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!numberOfSteps) // NumberOfSteps == 0 -> ignore and return success
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnValue(numberOfSteps != 0, CHIP_NO_ERROR);

    // Check if Speed is valid, assuming some validation function is defined
    VerifyOrReturnError(Clusters::EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                CHIP_ERROR_INVALID_ARGUMENT);

    ClusterState state = ClosureDimensionManager::GetManagerInstance().getLogic().GetState();  // mLogic.GetState();

    // Convert step to position delta
    int32_t delta    = numberOfSteps * state.stepValue;
    int32_t newPos   = 0;

    // Get current position
    uint16_t currentPos = state.currentState.position.HasValue() ? static_cast<uint16_t>(state.currentState.position.Value()) : 0; // 0 - 10000

    bool limitSupported = ClosureDimensionManager::GetManagerInstance().getConformance().HasFeature(Feature::kLimitation) ? true : false;

    switch (direction)
    {
        case StepDirectionEnum::kDecrease:
            newPos = currentPos - delta;
            newPos = limitSupported ? std::max(newPos, static_cast<int32_t>(state.limitRange.min)) : std::max(newPos, (int32_t)0);
            break;
        case StepDirectionEnum::kIncrease:
            newPos = currentPos + delta;
            newPos = limitSupported ? std::min(newPos, static_cast<int32_t>(state.limitRange.min)) : std::min(newPos, (int32_t)10000);
            break;
        default:
            // Should never reach here due to earlier VerifyOrReturnError check
            ChipLogError(AppServer, "Unhandled StepDirectionEnum value");
            return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Set new position
    ClosureDimensionManager::GetManagerInstance().MoveToPosition(static_cast<uint16_t>(newPos));

    if (speed.HasValue())
    {
        ChipLogProgress(NotSpecified, "Speed mode: %d", static_cast<uint8_t>(speed.Value()));
    }

    return CHIP_NO_ERROR;
}

void ClosureDimensionManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}