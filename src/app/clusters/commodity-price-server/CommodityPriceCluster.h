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

/// Commands that this cluster supports only if the application opts into them.
///
/// Both CommodityPrice commands are optionally conformant. GetDetailedForecastRequest is
/// additionally gated on the Forecasting (FORE) feature.
enum class OptionalCommands : uint32_t
{
    kSupportsGetDetailedPriceRequest    = 0x1,
    kSupportsGetDetailedForecastRequest = 0x2
};

class CommodityPriceCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a CommodityPrice cluster instance.
     * @param endpointId The endpoint on which this cluster exists.
     * @param features The FeatureMap value for this instance.
     * @param optionalCommands The optionally conformant commands this instance supports.
     */
    CommodityPriceCluster(EndpointId endpointId, BitMask<Feature> features, BitMask<OptionalCommands> optionalCommands) :
        DefaultServerCluster({ endpointId, CommodityPrice::Id }), mFeatures(features), mOptionalCommands(optionalCommands)
    {}

    bool HasFeature(Feature feature) const { return mFeatures.Has(feature); }

    /// GetDetailedPriceRequest and its mandatory GetDetailedPriceResponse are supported together.
    bool SupportsDetailedPrice() const { return mOptionalCommands.Has(OptionalCommands::kSupportsGetDetailedPriceRequest); }

    /// GetDetailedForecastRequest is optionally conformant only when FORE is supported. Its
    /// GetDetailedForecastResponse is mandatory whenever the request is supported.
    bool SupportsDetailedForecast() const
    {
        return HasFeature(Feature::kForecasting) && mOptionalCommands.Has(OptionalCommands::kSupportsGetDetailedForecastRequest);
    }

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
    const BitMask<OptionalCommands> mOptionalCommands;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
