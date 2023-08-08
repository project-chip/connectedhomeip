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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

Instance::Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium, uint32_t aFeature) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mEndpointId(aEndpointId), mClusterId(aClusterId),
    mMeasurementMedium(aMeasurementMedium), mFeature(aFeature)
{
    mMeasuredValue.SetNull();
    mMinMeasuredValue.SetNull();
    mMaxMeasuredValue.SetNull();
    mPeakMeasuredValue.SetNull();
    mAverageMeasuredValue.SetNull();

    mMeasurementUnit = MeasurementUnitEnum::kUnknownEnumValue;
    mLevel           = LevelValueEnum::kLow;

    VerifyOrDieWithMsg(Init() == CHIP_NO_ERROR, Zcl, "Concentration Measurement failed to initialize for endpoint:%d", mEndpointId);
}

Instance::Instance(EndpointId aEndpointId, ClusterId aClusterId, MeasurementMediumEnum aMeasurementMedium,
                   MeasurementUnitEnum aMeasurementUnit, uint32_t aFeature) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId),
    mEndpointId(aEndpointId), mClusterId(aClusterId), mMeasurementMedium(aMeasurementMedium), mMeasurementUnit(aMeasurementUnit),
    mFeature(aFeature)
{
    VerifyOrDieWithMsg(HasFeature(Feature::kNumericMeasurement), Zcl,
                       "For endpoint %d, the numeric measurement feature must be supported if setting a unit", mEndpointId);

    mMeasuredValue.SetNull();
    mMinMeasuredValue.SetNull();
    mMaxMeasuredValue.SetNull();
    mPeakMeasuredValue.SetNull();
    mAverageMeasuredValue.SetNull();

    mLevel = LevelValueEnum::kLow;

    VerifyOrDieWithMsg(Init() == CHIP_NO_ERROR, Zcl, "Concentration Measurement failed to initialize for endpoint:%d", mEndpointId);
}

bool Instance::HasFeature(Feature feature) const
{
    return (mFeature & to_underlying(feature)) != 0;
}

CHIP_ERROR Instance::SetMeasuredValue(DataModel::Nullable<float> aMeasuredValue)
{
    if (!HasFeature(Feature::kNumericMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (!aMeasuredValue.IsNull())
    {
        if (!mMaxMeasuredValue.IsNull())
        {
            if (aMeasuredValue.Value() > mMaxMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (!mMinMeasuredValue.IsNull())
        {
            if (aMeasuredValue.Value() < mMinMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        DataModel::Nullable<float> oldValue = mMeasuredValue;
        mMeasuredValue                      = aMeasuredValue;

        if (oldValue != mMeasuredValue)
        {
            ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::MeasuredValue::Id);
            MatterReportingAttributeChangeCallback(path);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMinMeasuredValue(DataModel::Nullable<float> aMinMeasuredValue)
{
    if (!HasFeature(Feature::kNumericMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (!aMinMeasuredValue.IsNull())
    {
        if (!mMaxMeasuredValue.IsNull())
        {
            if (aMinMeasuredValue.Value() > mMaxMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (!mMeasuredValue.IsNull())
        {
            if (aMinMeasuredValue.Value() > mMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
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

CHIP_ERROR Instance::SetMaxMeasuredValue(DataModel::Nullable<float> aMaxMeasuredValue)
{
    if (!HasFeature(Feature::kNumericMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (!aMaxMeasuredValue.IsNull())
    {
        if (!mMinMeasuredValue.IsNull())
        {
            if (aMaxMeasuredValue.Value() < mMinMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (!mMeasuredValue.IsNull())
        {
            if (aMaxMeasuredValue.Value() < mMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
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

CHIP_ERROR Instance::SetPeakMeasuredValue(DataModel::Nullable<float> aPeakMeasuredValue)
{
    if (!HasFeature(Feature::kPeakMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (!aPeakMeasuredValue.IsNull() &&
        ((aPeakMeasuredValue.Value() > mMaxMeasuredValue.Value()) || (aPeakMeasuredValue.Value() < mMinMeasuredValue.Value())))
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

CHIP_ERROR Instance::SetPeakMeasuredValueWindow(uint32_t aPeakMeasuredValueWindow)
{
    if (!HasFeature(Feature::kPeakMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (aPeakMeasuredValueWindow > WINDOW_MAX)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint32_t oldValue        = mPeakMeasuredValueWindow;
    mPeakMeasuredValueWindow = aPeakMeasuredValueWindow;

    if (oldValue != mPeakMeasuredValueWindow)
    {
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::PeakMeasuredValueWindow::Id);
        MatterReportingAttributeChangeCallback(path);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetAverageMeasuredValue(DataModel::Nullable<float> aAverageMeasuredValue)
{
    if (!HasFeature(Feature::kAverageMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (!aAverageMeasuredValue.IsNull())
    {
        if (!mMaxMeasuredValue.IsNull())
        {
            if (aAverageMeasuredValue.Value() > mMaxMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (!mMinMeasuredValue.IsNull())
        {
            if (aAverageMeasuredValue.Value() < mMinMeasuredValue.Value())
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
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

CHIP_ERROR Instance::SetAverageMeasuredValueWindow(uint32_t aAverageMeasuredValueWindow)
{
    if (!HasFeature(Feature::kAverageMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (aAverageMeasuredValueWindow > WINDOW_MAX)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint32_t oldValue           = mAverageMeasuredValueWindow;
    mAverageMeasuredValueWindow = aAverageMeasuredValueWindow;

    if (oldValue != mAverageMeasuredValueWindow)
    {
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::AverageMeasuredValueWindow::Id);
        MatterReportingAttributeChangeCallback(path);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetUncertainty(float aUncertainty)
{
    if (!HasFeature(Feature::kNumericMeasurement))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    float oldValue = mUncertainty;
    mUncertainty   = aUncertainty;

    if (oldValue != mUncertainty)
    {
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, mClusterId, Attributes::Uncertainty::Id);
        MatterReportingAttributeChangeCallback(path);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetLevel(LevelValueEnum aLevel)
{
    if (!HasFeature(Feature::kLevelIndication))
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (aLevel == LevelValueEnum::kUnknownEnumValue)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!HasFeature(Feature::kMediumLevel) && aLevel == LevelValueEnum::kMedium)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!HasFeature(Feature::kCriticalLevel) && aLevel == LevelValueEnum::kCritical)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
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

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::MeasuredValue::Id:
        if (HasFeature(Feature::kNumericMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mMeasuredValue));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::MinMeasuredValue::Id:
        if (HasFeature(Feature::kNumericMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mMinMeasuredValue));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::MaxMeasuredValue::Id:
        if (HasFeature(Feature::kNumericMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mMaxMeasuredValue));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::PeakMeasuredValue::Id:
        if (HasFeature(Feature::kNumericMeasurement) && HasFeature(Feature::kPeakMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mPeakMeasuredValue));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::PeakMeasuredValueWindow::Id:
        if (HasFeature(Feature::kNumericMeasurement) && HasFeature(Feature::kPeakMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mPeakMeasuredValueWindow));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::AverageMeasuredValue::Id:
        if (HasFeature(Feature::kNumericMeasurement) && HasFeature(Feature::kAverageMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mAverageMeasuredValue));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::AverageMeasuredValueWindow::Id:
        if (HasFeature(Feature::kNumericMeasurement) && HasFeature(Feature::kAverageMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mAverageMeasuredValueWindow));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::Uncertainty::Id:
        if (HasFeature(Feature::kNumericMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mUncertainty));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::MeasurementUnit::Id:
        if (HasFeature(Feature::kNumericMeasurement))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mMeasurementUnit));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;

    case Attributes::MeasurementMedium::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMeasurementMedium));
        break;

    case Attributes::LevelValue::Id:
        if (HasFeature(Feature::kLevelIndication))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mLevel));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(IsValidAliasCluster(), CHIP_ERROR_INCORRECT_STATE);

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(emberAfContainsServer(mEndpointId, mClusterId), CHIP_ERROR_INCORRECT_STATE);

    // Check for conformance of Feature Map - Either MEA or LEV must be supported
    VerifyOrReturnError((HasFeature(Feature::kNumericMeasurement) || HasFeature(Feature::kLevelIndication)),
                        CHIP_ERROR_INCORRECT_STATE);

    // Register the object as attribute provider
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

bool Instance::IsValidAliasCluster() const
{
    for (unsigned int AliasedCluster : AliasedClusters)
    {
        if (mClusterId == AliasedCluster)
        {
            return true;
        }
    }
    return false;
}

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
