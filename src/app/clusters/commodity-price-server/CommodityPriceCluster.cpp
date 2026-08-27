/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "CommodityPriceCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/Commands.h>
#include <clusters/CommodityPrice/Events.h>
#include <clusters/CommodityPrice/Metadata.h>
#include <clusters/shared/EnumsCheck.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace Globals {
namespace Structs {
namespace CurrencyStruct {

inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.currency == rhs.currency) && (lhs.decimalPoints == rhs.decimalPoints);
}

} // namespace CurrencyStruct
} // namespace Structs
} // namespace Globals

namespace CommodityPrice {
namespace {

/// Attribute reads expose neither the description nor the components of a price.
constexpr BitMask<CommodityPriceDetailBitmap> kNoPriceDetails;

/// Returns a copy of `price` keeping only the optional members selected by `details`.
Structs::CommodityPriceStruct::Type FilterDetails(const Structs::CommodityPriceStruct::Type & price,
                                                  BitMask<CommodityPriceDetailBitmap> details)
{
    Structs::CommodityPriceStruct::Type filtered = price;

    if (!details.Has(CommodityPriceDetailBitmap::kDescription))
    {
        filtered.description.ClearValue();
    }

    if (!details.Has(CommodityPriceDetailBitmap::kComponents))
    {
        filtered.components.ClearValue();
    }

    return filtered;
}

/// The Details argument of both commands may only carry the two bits the spec defines.
bool IsValidDetails(BitMask<CommodityPriceDetailBitmap> details)
{
    const bool valid = details.HasOnly(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents);

    if (!valid)
    {
        ChipLogError(Zcl, "CommodityPrice: invalid Details 0x%02x", static_cast<unsigned>(details.Raw()));
    }

    return valid;
}

size_t DescriptionLength(const Optional<CharSpan> & description)
{
    return description.HasValue() ? description.Value().size() : 0;
}

/// Checks `price` against the constraints the spec places on a CommodityPriceStruct.
CHIP_ERROR ValidatePrice(const Structs::CommodityPriceStruct::Type & price)
{
    VerifyOrReturnError(price.periodEnd.IsNull() || (price.periodStart <= price.periodEnd.Value()),
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // A price has to carry a value of one kind or the other.
    VerifyOrReturnError(price.price.HasValue() || price.priceLevel.HasValue(), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(DescriptionLength(price.description) <= kMaxDescriptionLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (price.components.HasValue())
    {
        VerifyOrReturnError(price.components.Value().size() <= kMaxComponentsPerPriceEntry, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        for (const auto & component : price.components.Value())
        {
            VerifyOrReturnError(DescriptionLength(component.description) <= kMaxDescriptionLength,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR CommodityPriceCluster::PriceStorage::Set(Span<const Structs::CommodityPriceStruct::Type> prices)
{
    size_t componentCount   = 0;
    size_t descriptionChars = 0;

    // Validate and size the copy before dropping the current contents, so that a rejected value
    // leaves the stored one intact.
    for (const auto & price : prices)
    {
        ReturnErrorOnFailure(ValidatePrice(price));

        descriptionChars += DescriptionLength(price.description);

        if (price.components.HasValue())
        {
            componentCount += price.components.Value().size();
            for (const auto & component : price.components.Value())
            {
                descriptionChars += DescriptionLength(component.description);
            }
        }
    }

    // Past this point the previous contents are gone even if an allocation below fails. See the
    // declaration for why this is not made atomic.
    Clear();

    // Calloc(0) is not portable, so each buffer is only allocated if it holds anything. Running out
    // of memory part way through hands back whatever was taken before the failure: holding on to it
    // would strand the bulk of a forecast at the moment the device has none to spare.
    if ((!prices.empty() && !mPrices.Calloc(prices.size())) ||
        ((componentCount != 0) && !mComponents.Calloc(componentCount)) ||
        ((descriptionChars != 0) && !mDescriptions.Calloc(descriptionChars)))
    {
        Clear();
        return CHIP_ERROR_NO_MEMORY;
    }

    size_t componentOffset   = 0;
    size_t descriptionOffset = 0;

    for (size_t i = 0; i < prices.size(); i++)
    {
        const Structs::CommodityPriceStruct::Type & src = prices[i];
        Structs::CommodityPriceStruct::Type & dest      = mPrices[i];

        dest.periodStart = src.periodStart;
        dest.periodEnd   = src.periodEnd;
        dest.price       = src.price;
        dest.priceLevel  = src.priceLevel;

        if (src.description.HasValue())
        {
            dest.description.SetValue(CopyDescription(src.description.Value(), descriptionOffset));
        }

        if (src.components.HasValue())
        {
            const auto & components = src.components.Value();

            for (size_t j = 0; j < components.size(); j++)
            {
                Structs::CommodityPriceComponentStruct::Type & destComponent = mComponents[componentOffset + j];

                destComponent.price             = components[j].price;
                destComponent.source            = components[j].source;
                destComponent.tariffComponentID = components[j].tariffComponentID;

                if (components[j].description.HasValue())
                {
                    destComponent.description.SetValue(CopyDescription(components[j].description.Value(), descriptionOffset));
                }
            }

            const Structs::CommodityPriceComponentStruct::Type * destComponents =
                components.empty() ? nullptr : mComponents.Get() + componentOffset;

            dest.components.SetValue(
                DataModel::List<const Structs::CommodityPriceComponentStruct::Type>(destComponents, components.size()));
            componentOffset += components.size();
        }
    }

    mPriceCount = prices.size();

    return CHIP_NO_ERROR;
}

void CommodityPriceCluster::PriceStorage::Clear()
{
    mPrices.Free();
    mComponents.Free();
    mDescriptions.Free();
    mPriceCount = 0;
}

CharSpan CommodityPriceCluster::PriceStorage::CopyDescription(CharSpan description, size_t & offset)
{
    // Offsetting a null buffer is undefined even by zero, so there is nothing to copy into.
    VerifyOrReturnValue(!description.empty(), CharSpan());

    char * dest = mDescriptions.Get() + offset;

    memcpy(dest, description.data(), description.size());
    offset += description.size();

    return CharSpan(dest, description.size());
}

CHIP_ERROR CommodityPriceCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kForecasting), Attributes::PriceForecast::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR CommodityPriceCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;

    if (SupportsDetailedPrice())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedPriceRequest::kMetadataEntry }));
    }

    if (SupportsDetailedForecast())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedForecastRequest::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;

    // Each response is mandatory whenever its request is supported.
    if (SupportsDetailedPrice())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedPriceResponse::Id }));
    }

    if (SupportsDetailedForecast())
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetDetailedForecastResponse::Id }));
    }

    return CHIP_NO_ERROR;
}

DataModel::Nullable<Structs::CommodityPriceStruct::Type>
CommodityPriceCluster::GetCurrentPrice(BitMask<CommodityPriceDetailBitmap> details) const
{
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> currentPrice;
    Span<const Structs::CommodityPriceStruct::Type> stored = mCurrentPrice.Get();

    if (!stored.empty())
    {
        currentPrice.SetNonNull(FilterDetails(stored[0], details));
    }

    return currentPrice;
}

DataModel::ActionReturnStatus CommodityPriceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::TariffUnit::Id:
        return encoder.Encode(mTariffUnit);
    case Attributes::Currency::Id:
        return encoder.Encode(mCurrency);
    case Attributes::CurrentPrice::Id:
        return encoder.Encode(GetCurrentPrice(kNoPriceDetails));
    case Attributes::PriceForecast::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (const auto & entry : mPriceForecast.Get())
            {
                ReturnErrorOnFailure(listEncoder.Encode(FilterDetails(entry, kNoPriceDetails)));
            }
            return CHIP_NO_ERROR;
        });
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> CommodityPriceCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case GetDetailedPriceRequest::Id: {
        GetDetailedPriceRequest::DecodableType requestData;
        ReturnErrorOnFailure(requestData.Decode(input_arguments));
        return HandleGetDetailedPriceRequest(request.path, requestData.details, handler);
    }
    case GetDetailedForecastRequest::Id: {
        GetDetailedForecastRequest::DecodableType requestData;
        ReturnErrorOnFailure(requestData.Decode(input_arguments));
        return HandleGetDetailedForecastRequest(request.path, requestData.details, handler);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus>
CommodityPriceCluster::HandleGetDetailedPriceRequest(const ConcreteCommandPath & path, BitMask<CommodityPriceDetailBitmap> details,
                                                     CommandHandler * handler) const
{
    VerifyOrReturnValue(IsValidDetails(details), Status::InvalidCommand);

    Commands::GetDetailedPriceResponse::Type response;
    response.currentPrice = GetCurrentPrice(details);

    handler->AddResponse(path, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
CommodityPriceCluster::HandleGetDetailedForecastRequest(const ConcreteCommandPath & path,
                                                        BitMask<CommodityPriceDetailBitmap> details, CommandHandler * handler) const
{
    VerifyOrReturnValue(IsValidDetails(details), Status::InvalidCommand);

    Span<const Structs::CommodityPriceStruct::Type> forecast = mPriceForecast.Get();

    // Filtering means copying the forecast, so it only happens when something has to be left out.
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> filtered;
    if (!forecast.empty() && !details.HasAll(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents))
    {
        VerifyOrReturnValue(filtered.Calloc(forecast.size()), Status::ResourceExhausted);

        for (size_t i = 0; i < forecast.size(); i++)
        {
            filtered[i] = FilterDetails(forecast[i], details);
        }

        forecast = Span<const Structs::CommodityPriceStruct::Type>(filtered.Get(), forecast.size());
    }

    Commands::GetDetailedForecastResponse::Type response;
    response.priceForecast = DataModel::List<const Structs::CommodityPriceStruct::Type>(forecast);

    handler->AddResponse(path, response);

    return std::nullopt;
}

CHIP_ERROR CommodityPriceCluster::SetTariffUnit(Globals::TariffUnitEnum tariffUnit)
{
    VerifyOrReturnError(EnsureKnownEnumValue(tariffUnit) != Globals::TariffUnitEnum::kUnknownEnumValue,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    SetAttributeValue(mTariffUnit, tariffUnit, Attributes::TariffUnit::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::SetCurrency(const DataModel::Nullable<Globals::Structs::CurrencyStruct::Type> & currency)
{
    if (currency.IsNull())
    {
        SetAttributeValue(mCurrency, DataModel::NullNullable, Attributes::Currency::Id);
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(currency.Value().currency <= kMaxCurrencyValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (SetAttributeValue(mCurrency, currency.Value(), Attributes::Currency::Id))
    {
        ChipLogDetail(AppServer, "Endpoint %u - Currency updated to %u with %u decimal points", mPath.mEndpointId,
                      mCurrency.Value().currency, mCurrency.Value().decimalPoints);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & currentPrice)
{
    Span<const Structs::CommodityPriceStruct::Type> price;
    if (!currentPrice.IsNull())
    {
        price = Span<const Structs::CommodityPriceStruct::Type>(&currentPrice.Value(), 1);
    }

    CHIP_ERROR err = mCurrentPrice.Set(price);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        // Set dropped the price it held, so the attribute really did change to null. Reporting it
        // keeps subscribers from holding a value the cluster no longer has. No PriceChange event is
        // generated here because generating one allocates, which has just failed.
        NotifyAttributeChanged(Attributes::CurrentPrice::Id);
    }
    ReturnErrorOnFailure(err);

    // Comparing the previous value would mean a deep comparison of the copy, which is not worth the
    // flash it would cost, so every set is reported.
    NotifyAttributeChanged(Attributes::CurrentPrice::Id);

    // The attribute is already updated at this point, so a failure to generate the event is not
    // handed back to the caller.
    (void) GeneratePriceChangeEvent();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::SetForecast(Span<const Structs::CommodityPriceStruct::Type> priceForecast)
{
    VerifyOrReturnError(HasFeature(Feature::kForecasting), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(priceForecast.size() <= kMaxForecastEntries, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    CHIP_ERROR err = mPriceForecast.Set(priceForecast);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        // Set dropped the forecast it held, so the attribute really did change to an empty list.
        NotifyAttributeChanged(Attributes::PriceForecast::Id);
    }
    ReturnErrorOnFailure(err);

    NotifyAttributeChanged(Attributes::PriceForecast::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceCluster::GeneratePriceChangeEvent()
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // The event carries the price without its description or components.
    Events::PriceChange::Type event;
    event.currentPrice = GetCurrentPrice(kNoPriceDetails);

    VerifyOrReturnError(mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
