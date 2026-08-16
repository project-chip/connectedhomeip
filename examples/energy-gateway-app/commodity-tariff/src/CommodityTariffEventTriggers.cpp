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
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

static constexpr uint32_t kSecondsPer4hr = 14400; // 4 hours in seconds

static uint8_t presetIndex = 0;

// Test Time Management Implementation
namespace {

class TestTimeManager
{
public:
    TestTimeManager() = default;
    ~TestTimeManager() { ShutdownMockClock(); }

    void EnableTestTime(bool enable, uint32_t aInitialTimeValue_s = 0);
    void AdvanceTestTime(chip::System::Clock::Seconds32 offset);
    bool IsTestTimeEnabled() const { return mTestTimeEnabled; }

private:
    void InitializeMockClock(uint32_t aInitialTimeValue_s = 0);
    void ShutdownMockClock();
    void AdvanceMockTime(chip::System::Clock::Seconds32 offset);

    chip::System::Clock::Internal::MockClock * pMockClock = nullptr;
    chip::System::Clock::ClockBase * pRealClock           = nullptr;
    bool mTestTimeEnabled                                 = false;
};

TestTimeManager gTestTimeManager;

void TestTimeManager::InitializeMockClock(uint32_t aInitialTimeValue_s)
{
    // Create and configure the mock clock
    pMockClock = new Clock::Internal::MockClock();
    pRealClock = &SystemClock();
    Microseconds64 realTime_us;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aInitialTimeValue_s == 0)
    {
        // Get current real time to use as initial mock time
        err = chip::System::SystemClock().GetClock_RealTime(realTime_us);
    }
    else
    {
        realTime_us = aInitialTimeValue_s * 1'000'000_us; // seconds to microseconds
    }

    if (err == CHIP_NO_ERROR)
    {
        TEMPORARY_RETURN_IGNORED pMockClock->SetClock_RealTime(realTime_us);

        // Also set monotonic time to maintain consistency
        auto monotonicTime = std::chrono::duration_cast<Milliseconds64>(realTime_us);
        pMockClock->SetMonotonic(monotonicTime);

        ChipLogProgress(DeviceLayer, "Mock clock initialized with current real time");
    }
    else
    {
        // Fallback: use a reasonable default time if real time is unavailable
        Microseconds64 defaultTime(std::chrono::seconds(1704067200)); // Jan 1, 2024
        TEMPORARY_RETURN_IGNORED pMockClock->SetClock_RealTime(defaultTime);
        pMockClock->SetMonotonic(std::chrono::duration_cast<Milliseconds64>(defaultTime));

        ChipLogProgress(DeviceLayer, "Mock clock initialized with default time");
    }

    // Install the mock clock globally
    Clock::Internal::SetSystemClockForTesting(pMockClock);
}

void TestTimeManager::ShutdownMockClock()
{
    if (pMockClock)
    {
        // Restore the real system clock
        Clock::Internal::SetSystemClockForTesting(pRealClock);
        delete pMockClock;
        pMockClock = nullptr;
    }
}

void TestTimeManager::AdvanceMockTime(Seconds32 offset)
{
    if (!mTestTimeEnabled || !pMockClock)
    {
        ChipLogError(DeviceLayer, "Cannot advance time - test time not enabled");
        return;
    }

    // Get current mock time
    Microseconds64 currentTime, newTime;

    TEMPORARY_RETURN_IGNORED pMockClock->GetClock_RealTime(currentTime);

    // Update both real time and monotonic time consistently
    pMockClock->AdvanceRealTime(std::chrono::duration_cast<Milliseconds64>(offset));
    pMockClock->AdvanceMonotonic(std::chrono::duration_cast<Milliseconds64>(offset));

    // Update base time reference
    TEMPORARY_RETURN_IGNORED pMockClock->GetClock_RealTime(newTime);

    ChipLogProgress(DeviceLayer, "Advanced mock time: %" PRIu32 "s -> %" PRIu32 "s (+%" PRIu32 "s)",
                    std::chrono::duration_cast<Seconds32>(currentTime).count(),
                    std::chrono::duration_cast<Seconds32>(newTime).count(), offset.count());
}

void TestTimeManager::EnableTestTime(bool enable, uint32_t aInitialTimeValue_s)
{
    if (enable == mTestTimeEnabled)
    {
        // No change needed, but still trigger updates if re-enabling with same state
        if (enable)
        {
            ChipLogProgress(DeviceLayer, "Test time already enabled");
        }
        else
        {
            ChipLogProgress(DeviceLayer, "Test time already disabled");
        }
        return;
    }

    if (enable)
    {
        // Enable test time mode
        InitializeMockClock(aInitialTimeValue_s);
        mTestTimeEnabled = true;
        ChipLogProgress(DeviceLayer, "🔧 Test time mode ENABLED - using mock clock");
    }
    else
    {
        // Disable test time mode - this effectively resets to real time
        ShutdownMockClock();
        mTestTimeEnabled = false;
        ChipLogProgress(DeviceLayer, "⏰ Test time mode DISABLED - restored real system clock");
    }
}

void TestTimeManager::AdvanceTestTime(Seconds32 offset)
{
    if (!mTestTimeEnabled)
    {
        ChipLogError(DeviceLayer, "Cannot advance time - test time not enabled. Call EnableTestTime(true) first.");
        return;
    }

    if (offset.count() > 0)
    {
        AdvanceMockTime(offset);
        ChipLogProgress(DeviceLayer, "⏩ Time advanced by %" PRIu32 " seconds (%" PRIu32 " minutes, %" PRIu32 " hours)",
                        offset.count(), offset.count() / 60, offset.count() / 3600);
    }

    // Trigger tariff time synchronization
    CommodityTariffInstance * instance = GetCommodityTariffInstance();
    if (instance)
    {
        instance->TariffTimeAttrsSync();
    }
}

} // anonymous namespace

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
    RETURN_SAFELY_IGNORED process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDefaultRandomizationOffset),
                                            tariff_preset.DefaultRandomizationOffset, "DefaultRandomizationOffset", false);
    RETURN_SAFELY_IGNORED process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDefaultRandomizationType),
                                            tariff_preset.DefaultRandomizationType, "DefaultRandomizationType", false);
    RETURN_SAFELY_IGNORED process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kDayPatterns), tariff_preset.DayPatterns,
                                            "DayPatterns", false);
    RETURN_SAFELY_IGNORED process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kIndividualDays),
                                            tariff_preset.IndividualDays, "IndividualDays", false);
    RETURN_SAFELY_IGNORED process_attribute(dg->GetMgmtObj(CommodityTariffAttrTypeEnum::kCalendarPeriods),
                                            tariff_preset.CalendarPeriods, "CalendarPeriods", false);

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

    // Enable test time with the preset timestamp
    gTestTimeManager.EnableTestTime(true, tariff_preset.TariffTestTimestamp);

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
    gTestTimeManager.EnableTestTime(true);
    gTestTimeManager.AdvanceTestTime(chip::System::Clock::Seconds32(kSecondsPerDay));
}

/*
 * Forces a day entry update by scheduling at next 4-hour interval
 * Handles midnight crossing and resets counter when day changes
 */
void SetTestEventTrigger_TimeShift4h()
{
    gTestTimeManager.EnableTestTime(true);
    gTestTimeManager.AdvanceTestTime(chip::System::Clock::Seconds32(kSecondsPer4hr));
}

void SetTestEventTrigger_TimeShiftDisable()
{
    gTestTimeManager.EnableTestTime(false);
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
