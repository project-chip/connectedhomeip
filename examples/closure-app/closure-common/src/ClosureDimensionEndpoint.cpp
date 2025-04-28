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
namespace {
    constexpr chip::Percent100ths kLimitRangeMin = 0;
    constexpr chip::Percent100ths kLimitRangaMax = 10000;
    constexpr chip::Percent100ths kStep            = 1000;
    const uint32_t kExampleMotionCountDown = 5;
    const uint32_t kExampleStepCountDown   = 3000;
} // namespace

using namespace chip;
using namespace chip::app::Clusters::ClosureDimension;
using Protocols::InteractionModel::Status;
 
CHIP_ERROR ClosureDimensionDelegate::Init()
{
    GenericCurrentStateStruct currentState{ Optional<Percent100ths>(0), Optional<bool>(false),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    ReturnErrorOnFailure(mLogic->SetCurrentState(chip::app::DataModel::MakeNullable(currentState)));
    
    GenericTargetStruct targetState{ Optional<Percent100ths>(0), Optional<bool>(false),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    ReturnErrorOnFailure(mLogic->SetTarget(chip::app::DataModel::MakeNullable(targetState)));

    Structs::RangePercent100thsStruct::Type limitRange;
    limitRange.min = kLimitRangeMin;
    limitRange.max = kLimitRangaMax;
    ReturnErrorOnFailure(mLogic->SetLimitRange(limitRange));

    ReturnErrorOnFailure(mLogic->SetStepValue(kStep));
    
    return CHIP_NO_ERROR;
}

static void MotionTimerEventHandler(System::Layer * systemLayer, void * data)
{
    ClosureDimensionDelegate * delegate = reinterpret_cast<ClosureDimensionDelegate *>(data);
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
}


Status ClosureDimensionDelegate::HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    (void) DeviceLayer::SystemLayer().CancelTimer(MotionTimerEventHandler, this);
    
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(kExampleMotionCountDown), MotionTimerEventHandler, this);
    
    // Trigger Motion Action

    return Status::Success;;
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
        newPos = std::max((state.target.Value().position.Value() + 0), (state.currentState.Value().position.Value() - state.stepValue));
    }
    else
    {
        newPos = std::min((state.target.Value().position.Value() + 0), (state.currentState.Value().position.Value() + state.stepValue));
    }
    state.currentState.Value().position.SetValue(static_cast<Percent100ths>(newPos));
    delegate->GetLogic()->SetCurrentState(state.currentState);
    if (state.target.Value().position.Value() == state.currentState.Value().position.Value())
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleStepMotion, delegate);
    }
    else
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown),
                                                     HandleStepMotion, delegate);
        // Trigger Step Action
    }
}

Status ClosureDimensionDelegate::HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    mTargetDirection = direction;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds16(kExampleStepCountDown), HandleStepMotion, this);
    
    //Trigger Step Action
    return Status::Success;
    
}

bool ClosureDimensionDelegate::IsManualLatchingNeeded()
{
     // Check if closure needs manual latching.(manufacture specific)
    return false;
}
 
CHIP_ERROR ClosureDimensionEndpoint::Init()
{
    ChipLogProgress(AppServer, "ClosureDimensionEndpoint::Init start");
    
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning)
                            .Set(Feature::kMotionLatching)
                            .Set(Feature::kUnit)
                            .Set(Feature::kLimitation)
                            .Set(Feature::kSpeed);
    conformance.OptionalAttributes().Set(OptionalAttributeEnum::kOverflow);
    
    ClusterInitParameters clusterInitParameters;
    
    ReturnErrorOnFailure(mLogic.Init(conformance, clusterInitParameters));
    ChipLogProgress(AppServer, "mLogicInit done");
    
    ReturnErrorOnFailure(mInterface.Init());
    ChipLogProgress(AppServer, "interfaceInit done");
    
    ReturnErrorOnFailure(mDelegate.Init());
    ChipLogProgress(AppServer, "ClosureDimensionEndpoint::Init end");
    return CHIP_NO_ERROR;
}