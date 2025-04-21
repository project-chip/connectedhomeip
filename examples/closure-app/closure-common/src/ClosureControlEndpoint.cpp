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

using namespace chip;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

CHIP_ERROR PrintOnlyDelegate::Init()
{
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init start");
    // Add the Init logic here
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init done");
    return CHIP_NO_ERROR;
}

Status PrintOnlyDelegate::HandleCalibrateCommand()
{
    ChipLogProgress(AppServer, "HandleCalibrateCommand");
    // Add the calibration logic here
    return Status::Success;
}

Status PrintOnlyDelegate::HandleMoveToCommand(const Optional<TargetPositionEnum> & tag, const Optional<bool> & latch,
                                              const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleMoveToCommand");
    // Add the move to logic here
    return Status::Success;
}

Status PrintOnlyDelegate::HandleStopCommand()
{
    ChipLogProgress(AppServer, "HandleStopCommand");
    // Add the stop logic here
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

ElapsedS PrintOnlyDelegate::GetCalibrationCountdownTime()
{
    ChipLogProgress(AppServer, "GetCalibrationCountdownTime");
    // Add the GetCalibrationCountdownTime logic here
    return static_cast<ElapsedS>(5);
}

ElapsedS PrintOnlyDelegate::GetMovingCountdownTime()
{
    ChipLogProgress(AppServer, "GetMovingCountdownTime");
    // Add the GetMovingCountdownTime logic here
    return static_cast<ElapsedS>(10);
}

ElapsedS PrintOnlyDelegate::GetWaitingForMotionCountdownTime()
{
    ChipLogProgress(AppServer, "GetWaitingForMotionCountdownTime");
    // Add the GetWaitingForMotionCountdownTime logic here
    return static_cast<ElapsedS>(3);
}

CHIP_ERROR PrintOnlyDelegate::GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError)
{
    if (index >= currentErrors.size()) {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; // Invalid index
    }
    
    auto it = currentErrors.begin();
    std::advance(it, index);
    closureError = *it;
    
    return CHIP_NO_ERROR;     
}

CHIP_ERROR PrintOnlyDelegate::SetCurrentErrorInList(const ClosureErrorEnum & closureError)
{
    if (currentErrors.size() >= 10) {
        return CHIP_ERROR_INVALID_LIST_LENGTH; // List is full
    }

    if (currentErrors.find(closureError) != currentErrors.end()) {
        return CHIP_ERROR_DUPLICATE_KEY_ID; // Duplicate error
    }
    
    currentErrors.insert(closureError);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlEndpoint::Init()
{
    ChipLogProgress(AppServer, "ClosureControlEndpoint::Init start");
    
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kInstantaneous).Set(Feature::kSpeed).Set(Feature::kVentilation)
                            .Set(Feature::kPedestrian).Set(Feature::kCalibration).Set(Feature::kProtection).Set(Feature::kManuallyOperable);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kCountdownTime);
    
    ReturnErrorOnFailure(mLogic.Init(conformance));
    ReturnErrorOnFailure(mInterface.Init());
    ReturnErrorOnFailure(mDelegate.Init());
    
    
     
    ChipLogProgress(AppServer, "ClosureControlEndpoint::Init end");
     
    return CHIP_NO_ERROR;
 }