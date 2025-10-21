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

#include <CommodityTariffInstance.h>
#include <CommodityTariffMain.h>
#include <CommodityTariffSamples.h>
#include <app/clusters/commodity-tariff-server/CommodityTariffTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::Clusters::CommodityTariff::TariffDataSamples;

static constexpr uint32_t kSecondsPer4hr = 14400; // 4 hours in seconds

static uint8_t presetIndex = 0;

// Number of presets (compile-time constant)

// Safe accessor function
static const TariffDataSet & GetNextPreset()
{
    const TariffDataSet & preset = kTariffPresets[presetIndex];
    presetIndex                  = (presetIndex + 1) % kCount;
    return preset;
}

void SetTestEventTrigger_TariffDataUpdated()
{
    const TariffDataSet & tariff_preset = GetNextPreset();
    CommodityTariffDelegate * dg        = GetCommodityTariffDelegate();
    CommodityTariffInstance * instance  = GetCommodityTariffInstance();

    using namespace chip::app::CommodityTariffAttrsDataMgmt;
    using CommodityTariffAttrTypeEnum = chip::app::Clusters::CommodityTariff::CommodityTariffDelegate::CommodityTariffAttrTypeEnum;

    auto process_attribute = [](auto & mgmt_obj, const auto preset_value, const char * name, bool is_required) -> CHIP_ERROR {
        if (!preset_value.IsNull())
        {
            CHIP_ERROR err = mgmt_obj.SetNewValueFromVoid(&preset_value);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Unable to load tariff data for \"%s\": %" CHIP_ERROR_FORMAT, name, err.Format());
                return err;
            }
        }
        else if (is_required)
        {
            ChipLogError(AppServer, "Invalid tariff data: mandatory field \"%s\" not present", name);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_NO_ERROR;
    };

    // Process optional attributes
    process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDefaultRandomizationOffset),
                      tariff_preset.DefaultRandomizationOffset, "DefaultRandomizationOffset", false);
    process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDefaultRandomizationType),
                      tariff_preset.DefaultRandomizationType, "DefaultRandomizationType", false);
    process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDayPatterns), tariff_preset.DayPatterns, "DayPatterns", false);
    process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kIndividualDays), tariff_preset.IndividualDays, "IndividualDays",
                      false);
    process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kCalendarPeriods), tariff_preset.CalendarPeriods,
                      "CalendarPeriods", false);

    // Process mandatory attributes
    CHIP_ERROR err = CHIP_NO_ERROR;
    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffUnit), tariff_preset.TariffUnit, "TariffUnit", true);
    if (err != CHIP_NO_ERROR)
        return;

    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kStartDate), tariff_preset.StartDate, "StartDate", true);
    if (err != CHIP_NO_ERROR)
        return;

    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffInfo), tariff_preset.TariffInfo, "TariffInfo", true);
    if (err != CHIP_NO_ERROR)
        return;

    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDayEntries), tariff_preset.DayEntries, "DayEntries", true);
    if (err != CHIP_NO_ERROR)
        return;

    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffComponents), tariff_preset.TariffComponents,
                            "TariffComponents", true);
    if (err != CHIP_NO_ERROR)
        return;

    err = process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffPeriods), tariff_preset.TariffPeriods,
                            "TariffPeriods", true);
    if (err != CHIP_NO_ERROR)
        return;

    if (instance)
    {
        instance->EnableTestTime(true);
        instance->AdvanceTestTime(chip::System::Clock::Seconds32(tariff_preset.TariffTestTimestamp));
    }
    else
    {
        ChipLogError(AppServer, "The sever instance is null");
    }

    if (dg)
    {
        dg->TariffDataUpdate(tariff_preset.TariffTestTimestamp);
    }
    else
    {
        ChipLogError(AppServer, "The tariff provider instance is null");
    }
}

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
        instance->EnableTestTime(true);
        instance->AdvanceTestTime(chip::System::Clock::Seconds32(kSecondsPerDay));
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
        instance->EnableTestTime(true);
        instance->AdvanceTestTime(chip::System::Clock::Seconds32(kSecondsPer4hr));
    }
}

void SetTestEventTrigger_TimeShiftDisable()
{
    CommodityTariffInstance * instance = GetCommodityTariffInstance();

    if (instance)
    {
        instance->EnableTestTime(false);
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
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced FourHours Forward");
        SetTestEventTrigger_TimeShift4h();
        break;
    case CommodityTariffTrigger::kTimeShiftDisable:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Time Shift Disabled");
        SetTestEventTrigger_TimeShiftDisable();
        break;
    default:
        return false;
    }

    return true;
}
