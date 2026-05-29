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

#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>
#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * @brief Codegen integration helper for the Proximity Ranging cluster.
 *
 * Owns a single ProximityRangingCluster instance bound to an endpoint and
 * registers it with the codegen data model provider. The driver reference
 * supplied to this server must outlive the server.
 *
 * Construction does not register the cluster; callers must invoke Init()
 * (typically from the application's post-Server::Init hook) so the cluster
 * is ready before the IM event loop processes traffic.
 */
class ProximityRangingServer
{
public:
    /**
     * @param endpointId Endpoint hosting the Proximity Ranging cluster. Must
     *                   match the .zap configuration.
     * @param driver     Reference used by the cluster for all technology-specific
     *                   operations. Must outlive this object.
     */
    ProximityRangingServer(EndpointId endpointId, ProximityRangingDriver & driver) : mEndpointId(endpointId), mDriver(driver) {}

    ~ProximityRangingServer();

    /**
     * Construct the underlying cluster with the supplied feature set and register
     * it with the codegen data model provider.
     */
    CHIP_ERROR Init(BitMask<Feature> features);

    /**
     * Unregister and tear down the underlying cluster. Safe to call if Init() was
     * never called or has already been deinitialised.
     */
    void Deinit();

private:
    const EndpointId mEndpointId;
    ProximityRangingDriver & mDriver;
    LazyRegisteredServerCluster<ProximityRangingCluster> mCluster;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
