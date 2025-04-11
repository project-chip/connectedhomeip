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

#include "CommodityPriceMain.h"
#include <app/clusters/commodity-price-server/CommodityPriceTestEventTriggerHandler.h>
#include <app/util/af-types.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;
using namespace chip::app::Clusters::CommodityPrice::Structs;

CommodityPriceDelegate * GetCommodityPriceDelegate()
{
    CommodityPriceInstance * mInst = GetCommodityPriceInstance();
    VerifyOrDieWithMsg(mInst != nullptr, AppServer, "CommodityPriceInstance is null");
    CommodityPriceDelegate * dg = mInst->GetDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "CommodityPriceDelegate is null");

    return dg;
}

void SetTestEventTrigger_PriceUpdate()
{
    // Change the value of CurrentPrice
    Structs::CommodityPriceStruct::Type newPriceStruct;
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> newPrice;

    newPriceStruct.periodStart = 1000; // TODO call getEpoch-s
    newPriceStruct.periodEnd.SetNonNull(newPriceStruct.periodStart + 30 * 60);
    newPriceStruct.price.currency.currency      = 981;
    newPriceStruct.price.currency.decimalPoints = 5;
    newPriceStruct.price.amount                 = 15916; // 15.916 p/kWh

    newPrice.SetNonNull(newPriceStruct);

    CommodityPriceDelegate * dg = GetCommodityPriceDelegate();
    dg->SetCurrentPrice(newPrice);
}

void SetTestEventTrigger_ForecastUpdate()
{
    // TODO
}

bool HandleCommodityPriceTestEventTrigger(uint64_t eventTrigger)
{
    CommodityPriceTrigger trigger = static_cast<CommodityPriceTrigger>(eventTrigger);

    switch (trigger)
    {
    case CommodityPriceTrigger::kPriceUpdate:
        ChipLogProgress(Support, "[CommodityPrice-Test-Event] => Price Update");
        SetTestEventTrigger_PriceUpdate();
        break;
    case CommodityPriceTrigger::kForecastUpdate:
        ChipLogProgress(Support, "[CommodityPrice-Test-Event] => Forecast Update");
        SetTestEventTrigger_ForecastUpdate();
        break;

    default:
        return false;
    }

    return true;
}
