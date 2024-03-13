/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <chef-resource-monitoring-delegates.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ActivatedCarbonFilterMonitoring;
using namespace chip::app::Clusters::HepaFilterMonitoring;
using chip::Protocols::InteractionModel::Status;

const chip::BitMask<ResourceMonitoring::Feature> gHepaFilterFeatureMap(ResourceMonitoring::Feature::kCondition,
                                                                       ResourceMonitoring::Feature::kWarning,
                                                                       ResourceMonitoring::Feature::kReplacementProductList);
const chip::BitMask<ResourceMonitoring::Feature> gActivatedCarbonFeatureMap(ResourceMonitoring::Feature::kCondition,
                                                                            ResourceMonitoring::Feature::kWarning,
                                                                            ResourceMonitoring::Feature::kReplacementProductList);

static std::unique_ptr<ActivatedCarbonFilterMonitoringDelegate> gActivatedCarbonFilterDelegate;
static std::unique_ptr<ResourceMonitoring::Instance> gActivatedCarbonFilterInstance;

static std::unique_ptr<HepaFilterMonitoringDelegate> gHepaFilterDelegate;
static std::unique_ptr<ResourceMonitoring::Instance> gHepaFilterInstance;

static ImmutableReplacementProductListManager sReplacementProductListManager;

//-- Activated Carbon Filter Monitoring delegate methods
CHIP_ERROR ActivatedCarbonFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");
    GetInstance()->SetReplacementProductListManagerInstance(&sReplacementProductListManager);
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

void ActivatedCarbonFilterMonitoring::Shutdown()
{
    gActivatedCarbonFilterInstance.reset();
    gActivatedCarbonFilterDelegate.reset();
}

//-- Hepa Filter Monitoring delegate methods
CHIP_ERROR HepaFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::Init()");
    GetInstance()->SetReplacementProductListManagerInstance(&sReplacementProductListManager);
    return CHIP_NO_ERROR;
}

Status HepaFilterMonitoringDelegate::PreResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::PreResetCondition()");
    return Status::Success;
}

Status HepaFilterMonitoringDelegate::PostResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::PostResetCondition()");
    return Status::Success;
}

void HepaFilterMonitoring::Shutdown()
{
    gHepaFilterInstance.reset();
    gHepaFilterDelegate.reset();
}

void emberAfActivatedCarbonFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(!gActivatedCarbonFilterInstance && !gActivatedCarbonFilterDelegate);
    gActivatedCarbonFilterDelegate       = std::make_unique<ActivatedCarbonFilterMonitoringDelegate>();
    bool bResetConditionCommandSupported = true; // The ResetCondition command is supported by the ResourceMonitor cluster
    gActivatedCarbonFilterInstance       = std::make_unique<ResourceMonitoring::Instance>(
        gActivatedCarbonFilterDelegate.get(), endpoint, ActivatedCarbonFilterMonitoring::Id,
        static_cast<uint32_t>(gActivatedCarbonFeatureMap.Raw()), ResourceMonitoring::DegradationDirectionEnum::kDown,
        bResetConditionCommandSupported);
    gActivatedCarbonFilterInstance->Init();
}

void emberAfHepaFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(!gHepaFilterInstance && !gHepaFilterDelegate);

    gHepaFilterDelegate                  = std::make_unique<HepaFilterMonitoringDelegate>();
    bool bResetConditionCommandSupported = true; // The ResetCondition command is supported by the ResourceMonitor cluster
    gHepaFilterInstance                  = std::make_unique<ResourceMonitoring::Instance>(
        gHepaFilterDelegate.get(), endpoint, HepaFilterMonitoring::Id, static_cast<uint32_t>(gHepaFilterFeatureMap.Raw()),
        ResourceMonitoring::DegradationDirectionEnum::kDown, bResetConditionCommandSupported);
    gHepaFilterInstance->Init();
}

CHIP_ERROR ImmutableReplacementProductListManager::Next(ReplacementProductStruct & item)
{
    if (mIndex >= kReplacementProductListMaxSize)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    switch (mIndex)
    {
    case 0:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kUpc);
        item.SetProductIdentifierValue(CharSpan::fromCharString("111112222233"));
        break;
    case 1:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin8xxx"));
        break;
    case 2:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kEan);
        item.SetProductIdentifierValue(CharSpan::fromCharString("4444455555666"));
        break;
    case 3:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin14xxxxxxxx"));
        break;
    case 4:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kOem);
        item.SetProductIdentifierValue(CharSpan::fromCharString("oem20xxxxxxxxxxxxxxx"));
        break;
    default:
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    mIndex++;
    return CHIP_NO_ERROR;
}
