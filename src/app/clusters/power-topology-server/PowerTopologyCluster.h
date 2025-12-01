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

#include "PowerTopologyDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>

namespace chip::app::Clusters::PowerTopology {

using OptionalAttributesSet = OptionalAttributeSet< //
    Attributes::AvailableEndpoints::Id,             //
    Attributes::ActiveEndpoints::Id                 //
    >;

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
        DefaultServerCluster({ config.endpointId, PowerTopology::Id }), mDelegate(config.delegate), mFeatureFlags(config.features),
        mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attributes;
            attributes.Set<Attributes::AvailableEndpoints::Id>(config.features.Has(Feature::kSetTopology));
            attributes.Set<Attributes::ActiveEndpoints::Id>(config.features.Has(Feature::kDynamicPowerFlow));
            return attributes;
        }())
    {}

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
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
    BitMask<PowerTopology::Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
};

} // namespace chip::app::Clusters::PowerTopology
