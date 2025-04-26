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

const CurrencyStruct::Type currencyGBP = { .currency = kCurrencyGBP, .decimalPoints = 5 };
// -- Uncomment to use these - commented out to avoid unused warning
// const CurrencyStruct::Type currencyCHF  = { .currency = kCurrencyCHF, .decimalPoints = 5 };
// const CurrencyStruct::Type currencyEURO = { .currency = kCurrencyEURO, .decimalPoints = 5 };
// const CurrencyStruct::Type currencyNOK  = { .currency = kCurrencyNOK, .decimalPoints = 5 };
// const CurrencyStruct::Type currencySEK  = { .currency = kCurrencySEK, .decimalPoints = 5 };
// const CurrencyStruct::Type currencyUSD  = { .currency = kCurrencyUSD, .decimalPoints = 5 };

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
