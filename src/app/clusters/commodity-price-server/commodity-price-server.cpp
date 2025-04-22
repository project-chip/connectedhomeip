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

        if (!forecastBuffer.Calloc(kMaxForecastEntries))
        {
            ChipLogError(AppServer, "Memory allocation failed for forecast buffer");
            return CHIP_ERROR_NO_MEMORY;
        }
        // Call GetDetailedForecastRequest with details = 0 to
        // strip out .components and .description if present
        ReturnErrorOnFailure(GetDetailedForecastRequest(0, forecastBuffer, kMaxForecastEntries, forecastList, false, false));

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
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> priceStruct;

    BitMask<CommodityPriceDetailBitmap> details = commandData.details;
    if (!details.HasOnly(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents))
    {
        ChipLogError(Zcl, "Invalid details bitmap recvd in GetDetailedPriceRequest");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Commands::GetDetailedPriceResponse::Type response;

    CHIP_ERROR err = GetDetailedPriceRequest(details, priceStruct);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    else
    {
        response.currentPrice = priceStruct;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
}

void Instance::HandleGetDetailedForecastRequest(HandlerContext & ctx,
                                                const Commands::GetDetailedForecastRequest::DecodableType & commandData)
{
    chip::Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> forecastBuffer;
    DataModel::List<const Structs::CommodityPriceStruct::Type> forecastList;

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

    if (!forecastBuffer.Calloc(kMaxForecastEntries))
    {
        ChipLogError(AppServer, "Memory allocation failed for forecast buffer");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
        return;
    }

    Commands::GetDetailedForecastResponse::Type response;
    CHIP_ERROR err = GetDetailedForecastRequest(details, forecastBuffer, kMaxForecastEntries, forecastList, false, true);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    else
    {
        response.priceForecast = forecastList;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
}

CHIP_ERROR
Instance::GetDetailedPriceRequest(chip::BitMask<CommodityPriceDetailBitmap> details,
                                  DataModel::Nullable<Structs::CommodityPriceStruct::Type> & priceStruct)
{

    if (mCurrentPrice.IsNull())
    {
        priceStruct.SetNull();
        return CHIP_NO_ERROR;
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
Instance::GetDetailedForecastRequest(chip::BitMask<CommodityPriceDetailBitmap> details,
                                     chip::Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> & forecastBuffer,
                                     size_t bufferSize, DataModel::List<const Structs::CommodityPriceStruct::Type> & forecastList,
                                     bool isEvent, bool isCommand)
{

    size_t count = 0;

    // TODO work out what the max udp packet size is
    // For events this needs to be about 500
    // Without events it can be 900
    size_t kMaxByteCount      = (isEvent) ? 500 : 900;
    size_t estimatedByteCount = 0;
    for (const auto & srcPrice : mPriceForecast)
    {
        if (count >= bufferSize)
            break; // Avoid overflow

        Structs::CommodityPriceStruct::Type copy = srcPrice;

        estimatedByteCount += 4 + 1; // periodStart (epoch_s)
        if (!copy.periodEnd.IsNull())
            estimatedByteCount += 4 + 1; // periodEnd (epoch_s) is optional
        if (copy.price.HasValue())
            estimatedByteCount += 8 + 1; // price (int64_t) is optional
        if (copy.priceLevel.HasValue())
            estimatedByteCount += 2 + 1; // priceLevel (int16_t) is optional

        if (!details.Has(CommodityPriceDetailBitmap::kComponents))
        {
            copy.components.ClearValue();
        }
        else
        {
            for (const auto & component : copy.components.Value())
            {
                if (component.tariffComponentID.HasValue())
                {
                    estimatedByteCount += 4 + 1;
                }
                if (component.description.HasValue())
                {
                    estimatedByteCount += component.description.Value().size() + 2;
                }
                // price is int64_t, tariffPriceEnum is unit8 + tags
                estimatedByteCount += 8 + 1 + 2;
            }
        }

        if (!details.Has(CommodityPriceDetailBitmap::kDescription))
        {
            copy.description.ClearValue();
        }
        else
        {
            if (copy.description.HasValue())
            {
                estimatedByteCount += copy.description.Value().size() + 2;
            }
        }

        if ((!isCommand && !isEvent) || (estimatedByteCount < kMaxByteCount))
        {
            // For Events and Commands we need to check the size,
            // for Attributes we can rely on chunking which needs
            // to be handled by the parent calling EncodeList
            forecastBuffer[count++] = copy;
        }
        else
        {
            // Packet is likely to be full now so stop
            break;
        }
    }

    // Now wrap in Span + List
    forecastList = DataModel::List<const Structs::CommodityPriceStruct::Type>(
        chip::Span<Structs::CommodityPriceStruct::Type>(forecastBuffer.Get(), count));

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
        ChipLogDetail(AppServer, "mTariffUnit updated to %d", static_cast<int>(mTariffUnit));
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
        ChipLogDetail(AppServer, "mCurrency updated to Currency: %d DecimalPoints: %d", static_cast<int>(mCurrency.currency),
                      static_cast<int>(mCurrency.decimalPoints));
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, Currency::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> newValue)
{
    // Check PeriodStart  < PeriodEnd (if not null)
    if (!newValue.IsNull())
    {
        Structs::CommodityPriceStruct::Type tempValue;
        if (!newValue.Value().periodEnd.IsNull() && (newValue.Value().periodStart > newValue.Value().periodEnd.Value()))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    mCurrentPrice = newValue;

    ChipLogDetail(AppServer, "mCurrentPrice updated");
    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, CurrentPrice::Id);

    // generate a PriceChange Event
    SendPriceChangeEvent();

    return CHIP_NO_ERROR;
}

Status Instance::SendPriceChangeEvent()
{

    CHIP_ERROR err;
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> priceStruct;
    Events::PriceChange::Type event;
    EventNumber eventNumber;

    /*
     * The Event's CurrentPrice must not include .description or .components
     * call our function to copy the CurrentPrice without these
     */
    err = GetDetailedPriceRequest(0, priceStruct);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);

    event.currentPrice = priceStruct;

    err = LogEvent(event, mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

CHIP_ERROR Instance::SetForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> & priceForecast)
{
    assertChipStackLockedByCurrentThread();

    mPriceForecast = priceForecast;

    ChipLogDetail(AppServer, "mPriceForecast updated");

    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, PriceForecast::Id);

    // generate a ForecastChange Event
    SendForecastChangeEvent();

    return CHIP_NO_ERROR;
}

Status Instance::SendForecastChangeEvent()
{

    CHIP_ERROR err;
    chip::Platform::ScopedMemoryBuffer<Structs::CommodityPriceStruct::Type> forecastBuffer;
    DataModel::List<const Structs::CommodityPriceStruct::Type> forecastList;

    Events::ForecastChange::Type event;
    EventNumber eventNumber;

    /*
     * The Event's PriceForecast must not include .description or .components
     * call our function to copy the PriceForecast attribute without these
     */
    if (!forecastBuffer.Calloc(kMaxForecastEntries))
    {
        ChipLogError(AppServer, "Memory allocation failed for forecast buffer") return Status::ResourceExhausted;
    }

    err = GetDetailedForecastRequest(0, forecastBuffer, kMaxForecastEntries, forecastList, true, false);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);

    event.priceForecast = forecastList;

    err = LogEvent(event, mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
