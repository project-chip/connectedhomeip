/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include "concentration-measurement-cluster-objects.h"
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

// Delegate — abstract interface your application implements to supply attribute values.
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    // Called once by ConcentrationMeasurementCluster::Startup().
    // Override to initialise your hardware or sensor driver. Default is a no-op.
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    // ── Always required ──────────────────────────────────────────────────────
    virtual MeasurementMediumEnum GetMeasurementMedium() { return MeasurementMediumEnum::kAir; }

    // ── Feature::kNumericMeasurement ─────────────────────────────────────────
    // Override if cluster was constructed with Feature::kNumericMeasurement.
    virtual DataModel::Nullable<float> GetMeasuredValue() { return DataModel::Nullable<float>(); }
    virtual DataModel::Nullable<float> GetMinMeasuredValue() { return DataModel::Nullable<float>(); }
    virtual DataModel::Nullable<float> GetMaxMeasuredValue() { return DataModel::Nullable<float>(); }
    virtual float GetUncertainty() { return 0.0f; }
    virtual MeasurementUnitEnum GetMeasurementUnit() { return MeasurementUnitEnum::kPpm; }

    // ── Feature::kPeakMeasurement (requires kNumericMeasurement) ─────────────
    virtual DataModel::Nullable<float> GetPeakMeasuredValue() { return DataModel::Nullable<float>(); }
    virtual uint32_t GetPeakMeasuredValueWindow() { return 0; }

    // ── Feature::kAverageMeasurement (requires kNumericMeasurement) ──────────
    virtual DataModel::Nullable<float> GetAverageMeasuredValue() { return DataModel::Nullable<float>(); }
    virtual uint32_t GetAverageMeasuredValueWindow() { return 0; }

    // ── Feature::kLevelIndication ─────────────────────────────────────────────
    virtual LevelValueEnum GetLevelValue() { return LevelValueEnum::kUnknown; }

    // ── Writable setters — called by ConcentrationMeasurementCluster::Set*() after validation.
    // Override these (not Handle*()) when implementing a custom Delegate.
    // Default implementations are no-ops; DefaultDelegate provides storage.
    // Fixed attributes (MinMeasuredValue, MaxMeasuredValue, Uncertainty) have no setters —
    // supply them at construction time instead.
    virtual void SetMeasuredValue(DataModel::Nullable<float>) {}
    virtual void SetPeakMeasuredValue(DataModel::Nullable<float>) {}
    virtual void SetPeakMeasuredValueWindow(uint32_t) {}
    virtual void SetAverageMeasuredValue(DataModel::Nullable<float>) {}
    virtual void SetAverageMeasuredValueWindow(uint32_t) {}
    virtual void SetLevelValue(LevelValueEnum) {}
};

/**
 * DefaultDelegate — the recommended delegate for most applications.
 *
 * DefaultDelegate  handles all 10 aliased
 * concentration clusters.
 * Full example — three pollutants, one file, no subclassing:
 *
 *   static ConcentrationMeasuremntCluster gCO2(ep, CarbonDioxideConcentrationMeasurement::Id,
 *       BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kLevelIndication),
 *       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
 *
 *   static ConcentrationMeasuremntCluster gCO(ep, CarbonMonoxideConcentrationMeasurement::Id,
 *       BitFlags<Feature>(Feature::kNumericMeasurement),
 *       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
 *
 *   static ConcentrationMeasuremntCluster gPM25(ep, Pm25ConcentrationMeasurement::Id,
 *       BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement),
 *       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
 *
 *   // Push new readings from your sensor driver:
 *   gCO2.GetDelegate().HandleNewMeasuredValue(DataModel::MakeNullable(412.5f));
 *   gCO2.GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
 *   gPM25.GetDelegate().HandleNewPeakMeasuredValue(DataModel::MakeNullable(18.0f));
 *
   For better RAM usage you can also choose to implement your own delegate class and only
   implement the features you need, then pass an instance of that to ConcentrationMeasurementCluster.
   class CO2SensorDelegate : public ConcentrationMeasurement::Delegate
   {
   public:
      MeasurementMediumEnum GetMeasurementMedium() override
    { return MeasurementMediumEnum::kAir; }

    MeasurementUnitEnum GetMeasurementUnit() override
    { return MeasurementUnitEnum::kPpm; }

    DataModel::Nullable<float> GetMeasuredValue() override { return mValue; }
    DataModel::Nullable<float> GetMinMeasuredValue() override { return mMin; }
    DataModel::Nullable<float> GetMaxMeasuredValue() override { return mMax; }
    LevelValueEnum GetLevelValue() override { return mLevel; }

    // Setters for app code
    void SetMeasuredValue(DataModel::Nullable<float> v) { mValue = v; }
    void SetLevelValue(LevelValueEnum v) { mLevel = v; }

private:
    // Only the fields this sensor needs — peak/average NOT compiled
    DataModel::Nullable<float> mValue;
    DataModel::Nullable<float> mMin;
    DataModel::Nullable<float> mMax;
    LevelValueEnum mLevel = LevelValueEnum::kUnknown;
    // Total: ~28 bytes (vs ~60 bytes if all fields were forced)
};
*/

class DefaultDelegate : public Delegate
{
public:
    /**
     * @param medium       Measurement medium (Air, Water, Soil). Fixed at construction.
     * @param unit         Measurement unit (ppm, ppb, µg/m³ …). Fixed at construction.
     * @param minMeasured  Sensor minimum range. Fixed at construction.
     * @param maxMeasured  Sensor maximum range. Fixed at construction.
     * @param uncertainty  Sensor uncertainty. Fixed at construction.
     */
    // minMeasured defaults to 0.0 — the spec-defined lower bound for any concentration (non-negative).
    // maxMeasured should be set to the sensor's physical upper range; applications that use a
    // broader range should pass an explicit value.
    DefaultDelegate(MeasurementMediumEnum medium, MeasurementUnitEnum unit,
                    DataModel::Nullable<float> minMeasured = DataModel::MakeNullable(0.0f),
                    DataModel::Nullable<float> maxMeasured = DataModel::Nullable<float>(), float uncertainty = 0.0f) :
        mMinMeasuredValue(minMeasured),
        mMaxMeasuredValue(maxMeasured), mUncertainty(uncertainty), mMedium(medium), mUnit(unit)
    {}

    // ── Getter overrides ─────────────────────────────────────────────────────
    MeasurementMediumEnum GetMeasurementMedium() override { return mMedium; }
    MeasurementUnitEnum GetMeasurementUnit() override { return mUnit; }
    DataModel::Nullable<float> GetMeasuredValue() override { return mMeasuredValue; }
    DataModel::Nullable<float> GetMinMeasuredValue() override { return mMinMeasuredValue; }
    DataModel::Nullable<float> GetMaxMeasuredValue() override { return mMaxMeasuredValue; }
    float GetUncertainty() override { return mUncertainty; }
    DataModel::Nullable<float> GetPeakMeasuredValue() override { return mPeakMeasuredValue; }
    uint32_t GetPeakMeasuredValueWindow() override { return mPeakMeasuredValueWindow; }
    DataModel::Nullable<float> GetAverageMeasuredValue() override { return mAverageMeasuredValue; }
    uint32_t GetAverageMeasuredValueWindow() override { return mAverageMeasuredValueWindow; }
    LevelValueEnum GetLevelValue() override { return mLevelValue; }

    // ── Setter overrides ─────────────────────────────────────────────────────
    void SetMeasuredValue(DataModel::Nullable<float> v) override { mMeasuredValue = v; }
    void SetMinMeasuredValue(DataModel::Nullable<float> v) { mMinMeasuredValue = v; }
    void SetMaxMeasuredValue(DataModel::Nullable<float> v) { mMaxMeasuredValue = v; }
    void SetUncertainty(float v) { mUncertainty = v; }
    void SetPeakMeasuredValue(DataModel::Nullable<float> v) override { mPeakMeasuredValue = v; }
    void SetPeakMeasuredValueWindow(uint32_t v) override { mPeakMeasuredValueWindow = v; }
    void SetAverageMeasuredValue(DataModel::Nullable<float> v) override { mAverageMeasuredValue = v; }
    void SetAverageMeasuredValueWindow(uint32_t v) override { mAverageMeasuredValueWindow = v; }
    void SetLevelValue(LevelValueEnum v) override { mLevelValue = v; }

    // ── Handle*() — backwards-compatible aliases for Set*()
    void HandleNewMeasuredValue(DataModel::Nullable<float> v) { SetMeasuredValue(v); }
    void HandleNewPeakMeasuredValue(DataModel::Nullable<float> v) { SetPeakMeasuredValue(v); }
    void HandleNewPeakMeasuredValueWindow(uint32_t v) { SetPeakMeasuredValueWindow(v); }
    void HandleNewAverageMeasuredValue(DataModel::Nullable<float> v) { SetAverageMeasuredValue(v); }
    void HandleNewAverageMeasuredValueWindow(uint32_t v) { SetAverageMeasuredValueWindow(v); }
    void HandleNewLevelValue(LevelValueEnum v) { SetLevelValue(v); }

private:
    // All fields start null/zero/unknown — safe defaults before first reading.
    DataModel::Nullable<float> mMeasuredValue;
    DataModel::Nullable<float> mMinMeasuredValue;
    DataModel::Nullable<float> mMaxMeasuredValue;
    DataModel::Nullable<float> mPeakMeasuredValue;
    uint32_t mPeakMeasuredValueWindow = 0;
    DataModel::Nullable<float> mAverageMeasuredValue;
    uint32_t mAverageMeasuredValueWindow = 0;
    float mUncertainty                   = 0.0f;
    MeasurementMediumEnum mMedium;
    MeasurementUnitEnum mUnit;
    LevelValueEnum mLevelValue = LevelValueEnum::kUnknown;
};

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
