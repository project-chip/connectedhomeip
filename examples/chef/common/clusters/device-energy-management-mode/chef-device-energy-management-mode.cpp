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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <clusters/mode-base/chef-mode-base-default.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>

#ifdef MATTER_DM_PLUGIN_DEVICE_ENERGY_MANAGEMENT_MODE_SERVER

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;
using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

namespace {

constexpr uint8_t kModeAuto = 0;

static const ModeTagStructType kModeTagsAuto[] = { { .value = to_underlying(DeviceEnergyManagementMode::ModeTag::kAuto) } };

static const detail::Structs::ModeOptionStruct::Type kModeOptions[] = {
    { .label = "Auto"_span, .mode = kModeAuto, .modeTags = DataModel::List<const ModeTagStructType>(kModeTagsAuto) },
};

static const Span<const detail::Structs::ModeOptionStruct::Type> kSupportedModes(kModeOptions);

static DefaultChefDelegate gDeviceEnergyManagementModeDelegate(kSupportedModes);
static std::unique_ptr<Instance> gDeviceEnergyManagementModeInstance;

void MatterDeviceEnergyManagementModeClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1);
    VerifyOrDie(!gDeviceEnergyManagementModeInstance);
    gDeviceEnergyManagementModeInstance =
        std::make_unique<Instance>(&gDeviceEnergyManagementModeDelegate, endpointId, DeviceEnergyManagementMode::Id, 0);
    TEMPORARY_RETURN_IGNORED gDeviceEnergyManagementModeInstance->Init();
    ChipLogProgress(DeviceLayer, "DeviceEnergyManagementMode initialized on endpoint %u", endpointId);
}

void MatterDeviceEnergyManagementModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType)
{
    VerifyOrDie(endpointId == 1);
    if (gDeviceEnergyManagementModeInstance)
    {
        gDeviceEnergyManagementModeInstance->Shutdown();
        gDeviceEnergyManagementModeInstance.reset();
    }
    DeviceEnergyManagementMode::Shutdown();
}

} // namespace

#else

void MatterDeviceEnergyManagementModeClusterInitCallback(chip::EndpointId endpointId) {}
void MatterDeviceEnergyManagementModeClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType) {}

#endif // MATTER_DM_PLUGIN_DEVICE_ENERGY_MANAGEMENT_MODE_SERVER
