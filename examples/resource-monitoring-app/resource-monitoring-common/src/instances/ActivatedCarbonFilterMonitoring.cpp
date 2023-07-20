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
#include <app/util/endpoint-config-api.h>
#include <instances/ActivatedCarbonFilterMonitoring.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

#define ACTIVATED_CARBON_FILTER_REPLACEMENT_PRODUCT_LIST_SIZE 3

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ActivatedCarbonFilterMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

static ResourceMonitoring::Attributes::ReplacementProductStruct::Type
    sActivatedCarbonFilterReplacementProductsList[ACTIVATED_CARBON_FILTER_REPLACEMENT_PRODUCT_LIST_SIZE];
StaticReplacementProductListManager
    sActivatedCarbonFilterReplacementProductListManager(&sActivatedCarbonFilterReplacementProductsList[0],
                                                        ACTIVATED_CARBON_FILTER_REPLACEMENT_PRODUCT_LIST_SIZE);

//-- Activated carbon filter Monitoring Instance methods
CHIP_ERROR ActivatedCarbonFilterMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");

    sActivatedCarbonFilterReplacementProductsList[0].productIdentifierType  = ProductIdentifierTypeEnum::kUpc;
    sActivatedCarbonFilterReplacementProductsList[0].productIdentifierValue = "upc12xcarbon";
    sActivatedCarbonFilterReplacementProductsList[1].productIdentifierType  = ProductIdentifierTypeEnum::kGtin8;
    sActivatedCarbonFilterReplacementProductsList[1].productIdentifierValue = "gtin8xca";
    sActivatedCarbonFilterReplacementProductsList[2].productIdentifierType  = ProductIdentifierTypeEnum::kEan;
    sActivatedCarbonFilterReplacementProductsList[2].productIdentifierValue = "ean13xacarbon";

    SetReplacementProductListManagerInstance(&sActivatedCarbonFilterReplacementProductListManager);

    return CHIP_NO_ERROR;
}

Status ActivatedCarbonFilterMonitoringInstance::PreResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringInstance::PreResetCondition()");
    return Status::Success;
}

Status ActivatedCarbonFilterMonitoringInstance::PostResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringInstance::PostResetCondition()");
    return Status::Success;
}
