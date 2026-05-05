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
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Metadata.h>
#include <clusters/CarbonMonoxideConcentrationMeasurement/Metadata.h>
#include <clusters/FormaldehydeConcentrationMeasurement/Metadata.h>
#include <clusters/NitrogenDioxideConcentrationMeasurement/Metadata.h>
#include <clusters/OzoneConcentrationMeasurement/Metadata.h>
#include <clusters/Pm10ConcentrationMeasurement/Metadata.h>
#include <clusters/Pm1ConcentrationMeasurement/Metadata.h>
#include <clusters/Pm25ConcentrationMeasurement/Metadata.h>
#include <clusters/RadonConcentrationMeasurement/Metadata.h>
#include <clusters/TotalVolatileOrganicCompoundsConcentrationMeasurement/Metadata.h>
#include <lib/support/BitFlags.h>

static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::FormaldehydeConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::OzoneConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm10ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm1ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::Pm25ConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::RadonConcentrationMeasurement::kRevision);
static_assert(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision ==
              chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::kRevision);

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

/**
 * Single implementation for all 10 aliased Concentration Measurement clusters.
 * The cluster owns all attribute storage. Push readings via Set*().
 */
class ConcentrationMeasurementCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        ClusterId clusterId;
        BitFlags<Feature> features;
        MeasurementMediumEnum medium;
        MeasurementUnitEnum unit;
        DataModel::Nullable<float> minMeasured = DataModel::MakeNullable(0.0f);
        DataModel::Nullable<float> maxMeasured = DataModel::Nullable<float>();
        float uncertainty                      = 0.0f;
    };

    using OptionalAttributeSet =
        app::OptionalAttributeSet<Attributes::MeasuredValue::Id, Attributes::MinMeasuredValue::Id, Attributes::MaxMeasuredValue::Id,
                                  Attributes::Uncertainty::Id, Attributes::MeasurementUnit::Id, Attributes::PeakMeasuredValue::Id,
                                  Attributes::PeakMeasuredValueWindow::Id, Attributes::AverageMeasuredValue::Id,
                                  Attributes::AverageMeasuredValueWindow::Id, Attributes::LevelValue::Id>;

    ConcentrationMeasurementCluster(EndpointId endpointId, const Config & config);

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    bool HasFeature(Feature f) const { return mFeatures.Has(f); }

    // Push a new sensor reading. Validates range, stores, and notifies subscribers.
    // Returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the relevant feature flag was not set at construction.
    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<float> value);
    CHIP_ERROR SetPeakMeasuredValue(DataModel::Nullable<float> value);
    CHIP_ERROR SetPeakMeasuredValueWindow(uint32_t value);
    CHIP_ERROR SetAverageMeasuredValue(DataModel::Nullable<float> value);
    CHIP_ERROR SetAverageMeasuredValueWindow(uint32_t value);
    CHIP_ERROR SetLevelValue(LevelValueEnum value);

private:
    static constexpr uint32_t kWindowMaxSeconds = 604800;

    static bool IsInRange(DataModel::Nullable<float> value, DataModel::Nullable<float> minV, DataModel::Nullable<float> maxV)
    {
        if (value.IsNull())
            return true;
        if (!minV.IsNull() && value.Value() < minV.Value())
            return false;
        if (!maxV.IsNull() && value.Value() > maxV.Value())
            return false;
        return true;
    }
    static constexpr uint16_t kClusterRevision =
        static_cast<uint16_t>(chip::app::Clusters::CarbonDioxideConcentrationMeasurement::kRevision);

    const BitFlags<Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;

    MeasurementMediumEnum mMedium;
    MeasurementUnitEnum mUnit;
    DataModel::Nullable<float> mMinMeasuredValue;
    DataModel::Nullable<float> mMaxMeasuredValue;
    float mUncertainty;
    struct AttrDispatch
    {
        AttributeId id;
        ptrdiff_t offset;
        CHIP_ERROR (*encode)(AttributeValueEncoder &, const char *);
    };

    static const AttrDispatch kDispatchTable[];

    __attribute__((noinline)) CHIP_ERROR SetNullableFloat(Feature feature, DataModel::Nullable<float> & field, AttributeId id,
                                                          DataModel::Nullable<float> value);
    __attribute__((noinline)) CHIP_ERROR SetWindow(Feature feature, uint32_t & field, AttributeId id, uint32_t value);
    DataModel::Nullable<float> mMeasuredValue;
    DataModel::Nullable<float> mPeakMeasuredValue;
    uint32_t mPeakMeasuredValueWindow = 0;
    DataModel::Nullable<float> mAverageMeasuredValue;
    uint32_t mAverageMeasuredValueWindow = 0;
    LevelValueEnum mLevelValue           = LevelValueEnum::kUnknown;
};

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
