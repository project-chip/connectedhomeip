/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <clusters/mode-base/chef-mode-base-default.h>
#include <clusters/water-heater-mode/chef-water-heater-mode.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>

#if MATTER_DM_WATER_HEATER_MODE_CLUSTER_SERVER_ENDPOINT_COUNT > 0

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;

namespace {

static DefaultChefDelegate gWaterHeaterModeDelegate(WaterHeaterMode::Chef::kSupportedModes);
static std::unique_ptr<Instance> gWaterHeaterModeInstance;

} // namespace

void MatterWaterHeaterModeClusterInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1);
    VerifyOrDie(!gWaterHeaterModeInstance);
    gWaterHeaterModeInstance = std::make_unique<Instance>(&gWaterHeaterModeDelegate, endpointId, WaterHeaterMode::Id, 0);
    TEMPORARY_RETURN_IGNORED gWaterHeaterModeInstance->Init();
    ChipLogProgress(DeviceLayer, "WaterHeaterMode initialized on endpoint %u", endpointId);
}

void MatterWaterHeaterModeClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1);
    if (gWaterHeaterModeInstance)
    {
        gWaterHeaterModeInstance->Shutdown();
        gWaterHeaterModeInstance.reset();
    }
}

#endif // MATTER_DM_WATER_HEATER_MODE_CLUSTER_SERVER_ENDPOINT_COUNT > 0
