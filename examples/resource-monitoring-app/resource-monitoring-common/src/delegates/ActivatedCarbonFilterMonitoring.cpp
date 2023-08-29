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

#include <StaticReplacementProductListManager.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <delegates/ActivatedCarbonFilterMonitoring.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ActivatedCarbonFilterMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring::Attributes;
using chip::Protocols::InteractionModel::Status;

static ReplacementProductStruct sActivatedCarbonFilterReplacementProductsList[] = {
    { ResourceMonitoring::ProductIdentifierTypeEnum::kUpc, CharSpan::fromCharString("111112222233") },
    { ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8, CharSpan::fromCharString("gtin8xca") },
    { ResourceMonitoring::ProductIdentifierTypeEnum::kEan, CharSpan::fromCharString("4444455555666") },
    { ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14, CharSpan::fromCharString("gtin14xcarbonx") },
    { ResourceMonitoring::ProductIdentifierTypeEnum::kOem, CharSpan::fromCharString("oem20xcarbonxxxxxxxx") },
};
StaticReplacementProductListManager
    sActivatedCarbonFilterReplacementProductListManager(&sActivatedCarbonFilterReplacementProductsList[0],
                                                        ArraySize(sActivatedCarbonFilterReplacementProductsList));

//-- Activated carbon filter Monitoring Delegate methods
CHIP_ERROR ActivatedCarbonFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");

    GetInstance()->SetReplacementProductListManagerInstance(&sActivatedCarbonFilterReplacementProductListManager);

    return CHIP_NO_ERROR;
}

Status ActivatedCarbonFilterMonitoringDelegate::PreResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::PreResetCondition()");
    return Status::Success;
}

Status ActivatedCarbonFilterMonitoringDelegate::PostResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::PostResetCondition()");
    return Status::Success;
}
