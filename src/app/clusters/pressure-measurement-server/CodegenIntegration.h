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

#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>

namespace chip::app::Clusters::PressureMeasurement {

/// Returns the cluster instance registered on the given endpoint, nullptr otherwise
PressureMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId);

/// Convenience helper to set new measured value
CHIP_ERROR SetMeasuredValue(EndpointId endpointId, DataModel::Nullable<int16_t> measuredValue);

/// Convenience helper to set new scaled value (requires EXT feature)
CHIP_ERROR SetScaledValue(EndpointId endpointId, DataModel::Nullable<int16_t> scaledValue);

} // namespace chip::app::Clusters::PressureMeasurement
