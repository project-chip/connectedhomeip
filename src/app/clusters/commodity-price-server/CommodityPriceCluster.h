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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/CommodityPrice/ClusterId.h>
#include <clusters/CommodityPrice/Enums.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

class CommodityPriceCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a CommodityPrice cluster instance.
     * @param endpointId The endpoint on which this cluster exists.
     * @param features The FeatureMap value for this instance.
     */
    CommodityPriceCluster(EndpointId endpointId, BitMask<Feature> features) :
        DefaultServerCluster({ endpointId, CommodityPrice::Id }), mFeatures(features)
    {}

    bool HasFeature(Feature feature) const { return mFeatures.Has(feature); }

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    const BitMask<Feature> mFeatures;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
