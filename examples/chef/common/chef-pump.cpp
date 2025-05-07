/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "chef-pump.h"
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER

namespace chef {
namespace pump {

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
std::unique_ptr<DataModel::Nullable<uint8_t>> gLevel[MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

void handleMoveToLevel(EndpointId endpoint, uint8_t level)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleMoveToLevel");
}
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER

#ifdef MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
std::unique_ptr<DataModel::Nullable<int16_t>> gTemperature[MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif // MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER

#ifdef MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER
std::unique_ptr<DataModel::Nullable<int16_t>> gPressure[MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif // MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER

#ifdef MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
std::unique_ptr<DataModel::Nullable<int16_t>> gPressure[MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER

void handleOnOff(EndpointId endpoint, bool value)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleOnOff");
}

} // namespace pump
} // namespace chef

#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
