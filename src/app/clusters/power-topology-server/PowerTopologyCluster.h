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

#include <app/clusters/power-topology-server/PowerTopologyDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/PowerTopology/AttributeIds.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <clusters/PowerTopology/Enums.h>

namespace chip::app::Clusters::PowerTopology {

class PowerTopologyCluster : public DefaultServerCluster
{

public:
    struct Config
    {
        EndpointId endpointId;
        Delegate & delegate;
        BitMask<Feature> features;
    };

    PowerTopologyCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, PowerTopology::Id }), mDelegate(config.delegate), mFeatureFlags(config.features)
    {}

    const BitFlags<PowerTopology::Feature> & Features() const { return mFeatureFlags; }

    // Getters - return copies with error checking
    CHIP_ERROR GetAvailableEndpoints(AttributeValueEncoder & aEncoder) const;
    CHIP_ERROR GetActiveEndpoints(AttributeValueEncoder & aEncoder) const;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    PowerTopology::Delegate & mDelegate;
    const BitMask<PowerTopology::Feature> mFeatureFlags;
};

} // namespace chip::app::Clusters::PowerTopology
