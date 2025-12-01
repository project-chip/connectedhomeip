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
#pragma once

#include <lib/core/Optional.h>

#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/clusters/power-topology-server/PowerTopologyDelegate.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

enum class OptionalAttributes : uint32_t
{
    kOptionalAttributeAvailableEndpoints = 0x1,
    kOptionalAttributeActiveEndpoints    = 0x2,
};

class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature,
             BitMask<OptionalAttributes> aOptionalAttributes) :
        mFeature(aFeature),
        mOptionalAttrs(aOptionalAttributes), mCluster(PowerTopologyCluster::Config{
                                                 .endpointId = aEndpointId,
                                                 .delegate   = aDelegate,
                                                 .features   = aFeature,
                                             })
    {}
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

private:
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;
    RegisteredServerCluster<PowerTopologyCluster> mCluster;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAvailableEndpoints(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadActiveEndpoints(AttributeValueEncoder & aEncoder);
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
