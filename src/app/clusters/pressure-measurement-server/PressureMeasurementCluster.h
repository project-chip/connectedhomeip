/*
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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PressureMeasurement/Attributes.h>
#include <clusters/PressureMeasurement/Enums.h>
#include <clusters/PressureMeasurement/Metadata.h>

namespace chip::app::Clusters {

class PressureMeasurementCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<PressureMeasurement::Attributes::Tolerance::Id,
                                                           PressureMeasurement::Attributes::ScaledValue::Id,
                                                           PressureMeasurement::Attributes::MinScaledValue::Id,
                                                           PressureMeasurement::Attributes::MaxScaledValue::Id,
                                                           PressureMeasurement::Attributes::ScaledTolerance::Id,
                                                           PressureMeasurement::Attributes::Scale::Id>;

    struct Config
    {
        Config() : minMeasuredValue(), maxMeasuredValue() {}

        Config & WithTolerance(uint16_t value)
        {
            mTolerance = value;
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::Tolerance::Id>();
            return *this;
        }

        Config & WithExtendedFeature(DataModel::Nullable<int16_t> minScaledValue, DataModel::Nullable<int16_t> maxScaledValue,
                              int8_t scale)
        {
            mFeatureMap.Set(PressureMeasurement::Feature::kExtended);
            mMinScaledValue = minScaledValue;
            mMaxScaledValue = maxScaledValue;
            mScale          = scale;
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::ScaledValue::Id>();
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::MinScaledValue::Id>();
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::MaxScaledValue::Id>();
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::Scale::Id>();
            return *this;
        }

        Config & WithScaledTolerance(uint16_t value)
        {
            mScaledTolerance = value;
            mOptionalAttributeSet.template Set<PressureMeasurement::Attributes::ScaledTolerance::Id>();
            return *this;
        }

        DataModel::Nullable<int16_t> minMeasuredValue;
        DataModel::Nullable<int16_t> maxMeasuredValue;
        OptionalAttributeSet mOptionalAttributeSet;
        BitMask<PressureMeasurement::Feature> mFeatureMap;
        uint16_t mTolerance      = 0;
        DataModel::Nullable<int16_t> mMinScaledValue;
        DataModel::Nullable<int16_t> mMaxScaledValue;
        uint16_t mScaledTolerance = 0;
        int8_t mScale             = 0;
    };

    explicit PressureMeasurementCluster(EndpointId endpointId);
    PressureMeasurementCluster(EndpointId endpointId, const Config & config);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<int16_t> measuredValue);
    DataModel::Nullable<int16_t> GetMeasuredValue() const { return mMeasuredValue; }

    DataModel::Nullable<int16_t> GetMinMeasuredValue() const { return mMinMeasuredValue; }
    DataModel::Nullable<int16_t> GetMaxMeasuredValue() const { return mMaxMeasuredValue; }

    CHIP_ERROR SetScaledValue(DataModel::Nullable<int16_t> scaledValue);
    DataModel::Nullable<int16_t> GetScaledValue() const { return mScaledValue; }

protected:
    CHIP_ERROR SetMeasuredValueRange(DataModel::Nullable<int16_t> minMeasuredValue,
                                     DataModel::Nullable<int16_t> maxMeasuredValue);

private:
    const OptionalAttributeSet mOptionalAttributeSet;
    const BitMask<PressureMeasurement::Feature> mFeatureMap;
    DataModel::Nullable<int16_t> mMeasuredValue{};
    DataModel::Nullable<int16_t> mMinMeasuredValue{};
    DataModel::Nullable<int16_t> mMaxMeasuredValue{};
    uint16_t mTolerance{};
    DataModel::Nullable<int16_t> mScaledValue{};
    DataModel::Nullable<int16_t> mMinScaledValue{};
    DataModel::Nullable<int16_t> mMaxScaledValue{};
    uint16_t mScaledTolerance{};
    int8_t mScale{};
};

} // namespace chip::app::Clusters
