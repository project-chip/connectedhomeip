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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/IntrusiveList.h>
#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

namespace Detail {

struct DummyNumericMeasurementMembers
{
};

struct DummyPeakMeasurementMembers
{
};

struct DummyAverageMeasurementMembers
{
};

struct DummyLevelIndicationMembers
{
};

class NumericMeasurementMembers
{
protected:
    DataModel::Nullable<float> mMeasuredValue;
    DataModel::Nullable<float> mMinMeasuredValue;
    DataModel::Nullable<float> mMaxMeasuredValue;
    MeasurementUnitEnum mMeasurementUnit;
    float mUncertainty;
};

class PeakMeasurementMembers
{
protected:
    DataModel::Nullable<float> mPeakMeasuredValue;
    uint32_t mPeakMeasuredValueWindow;
};

class AverageMeasurementMembers
{
protected:
    DataModel::Nullable<float> mAverageMeasuredValue;
    uint32_t mAverageMeasuredValueWindow;
};

class LevelIndicationMembers
{
protected:
    LevelValueEnum mLevel;
};

} // namespace Detail

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
class Instance
    : public AttributeAccessInterface,
      protected std::conditional_t<NumericMeasurementEnabled, Detail::NumericMeasurementMembers,
                                   Detail::DummyNumericMeasurementMembers>,
      protected std::conditional_t<PeakMeasurementEnabled, Detail::PeakMeasurementMembers, Detail::DummyPeakMeasurementMembers>,
      protected std::conditional_t<AverageMeasurementEnabled, Detail::AverageMeasurementMembers,
                                   Detail::DummyAverageMeasurementMembers>,
      protected std::conditional_t<LevelIndicationEnabled, Detail::LevelIndicationMembers, Detail::DummyLevelIndicationMembers>
{
private:
    static const int WINDOW_MAX = 604800;

    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    MeasurementMediumEnum mMeasurementMedium;

    uint32_t mFeatureMap = 0;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mMeasuredValue));
            }
            break;

        case Attributes::MinMeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mMinMeasuredValue));
            }
            break;

        case Attributes::MaxMeasuredValue::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mMaxMeasuredValue));
            }
            break;

        case Attributes::Uncertainty::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mUncertainty));
            }
            break;

        case Attributes::MeasurementUnit::Id:
            if constexpr (NumericMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mMeasurementUnit));
                break;
            }

        case Attributes::PeakMeasuredValue::Id:
            if constexpr (PeakMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mPeakMeasuredValue));
            }
            break;

        case Attributes::PeakMeasuredValueWindow::Id:
            if constexpr (PeakMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mPeakMeasuredValueWindow));
            }
            break;

        case Attributes::AverageMeasuredValue::Id:
            if constexpr (AverageMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mAverageMeasuredValue));
            }
            break;

        case Attributes::AverageMeasuredValueWindow::Id:
            if constexpr (AverageMeasurementEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mAverageMeasuredValueWindow));
            }
            break;

        case Attributes::LevelValue::Id:
            if constexpr (LevelIndicationEnabled)
            {
                ReturnErrorOnFailure(aEncoder.Encode(this->mLevel));
            }
            break;

        case Attributes::MeasurementMedium::Id:
            ReturnErrorOnFailure(aEncoder.Encode(mMeasurementMedium));
            break;

        case Attributes::FeatureMap::Id:
            ReturnErrorOnFailure(aEncoder.Encode(mFeatureMap));
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
     * This checks if a given nullable float is within the min and max constraints or two other nullable floats.
     * @param value The value to check.
     * @param minValue The minimum value.
     * @param maxValue The maximum value.
     * @return true if the value is within the min and max constraints. If either of the pair of values being compared is null,
     * that's considered to be within the constraint.
     */
    static bool CheckConstraintMinMax(DataModel::Nullable<float> value, DataModel::Nullable<float> minValue,
                                      DataModel::Nullable<float> maxValue)
    {
        return (minValue.IsNull() || value.IsNull() || (value.Value() >= minValue.Value())) &&
            (maxValue.IsNull() || value.IsNull() || (value.Value() <= maxValue.Value()));
    };

    /**
     * This checks if a given nullable float is less than or equal to another given nullable float.
     * @param value The value to check.
     * @param valueToBeLessThanOrEqualTo The value to be less than or equal to.
     * @return true if value is less than or equal to valueToBeLessThanOrEqualTo, or if either of the values is Null.
     */
    static bool CheckConstraintsLessThanOrEqualTo(DataModel::Nullable<float> value,
                                                  DataModel::Nullable<float> valueToBeLessThanOrEqualTo)
    {
        return valueToBeLessThanOrEqualTo.IsNull() || value.IsNull() || (value.Value() <= valueToBeLessThanOrEqualTo.Value());
    };

    /**
     * This checks if a given nullable float is greater than or equal to another given nullable float.
     * @param value The value to check.
     * @param valueToBeGreaterThanOrEqualTo The value to be greater than or equal to.
     * @return true if value is greater than or equal to valueToBeGreaterThanOrEqualTo, or if either of the values is Null.
     */
    static bool CheckConstraintsGreaterThanOrEqualTo(DataModel::Nullable<float> value,
                                                     DataModel::Nullable<float> valueToBeGreaterThanOrEqualTo)
    {
        return valueToBeGreaterThanOrEqualTo.IsNull() || value.IsNull() || (value.Value() >= valueToBeGreaterThanOrEqualTo.Value());
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
        mEndpointId(aEndpointId), mClusterId(aClusterId), mMeasurementMedium(aMeasurementMedium)
    {
        this->mMeasurementUnit = aMeasurementUnit;
    };

    ~Instance() override { AttributeAccessInterfaceRegistry::Instance().Unregister(this); };

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
        VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

        mFeatureMap = GenerateFeatureMap();

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = NumericMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<float> aMeasuredValue)
    {
        if (!CheckConstraintMinMax(aMeasuredValue, this->mMinMeasuredValue, this->mMaxMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mMeasuredValue != aMeasuredValue, CHIP_NO_ERROR);
        this->mMeasuredValue = aMeasuredValue;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::MeasuredValue::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = NumericMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetMinMeasuredValue(DataModel::Nullable<float> aMinMeasuredValue)
    {
        if (!CheckConstraintsLessThanOrEqualTo(aMinMeasuredValue, this->mMaxMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (!CheckConstraintsLessThanOrEqualTo(aMinMeasuredValue, this->mMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mMinMeasuredValue != aMinMeasuredValue, CHIP_NO_ERROR);
        this->mMinMeasuredValue = aMinMeasuredValue;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::MinMeasuredValue::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = NumericMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetMaxMeasuredValue(DataModel::Nullable<float> aMaxMeasuredValue)
    {
        if (!CheckConstraintsGreaterThanOrEqualTo(aMaxMeasuredValue, this->mMinMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (!CheckConstraintsGreaterThanOrEqualTo(aMaxMeasuredValue, this->mMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mMaxMeasuredValue != aMaxMeasuredValue, CHIP_NO_ERROR);
        this->mMaxMeasuredValue = aMaxMeasuredValue;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::MaxMeasuredValue::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = NumericMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetUncertainty(float aUncertainty)
    {
        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mUncertainty != aUncertainty, CHIP_NO_ERROR);
        this->mUncertainty = aUncertainty;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::Uncertainty::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = PeakMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetPeakMeasuredValue(DataModel::Nullable<float> aPeakMeasuredValue)
    {
        if (!CheckConstraintMinMax(aPeakMeasuredValue, this->mMinMeasuredValue, this->mMaxMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mPeakMeasuredValue != aPeakMeasuredValue, CHIP_NO_ERROR);
        this->mPeakMeasuredValue = aPeakMeasuredValue;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::PeakMeasuredValue::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = PeakMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetPeakMeasuredValueWindow(uint32_t aPeakMeasuredValueWindow)
    {
        if (aPeakMeasuredValueWindow > WINDOW_MAX)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mPeakMeasuredValueWindow != aPeakMeasuredValueWindow, CHIP_NO_ERROR);
        this->mPeakMeasuredValueWindow = aPeakMeasuredValueWindow;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::PeakMeasuredValueWindow::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = AverageMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetAverageMeasuredValue(DataModel::Nullable<float> aAverageMeasuredValue)
    {
        if (!CheckConstraintMinMax(aAverageMeasuredValue, this->mMinMeasuredValue, this->mMaxMeasuredValue))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mAverageMeasuredValue != aAverageMeasuredValue, CHIP_NO_ERROR);
        this->mAverageMeasuredValue = aAverageMeasuredValue;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::AverageMeasuredValue::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = AverageMeasurementEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetAverageMeasuredValueWindow(uint32_t aAverageMeasuredValueWindow)
    {
        if (aAverageMeasuredValueWindow > WINDOW_MAX)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mAverageMeasuredValueWindow != aAverageMeasuredValueWindow, CHIP_NO_ERROR);
        this->mAverageMeasuredValueWindow = aAverageMeasuredValueWindow;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::AverageMeasuredValueWindow::Id);

        return CHIP_NO_ERROR;
    };

    template <bool Enabled = LevelIndicationEnabled, typename = std::enable_if_t<Enabled, CHIP_ERROR>>
    CHIP_ERROR SetLevelValue(LevelValueEnum aLevel)
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

        // Check to see if a change has ocurred
        VerifyOrReturnError(this->mLevel != aLevel, CHIP_NO_ERROR);
        this->mLevel = aLevel;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::LevelValue::Id);

        return CHIP_NO_ERROR;
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
