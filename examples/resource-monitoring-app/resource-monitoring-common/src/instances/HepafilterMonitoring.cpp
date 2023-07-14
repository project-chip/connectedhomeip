/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/endpoint-config-api.h>
#include <instances/HepaFilterMonitoring.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::HepaFilterMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

//-- Hepa filter Monitoring instance methods
CHIP_ERROR HepaFilterMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringInstance::Init()");
    return CHIP_NO_ERROR;
}

Status HepaFilterMonitoringInstance::OnResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringInstance::OnResetCondition()");

    if (GetDegradationDirection() == DegradationDirectionEnum::kDown)
    {
        UpdateCondition(100);
    }
    else if (GetDegradationDirection() == DegradationDirectionEnum::kUp)
    {
        UpdateCondition(0);
    }
    UpdateChangeIndication(ChangeIndicationEnum::kOk);
    if (emberAfContainsAttribute(GetEndpointId(), Clusters::HepaFilterMonitoring::Id, Attributes::LastChangedTime::Id))
    {
        System::Clock::Milliseconds64 currentUnixTimeMS;
        System::Clock::ClockImpl clock;
        CHIP_ERROR err = clock.GetClock_RealTimeMS(currentUnixTimeMS);
        if (err == CHIP_NO_ERROR)
        {
            System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
            UpdateLastChangedTime(DataModel::MakeNullable(currentUnixTime.count()));
        }
    }

    return Status::Success;
}
