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

#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/CommodityPrice/ClusterId.h>
#include <clusters/CommodityPrice/Enums.h>
#include <clusters/CommodityPrice/Structs.h>
#include <clusters/shared/Enums.h>
#include <clusters/shared/Structs.h>
#include <lib/support/BitMask.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

#include <cstddef>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

// Spec-defined constraints.
inline constexpr size_t kMaxForecastEntries         = 56;
inline constexpr size_t kMaxDescriptionLength       = 32;
inline constexpr size_t kMaxComponentsPerPriceEntry = 10;
inline constexpr uint16_t kMaxCurrencyValue         = 999;

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
     * @brief Attribute setters.
     *
     * All of these reject values that violate a spec constraint with
     * CHIP_IM_GLOBAL_STATUS(ConstraintError), leaving the stored value unchanged.
     */

    CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum tariffUnit);

    CHIP_ERROR SetCurrency(const DataModel::Nullable<Globals::Structs::CurrencyStruct::Type> & currency);

    /// Sets CurrentPrice and generates a PriceChange event.
    ///
    /// `currentPrice` is deep copied, so the caller keeps ownership of the description and components
    /// it references.
    CHIP_ERROR SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & currentPrice);

    /// Sets PriceForecast, which requires the Forecasting feature. Entries are deep copied.
    CHIP_ERROR SetForecast(Span<const Structs::CommodityPriceStruct::Type> priceForecast);

    /// Generates a PriceChange event carrying CurrentPrice without its description or components.
    CHIP_ERROR GeneratePriceChangeEvent();

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

private:
    /// Owns a deep copy of a set of CommodityPriceStructs.
    ///
    /// A price references its description and its components by span, so the storage has to outlive
    /// the caller's value. Every description of every entry shares one character buffer and every
    /// component shares one array, which keeps a set of any size down to three allocations.
    class PriceStorage
    {
    public:
        /// Replaces the contents with a deep copy of `prices`.
        ///
        /// Returns CHIP_IM_GLOBAL_STATUS(ConstraintError) without touching the stored value if any
        /// entry is invalid.
        ///
        /// Returns CHIP_ERROR_NO_MEMORY if the copy does not fit in memory, in which case the
        /// storage is left EMPTY rather than holding its previous contents: keeping the old copy
        /// alive while building the new one would double the peak allocation of a full forecast,
        /// which is not a good trade on a constrained device. The attribute value therefore does
        /// change on failure, and the caller owes a NotifyAttributeChanged for it.
        CHIP_ERROR Set(Span<const Structs::CommodityPriceStruct::Type> prices);

        void Clear();

        Span<const Structs::CommodityPriceStruct::Type> Get() const { return { mPrices.Get(), mPriceCount }; }

    private:
        /// Copies `description` into the character buffer at `offset`, advancing it past the copy.
        CharSpan CopyDescription(CharSpan description, size_t & offset);

        Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> mPrices;
        Platform::ScopedMemoryBuffer<Structs::CommodityPriceComponentStruct::Type> mComponents;
        Platform::ScopedMemoryBuffer<char> mDescriptions;
        size_t mPriceCount = 0;
    };

    /// CurrentPrice keeping only the optional members selected by `details`. Null if none is set.
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> GetCurrentPrice(BitMask<CommodityPriceDetailBitmap> details) const;

    std::optional<DataModel::ActionReturnStatus> HandleGetDetailedPriceRequest(const ConcreteCommandPath & path,
                                                                               BitMask<CommodityPriceDetailBitmap> details,
                                                                               CommandHandler * handler) const;

    std::optional<DataModel::ActionReturnStatus> HandleGetDetailedForecastRequest(const ConcreteCommandPath & path,
                                                                                  BitMask<CommodityPriceDetailBitmap> details,
                                                                                  CommandHandler * handler) const;

    const BitMask<Feature> mFeatures;
    const BitMask<OptionalCommands> mOptionalCommands;

    Globals::TariffUnitEnum mTariffUnit = Globals::TariffUnitEnum::kKWh;
    DataModel::Nullable<Globals::Structs::CurrencyStruct::Type> mCurrency;

    // CurrentPrice and PriceForecast are stored with their optional description and components. Both
    // are stripped when the values are read as attributes, and are filtered by the requested detail
    // level when returned by GetDetailedPriceRequest / GetDetailedForecastRequest.
    PriceStorage mCurrentPrice; // holds no price while the attribute is null, one otherwise
    PriceStorage mPriceForecast;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
