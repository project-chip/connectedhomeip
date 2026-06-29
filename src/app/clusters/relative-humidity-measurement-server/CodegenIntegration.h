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

#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>

namespace chip::app::Clusters::RelativeHumidityMeasurement {

/// Returns the cluster instance registered on the given endpoint, or nullptr if not found.
RelativeHumidityMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId);

/// Convenience helper — pushes a new measured value to the cluster on the given endpoint.
CHIP_ERROR SetMeasuredValue(EndpointId endpointId, DataModel::Nullable<uint16_t> measuredValue);

} // namespace chip::app::Clusters::RelativeHumidityMeasurement
