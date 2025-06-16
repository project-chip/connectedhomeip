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

#include <CommodityTariffMain.h>
#include <app/clusters/commodity-tariff-server/CommodityTariffTestEventTriggerHandler.h>
#include <app/util/af-types.h>

#include <fstream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;

uint8_t presetIndex = 0;

namespace TariffPresets {
static constexpr const char * kTariff1 = "./DefaultTariff.json";
;
static constexpr const char * kTariff2 = "./full_complex_tariff_1.json";
static constexpr const char * kTariff3 = "./full_complex_tariff_2.json";

// Array of all presets
static constexpr std::array<const char *, 3> kAllPresets = { kTariff1, kTariff2, kTariff3 };

// Number of presets (compile-time constant)
static constexpr size_t kCount = kAllPresets.size();

// Safe accessor function
static constexpr const char * GetPreset(size_t index)
{
    return (index < kCount) ? kAllPresets[index] : nullptr;
}
} // namespace TariffPresets

static bool LoadJsonFile(const char * aFname, Json::Value & jsonValue)
{
    bool is_ok = false;
    std::ifstream ifs;
    Json::CharReaderBuilder builder;
    Json::String errs;

    ifs.open(aFname);

    if (!ifs.good())
    {
        ChipLogError(NotSpecified, "AllClusters App: Error open file %s", aFname);
        goto exit;
    }

    if (!parseFromStream(builder, ifs, &jsonValue, &errs))
    {
        ChipLogError(NotSpecified, "AllClusters App: Error parsing JSON file %s with error %s:", aFname, errs.c_str());
        goto exit;
    }

    if (jsonValue.empty() || !jsonValue.isObject())
    {
        ChipLogError(NotSpecified, "Invalid file format %s", aFname);
        goto exit;
    }

    is_ok = true;

exit:
    return is_ok;
}

void SetTestEventTrigger_TariffDataUpdated()
{
    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();

    if (const char * preset = TariffPresets::GetPreset(presetIndex))
    {
        Json::Value json_root;
        ChipLogProgress(NotSpecified, "Tariff preset file %s", preset);
        if (LoadJsonFile(preset, json_root))
        {
            ChipLogProgress(NotSpecified, "The tariff file opened successfully");
            if ( CHIP_NO_ERROR == dg->LoadTariffData(json_root) )
            {
                dg->TariffDataUpdate();
            }
        }
        else
        {
            ChipLogError(NotSpecified, "Unable to load tariff file");
        }
        presetIndex++;
    }
    else
    {
        ChipLogProgress(NotSpecified, "Tariff data erasing");
        dg->CleanupTariffData();
        presetIndex = 0;
    }
}

void SetTestEventTrigger_TariffDataClear()
{
    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();

    dg->CleanupTariffData();
}

void SetTestEventTrigger_ForcedOneDayForward()
{
    // TODO
}

void SetTestEventTrigger_ForcedDayEntryForward()
{
    // TODO
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
    case CommodityTariffTrigger::kForcedOneDayForward:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced OneDay Forward");
        SetTestEventTrigger_ForcedOneDayForward();
        break;
    case CommodityTariffTrigger::kForcedOneDayEntryForward:
        ChipLogProgress(Support, "[CommodityTariff-Test-Event] => Forced DayEntry Forward");
        SetTestEventTrigger_ForcedDayEntryForward();
        break;

    default:
        return false;
    }

    return true;
}
