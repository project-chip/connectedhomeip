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
#include <resource-monitoring-instances.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

constexpr std::bitset<4> gHepaFilterFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                static_cast<uint32_t>(Feature::kWarning) |
                                                static_cast<uint32_t>(Feature::kReplacementProductList) };
constexpr std::bitset<4> gActivatedCarbonFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                     static_cast<uint32_t>(Feature::kWarning) |
                                                     static_cast<uint32_t>(Feature::kReplacementProductList) };

static HepaFilterMonitoringInstance * gHepaFilterInstance                       = nullptr;
static ActivatedCarbonFilterMonitoringInstance * gActivatedCarbonFilterInstance = nullptr;

static ResourceMonitoring::Attributes::ReplacementProductStruct::Type sReplacementProductsList[] = {
    { .productIdentifierType = ProductIdentifierTypeEnum::kUpc, .productIdentifierValue = CharSpan("upc12xxxxxxx") },
    { .productIdentifierType = ProductIdentifierTypeEnum::kGtin8, .productIdentifierValue = CharSpan("gtin8xxx") },
    { .productIdentifierType = ProductIdentifierTypeEnum::kEan, .productIdentifierValue = CharSpan("ean13xxxxxxxx") },
    { .productIdentifierType = ProductIdentifierTypeEnum::kGtin14, .productIdentifierValue = CharSpan("gtin14xxxxxxxx") },
    { .productIdentifierType = ProductIdentifierTypeEnum::kOem, .productIdentifierValue = CharSpan("oem20xxxxxxxxxxxxxxx") },
};
StaticReplacementProductListManager sReplacementProductListManager(
    &sReplacementProductsList[0],
    sizeof(sReplacementProductsList) / sizeof(ResourceMonitoring::Attributes::ReplacementProductStruct::Type));

//-- Activated Carbon Filter Monitoring Instance methods
CHIP_ERROR ActivatedCarbonFilterMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");
    SetReplacementProductListManagerInstance(&sReplacementProductListManager);
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

//-- Hepa Filter Monitoring instance methods
CHIP_ERROR HepaFilterMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringInstance::Init()");
    SetReplacementProductListManagerInstance(&sReplacementProductListManager);
    return CHIP_NO_ERROR;
}

Status HepaFilterMonitoringInstance::PreResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringInstance::PreResetCondition()");
    return Status::Success;
}

Status HepaFilterMonitoringInstance::PostResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringInstance::PostResetCondition()");
    return Status::Success;
}

void emberAfActivatedCarbonFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(gActivatedCarbonFilterInstance == nullptr);
    gActivatedCarbonFilterInstance = new ActivatedCarbonFilterMonitoringInstance(
        endpoint, static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()), DegradationDirectionEnum::kDown, true);
    gActivatedCarbonFilterInstance->Init();
}
void emberAfHepaFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    gHepaFilterInstance = new HepaFilterMonitoringInstance(endpoint, static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                                                           DegradationDirectionEnum::kDown, true);
    gHepaFilterInstance->Init();
}

CHIP_ERROR StaticReplacementProductListManager::Next(Attributes::ReplacementProductStruct::Type & item)
{
    ChipLogDetail(Zcl, "StaticReplacementProductListManager::Next()");

    if (mReplacementProductListSize > mIndex)
    {
        item = mReplacementProductsList[mIndex];
        mIndex++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
