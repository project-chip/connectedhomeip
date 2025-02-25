/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <ClosureControlDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

//Mock Error List generated for sample application usage.
const ClosureErrorEnum kCurrentErrorList[] = {
    {
        ClosureErrorEnum::kBlocked,
    },
    {
        ClosureErrorEnum::kInternalInterference,
    },
    {
        ClosureErrorEnum::kMaintenanceRequired,
    },
    {
        ClosureErrorEnum::kTemperatureLimited,
    },
};

ClosureControlDelegate::ClosureControlDelegate(EndpointId clustersEndpoint) :
    mpClosureControlInstance(nullptr)
{}

void ClosureControlDelegate::SetClosureControlInstance(ClosureControl::Instance & instance)
{
    mpClosureControlInstance = &instance;
}

/*********************************************************************************
 *
 * Methods implementing the ClosureControl::Delegate interace
 *
 *********************************************************************************/
DataModel::Nullable<uint32_t> ClosureControlDelegate::GetCountdownTime()
{
    return DataModel::Nullable<uint32_t>();
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
RestingProcedureEnum ClosureControlDelegate::GetRestingProcedure()
{
    return RestingProcedureEnum::kDoNothing;
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
TriggerConditionEnum ClosureControlDelegate::GetTriggerCondition()
{
    return TriggerConditionEnum::kAfterDelay;
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
TriggerPositionEnum ClosureControlDelegate::GetTriggerPosition()
{
    return TriggerPositionEnum::kAtFullyClosed;
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
uint32_t ClosureControlDelegate::GetWaitingDelay()
{
    return 0;
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
uint32_t ClosureControlDelegate::GetKickoffTimer()
{
    return 0;
}

CHIP_ERROR ClosureControlDelegate::StartCurrentErrorListRead()
{
    return CHIP_NO_ERROR;    
}
CHIP_ERROR ClosureControlDelegate::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    if (Index >= MATTER_ARRAY_SIZE(kCurrentErrorList))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    closureError = kCurrentErrorList[Index];
    
    return CHIP_NO_ERROR; 
}
CHIP_ERROR ClosureControlDelegate::EndCurrentErrorListRead()
{
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ClosureControlDelegate::Stop(){
    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlDelegate::MoveTo(const Optional<TagPositionEnum> tag, const Optional<TagLatchEnum> latch,
                                                       const Optional<Globals::ThreeLevelAutoEnum> speed) 
{
    return Status::Success;                                                       
}

Protocols::InteractionModel::Status ClosureControlDelegate::Calibrate()
{
    return Status::Success;
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
Protocols::InteractionModel::Status ClosureControlDelegate::ConfigureFallback(const Optional<RestingProcedureEnum> restingProcedure,
    const Optional<TriggerConditionEnum> triggerCondition,
    const Optional<TriggerPositionEnum> triggerPosition,
    const Optional<uint32_t> waitingDelay) 
{
    return Status::Success;   
}

//Below Function will be removed along removal of Fallback feature from closure control cluster
Protocols::InteractionModel::Status ClosureControlDelegate::CancelFallback()
{
    return Status::Success;   
}
