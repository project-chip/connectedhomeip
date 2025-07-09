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

#include <fstream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Structs;

static constexpr uint32_t kSecondsPer4hr = 14400; // 4 hours in seconds

uint8_t presetIndex = 0;

static uint8_t days_ctr    = 0;
static uint8_t entries_ctr = 0;
bool first_start           = true;
namespace TariffPresets {
static constexpr const char * kTariff1 = "./tariff_sample_1.json";
static constexpr const char * kTariff2 = "./tariff_sample_2.json";

// Array of all presets
static constexpr std::array<const char *, 2> kAllPresets = { kTariff1, kTariff2 };

// Number of presets (compile-time constant)
static constexpr size_t kCount = kAllPresets.size();

// Safe accessor function
static constexpr const char * GetPreset(size_t index)
{
    if (index == kCount)
    {
        index = presetIndex = 0;
    }
    else
    {
        presetIndex++;
    }
    return kAllPresets[index];
}
} // namespace TariffPresets

void SetTestEventTrigger_TariffDataUpdated()
{
    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();

    if (const char * preset = TariffPresets::GetPreset(presetIndex))
    {
        LoadTariffFromJSONFile(preset, dg);
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

static uint32_t GetCurrentTimestamp(void)
{
    System::Clock::Microseconds64 utcTimeUnix;
    uint64_t chipEpochTime;
    System::SystemClock().GetClock_RealTime(utcTimeUnix);
    UnixEpochToChipEpochMicros(utcTimeUnix.count(), chipEpochTime);

    return static_cast<uint32_t>(chipEpochTime / chip::kMicrosecondsPerSecond);
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
        const uint32_t now                  = GetCurrentTimestamp();
        const uint32_t secondsSinceMidnight = now % kSecondsPerDay;
        const uint32_t delay                = (now - secondsSinceMidnight) + (days_ctr++) * kSecondsPerDay;

        entries_ctr = 0;
        instance->SetupTestTimeShiftInterval(delay);
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
        const uint32_t now                  = GetCurrentTimestamp();
        const uint32_t secondsSinceMidnight = now % kSecondsPerDay;
        uint32_t delay                      = (now - secondsSinceMidnight);

        if (first_start)
        {
            entries_ctr = static_cast<uint8_t>(secondsSinceMidnight / kSecondsPer4hr);
            first_start = false;
        }

        delay += ++entries_ctr * kSecondsPer4hr;

        ChipLogDetail(NotSpecified, "ForceDayEntriesAttrsUpdate: entry upd delay: %d", delay);

        if (delay % kSecondsPerDay == 0)
        {
            SetTestEventTrigger_TimeShift24h();
        }
        else
        {
            // Move to next 4-hour interval for next call
            delay += days_ctr * kSecondsPerDay;
            instance->SetupTestTimeShiftInterval(delay);
        }
    }
}

void SetTestEventTrigger_TimeShiftDisable()
{
    days_ctr    = 0;
    entries_ctr = 0;
    first_start = true;
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
