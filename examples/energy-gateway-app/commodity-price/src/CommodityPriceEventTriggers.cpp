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

static constexpr char kExVATStr[] = "ExVAT";
static constexpr char kVATStr[]   = "VAT";

void SetTestEventTrigger_PriceUpdate()
{
    // Change the value of CurrentPrice
    Structs::CommodityPriceStruct::Type newPriceStruct;
    DataModel::Nullable<Structs::CommodityPriceStruct::Type> newPrice;
    uint32_t matterEpoch = 0;

    CHIP_ERROR err = System::Clock::GetClock_MatterEpochS(matterEpoch);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_PriceUpdate() could not get time");
    }

    newPriceStruct.periodStart = matterEpoch;
    newPriceStruct.periodEnd.SetNonNull(newPriceStruct.periodStart + 30 * 60);

    Money amount = 15916; // 15.916 p/kWh
    newPriceStruct.price.SetValue(amount);
    newPriceStruct.priceLevel.SetValue(3);

    // Main price description
    const char * desc = (amount < 10000) ? "Low" : (amount < 24000) ? "Medium" : "High";
    newPriceStruct.description.SetValue(CharSpan::fromCharString(desc));

    // Components
    static Structs::CommodityPriceComponentStruct::Type sComponents[2];

    sComponents[0].source = Globals::TariffPriceTypeEnum::kStandard;
    sComponents[0].price  = static_cast<Money>(amount * 95 / 100);
    sComponents[0].description.SetValue(CharSpan::fromCharString(kExVATStr));

    sComponents[1].source = Globals::TariffPriceTypeEnum::kStandard;
    sComponents[1].price  = static_cast<Money>(amount * 5 / 100);
    sComponents[1].description.SetValue(CharSpan::fromCharString(kVATStr));

    // Assign the component span to the Commodity Price Struct
    newPriceStruct.components.SetValue(Span<const Structs::CommodityPriceComponentStruct::Type>(sComponents));

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

    uint32_t matterEpoch = 0;
    if (System::Clock::GetClock_MatterEpochS(matterEpoch) != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "SetTestEventTrigger_ForecastUpdate() could not get time");
        return;
    }

    constexpr size_t kForecastSize = 56;

    constexpr uint16_t kMinPrice        = 4000;  // 4p / kWh
    constexpr uint16_t kMaxPrice        = 32000; // 32p / kWh
    constexpr uint32_t k30MinsInSeconds = 30 * 60;

    static Structs::CommodityPriceStruct::Type sForecastEntries[kForecastSize];
    static Structs::CommodityPriceComponentStruct::Type sComponentBuffers[kForecastSize][2]; // Per-entry

    uint32_t currentStart = matterEpoch;

    for (size_t i = 0; i < kForecastSize; ++i)
    {
        Structs::CommodityPriceStruct::Type & newPriceStruct = sForecastEntries[i];

        Money amount = kMinPrice + rand() % (kMaxPrice - kMinPrice + 1);

        newPriceStruct.price.SetValue(amount);
        newPriceStruct.priceLevel.SetValue((amount < 10000) ? 1 : (amount < 24000) ? 2 : 3);

        newPriceStruct.periodStart = currentStart;
        newPriceStruct.periodEnd.SetNonNull(currentStart + k30MinsInSeconds - 1);
        currentStart += k30MinsInSeconds;

        // Main price description
        const char * desc = (amount < 10000) ? "Low" : (amount < 24000) ? "Medium" : "High";
        newPriceStruct.description.SetValue(CharSpan::fromCharString(desc));

        // Fill in components for this entry
        auto & components = sComponentBuffers[i];

        components[0].source = Globals::TariffPriceTypeEnum::kStandard;
        components[0].price  = static_cast<Money>(amount * 95 / 100);
        components[0].description.SetValue(CharSpan::fromCharString(kExVATStr));

        components[1].source = Globals::TariffPriceTypeEnum::kStandard;
        components[1].price  = static_cast<Money>(amount * 5 / 100);
        components[1].description.SetValue(CharSpan::fromCharString(kVATStr));

        // Assign the component span to the forecast entry
        newPriceStruct.components.SetValue(Span<const Structs::CommodityPriceComponentStruct::Type>(components, 2));
    }

    // Create list from the static array
    DataModel::List<Structs::CommodityPriceStruct::Type> forecastList(
        Span<Structs::CommodityPriceStruct::Type>(sForecastEntries, kForecastSize));

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
