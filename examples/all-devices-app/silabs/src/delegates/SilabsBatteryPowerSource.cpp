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
#include "SilabsBatteryPowerSource.h"

namespace chip::app {

constexpr System::Clock::Seconds16 kDecreaseBatteryLevelInterval = System::Clock::Seconds16(30);

SilabsBatteryPowerSource::SilabsBatteryPowerSource() :
    BatteryPowerSource("Decreasing Battery Power Source"_span, Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                       mTimerDelegate)
{}

SilabsBatteryPowerSource::~SilabsBatteryPowerSource()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR SilabsBatteryPowerSource::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    ReturnErrorOnFailure(BatteryPowerSource::Register(endpoint, provider, composition));
    // Kick off the timer loop to decrease battery level every few seconds
    return mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval);
}

void SilabsBatteryPowerSource::Unregister(CodeDrivenDataModelProvider & provider)
{
    BatteryPowerSource::Unregister(provider);
    mTimerDelegate.CancelTimer(this);
}

void SilabsBatteryPowerSource::TimerFired()
{

    // TODO @jepenven-silabs
    // Add VMCU power sampling
    auto & batteryCluster = BatteryPowerSourceCluster();
    auto batteryLevel     = batteryCluster.GetBatPercentRemaining();

    if (batteryLevel.IsNull())
    {
        batteryLevel.SetNonNull(200); // 100% (doubled percentage)
    }
    else if (batteryLevel.Value() > 0)
    {
        batteryLevel.SetNonNull(batteryLevel.Value() - 5); // decrease by 2.5%
    }
    else
    {
        batteryLevel.SetNull(); // set to null when it reaches 0%
    }

    LogErrorOnFailure(batteryCluster.SetBatPercentRemaining(batteryLevel));

    // Also drop the battery voltage linearly with the remaining percentage so
    // commissioners that periodically read attributes can display a voltage reading.
    constexpr uint32_t kFullVoltageMv  = 3000;
    constexpr uint32_t kEmptyVoltageMv = 2000;
    DataModel::Nullable<uint32_t> batteryVoltage;
    if (batteryLevel.IsNull())
    {
        batteryVoltage.SetNull();
    }
    else
    {
        // batteryLevel is a doubled percentage in the range [0, 200].
        const uint32_t percent = batteryLevel.Value();
        const uint32_t voltage = kEmptyVoltageMv + ((kFullVoltageMv - kEmptyVoltageMv) * percent) / 200;
        batteryVoltage.SetNonNull(voltage);
    }
    batteryCluster.SetBatVoltage(batteryVoltage);

    // Restart the timer to continue decreasing the battery level
    SuccessOrDie(mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval));
}

} // namespace chip::app




// TODO migrate
#include "BatteryMonitor.h"

#include "em_cmu.h"
#include "em_iadc.h"
#include "sl_sleeptimer.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>
#include <app/icd/server/ICDManager.h>
#include <lib/core/CHIPError.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

constexpr EndpointId kPowerSourceEndpoint = 1; //TODO Adjust this value

BatteryMonitor & BatteryMonitor::Instance()
{
    static BatteryMonitor instance;
    return instance;
}

void BatteryMonitor::Init()
{
    InitAdc();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    auto *observer = Server::GetInstance().GetICDManager().RegisterObserver(this);
    mObserverRegistered = (observer != nullptr);
#else
    mObserverRegistered = false;
#endif
}

void BatteryMonitor::InitAdc()
{
    IADC_Init_t init = IADC_INIT_DEFAULT;
    IADC_AllConfigs_t configs = IADC_ALLCONFIGS_DEFAULT;
    IADC_InitSingle_t singleInit = IADC_INITSINGLE_DEFAULT;

    IADC_SingleInput_t input = {
        iadcNegInputGnd,
        iadcPosInputAvdd,
        0,
        false,
    };

    CMU_ClockEnable(cmuClock_IADC0, true);
    IADC_reset(IADC0);
    CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

    init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, kAdcSourceFrequency, 0);

    configs.configs[0].reference = iadcCfgReferenceInt1V2;
    configs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                               kAdcFrequency,
                                                               0,
                                                               iadcCfgModeNormal,
                                                               init.srcClkPrescale);

    IADC_init(IADC0, &init, &configs);

    singleInit.dataValidLevel = (IADC_FifoCfgDvl_t)IADC_SCANFIFOCFG_DVL_VALID4;
    IADC_initSingle(IADC0, &singleInit, &input);

    mAdcEnabled = true;
}

void BatteryMonitor::EnableAdc()
{
    if (!mAdcEnabled)
    {
        IADC0->EN_SET = IADC_EN_EN;
        mAdcEnabled = true;
    }
}

void BatteryMonitor::DisableAdc()
{
    if (mAdcEnabled)
    {
        IADC0->EN_CLR = IADC_EN_EN;
        mAdcEnabled = false;
    }
}

uint16_t BatteryMonitor::ReadVoltageMv()
{
    EnableAdc();

    IADC_InitSingle_t singleInit = IADC_INITSINGLE_DEFAULT;
    IADC_SingleInput_t input = {
        iadcNegInputGnd,
        iadcPosInputAvdd,
        0,
        false,
    };

    singleInit.dataValidLevel = (IADC_FifoCfgDvl_t)IADC_SCANFIFOCFG_DVL_VALID4;
    IADC_initSingle(IADC0, &singleInit, &input);

    IADC_command(IADC0, iadcCmdStartSingle);

    while ((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK | _IADC_STATUS_SINGLEFIFODV_MASK)) !=
           IADC_STATUS_SINGLEFIFODV)
    {
    }

    IADC_Result_t sample = IADC_readSingleResult(IADC0);

    constexpr uint32_t kAdcMax = 0xFFF;

    // Same AVDD conversion used by the original Silicon Labs battery monitor.
    // The factor of 4 comes from the internal IADC scaling used for AVDD.
    const uint32_t voltageMv =
        (static_cast<uint32_t>(sample.data) * kAdcReferenceMv * 4U) / kAdcMax;

    return static_cast<uint16_t>(voltageMv);
}

uint16_t BatteryMonitor::FilterVoltage(uint16_t sample)
{
    if (!mFilterInitialized)
    {
        for (size_t i = 0; i < kFilterSize; ++i)
        {
            mSamples[i] = sample;
        }

        mFilterInitialized = true;
        return sample;
    }

    mSamples[mSampleIndex++] = sample;

    if (mSampleIndex >= kFilterSize)
    {
        mSampleIndex = 0;
    }

    uint32_t sum = 0;
    for (size_t i = 0; i < kFilterSize; ++i)
    {
        sum += mSamples[i];
    }

    return static_cast<uint16_t>(sum / kFilterSize);
}

uint8_t BatteryMonitor::VoltageToMatterPercent(uint16_t voltageMv) const
{
    if (voltageMv <= kBatteryEmptyMv)
    {
        return 0;
    }

    if (voltageMv >= kBatteryFullMv)
    {
        return 200;
    }

    const uint32_t value =
        (static_cast<uint32_t>(voltageMv - kBatteryEmptyMv) * 200U) /
        (kBatteryFullMv - kBatteryEmptyMv);

    return static_cast<uint8_t>(value);
}

bool BatteryMonitor::ShouldMeasure(uint32_t nowMs) const
{
    if (!mHasMeasurement)
    {
        return true;
    }

    return static_cast<uint32_t>(nowMs - mLastMeasurementMs) >= kMeasurementIntervalMs;
}

void BatteryMonitor::Update()
{
    const uint32_t nowMs = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

    if (!ShouldMeasure(nowMs))
    {
        return;
    }

    const uint16_t sample = ReadVoltageMv();
    const uint16_t newVoltageMv = FilterVoltage(sample);
    const uint8_t newPercent = VoltageToMatterPercent(newVoltageMv);

    bool report = !mHasMeasurement;

    if (mHasMeasurement)
    {
        const uint16_t voltageDelta = (newVoltageMv > mVoltageMv)
            ? static_cast<uint16_t>(newVoltageMv - mVoltageMv)
            : static_cast<uint16_t>(mVoltageMv - newVoltageMv);

        const uint8_t percentDelta = (newPercent > mBatteryPercent)
            ? static_cast<uint8_t>(newPercent - mBatteryPercent)
            : static_cast<uint8_t>(mBatteryPercent - newPercent);

        report = (voltageDelta >= kVoltageReportThresholdMv) ||
                 (percentDelta >= kPercentReportThreshold);
    }

    mVoltageMv = newVoltageMv;
    mBatteryPercent = newPercent;
    mLastMeasurementMs = nowMs;
    mHasMeasurement = true;

    if (report)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        UpdateMatterAttributes();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}

void BatteryMonitor::UpdateMatterAttributes()
{
    // BatVoltage is represented in millivolts.
    (void) PowerSource::Attributes::BatVoltage::Set(kPowerSourceEndpoint, mVoltageMv);

    // BatPercentRemaining is represented in 0.5% units (0..200).
    (void) PowerSource::Attributes::BatPercentRemaining::Set(kPowerSourceEndpoint, mBatteryPercent);
}

void BatteryMonitor::OnEnterActiveMode()
{
    EnableAdc();
}

void BatteryMonitor::OnTransitionToIdle()
{
    Update();
}

void BatteryMonitor::OnEnterIdleMode()
{
    DisableAdc();
}

void BatteryMonitor::OnICDModeChange()
{
    // No action required for the battery monitor.
}
