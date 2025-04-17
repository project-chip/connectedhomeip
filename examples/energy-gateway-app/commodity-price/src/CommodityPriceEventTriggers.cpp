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

static constexpr char kExVATStr[] = "ExVAT";
static constexpr char kVATStr[]   = "VAT";

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

    Globals::Structs::PriceStruct::Type price; // Contains .amount  and .currency
    newPriceStruct.periodStart = chipEpoch;
    newPriceStruct.periodEnd.SetNonNull(newPriceStruct.periodStart + 30 * 60);

    price.amount                 = 15916; // 15.916 p/kWh
    price.currency.currency      = kCurrencyGBP;
    price.currency.decimalPoints = 5;
    newPriceStruct.price.SetValue(price);
    newPriceStruct.priceLevel.SetValue(3);

    // Main price description
    const char * desc = (price.amount < 10000) ? "Low" : (price.amount < 24000) ? "Medium" : "High";
    newPriceStruct.description.SetValue(chip::Span<const char>(desc, strlen(desc)));

    // Components
    static Structs::CommodityPriceComponentStruct::Type sComponents[2];

    sComponents[0].source = Globals::TariffPriceTypeEnum::kStandard;
    sComponents[0].price.SetValue(static_cast<Money>(price.amount * 95 / 100));
    sComponents[0].description.SetValue(chip::Span<const char>(kExVATStr, strlen(kExVATStr)));

    sComponents[1].source = Globals::TariffPriceTypeEnum::kStandard;
    sComponents[1].price.SetValue(static_cast<Money>(price.amount * 5 / 100));
    sComponents[1].description.SetValue(chip::Span<const char>(kVATStr, strlen(kVATStr)));

    // Assign the component span to the
    newPriceStruct.components.SetValue(chip::Span<const Structs::CommodityPriceComponentStruct::Type>(sComponents, 2));

    newPrice.SetNonNull(newPriceStruct);

    CommodityPriceInstance * inst = GetCommodityPriceInstance();
    inst->SetCurrentPrice(newPrice);
}

void SetTestEventTrigger_ForecastUpdate()
{
    CommodityPriceInstance * inst = GetCommodityPriceInstance();
    if (inst == nullptr)
    {
        ChipLogError(Support, "CommodityPriceInstance not available.");
        return;
    }

    uint32_t chipEpoch = 0;
    if (DeviceEnergyManagement::GetEpochTS(chipEpoch) != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_ForecastUpdate() could not get time");
        return;
    }

    constexpr size_t kForecastSize = 4; // TODO this should be bigger
    // but causes issues when too large in attribute and command encoding
    constexpr uint16_t kMinPrice        = 4000;  // 4p / kWh
    constexpr uint16_t kMaxPrice        = 32000; // 32p / kWh
    constexpr uint32_t k30MinsInSeconds = 30 * 60;

    static Structs::CommodityPriceStruct::Type sForecastEntries[kForecastSize];
    static Structs::CommodityPriceComponentStruct::Type sComponentBuffers[kForecastSize][2]; // Per-entry

    uint32_t currentStart = chipEpoch;

    for (size_t i = 0; i < kForecastSize; ++i)
    {
        Structs::CommodityPriceStruct::Type & newPriceStruct = sForecastEntries[i];

        Globals::Structs::PriceStruct::Type price;
        price.currency.currency      = kCurrencyGBP;
        price.currency.decimalPoints = 5;
        price.amount                 = kMinPrice + rand() % (kMaxPrice - kMinPrice + 1);

        newPriceStruct.price.SetValue(price);
        newPriceStruct.priceLevel.SetValue((price.amount < 10000) ? 1 : (price.amount < 24000) ? 2 : 3);

        newPriceStruct.periodStart = currentStart;
        newPriceStruct.periodEnd.SetNonNull(currentStart + k30MinsInSeconds - 1);
        currentStart += k30MinsInSeconds;

        // Main price description
        const char * desc = (price.amount < 10000) ? "Low" : (price.amount < 24000) ? "Medium" : "High";
        newPriceStruct.description.SetValue(chip::Span<const char>(desc, strlen(desc)));

        // Fill in components for this entry
        auto & components = sComponentBuffers[i];

        components[0].source = Globals::TariffPriceTypeEnum::kStandard;
        components[0].price.SetValue(static_cast<Money>(price.amount * 95 / 100));
        components[0].description.SetValue(chip::Span<const char>(kExVATStr, strlen(kExVATStr)));

        components[1].source = Globals::TariffPriceTypeEnum::kStandard;
        components[1].price.SetValue(static_cast<Money>(price.amount * 5 / 100));
        components[1].description.SetValue(chip::Span<const char>(kVATStr, strlen(kVATStr)));

        // Assign the component span to the forecast entry
        newPriceStruct.components.SetValue(chip::Span<const Structs::CommodityPriceComponentStruct::Type>(components, 2));
    }

    // Create list from the static array
    DataModel::List<Structs::CommodityPriceStruct::Type> forecastList(
        chip::Span<Structs::CommodityPriceStruct::Type>(sForecastEntries, kForecastSize));

    inst->SetForecast(forecastList); // Should now be safe: all memory lives long enough
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
