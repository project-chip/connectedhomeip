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
 
using namespace chip;
using namespace chip::app::Clusters::ClosureDimension;
 
using Protocols::InteractionModel::Status;
 
CHIP_ERROR PrintOnlyDelegate::Init()
{
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init start");
    // Add the Init logic here
    ChipLogProgress(AppServer, "PrintOnlyDelegate::Init done");
    return CHIP_NO_ERROR;
}

Status PrintOnlyDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleSetTarget");
    // Add the SetTarget handling logic here
    return Status::Success;
}

Status PrintOnlyDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(AppServer, "HandleStep");
    // Add the Step handling logic here
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