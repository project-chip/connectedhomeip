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

#include <app/clusters/electrical-alarm-server/CodegenIntegration.h>

#include <clusters/ElectricalAlarm/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters::ElectricalAlarm {

/// Create and register the Electrical Alarm cluster on `endpointId` with `features`. The caller
/// chooses them, as the Power Topology stub next door does, so the app states what the endpoint
/// offers rather than burying it here. At least one alarm class is required; the Electrical Circuit
/// Breaker device type makes OverCurrent mandatory wherever this cluster appears.
/// Call from ApplicationInit().
CHIP_ERROR ElectricalAlarmInit(EndpointId endpointId, BitMask<Feature> features);

/// Unregister and destroy the cluster. Call from ApplicationShutdown().
void ElectricalAlarmShutdown();

} // namespace chip::app::Clusters::ElectricalAlarm
