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

#pragma once

#include "PowerSourceCluster.h"

#include <app/server-cluster/SingleEndpointServerClusterRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {

// A minimal wrapper for creating and registering a `PowerSource` cluster instance.
// The instance is `lazy` because it doesn't construct a `PowerSourceCluster` when created.
// `PowerSourceCluster` is created with the `Create` method.
struct LazyInstance
{
    LazyRegisteredServerCluster<PowerSourceCluster> server;
    void Create(EndpointId endpointId, Span<const AttributeId> optionalAttributes, const PowerSourceCluster::WiredConfiguration & config);
    void Create(EndpointId endpointId, Span<const AttributeId> optionalAttributes, const PowerSourceCluster::BatteryConfiguration & config);
    CHIP_ERROR Register();
    CHIP_ERROR Unregister();
    PowerSourceCluster & Cluster();
    const PowerSourceCluster & Cluster() const;
};

PowerSourceCluster * FindClusterOnEndpoint(EndpointId id);

} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip
