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
#include <ClosureControlDelegate.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

// Mock Error List generated for sample application usage.
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

ClosureControlDelegate::ClosureControlDelegate(EndpointId clustersEndpoint) : mpClosureControlInstance(nullptr) {}

void ClosureControlDelegate::SetClosureControlInstance(ClosureControl::Instance & instance)
{
    mpClosureControlInstance = &instance;
}

/*********************************************************************************
 *
 * Methods implementing the ClosureControl::Delegate interace
 *
 *********************************************************************************/

// Return default value, will add timers and attribute handling in next phase
DataModel::Nullable<uint32_t> ClosureControlDelegate::GetCountdownTime()
{
    return DataModel::NullNullable;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlDelegate::StartCurrentErrorListRead()
{
    return CHIP_NO_ERROR;
}

// Return emualted error list, will add attribute handling in next phase
CHIP_ERROR ClosureControlDelegate::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    if (Index >= MATTER_ARRAY_SIZE(kCurrentErrorList))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    closureError = kCurrentErrorList[Index];

    return CHIP_NO_ERROR;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlDelegate::EndCurrentErrorListRead()
{
    return CHIP_NO_ERROR;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlDelegate::Stop()
{
    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlDelegate::MoveTo(const Optional<TagPositionEnum> & tag,
                                                                   const Optional<TagLatchEnum> & latch,
                                                                   const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlDelegate::Calibrate()
{
    return Status::Success;
}
