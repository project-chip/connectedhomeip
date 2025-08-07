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

#include <app/clusters/commodity-tariff-server/CommodityTariffTestEventTriggerHandler.h>
#include <CommodityTariffMain.h>
#include <CommodityTariffSamples.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::Clusters::CommodityTariff::TariffDataSamples;

static constexpr uint32_t kSecondsPer4hr = 14400; // 4 hours in seconds

uint8_t presetIndex = 0;

// Number of presets (compile-time constant)

// Safe accessor function
static const TariffDataSet & GetPreset(size_t index)
{
    if (index == kCount - 1)
    {
        presetIndex = 0;
    }

    return kTariffPresets[index];
}

#define COMMODITY_TARIFF_ATTRIBUTES_REQ \
    X(TariffUnit)                   \
    X(StartDate)                    \
    X(TariffInfo)                   \
    X(DayEntries)                   \
    X(TariffComponents)             \
    X(TariffPeriods)

#define COMMODITY_TARIFF_ATTRIBUTES_OPT \
    X(DefaultRandomizationOffset)   \
    X(DefaultRandomizationType)     \
    X(DayPatterns)                  \
    X(IndividualDays)               \
    X(CalendarPeriods)

#define ATTR_ITEM(field) \
    {tariff_preset.field.IsNull(), { dg->Get##field##_MgmtObj().SetNewValue(tariff_preset.field), #field}}

void SetTestEventTrigger_TariffDataUpdated()
{
    const TariffDataSet & tariff_preset = GetPreset(presetIndex++);
    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();
    CommodityTariffInstance * instance = GetCommodityTariffInstance();

    const std::vector<std::pair<bool, std::pair<CHIP_ERROR, const char*>>> required_tariff_items = {
#define X(attrName) ATTR_ITEM(attrName),
    COMMODITY_TARIFF_ATTRIBUTES_REQ
#undef X
    };

    const std::vector<std::pair<bool, std::pair<CHIP_ERROR, const char*>>> optional_tariff_items = {
#define X(attrName) ATTR_ITEM(attrName),
    COMMODITY_TARIFF_ATTRIBUTES_OPT
#undef X
    };

    for (const auto & item : required_tariff_items)
    {
        if (item.first)
        {
             ChipLogError(NotSpecified, "Invalid tariff data: the mandatory field \"%s\"is not present", item.second.second);
             return;
        }
        else
        {
            if (item.second.first != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Unable to load tariff data for the \"%s\" field", item.second.second);
                return;
            }
        }
    }

    for (const auto & item : optional_tariff_items)
    {
        if (!item.first)
        {
            if (item.second.first != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Unable to load tariff data for the \"%s\" field", item.second.second);
                return;
            }
        }
    }

    instance->ActivateTariffTimeTracking(tariff_preset.TariffTestTimestamp);

    dg->TariffDataUpdate(tariff_preset.TariffTestTimestamp);
}

#undef COMMODITY_TARIFF_ATTRIBUTES_REQ
#undef COMMODITY_TARIFF_ATTRIBUTES_OPT
#undef TARIFF_ITEM

void SetTestEventTrigger_TariffDataClear()
{
    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();

    dg->CleanupTariffData();
}

/*
 * Forces a day change event by scheduling update at the end of current day
 * Adds remaining time until midnight to trigger next day event
 */
void SetTestEventTrigger_TimeShift24h()
{
    CommodityTariffInstance * instance = GetCommodityTariffInstance();

    if (instance)
    {
        instance->TariffTimeTrackingSetOffset(kSecondsPerDay);
        instance->TariffTimeAttrsSync();
    }
}

/*
 * Forces a day entry update by scheduling at next 4-hour interval
 * Handles midnight crossing and resets counter when day changes
 */
void SetTestEventTrigger_TimeShift4h()
{
    CommodityTariffInstance * instance = GetCommodityTariffInstance();

    if (instance)
    {    
        instance->TariffTimeTrackingSetOffset(kSecondsPer4hr);
        instance->TariffTimeAttrsSync();
    }
}

void SetTestEventTrigger_TimeShiftDisable()
{
    CommodityTariffInstance * instance = GetCommodityTariffInstance();

    if (instance)
    {    
        instance->TariffTimeTrackingSetOffset(0);
    }
}

bool HandleCommodityTariffTestEventTrigger(uint64_t eventTrigger)
{
    CommodityTariffTrigger trigger = static_cast<CommodityTariffTrigger>(eventTrigger);

    switch (trigger)
    {
    case CommodityTariffTrigger::kTariffDataUpdated:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Tariff Data Updated");
        SetTestEventTrigger_TariffDataUpdated();
        break;
    case CommodityTariffTrigger::kTariffDataClear:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Tariff Data Clear");
        SetTestEventTrigger_TariffDataClear();
        break;
    case CommodityTariffTrigger::kTimeShift24h:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced OneDay Forward");
        SetTestEventTrigger_TimeShift24h();
        break;
    case CommodityTariffTrigger::kTimeShift4h:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced DayEntry Forward");
        SetTestEventTrigger_TimeShift4h();
        break;
    case CommodityTariffTrigger::kTimeShiftDisable:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced DayEntry Forward");
        SetTestEventTrigger_TimeShiftDisable();
        break;
    default:
        return false;
    }

    return true;
}
