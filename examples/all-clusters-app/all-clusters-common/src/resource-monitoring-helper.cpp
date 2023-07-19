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
#include <instances/ActivatedCarbonFilterMonitoring.h>
#include <instances/HepaFilterMonitoring.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;

constexpr std::bitset<4> gHepaFilterFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                static_cast<uint32_t>(Feature::kWarning) };
constexpr std::bitset<4> gActivatedCarbonFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                     static_cast<uint32_t>(Feature::kWarning) };

static HepaFilterMonitoringInstance * gHepafilterInstance                       = nullptr;
static ActivatedCarbonFilterMonitoringInstance * gActivatedCarbonFilterInstance = nullptr;

void emberAfActivatedCarbonFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(gActivatedCarbonFilterInstance == nullptr);
    gActivatedCarbonFilterInstance = new ActivatedCarbonFilterMonitoringInstance(
        endpoint, static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()), DegradationDirectionEnum::kDown, true);
    gActivatedCarbonFilterInstance->Init();
}
void emberAfHepaFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(gActivatedCarbonFilterInstance == nullptr);
    gHepafilterInstance = new HepaFilterMonitoringInstance(endpoint, static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                                                           DegradationDirectionEnum::kDown, true);
    gHepafilterInstance->Init();
}
