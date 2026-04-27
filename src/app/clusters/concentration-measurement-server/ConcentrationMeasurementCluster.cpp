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

#include "ConcentrationMeasurementCluster.h"
#include "ConcentrationMeasurementDelegate.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters::ConcentrationMeasurement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ConcentrationMeasurement {

ConcentrationMeasurementCluster::ConcentrationMeasurementCluster(EndpointId endpointId, ClusterId clusterId,
                                                                 BitFlags<Feature> features, Delegate & delegate) :
    DefaultServerCluster({ endpointId, clusterId }),
    mFeatures(features), mDelegate(delegate)
{
    bool validCluster = false;
    for (ClusterId id : AliasedClusters)
    {
        if (id == clusterId)
        {
            validCluster = true;
            break;
        }
    }
    VerifyOrDie(validCluster);
    if (mFeatures.HasAny(Feature::kMediumLevel, Feature::kCriticalLevel))
    {
        mFeatures.Set(Feature::kLevelIndication);
    }

    // Peak and Average require Numeric
    if (mFeatures.HasAny(Feature::kPeakMeasurement, Feature::kAverageMeasurement))
    {
        mFeatures.Set(Feature::kNumericMeasurement);
    }
}

ConcentrationMeasurementCluster::~ConcentrationMeasurementCluster() {}

CHIP_ERROR ConcentrationMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    return mDelegate.Init();
}
DataModel::ActionReturnStatus ConcentrationMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case MeasurementMedium::Id:
        return encoder.Encode(mDelegate.GetMeasurementMedium());

    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(kClusterRevision);

    case MeasuredValue::Id:
        return encoder.Encode(mDelegate.GetMeasuredValue());

    case MinMeasuredValue::Id:
        return encoder.Encode(mDelegate.GetMinMeasuredValue());

    case MaxMeasuredValue::Id:
        return encoder.Encode(mDelegate.GetMaxMeasuredValue());

    case Uncertainty::Id:
        return encoder.Encode(mDelegate.GetUncertainty());

    case MeasurementUnit::Id:
        return encoder.Encode(mDelegate.GetMeasurementUnit());

    case PeakMeasuredValue::Id:
        return encoder.Encode(mDelegate.GetPeakMeasuredValue());

    case PeakMeasuredValueWindow::Id:
        return encoder.Encode(mDelegate.GetPeakMeasuredValueWindow());

    case AverageMeasuredValue::Id:
        return encoder.Encode(mDelegate.GetAverageMeasuredValue());

    case AverageMeasuredValueWindow::Id:
        return encoder.Encode(mDelegate.GetAverageMeasuredValueWindow());

    case LevelValue::Id:
        return encoder.Encode(mDelegate.GetLevelValue());

    default:
        return Status::UnsupportedAttribute;
    }
}
CHIP_ERROR ConcentrationMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // 1 mandatory + 5 numeric + 2 peak + 2 average + 1 level + 10 globals
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(21));

    ReturnErrorOnFailure(DefaultServerCluster::Attributes(path, builder));

    using Flags                                 = BitFlags<DataModel::AttributeQualityFlags>;
    constexpr chip::Access::Privilege kViewPriv = chip::Access::Privilege::kView;

    ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasurementMedium::Id, Flags{}, kViewPriv, std::nullopt)));

    if (mFeatures.Has(Feature::kNumericMeasurement))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasuredValue::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MinMeasuredValue::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MaxMeasuredValue::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(Uncertainty::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(MeasurementUnit::Id, Flags{}, kViewPriv, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kPeakMeasurement))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(PeakMeasuredValue::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(
            builder.Append(DataModel::AttributeEntry(PeakMeasuredValueWindow::Id, Flags{}, kViewPriv, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kAverageMeasurement))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(AverageMeasuredValue::Id, Flags{}, kViewPriv, std::nullopt)));
        ReturnErrorOnFailure(
            builder.Append(DataModel::AttributeEntry(AverageMeasuredValueWindow::Id, Flags{}, kViewPriv, std::nullopt)));
    }

    if (mFeatures.Has(Feature::kLevelIndication))
    {
        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry(LevelValue::Id, Flags{}, kViewPriv, std::nullopt)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kNumericMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    auto min = mDelegate.GetMinMeasuredValue();
    auto max = mDelegate.GetMaxMeasuredValue();
    if (!value.IsNull())
    {
        if (!min.IsNull() && value.Value() < min.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        if (!max.IsNull() && value.Value() > max.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    if (mDelegate.GetMeasuredValue() != value)
    {
        mDelegate.SetMeasuredValue(value);
        NotifyAttributeChanged(MeasuredValue::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    auto min = mDelegate.GetMinMeasuredValue();
    auto max = mDelegate.GetMaxMeasuredValue();
    if (!value.IsNull())
    {
        if (!min.IsNull() && value.Value() < min.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        if (!max.IsNull() && value.Value() > max.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    if (mDelegate.GetPeakMeasuredValue() != value)
    {
        mDelegate.SetPeakMeasuredValue(value);
        NotifyAttributeChanged(PeakMeasuredValue::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetPeakMeasuredValueWindow(uint32_t value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kPeakMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (mDelegate.GetPeakMeasuredValueWindow() != value)
    {
        mDelegate.SetPeakMeasuredValueWindow(value);
        NotifyAttributeChanged(PeakMeasuredValueWindow::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValue(DataModel::Nullable<float> value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    auto min = mDelegate.GetMinMeasuredValue();
    auto max = mDelegate.GetMaxMeasuredValue();
    if (!value.IsNull())
    {
        if (!min.IsNull() && value.Value() < min.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        if (!max.IsNull() && value.Value() > max.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    if (mDelegate.GetAverageMeasuredValue() != value)
    {
        mDelegate.SetAverageMeasuredValue(value);
        NotifyAttributeChanged(AverageMeasuredValue::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetAverageMeasuredValueWindow(uint32_t value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kAverageMeasurement), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value <= kWindowMaxSeconds, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (mDelegate.GetAverageMeasuredValueWindow() != value)
    {
        mDelegate.SetAverageMeasuredValueWindow(value);
        NotifyAttributeChanged(AverageMeasuredValueWindow::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConcentrationMeasurementCluster::SetLevelValue(LevelValueEnum value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kLevelIndication), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(value < LevelValueEnum::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if ((value == LevelValueEnum::kMedium) && !mFeatures.Has(Feature::kMediumLevel))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    if ((value == LevelValueEnum::kCritical) && !mFeatures.Has(Feature::kCriticalLevel))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    if (mDelegate.GetLevelValue() != value)
    {
        mDelegate.SetLevelValue(value);
        NotifyAttributeChanged(LevelValue::Id);
    }
    return CHIP_NO_ERROR;
}

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
