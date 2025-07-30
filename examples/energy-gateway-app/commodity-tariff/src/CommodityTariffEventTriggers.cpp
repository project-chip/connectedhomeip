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

static constexpr uint32_t kSecondsPer4hr = 14400; // 4 hours in seconds

uint8_t presetIndex = 0;

static uint8_t days_ctr    = 0;
static uint8_t entries_ctr = 0;
bool first_start           = true;
namespace TariffDataSamples {
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
} // namespace TariffDataSamples

static void ProcessTariffData(const TariffDataSamples::TariffDataSet &data) {
    if (!data.TariffInfo.IsNull())
    {
        printf("Processing tariff: %.*s\n", 
               static_cast<int>(data.TariffInfo.Value().tariffLabel.Value().size()),
               data.TariffInfo.Value().tariffLabel.Value().data());
    }

    if (!data.TariffComponents.IsNull())
    {
        printf("Contains %zu tariff components\n", data.TariffComponents.Value().size());
    }
}

void SetTestEventTrigger_TariffDataUpdated()
{
    const TariffDataSet & tariff_preset = TariffDataSamples::GetPreset(presetIndex++);

    CommodityTariffDelegate * dg = GetCommodityTariffDelegate();

    if (CHIP_NO_ERROR == dg->LoadTariffData(tariff_preset))
    {
        dg->TariffDataUpdate();
    }

    //ProcessTariffData();
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
        instance->SetupTimeShiftOffset(delay);
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
            instance->SetupTimeShiftOffset(delay);
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
