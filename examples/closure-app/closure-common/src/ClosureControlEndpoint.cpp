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

Status PrintOnlyDelegate::HandleCalibrateCommand()
{
    ChipLogProgress(AppServer, "HandleCalibrateCommand");
    // Add the calibration logic here
    return Status::Success;
}

Status PrintOnlyDelegate::HandleMoveToCommand()
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

    ReturnErrorOnFailure(mLogic.Init(conformance));
    ReturnErrorOnFailure(mInterface.Init());

    return CHIP_NO_ERROR;
}
