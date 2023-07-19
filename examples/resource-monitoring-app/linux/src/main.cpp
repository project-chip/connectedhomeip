/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppMain.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <bitset>
#include <instances/ActivatedCarbonFilterMonitoring.h>
#include <instances/HepaFilterMonitoring.h>
#include <stdint.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;

constexpr std::bitset<4> gHepaFilterFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                static_cast<uint32_t>(Feature::kWarning) };
constexpr std::bitset<4> gActivatedCarbonFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                     static_cast<uint32_t>(Feature::kWarning) };

static HepaFilterMonitoringInstance gHepaFilterInstance(0x1, static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                                                        Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown, true);
static ActivatedCarbonFilterMonitoringInstance
    gActivatedCarbonFilterInstance(0x1, static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()),
                                   Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown, true);

void ApplicationInit()
{
    gHepaFilterInstance.Init();
    gActivatedCarbonFilterInstance.Init();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
