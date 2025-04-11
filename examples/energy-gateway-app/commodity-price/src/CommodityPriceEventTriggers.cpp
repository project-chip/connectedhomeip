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
#include "EnergyTimeUtils.h"
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
    uint32_t chipEpoch = 0;

    CHIP_ERROR err = DeviceEnergyManagement::GetEpochTS(chipEpoch); // TODO remove from DEM
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_PriceUpdate() could not get time");
    }

    newPriceStruct.periodStart = chipEpoch;
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
    CommodityPriceDelegate * dg = GetCommodityPriceDelegate();

    uint32_t chipEpoch = 0;

    CHIP_ERROR err = DeviceEnergyManagement::GetEpochTS(chipEpoch);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_ForecastUpdate() could not get time");
    }

    Structs::CommodityPriceStruct::Type newPriceStruct;

    newPriceStruct.periodEnd.SetNonNull(chipEpoch - 1); // 1st time through the start period will be 'now' - see below
    newPriceStruct.price.currency.currency      = 981;  // We'll always use GBP
    newPriceStruct.price.currency.decimalPoints = 5;    // We'll always use 1000th of a pence

    constexpr uint16_t kMinPrice        = 4000;    // 4 p/kWh
    constexpr uint16_t kMaxPrice        = 32000;   // 32 p/kWh
    constexpr uint16_t k30MinsInSeconds = 30 * 60; // seconds in 30 mins

    for (uint8_t i = 0; i < 50; i++)
    {
        // Make up a price
        newPriceStruct.price.amount = kMinPrice + rand() % (kMaxPrice - kMinPrice + 1);

        // Set the start and end time (non overlapping)
        newPriceStruct.periodStart = newPriceStruct.periodEnd.Value() + 1;
        newPriceStruct.periodEnd.SetNonNull(newPriceStruct.periodStart + (k30MinsInSeconds - 1)); // 29mins 59secs

        // Set a description based on price level
        if (newPriceStruct.price.amount < 10000)
        {
            // priceDescription = "Low";
        }
        else if (newPriceStruct.price.amount < 24000)
        {
            // priceDescription = "Medium";
        }
        else
        {
            // priceDescription = "High";
        }

        // newPriceStruct.description.SetValue(//TODO);

        // Create 2 component entries
        PriceForecastMemMgr memMgr = dg->GetPriceForecastMemMgr();

        Structs::CommodityPriceComponentStruct::Type newPriceComponent;

        newPriceComponent.source = Globals::TariffPriceTypeEnum::kStandard;
        newPriceComponent.tariffComponentID.ClearValue(); // We don't have a tariff componentID

        newPriceComponent.price = static_cast<Money>(newPriceStruct.price.amount * 95 / 100); // exVAT
        // newPriceComponent.description.SetValue(//TODO ("ExVAT)"));
        memMgr.PreparePriceEntry(0);
        memMgr.AddPriceComponent(newPriceComponent);

        newPriceComponent.price = static_cast<Money>(newPriceStruct.price.amount * 5 / 100); // VAT (5%)
        // newPriceComponent.description.SetValue(//TODO ("VAT)"));

        memMgr.PreparePriceEntry(1);
        memMgr.AddPriceComponent(newPriceComponent);
    }
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
