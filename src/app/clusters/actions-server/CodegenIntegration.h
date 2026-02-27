/*
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

#pragma once

#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

/**
 * Initialize the Actions cluster for a specific endpoint.
 * This is called during Matter initialization to register the Actions cluster.
 *
 * @param endpointId The endpoint ID to initialize the Actions cluster for.
 */
void MatterActionsClusterInitCallback(EndpointId endpointId);

/**
 * Shutdown the Actions cluster for a specific endpoint.
 * This is called during Matter shutdown to unregister the Actions cluster.
 *
 * @param endpointId The endpoint ID to shutdown the Actions cluster for.
 * @param shutdownType The type of shutdown being performed.
 */
void MatterActionsClusterShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType);

/**
 * Initialize the Actions plugin.
 * This is called once during Matter initialization to set up global state.
 */
void MatterActionsPluginServerInitCallback();

/**
 * Shutdown the Actions plugin.
 * This is called once during Matter shutdown to clean up global state.
 */
void MatterActionsPluginServerShutdownCallback();

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
