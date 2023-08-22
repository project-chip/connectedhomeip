/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/AttributeAccessInterface.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/IntrusiveList.h>
#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

struct DummyType
{
};

/**
 * This class provides the base implementation for the server side of the Concentration Measurement cluster as well as an API for
 * setting the values of the attributes.
 *
 * @tparam NumericMeasurementEnabled whether the cluster supports numeric measurement
 * @tparam LevelIndicationEnabled whether the cluster supports level indication
 * @tparam MediumLevelEnabled whether the Level Indication Feature supports medium level
 * @tparam CriticalLevelEnabled whether the Level Indication Feature supports critical level
 * @tparam PeakMeasurementEnabled whether the Numeric Measurement Feature supports peak measurement
 * @tparam AverageMeasurementEnabled whether the Numeric Measurement Feature supports average measurement
 */
template <bool NumericMeasurementEnabled, bool LevelIndicationEnabled, bool MediumLevelEnabled, bool CriticalLevelEnabled,
          bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
class Instance : public AttributeAccessInterface
{
private:
    static const int WINDOW_MAX = 604800;

    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // Attribute data store
    MeasurementMediumEnum mMeasurementMedium;
    std::conditional_t<NumericMeasurementEnabled, DataModel::Nullable<float>, DummyType> mMeasuredValue;
    std::conditional_t<NumericMeasurementEnabled, DataModel::Nullable<float>, DummyType> mMinMeasuredValue;
    std::conditional_t<NumericMeasurementEnabled, DataModel::Nullable<float>, DummyType> mMaxMeasuredValue;
    std::conditional_t<NumericMeasurementEnabled, MeasurementUnitEnum, DummyType> mMeasurementUnit;
    std::conditional_t<NumericMeasurementEnabled, float, DummyType> mUncertainty;
    std::conditional_t<PeakMeasurementEnabled, DataModel::Nullable<float>, DummyType> mPeakMeasuredValue;
    std::conditional_t<PeakMeasurementEnabled, uint32_t, DummyType> mPeakMeasuredValueWindow;
    std::conditional_t<AverageMeasurementEnabled, DataModel::Nullable<float>, DummyType> mAverageMeasuredValue;
    std::conditional_t<AverageMeasurementEnabled, uint32_t, DummyType> mAverageMeasuredValueWindow;
    std::conditional_t<LevelIndicationEnabled, LevelValueEnum, DummyType> mLevel;

    uint32_t mFeature = 0;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mMeasuredValue));
            }
            break;

        case Attributes::MinMeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mMinMeasuredValue));
            }
            break;

        case Attributes::MaxMeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mMaxMeasuredValue));
            }
            break;

        case Attributes::Uncertainty::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mUncertainty));
            }
            break;

        case Attributes::MeasurementUnit::Id:
            ReturnErrorOnFailure(aEncoder.Encode(mMeasurementUnit));
            break;

        case Attributes::PeakMeasuredValue::Id:
            if constexpr (PeakMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mPeakMeasuredValue));
            }
            break;

        case Attributes::PeakMeasuredValueWindow::Id:
            if constexpr (PeakMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mPeakMeasuredValueWindow));
            }
            break;

        case Attributes::AverageMeasuredValue::Id:
            if constexpr (AverageMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mAverageMeasuredValue));
            }
            break;

        case Attributes::AverageMeasuredValueWindow::Id:
            if constexpr (AverageMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mAverageMeasuredValueWindow));
            }
            break;

        case Attributes::LevelValue::Id:
            if constexpr (LevelIndicationEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(mLevel));
            }
            break;

        case Attributes::MeasurementMedium::Id:
            ReturnErrorOnFailure(aEncoder.Encode(mMeasurementMedium));
            break;
        }

        return CHIP_NO_ERROR;
    };

    /**
     * This checks if the clusters instance is a valid ResourceMonitoring cluster based on the AliasedClusters list.
     * @return true if the cluster is a valid ResourceMonitoring cluster.
     */
    bool IsValidAliasCluster() const
    {
        for (unsigned int AliasedCluster : AliasedClusters)
        {
            if (mClusterId == AliasedCluster)
            {
                return true;
            }
        }
        return false;
    };

    /**
     * This generates a feature bitmap from the enabled features and then returns its raw value.
     * @return The raw feature bitmap.
     */
    uint32_t GenerateFeatureMap() const
    {
        BitMask<Feature, uint32_t> featureMap(0);

        if constexpr (NumericMeasurementEnabled)
        {
            featureMap.Set(Feature::kNumericMeasurement);
        }

        if constexpr (LevelIndicationEnabled)
        {
            featureMap.Set(Feature::kLevelIndication);
        }

        if constexpr (MediumLevelEnabled)
        {
            featureMap.Set(Feature::kMediumLevel);
        }

        if constexpr (CriticalLevelEnabled)
        {
            featureMap.Set(Feature::kCriticalLevel);
        }

        if constexpr (PeakMeasurementEnabled)
        {
            featureMap.Set(Feature::kPeakMeasurement);
        }

        if constexpr (AverageMeasurementEnabled)
        {
            featureMap.Set(Feature::kAverageMeasurement);
        }

        return featureMap.Raw();
    };

    /**
     * This checks is a given value is within the min and max constraints.
     * @param value The value to check.
     * @param minValue The minimum value.
     * @param maxValue The maximum value.
     * @return true if the value is within the min and max constraints.
     */
    static bool CheckConstraintMinMax(DataModel::Nullable<float> value, DataModel::Nullable<float> minValue,
                                      DataModel::Nullable<float> maxValue)
    {
        if (!minValue.IsNull() && !value.IsNull() && (minValue.Value() > value.Value()))
        {
            return false;
        }

        if (!maxValue.IsNull() && !value.IsNull() && (maxValue.Value() < value.Value()))
        {
            return false;
        }

        return true;
    };

    /**
     * This checks is a given value is greater than a given value.
     * @param value The value to check.
     * @param valueToBeGreaterThan The value to be greater than.
     * @return true if the value is greater than the given value.
     */
    static bool CheckConstraintsGreaterThan(DataModel::Nullable<float> value, DataModel::Nullable<float> valueToBeGreaterThan)
    {
        if (!valueToBeGreaterThan.IsNull() && !value.IsNull() && (valueToBeGreaterThan.Value() > value.Value()))
        {
            return false;
        }

        return true;
    };

    /**
     * This checks is a given value is less than a given value.
     * @param value The value to check.
     * @param valueToBeLessThan The value to be less than.
     * @return true if the value is less than the given value.
     */
    static bool CheckConstraintsLessThan(DataModel::Nullable<float> value, DataModel::Nullable<float> valueToBeLessThan)
    {
        if (!valueToBeLessThan.IsNull() && !value.IsNull() && (valueToBeLessThan.Value() < value.Value()))
        {
            return false;
        }

        return true;
    };

public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * This constructor should be used when not using the kNumericMeasurement feature.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aMeasurementMedium The measurement medium.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium) :
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mEndpointId(aEndpointId), mClusterId(aClusterId),
        mMeasurementMedium(aMeasurementMedium){};

    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * This constructor should be used when using the kNumericMeasurement feature.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aMeasurementMedium The measurement medium.
     * @param aMeasurementUnit The measurement unit.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium,
             MeasurementUnitEnum aMeasurementUnit) :
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mMeasurementMedium(aMeasurementMedium),
        mMeasurementUnit(aMeasurementUnit){};

    ~Instance() override { unregisterAttributeAccessOverride(this); };

    CHIP_ERROR Init()
    {
        static_assert(NumericMeasurementEnabled || LevelIndicationEnabled,
                      "At least one of NumericMeasurement or LevelIndication "
                      "should be enabled");

        static_assert(!MediumLevelEnabled || LevelIndicationEnabled,
                      "MediumLevelEnabled requires LevelIndicationEnabled to be true");

        static_assert(!CriticalLevelEnabled || LevelIndicationEnabled,
                      "CriticalLevelEnabled requires LevelIndicationEnabled to be true");

        static_assert(!PeakMeasurementEnabled || NumericMeasurementEnabled,
                      "PeakMeasurementEnabled requires NumericMeasurementEnabled to be true");

        static_assert(!AverageMeasurementEnabled || NumericMeasurementEnabled,
                      "AverageMeasurementEnabled requires NumericMeasurementEnabled to be true");

        VerifyOrReturnError(IsValidAliasCluster(), CHIP_ERROR_INCORRECT_STATE);

        // Check if the cluster has been selected in zap
        VerifyOrReturnError(emberAfContainsServer(mEndpointId, mClusterId), CHIP_ERROR_INCORRECT_STATE);

        // Register the object as attribute provider
        VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

        mFeature = GenerateFeatureMap();

        return CHIP_NO_ERROR;
    };

    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<float> aMeasuredValue)
    {
        if constexpr (NumericMeasurementEnabled)
        {
            if (!CheckConstraintMinMax(aMeasuredValue, mMinMeasuredValue, mMaxMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            DataModel::Nullable<float> oldValue = mMeasuredValue;
            mMeasuredValue                      = aMeasuredValue;

            if (oldValue != mMeasuredValue)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MeasuredValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetMinMeasuredValue(DataModel::Nullable<float> aMinMeasuredValue)
    {
        if constexpr (NumericMeasurementEnabled)
        {
            if (!CheckConstraintsLessThan(aMinMeasuredValue, mMaxMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!CheckConstraintsLessThan(aMinMeasuredValue, mMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            DataModel::Nullable<float> oldValue = mMinMeasuredValue;
            mMinMeasuredValue                   = aMinMeasuredValue;

            if (oldValue != mMinMeasuredValue)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MinMeasuredValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetMaxMeasuredValue(DataModel::Nullable<float> aMaxMeasuredValue)
    {
        if constexpr (NumericMeasurementEnabled)
        {
            if (!CheckConstraintsGreaterThan(aMaxMeasuredValue, mMinMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!CheckConstraintsGreaterThan(aMaxMeasuredValue, mMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            DataModel::Nullable<float> oldValue = mMaxMeasuredValue;
            mMaxMeasuredValue                   = aMaxMeasuredValue;

            if (oldValue != mMaxMeasuredValue)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MaxMeasuredValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetUncertainty(float aUncertainty)
    {
        if constexpr (NumericMeasurementEnabled)
        {
            float oldValue = mUncertainty;
            mUncertainty   = aUncertainty;

            if (oldValue != mUncertainty)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::Uncertainty::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetPeakMeasuredValue(DataModel::Nullable<float> aPeakMeasuredValue)
    {
        if constexpr (PeakMeasurementEnabled)
        {
            if (!CheckConstraintMinMax(aPeakMeasuredValue, mMinMeasuredValue, mMaxMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            DataModel::Nullable<float> oldValue = mPeakMeasuredValue;
            mPeakMeasuredValue                  = aPeakMeasuredValue;

            if (oldValue != mPeakMeasuredValue)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::PeakMeasuredValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetPeakMeasuredValueWindow(uint32_t aPeakMeasuredValueWindow)
    {
        if constexpr (PeakMeasurementEnabled)
        {
            if (aPeakMeasuredValueWindow > WINDOW_MAX)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            uint32_t oldValue        = mPeakMeasuredValueWindow;
            mPeakMeasuredValueWindow = aPeakMeasuredValueWindow;

            if (oldValue != mPeakMeasuredValueWindow)
            {
                ConcreteAttributePath path =
                    ConcreteAttributePath(mEndpointId, mClusterId, Attributes::PeakMeasuredValueWindow::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetAverageMeasuredValue(DataModel::Nullable<float> aAverageMeasuredValue)
    {
        if constexpr (AverageMeasurementEnabled)
        {
            if (!CheckConstraintMinMax(aAverageMeasuredValue, mMinMeasuredValue, mMaxMeasuredValue))
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            DataModel::Nullable<float> oldValue = mAverageMeasuredValue;
            mAverageMeasuredValue               = aAverageMeasuredValue;

            if (oldValue != mAverageMeasuredValue)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::AverageMeasuredValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetAverageMeasuredValueWindow(uint32_t aAverageMeasuredValueWindow)
    {
        if constexpr (AverageMeasurementEnabled)
        {
            if (aAverageMeasuredValueWindow > WINDOW_MAX)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            uint32_t oldValue           = mAverageMeasuredValueWindow;
            mAverageMeasuredValueWindow = aAverageMeasuredValueWindow;

            if (oldValue != mAverageMeasuredValueWindow)
            {
                ConcreteAttributePath path =
                    ConcreteAttributePath(mEndpointId, mClusterId, Attributes::AverageMeasuredValueWindow::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };

    CHIP_ERROR SetLevelValue(LevelValueEnum aLevel)
    {
        if constexpr (LevelIndicationEnabled)
        {
            if constexpr (!MediumLevelEnabled)
            {
                if (aLevel == LevelValueEnum::kMedium)
                {
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }
            }

            if constexpr (!CriticalLevelEnabled)
            {
                if (aLevel == LevelValueEnum::kCritical)
                {
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }
            }

            LevelValueEnum oldValue = mLevel;
            mLevel                  = aLevel;

            if (oldValue != mLevel)
            {
                ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LevelValue::Id);
                MatterReportingAttributeChangeCallback(path);
            }

            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    };
};

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with only the NumericMeasurement feature
 * enabled.
 *
 * @tparam PeakMeasurementEnabled Whether the PeakMeasurement feature is enabled.
 * @tparam AverageMeasurementEnabled Whether the AverageMeasurement feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @param aMeasurementUnit The measurement unit.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>
CreateNumericMeasurementConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium,
                                             MeasurementUnitEnum aMeasurementUnit)
{
    return Instance<true, false, false, false, PeakMeasurementEnabled, AverageMeasurementEnabled>(
        endpoint, clusterId, aMeasurementMedium, aMeasurementUnit);
}

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with only the Level Indication feature
 * enabled.
 *
 * @tparam MediumLevelEnabled Whether the MediumLevel feature is enabled.
 * @tparam CriticalLevelEnabled Whether the CriticalLevel feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool MediumLevelEnabled, bool CriticalLevelEnabled>
Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>
CreateLevelIndicationConcentrationCluster(EndpointId endpoint, ClusterId clusterId, MeasurementMediumEnum aMeasurementMedium)
{
    return Instance<false, true, MediumLevelEnabled, CriticalLevelEnabled, false, false>(endpoint, clusterId, aMeasurementMedium);
}

/**
 * A factory function to create a new instance of a Concentration Measurement Cluster with both the NumericMeasurement and Level
 * Indication features enabled.
 *
 * @tparam MediumLevelEnabled Whether the MediumLevel feature is enabled.
 * @tparam CriticalLevelEnabled Whether the CriticalLevel feature is enabled.
 * @tparam PeakMeasurementEnabled Whether the PeakMeasurement feature is enabled.
 * @tparam AverageMeasurementEnabled Whether the AverageMeasurement feature is enabled.
 * @param endpoint Endpoint that the cluster is on.
 * @param clusterId Cluster that the cluster is on.
 * @param aMeasurementMedium The measurement medium.
 * @param aMeasurementUnit The measurement unit.
 * @return A new instance of Concentration Measurement Cluster.
 */
template <bool MediumLevelEnabled, bool CriticalLevelEnabled, bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Instance<true, true, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled, AverageMeasurementEnabled>
CreateNumericMeasurementAndLevelIndicationConcentrationCluster(EndpointId endpoint, ClusterId clusterId,
                                                               MeasurementMediumEnum aMeasurementMedium,
                                                               MeasurementUnitEnum aMeasurementUnit)
{
    return Instance<true, true, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled, AverageMeasurementEnabled>(
        endpoint, clusterId, aMeasurementMedium, aMeasurementUnit);
}

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
