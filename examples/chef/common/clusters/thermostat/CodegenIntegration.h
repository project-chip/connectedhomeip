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

#pragma once

#include <app/util/config.h>

#if MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

#include "chef-thermostat.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Chef {

void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatDelegate & delegate);
void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatSetpointsDelegate & delegate);
void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatHoldDelegate & delegate);
void LoadDefaultsFromZAP(EndpointId endpointId, ChefThermostatOccupancyDelegate & delegate);
BitFlags<Feature> LoadFeaturesFromZAP(EndpointId endpointId);

CHIP_ERROR InitThermostatClusterOnEndpoint(EndpointId endpointId);

ThermostatClusterBase * FindClusterOnEndpoint(EndpointId endpointId);

Protocols::InteractionModel::Status SetOccupancy(EndpointId endpointId, BitMask<OccupancyBitmap> occupancy);

} // namespace Chef
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
