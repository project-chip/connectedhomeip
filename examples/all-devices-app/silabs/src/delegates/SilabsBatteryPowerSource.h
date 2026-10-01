/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <device/types/power-source/BatteryPowerSource.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app {

/**
 * Simulated battery power source that decreases the reported battery level over time.
 */
class SilabsBatteryPowerSource : public BatteryPowerSource, public TimerContext
{
public:
    SilabsBatteryPowerSource();
    ~SilabsBatteryPowerSource() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // TODO @jepenven-silabs
    // Add setBatterySamplingPeriod

    // TimerContext
    void TimerFired() override;

private:
    DefaultTimerDelegate mTimerDelegate;
};

} // namespace chip::app


// TODO migrate
#include <cstddef>
#include <cstdint>

#include <app/icd/server/ICDStateObserver.h>

class BatteryMonitor : public chip::app::ICDStateObserver
{
public:
    static BatteryMonitor & Instance();

    void Init();
    void Update();

    uint16_t GetVoltageMv() const { return mVoltageMv; }
    uint8_t GetMatterBatteryPercent() const { return mBatteryPercent; }

    void OnEnterActiveMode() override;
    void OnTransitionToIdle() override;
    void OnEnterIdleMode() override;
    void OnICDModeChange() override;

private:
    BatteryMonitor() = default;

    void InitAdc();
    void EnableAdc();
    void DisableAdc();

    uint16_t ReadVoltageMv();
    uint16_t FilterVoltage(uint16_t sample);
    uint8_t VoltageToMatterPercent(uint16_t voltageMv) const;
    void UpdateMatterAttributes();
    bool ShouldMeasure(uint32_t nowMs) const;

    static constexpr size_t kFilterSize = 16;

    static constexpr uint32_t kAdcSourceFrequency = 1000000;
    static constexpr uint32_t kAdcFrequency       = 100000;
    static constexpr uint32_t kAdcReferenceMv     = 1210;

    // Adjust these values for the actual battery chemistry.
    static constexpr uint16_t kBatteryFullMv  = 3000; //TODO Adjust this value
    static constexpr uint16_t kBatteryEmptyMv = 2000; //TODO Adjust this value

    // Matter BatPercentRemaining uses half-percent units.
    // A delta of 2 therefore represents 1%.
    static constexpr uint16_t kVoltageReportThresholdMv = 50;
    static constexpr uint8_t kPercentReportThreshold    = 2;

    // Measure at most once every 30 minutes.
    static constexpr uint32_t kMeasurementIntervalMs = 30UL * 60UL * 1000UL; //TODO Adjust this value

    uint16_t mVoltageMv = 0;
    uint8_t mBatteryPercent = 0;

    uint16_t mSamples[kFilterSize] = {};
    size_t mSampleIndex = 0;

    uint32_t mLastMeasurementMs = 0;

    bool mFilterInitialized = false;
    bool mAdcEnabled = false;
    bool mHasMeasurement = false;
    bool mObserverRegistered = false;
};
