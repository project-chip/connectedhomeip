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
// All Get*/Set* methods are pure virtual: every concrete delegate must implement them.
// To partially override only some methods, subclass DefaultDelegate instead.
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    // Called once by ConcentrationMeasurementCluster::Startup().
    // Override to initialise your hardware or sensor driver. Default is a no-op.
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    // ── Always required ──────────────────────────────────────────────────────
    virtual MeasurementMediumEnum GetMeasurementMedium() = 0;

    // ── Feature::kNumericMeasurement ─────────────────────────────────────────
    virtual DataModel::Nullable<float> GetMeasuredValue()    = 0;
    virtual DataModel::Nullable<float> GetMinMeasuredValue() = 0;
    virtual DataModel::Nullable<float> GetMaxMeasuredValue() = 0;
    virtual float GetUncertainty()                           = 0;
    virtual MeasurementUnitEnum GetMeasurementUnit()         = 0;

    // ── Feature::kPeakMeasurement (requires kNumericMeasurement) ─────────────
    virtual DataModel::Nullable<float> GetPeakMeasuredValue() = 0;
    virtual uint32_t GetPeakMeasuredValueWindow()              = 0;

    // ── Feature::kAverageMeasurement (requires kNumericMeasurement) ──────────
    virtual DataModel::Nullable<float> GetAverageMeasuredValue() = 0;
    virtual uint32_t GetAverageMeasuredValueWindow()              = 0;

    // ── Feature::kLevelIndication ─────────────────────────────────────────────
    virtual LevelValueEnum GetLevelValue() = 0;

    // ── Writable setters — called by ConcentrationMeasurementCluster::Set*() after validation.
    virtual void SetMeasuredValue(DataModel::Nullable<float>)    = 0;
    virtual void SetPeakMeasuredValue(DataModel::Nullable<float>) = 0;
    virtual void SetPeakMeasuredValueWindow(uint32_t)             = 0;
    virtual void SetAverageMeasuredValue(DataModel::Nullable<float>) = 0;
    virtual void SetAverageMeasuredValueWindow(uint32_t)             = 0;
    virtual void SetLevelValue(LevelValueEnum)                       = 0;
};

// DefaultDelegate — in-memory storage for every attribute. Recommended for most applications.
// For partial overrides (e.g. live hardware reads), subclass DefaultDelegate and override only
// what you need. See README.md for examples.
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
