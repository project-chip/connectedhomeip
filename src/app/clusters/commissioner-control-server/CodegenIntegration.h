/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/clusters/commissioner-control-server/CommissionerControlCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip::app::Clusters {

class CommissionerControlServer
{
public:
    /**
     * Creates a commissioner control server instance. This is just a backwards compatibility wrapper around the
     * CommissionerControlCluster.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CommissionerControlServer(EndpointId endpointId, CommissionerControl::Delegate * delegate);
    ~CommissionerControlServer();

    /**
     * Register the commissioner control cluster instance with the codegen data model provider.
     * @return Returns an error if registration fails.
     */
    CHIP_ERROR Init();

    // The Code Driven CommissionerControlCluster instance (lazy-initialized)
    chip::app::LazyRegisteredServerCluster<CommissionerControlCluster> mCluster;

private:
    EndpointId mEndpointId;
    CommissionerControl::Delegate * mDelegate;
};

} // namespace chip::app::Clusters
