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
// Prices are in 1000th of a penny / cent
const CurrencyStruct::Type currencyCHF  = { .currency = kCurrencyCHF, .decimalPoints = 5 };
const CurrencyStruct::Type currencyEURO = { .currency = kCurrencyEURO, .decimalPoints = 5 };
const CurrencyStruct::Type currencyGBP  = { .currency = kCurrencyGBP, .decimalPoints = 5 };
const CurrencyStruct::Type currencyNOK  = { .currency = kCurrencyNOK, .decimalPoints = 5 };
const CurrencyStruct::Type currencySEK  = { .currency = kCurrencySEK, .decimalPoints = 5 };
const CurrencyStruct::Type currencyUSD  = { .currency = kCurrencyUSD, .decimalPoints = 5 };

CHIP_ERROR CommodityPriceInstance::Init()
{
    return Instance::Init();
}

void CommodityPriceInstance::Shutdown()
{
    Instance::Shutdown();
}

CHIP_ERROR CommodityPriceInstance::AppInit()
{
    ReturnErrorOnFailure(Instance::SetCurrency(currencyGBP));
    ReturnErrorOnFailure(Instance::SetTariffUnit(TariffUnitEnum::kKWh));

    /* Manufacturers can optionally populate the CurrentPrice and
       Forecast attributes here, or set up some logic to do this periodically */

    return CHIP_NO_ERROR;
}

CommodityPriceDelegate::CommodityPriceDelegate() {}

//     Instance::SetTariffUnit(TariffUnitEnum::kKWh);
//     Instance::SetCurrency(currencyGBP);
//     Instance::SetCurrentPrice();
// }

// const DataModel::List<const CommodityPriceStruct::Type> &
// CommodityPriceDelegate::GetPriceForecast(CommodityPriceDetailBitmap bitmap)
// {
//     ChipLogDetail(Zcl, "CommodityPriceDelegate::GetForecast");

//     // TODO using the bitmap we need to knock out description/components if they are not required

//     return mPriceForecast;
// }

// /* @brief This function is called by the cluster server at the start of read cycle
//  *        This could take a semaphore to stop a background update of the data
//  */
// CHIP_ERROR CommodityPriceDelegate::StartPriceForecastRead(CommodityPriceDetailBitmap bitmap)
// {

//     // TODO - use bitmap to work out what 'types' need to be returned
//     // If this is a basic CurrentPrice then we do not include the Description or Components

//     /* Since we don't an implementation here we don't need to do anything here */
//     return CHIP_NO_ERROR;
// }

// CHIP_ERROR CommodityPriceDelegate::GetPriceForecastByIndex(uint8_t index,
//                                                            Structs::CommodityPriceStruct::Type & commodityPriceStruct)
// {

//     // TODO - use bitmap to work out what 'types' need to be returned
//     // If this is a basic CurrentPrice then we do not include the Description or Components

//     /** TODO - Manufacturers wanting to support this should
//      * implement an array of
//      * Structs::CommodityPriceStruct::Type mCommodityPriceStructs[];
//      *
//      * their application code should update the relevant CommodityPriceStruct information including
//      *   - .periodStart
//      *   - .periodEnd
//      *   - .price
//      *   - .description [optional]
//      *   - .components [optional list of CommodityPriceComponentStruct(s)]
//      *
//      *   if (index >= MATTER_ARRAY_SIZE(mCommodityPriceStructs))
//      *   {
//      *       return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
//      *   }
//      *
//      *   commodityPriceStruct = mCommodityPriceStructs[index];
//      *
//      *   return CHIP_NO_ERROR;
//      */

//     /* Return an empty list for now */
//     return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
// }

// /* @brief This function is called by the cluster server at the end of read cycle
//  *        This could release a semaphore to allow a background update of the data
//  */
// CHIP_ERROR CommodityPriceDelegate::EndPriceForecastRead()
// {
//     /* Since we don't an implementation here we don't need to do anything here */
//     return CHIP_NO_ERROR;
// }
