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

#include <CommodityPriceDelegate.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>

#include <app/clusters/commodity-price-server/commodity-price-server.h>

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

// From ISO 4217 (non exhaustive selection)
const CurrencyStruct::Type currencyCHF  = { .currency = 756, .decimalPoints = 2 };
const CurrencyStruct::Type currencyEURO = { .currency = 978, .decimalPoints = 2 };
const CurrencyStruct::Type currencyGBP  = { .currency = 826, .decimalPoints = 2 };
const CurrencyStruct::Type currencyNOK  = { .currency = 578, .decimalPoints = 2 };
const CurrencyStruct::Type currencySEK  = { .currency = 752, .decimalPoints = 2 };
const CurrencyStruct::Type currencyUSD  = { .currency = 840, .decimalPoints = 2 };

CHIP_ERROR CommodityPriceInstance::Init()
{
    return Instance::Init();
}

void CommodityPriceInstance::Shutdown()
{
    Instance::Shutdown();
}

CommodityPriceDelegate::CommodityPriceDelegate() : mTariffUnit(TariffUnitEnum::kKWh), mCurrencyStruct(currencyGBP), mCurrentPrice()
{}

// --------------- Internal Attribute Set APIs
CHIP_ERROR CommodityPriceDelegate::SetTariffUnit(TariffUnitEnum newValue)
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

CHIP_ERROR CommodityPriceDelegate::SetCurrency(CurrencyStruct::Type newValue)
{
    CurrencyStruct::Type oldValue = mCurrencyStruct;

    // Check currency type is within limits
    if (newValue.currency > kMaxCurrencyValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mCurrencyStruct = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mCurrencyStruct updated to Currency: %d DecimalPoints: %d",
                      static_cast<int>(mCurrencyStruct.currency), static_cast<int>(mCurrencyStruct.decimalPoints));
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, Currency::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceDelegate::SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type> newValue)
{
    // Check PeriodStart  < PeriodEnd (if not null)
    if (!newValue.IsNull())
    {
        Structs::CommodityPriceStruct::Type tempValue;
        if (!newValue.Value().periodEnd.IsNull() && (newValue.Value().periodStart > newValue.Value().periodEnd.Value()))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        tempValue.periodStart = newValue.Value().periodStart;
        tempValue.periodEnd   = newValue.Value().periodEnd;
        tempValue.price       = newValue.Value().price;
        tempValue.description.ClearValue(); // CurrentPrice Attribute does not have a description
        tempValue.components.ClearValue();  // CurrentPrice Attribute does not have a List of components

        mCurrentPrice.SetNonNull(tempValue);
    }
    else
    {
        mCurrentPrice.SetNull();
    }

    ChipLogDetail(AppServer, "mCurrentPrice updated to Currency: %d DecimalPoints: %d", static_cast<int>(mCurrencyStruct.currency),
                  static_cast<int>(mCurrencyStruct.decimalPoints));
    MatterReportingAttributeChangeCallback(mEndpointId, CommodityPrice::Id, CurrentPrice::Id);

    // generate a PriceChange Event
    SendPriceChangeEvent();

    return CHIP_NO_ERROR;
}

Status CommodityPriceDelegate::SendPriceChangeEvent()
{
    Events::PriceChange::Type event;
    EventNumber eventNumber;

    // TODO - I think the spec is wrong here and the event.currentPrice should be Nullable too.
    if (!mCurrentPrice.IsNull())                    // THIS LINE SHOULD GO
        event.currentPrice = mCurrentPrice.Value(); // THIS FAILS BECAUSE THE TYPES are different (shouldn't need .Value())

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

/* @brief This function is called by the cluster server at the start of read cycle
 *        This could take a semaphore to stop a background update of the data
 */
CHIP_ERROR CommodityPriceDelegate::StartPriceForecastRead(CommodityPriceDetailBitmap bitmap)
{

    // TODO - use bitmap to work out what 'types' need to be returned
    // If this is a basic CurrentPrice then we do not include the Description or Components

    /* Since we don't an implementation here we don't need to do anything here */
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityPriceDelegate::GetPriceForecastByIndex(uint8_t index,
                                                           Structs::CommodityPriceStruct::Type & commodityPriceStruct)
{

    // TODO - use bitmap to work out what 'types' need to be returned
    // If this is a basic CurrentPrice then we do not include the Description or Components

    /** TODO - Manufacturers wanting to support this should
     * implement an array of
     * Structs::CommodityPriceStruct::Type mCommodityPriceStructs[];
     *
     * their application code should update the relevant CommodityPriceStruct information including
     *   - .periodStart
     *   - .periodEnd
     *   - .price
     *   - .description [optional]
     *   - .components [optional list of CommodityPriceComponentStruct(s)]
     *
     *   if (index >= MATTER_ARRAY_SIZE(mCommodityPriceStructs))
     *   {
     *       return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
     *   }
     *
     *   commodityPriceStruct = mCommodityPriceStructs[index];
     *
     *   return CHIP_NO_ERROR;
     */

    /* Return an empty list for now */
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

/* @brief This function is called by the cluster server at the end of read cycle
 *        This could release a semaphore to allow a background update of the data
 */
CHIP_ERROR CommodityPriceDelegate::EndPriceForecastRead()
{
    /* Since we don't an implementation here we don't need to do anything here */
    return CHIP_NO_ERROR;
}
