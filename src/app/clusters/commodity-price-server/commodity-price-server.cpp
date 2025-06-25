/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "commodity-price-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityPrice;
using namespace chip::app::Clusters::CommodityPrice::Attributes;
using namespace chip::app::Clusters::CommodityPrice::Structs;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> priceStruct;
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> forecastBuffer;
    DataModel::List<const Structs::CommodityPriceStruct::Type> forecastList;

    switch (aPath.mAttributeId)
    {
    case TariffUnit::Id:
        return aEncoder.Encode(mTariffUnit);
    case Currency::Id:
        return aEncoder.Encode(mCurrency);
    case CurrentPrice::Id:
        // Call GetDetailedPriceRequest with details = 0 to strip out .components and .description if present
        ReturnErrorOnFailure(GetDetailedPriceRequest(0, priceStruct));
        err = aEncoder.Encode(priceStruct);

        return err;
    case PriceForecast::Id:
        /* FORE - Forecasting */
        if (!HasFeature(Feature::kForecasting))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        // Call GetDetailedForecastRequest with details = 0 to
        // strip out .components and .description if present
        ReturnErrorOnFailure(GetDetailedForecastRequest(0, forecastBuffer, forecastList, false));

        err = aEncoder.EncodeList([=](const auto & encoder) -> CHIP_ERROR {
            for (auto const & entry : forecastList)
            {
                ReturnErrorOnFailure(encoder.Encode(entry));
            }
            return CHIP_NO_ERROR;
        });

        return err;

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Commands;

    VerifyOrExit(callback(GetDetailedPriceRequest::Id, context) == Loop::Continue, /**/);

    if (HasFeature(Feature::kForecasting))
    {
        VerifyOrExit(callback(GetDetailedForecastRequest::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case GetDetailedPriceRequest::Id:
        HandleCommand<GetDetailedPriceRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleGetDetailedPriceRequest(ctx, commandData); });
        return;
    case GetDetailedForecastRequest::Id:
        if (!HasFeature(Feature::kForecasting))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<GetDetailedForecastRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleGetDetailedForecastRequest(ctx, commandData); });
        }
        return;
    }
}

void Instance::HandleGetDetailedPriceRequest(HandlerContext & ctx,
                                             const Commands::GetDetailedPriceRequest::DecodableType & commandData)
{
    BitMask<CommodityPriceDetailBitmap> details = commandData.details;
    if (!details.HasOnly(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents))
    {
        ChipLogError(Zcl, "Invalid details bitmap recvd in GetDetailedPriceRequest");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Commands::GetDetailedPriceResponse::Type response;

    CHIP_ERROR err = GetDetailedPriceRequest(details, response.currentPrice);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    }
    else
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
}

void Instance::HandleGetDetailedForecastRequest(HandlerContext & ctx,
                                                const Commands::GetDetailedForecastRequest::DecodableType & commandData)
{
    if (!HasFeature(Feature::kForecasting))
    {
        ChipLogError(Zcl, "GetDetailedForecastRequest not supported");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    BitMask<CommodityPriceDetailBitmap> details = commandData.details;
    if (!details.HasOnly(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents))
    {
        ChipLogError(Zcl, "Invalid details bitmap recvd in GetDetailedForecastRequest");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> forecastBuffer;
    Commands::GetDetailedForecastResponse::Type response;
    CHIP_ERROR err = GetDetailedForecastRequest(details, forecastBuffer, response.priceForecast, true);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    else
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
}

CHIP_ERROR
Instance::GetDetailedPriceRequest(BitMask<CommodityPriceDetailBitmap> details,
                                  DataModel::Nullable<Structs::CommodityPriceStruct::Type> & priceStruct)
{

    if (mCurrentPrice.IsNull())
    {
        priceStruct.SetNull();
    }
    else
    {
        priceStruct = mCurrentPrice;
        if (!details.Has(CommodityPriceDetailBitmap::kComponents))
        {
            // Remove the components
            priceStruct.Value().components.ClearValue();
        }

        if (!details.Has(CommodityPriceDetailBitmap::kDescription))
        {
            // Remove the description
            priceStruct.Value().description.ClearValue();
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
Instance::GetDetailedForecastRequest(BitMask<CommodityPriceDetailBitmap> details,
                                     Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> & forecastBuffer,
                                     DataModel::List<const Structs::CommodityPriceStruct::Type> & forecastList, bool isCommand)
{

    size_t count      = 0;
    size_t bufferSize = mPriceForecast.size();

    if (bufferSize == 0)
    {
        /* Special case when no forecast entries exist - calling calloc(0) returns NULL
           and results in an error on some platforms */
        forecastList = DataModel::List<const Structs::CommodityPriceStruct::Type>();

        return CHIP_NO_ERROR;
    }

    if (!forecastBuffer.Calloc(bufferSize))
    {
        ChipLogError(AppServer, "Memory allocation failed for forecast buffer");
        return CHIP_ERROR_NO_MEMORY;
    }

    for (const auto & srcPrice : mPriceForecast)
    {
        if (count >= bufferSize)
            break; // Avoid overflow

        Structs::CommodityPriceStruct::Type copy = srcPrice;

        if (!details.Has(CommodityPriceDetailBitmap::kComponents))
        {
            copy.components.ClearValue();
        }

        if (!details.Has(CommodityPriceDetailBitmap::kDescription))
        {
            copy.description.ClearValue();
        }

        forecastBuffer[count++] = copy;
    }

    // Now wrap in Span + List
    forecastList = DataModel::List<const Structs::CommodityPriceStruct::Type>(
        Span<Structs::CommodityPriceStruct::Type>(forecastBuffer.Get(), count));

    return CHIP_NO_ERROR;
}

// --------------- Internal Attribute Set APIs
CHIP_ERROR Instance::SetTariffUnit(TariffUnitEnum newValue)
{
    TariffUnitEnum oldValue = mTariffUnit;

    if (EnsureKnownEnumValue(newValue) == TariffUnitEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mTariffUnit = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "Endpoint: %d - mTariffUnit updated to %d", mEndpointId, to_underlying(mTariffUnit));
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, TariffUnit::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetCurrency(CurrencyStruct::Type newValue)
{
    CurrencyStruct::Type oldValue = mCurrency;

    // Check currency type is within limits
    if (newValue.currency > kMaxCurrencyValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mCurrency = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "Endpoint: %d - mCurrency updated to Currency: %d DecimalPoints: %d", mEndpointId,
                      mCurrency.currency, mCurrency.decimalPoints);
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, Currency::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> newValue)
{
    // Check PeriodStart  < PeriodEnd (if not null)
    if (!newValue.IsNull())
    {
        if (!newValue.Value().periodEnd.IsNull() && (newValue.Value().periodStart > newValue.Value().periodEnd.Value()))
        {
            return CHIP_ERROR_BAD_REQUEST;
        }

        if (!newValue.Value().price.HasValue() && !newValue.Value().priceLevel.HasValue())
        {
            // Must have Price or PriceLevel
            return CHIP_ERROR_BAD_REQUEST;
        }
    }

    // Do a deep copy of the newValue into mCurrentPrice
    ReturnErrorOnFailure(CopyPrice(newValue));

    ChipLogDetail(AppServer, "Endpoint: %d - mCurrentPrice updated", mEndpointId);
    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, CurrentPrice::Id);

    // generate a PriceChange Event
    GeneratePriceChangeEvent();

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::CopyCharSpan(const CharSpan src, Platform::ScopedMemoryBuffer<char> & bufferOut, CharSpan & spanOut)
{
    size_t len = src.size();
    if (bufferOut.Get() != nullptr)
    {
        bufferOut.Free();
    }

    if (!bufferOut.Calloc(len))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    memcpy(bufferOut.Get(), src.data(), len);
    spanOut = CharSpan(bufferOut.Get(), len);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::CopyPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> & src)
{

    // Free the priceStruct
    if (mOwnedCurrentPriceStructBuffer.Get() != nullptr)
    {
        mOwnedCurrentPriceStructBuffer.Free();
    }

    // Free the .components
    if (mOwnedCurrentPriceComponentBuffer.Get() != nullptr)
    {
        mOwnedCurrentPriceComponentBuffer.Free();
    }

    // The .description is held within a ScopedBuffer so the
    // CopyCharSpan() will take care of that for us

    // At this point we should have free'd all previous memory
    if (src.IsNull())
    {
        mCurrentPrice.SetNull();
    }
    else
    {
        // Do a basic copy of the CommodityPriceStruct trivial fields
        if (!mOwnedCurrentPriceStructBuffer.Calloc(1))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        auto mOwnedPriceStructPtr         = mOwnedCurrentPriceStructBuffer.Get();
        mOwnedPriceStructPtr->periodStart = src.Value().periodStart;
        mOwnedPriceStructPtr->periodEnd   = src.Value().periodEnd;
        mOwnedPriceStructPtr->price       = src.Value().price;
        mOwnedPriceStructPtr->priceLevel  = src.Value().priceLevel;

        // Deep copy description (if present)
        if (src.Value().description.HasValue())
        {
            CharSpan span;
            ReturnErrorOnFailure(CopyCharSpan(src.Value().description.Value(), mOwnedCurrentPriceDescriptionBuffer, span));
            mOwnedPriceStructPtr->description.SetValue(span);
        }

        // Deep copy the .components list (if present)
        if (src.Value().components.HasValue())
        {
            auto & components = src.Value().components.Value();
            if (!mOwnedCurrentPriceComponentBuffer.Calloc(components.size()))
            {
                return CHIP_ERROR_NO_MEMORY;
            }

            for (size_t i = 0; i < components.size(); i++)
            {
                // Do a copy for trivial types
                mOwnedCurrentPriceComponentBuffer[i] = components[i];

                // Components have an optional .description
                if (components[i].description.HasValue())
                {
                    CharSpan span;
                    auto desc = components[i].description.Value();
                    ReturnErrorOnFailure(CopyCharSpan(desc, mOwnedCurrentPriceComponentDescriptionBuffer[i], span));
                    mOwnedCurrentPriceComponentBuffer[i].description.SetValue(span);
                }
            }
            mOwnedPriceStructPtr->components.SetValue(
                Span<Structs::CommodityPriceComponentStruct::Type>(mOwnedCurrentPriceComponentBuffer.Get(), components.size()));
        }

        mCurrentPrice.SetNonNull(*mOwnedPriceStructPtr);
    }
    return CHIP_NO_ERROR;
}

Status Instance::GeneratePriceChangeEvent()
{
    Events::PriceChange::Type event;
    EventNumber eventNumber;

    /*
     * The Event's CurrentPrice must not include .description or .components
     * call our function to copy the CurrentPrice without these
     */
    CHIP_ERROR err = GetDetailedPriceRequest(0, event.currentPrice);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);

    err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PriceChange event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

CHIP_ERROR Instance::SetForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> & priceForecast)
{
    assertChipStackLockedByCurrentThread();

    // Do a deep copy of the newValue into mPriceForecast
    ReturnErrorOnFailure(CopyPriceForecast(priceForecast));

    ChipLogDetail(AppServer, "Endpoint %d - mPriceForecast updated", mEndpointId);

    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, PriceForecast::Id);

    return CHIP_NO_ERROR;
}

void Instance::CheckAndFreeForecastBuffers()
{
    // Free everything if allocated
    if (mOwnedForecastPriceStructBuffer.Get() != nullptr)
    {
        mOwnedForecastPriceStructBuffer.Free();
    }

    for (size_t i = 0; i < kMaxForecastEntries; i++)
    {
        if (mOwnedForecastPriceComponentBuffer[i].Get() != nullptr)
        {
            mOwnedForecastPriceComponentBuffer[i].Free();
        }

        for (size_t j = 0; j < kMaxComponentsPerPriceEntry; j++)
        {
            if (mOwnedForecastPriceComponentDescriptionBuffer[i][j].Get() != nullptr)
            {
                mOwnedForecastPriceComponentDescriptionBuffer[i][j].Free();
            }
        }

        if (mOwnedForecastPriceDescriptionBuffer[i].Get() != nullptr)
        {
            mOwnedForecastPriceDescriptionBuffer[i].Free();
        }
    }
}

CHIP_ERROR Instance::CopyPriceStructWithinForecast(
    Structs::CommodityPriceStruct::Type & destPriceStruct, Platform::ScopedMemoryBuffer<char> & dest_descriptionBuffer,
    Platform::ScopedMemoryBuffer<Structs::CommodityPriceComponentStruct::Type> & dest_componentsBuffer,
    Platform::ScopedMemoryBuffer<char> * dest_componentsDescriptionBuffer, const Structs::CommodityPriceStruct::Type & src)
{
    // Do a basic copy of the CommodityPriceStruct trivial fields
    destPriceStruct.periodStart = src.periodStart;
    destPriceStruct.periodEnd   = src.periodEnd;
    destPriceStruct.price       = src.price;
    destPriceStruct.priceLevel  = src.priceLevel;

    // Deep copy description (if present)
    if (src.description.HasValue())
    {
        CharSpan span;
        ReturnLogErrorOnFailure(CopyCharSpan(src.description.Value(), dest_descriptionBuffer, span));
        destPriceStruct.description.SetValue(span);
    }

    // Deep copy the .components list (if present)
    if (src.components.HasValue())
    {
        auto & components    = src.components.Value();
        size_t numComponents = components.size();
        if (numComponents > kMaxComponentsPerPriceEntry)
        {
            return CHIP_ERROR_BAD_REQUEST;
        }

        if (!dest_componentsBuffer.Calloc(numComponents))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        for (size_t j = 0; j < numComponents; j++)
        {
            // Do a copy for trivial types
            dest_componentsBuffer[j].price             = components[j].price;
            dest_componentsBuffer[j].source            = components[j].source;
            dest_componentsBuffer[j].tariffComponentID = components[j].tariffComponentID;

            // Components have an optional .description
            if (components[j].description.HasValue())
            {
                CharSpan span;
                auto desc = components[j].description.Value();
                ReturnLogErrorOnFailure(CopyCharSpan(desc, dest_componentsDescriptionBuffer[j], span));
                dest_componentsBuffer[j].description.SetValue(span);
            }
            else
            {
                dest_componentsBuffer[j].description.ClearValue();
            }
        }
        destPriceStruct.components.SetValue(
            Span<Structs::CommodityPriceComponentStruct::Type>(dest_componentsBuffer.Get(), components.size()));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::CopyPriceForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> & src)
{

    CheckAndFreeForecastBuffers();
    // At this point our local storage should be unallocated

    size_t entries = src.size();
    if (!mOwnedForecastPriceStructBuffer.Calloc(entries))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    for (size_t count = 0; count < entries; count++)
    {
        // Deep copy each PriceStruct in the src list
        ReturnLogErrorOnFailure(CopyPriceStructWithinForecast(
            mOwnedForecastPriceStructBuffer[count], mOwnedForecastPriceDescriptionBuffer[count],
            mOwnedForecastPriceComponentBuffer[count], &mOwnedForecastPriceComponentDescriptionBuffer[count][0], src[count]));
    }
    mPriceForecast = DataModel::List<const Structs::CommodityPriceStruct::Type>(
        Span<Structs::CommodityPriceStruct::Type>(mOwnedForecastPriceStructBuffer.Get(), entries));

    return CHIP_NO_ERROR;
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
