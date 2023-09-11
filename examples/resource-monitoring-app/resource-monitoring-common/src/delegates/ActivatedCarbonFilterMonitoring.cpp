/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
