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
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

namespace {

constexpr ElapsedS kDefaultCountdownTime = 30;

enum class ClosureControlTestEventTrigger : uint64_t
{
    // MainState is Error(3) Test Event | Simulate that the device is in error state, add at least one element to the
    // CurrentErrorList attribute
    kMainStateIsError = 0x0104000000000000,

    // MainState is Protected(5) Test Event | Simulate that the device is in protected state
    kMainStateIsProtected = 0x0104000000000001,

    // MainState is Disengaged(6) Test Event | Simulate that the device is in disengaged state
    kMainStateIsDisengaged = 0x0104000000000002,

    // MainState is SetupRequired(7) Test Event | Simulate that the device is in SetupRequired state
    kMainStateIsSetupRequired = 0x0104000000000003,

    // MainState Test clear Event | Returns the device to pre-test status for that test event.
    kClearEvent = 0x0104000000000004,

};

} // namespace

Status ClosureControlDelegate::HandleCalibrateCommand()
{
    ChipLogProgress(AppServer, "HandleCalibrateCommand");
    // Add the calibration logic here
    return Status::Success;
}

Status ClosureControlDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                                   const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleMoveToCommand");
    // Add the move to logic here
    return Status::Success;
}

Status ClosureControlDelegate::HandleStopCommand()
{
    ChipLogProgress(AppServer, "HandleStopCommand");
    // Add the stop logic here
    return Status::Success;
}

CHIP_ERROR ClosureControlDelegate::ClearErrorList()
{
    mCurrentErrorCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlDelegate::AddErrorToCurrentErrorList(ClosureErrorEnum error)
{
    VerifyOrReturnError(mCurrentErrorCount < kMaxErrorCount, CHIP_ERROR_BUFFER_TOO_SMALL,
                        ChipLogError(AppServer, "Error list full"));
    // Check for duplicates
    for (size_t i = 0; i < mCurrentErrorCount; ++i)
    {
        VerifyOrReturnError(mCurrentErrorList[i] != ClosureErrorEnum::kUnknownEnumValue,
                            CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(AppServer, "Unknown error in the list"));
    }
    mCurrentErrorList[mCurrentErrorCount++] = error;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    // This function should return the current error at the specified index.
    // For now, we dont have a ErrorList implemented, so will return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED.
    // return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    if (index >= mCurrentErrorCount)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    closureError = mCurrentErrorList[index];
    return CHIP_NO_ERROR;
}

bool ClosureControlDelegate::IsReadyToMove()
{
    // This function should return true if the closure is ready to move.
    // For now, we will return true.
    return true;
}

bool ClosureControlDelegate::IsManualLatchingNeeded()
{
    // This function should return true if manual latching is needed.
    // For now, we will return false.
    return false;
}

ElapsedS ClosureControlDelegate::GetCalibrationCountdownTime()
{
    // This function should return the calibration countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS ClosureControlDelegate::GetMovingCountdownTime()
{
    // This function should return the moving countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

ElapsedS ClosureControlDelegate::GetWaitingForMotionCountdownTime()
{
    // This function should return the waiting for motion countdown time.
    // For now, we will return kDefaultCountdownTime.
    return kDefaultCountdownTime;
}

CHIP_ERROR ClosureControlDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    eventTrigger                           = clearEndpointInEventTrigger(eventTrigger);
    ClosureControlTestEventTrigger trigger = static_cast<ClosureControlTestEventTrigger>(eventTrigger);
    ClusterLogic * logic                   = GetLogic();

    switch (trigger)
    {
    case ClosureControlTestEventTrigger::kMainStateIsSetupRequired:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kSetupRequired));
        ReturnErrorOnFailure(AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor));
        break;
    case ClosureControlTestEventTrigger::kMainStateIsProtected:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kProtected));
        break;
    case ClosureControlTestEventTrigger::kMainStateIsError:
        return logic->SetMainState(MainStateEnum::kError);
    case ClosureControlTestEventTrigger::kMainStateIsDisengaged:
        return logic->SetMainState(MainStateEnum::kDisengaged);
    case ClosureControlTestEventTrigger::kClearEvent:
        ReturnErrorOnFailure(logic->SetMainState(MainStateEnum::kStopped));
        ReturnErrorOnFailure(ClearErrorList());
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kVentilation)
        .Set(Feature::kPedestrian)
        .Set(Feature::kCalibration)
        .Set(Feature::kProtection)
        .Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);

    ClusterInitParameters initParams;

    ReturnErrorOnFailure(mLogic.Init(conformance, initParams));
    ReturnErrorOnFailure(mInterface.Init());

    return CHIP_NO_ERROR;
}
