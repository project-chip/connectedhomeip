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
    CHIP_ERROR err                                                                    = CHIP_NO_ERROR;
    const DataModel::Nullable<Structs::CommodityPriceStruct::Type> * pPriceStruct     = nullptr;
    const DataModel::List<const Structs::CommodityPriceStruct::Type> * pPriceForecast = nullptr;

    switch (aPath.mAttributeId)
    {
    case TariffUnit::Id:
        return aEncoder.Encode(mTariffUnit);
    case Currency::Id:
        return aEncoder.Encode(mCurrency);
    case CurrentPrice::Id:
        // Call GetDetailedPriceRequest with details = 0 to strip out .components and .description if present
        pPriceStruct = GetDetailedPriceRequest(0);
        if (pPriceStruct == nullptr)
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            err = aEncoder.Encode(*pPriceStruct);
            FreeCurrentPrice(pPriceStruct);
        }
        return err;
    case PriceForecast::Id:
        /* FORE - Forecasting */
        if (!HasFeature(Feature::kForecasting))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        // Call GetDetailedForecastRequest with details = 0 to
        // strip out .components and .description if present
        pPriceForecast = GetDetailedForecastRequest(0);
        if (pPriceForecast == nullptr)
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            err = aEncoder.Encode(*pPriceForecast);
            FreePriceForecast(pPriceForecast);
        }
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
    chip::BitMask<CommodityPriceDetailBitmap> details = commandData.details;
    if (!details.HasOnly(
            BitMask<CommodityPriceDetailBitmap>(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents)))
    {
        ChipLogError(Zcl, "Invalid details bitmap recvd in GetDetailedPriceRequest");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Commands::GetDetailedPriceResponse::Type response;

    const DataModel::Nullable<Structs::CommodityPriceStruct::Type> * pPriceStruct = GetDetailedPriceRequest(details);
    if (pPriceStruct == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    else
    {
        response.currentPrice = *pPriceStruct;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    FreeCurrentPrice(pPriceStruct);
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

    chip::BitMask<CommodityPriceDetailBitmap> details = commandData.details;
    if (!details.HasOnly(
            BitMask<CommodityPriceDetailBitmap>(CommodityPriceDetailBitmap::kDescription, CommodityPriceDetailBitmap::kComponents)))
    {
        ChipLogError(Zcl, "Invalid details bitmap recvd in GetDetailedForecastRequest");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Commands::GetDetailedForecastResponse::Type response;
    const DataModel::List<const Structs::CommodityPriceStruct::Type> * pPriceForecast = GetDetailedForecastRequest(details);
    if (pPriceForecast == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    else
    {
        response.priceForecast = *pPriceForecast;
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    FreePriceForecast(pPriceForecast);
}

const DataModel::Nullable<Structs::CommodityPriceStruct::Type> *
Instance::GetDetailedPriceRequest(chip::BitMask<CommodityPriceDetailBitmap> details)
{
    // Based on the bitmap we make a copy of mCurrentPrice with/without the .description and/or .components
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> * pPriceStruct = nullptr;

    // Make a copy of the mCurrentPrice
    pPriceStruct = new DataModel::Nullable<Structs::CommodityPriceStruct::Type>(mCurrentPrice);

    if (!pPriceStruct->IsNull())
    {
        if (!details.Has(CommodityPriceDetailBitmap::kComponents))
        {
            // Remove the components
            pPriceStruct->Value().components.ClearValue();
        }

        if (!details.Has(CommodityPriceDetailBitmap::kDescription))
        {
            // Remove the description
            pPriceStruct->Value().description.ClearValue();
        }
    }

    return pPriceStruct;
}

void Instance::FreeCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> * pPriceStruct)
{
    VerifyOrReturn(pPriceStruct != nullptr);
    delete pPriceStruct;
}

const DataModel::List<const Structs::CommodityPriceStruct::Type> *
Instance::GetDetailedForecastRequest(chip::BitMask<CommodityPriceDetailBitmap> details)
{

    // Allocate backing storage for the copy
    auto * buffer = new Structs::CommodityPriceStruct::Type[kMaxForecastEntries];

    size_t count = 0;

    for (const auto & srcPrice : mPriceForecast)
    {
        if (count >= kMaxForecastEntries)
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

        buffer[count++] = copy;
    }

    // Now wrap in Span + List
    auto * pPriceForecast = new DataModel::List<const Structs::CommodityPriceStruct::Type>(
        chip::Span<Structs::CommodityPriceStruct::Type>(buffer, count));

    return pPriceForecast;
}

void Instance::FreePriceForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> * pPriceForecast)
{
    VerifyOrReturn(pPriceForecast != nullptr);
    delete pPriceForecast;
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
    Events::PriceChange::Type event;
    EventNumber eventNumber;

    event.currentPrice = mCurrentPrice;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
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

    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, PriceForecast::Id);

    return CHIP_NO_ERROR;
}

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
