/*
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

#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/// Get access to the underlying cluster registered on the given endpoint.
/// Used for registering the required ProximityRangingDriver with the cluster.
ProximityRangingCluster * FindClusterOnEndpoint(EndpointId endpointId);

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
