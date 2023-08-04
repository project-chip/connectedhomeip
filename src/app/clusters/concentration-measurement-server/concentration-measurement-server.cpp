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
#include <app/clusters/mode-base-server/mode-base-server.h>
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

Instance::Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mEndpointId(aEndpointId), mClusterId(aClusterId),
    mFeature(aFeature)
{
    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mEndpointId, mClusterId) == true);

    // Check for conformance of Feature Map - Either MEA or LEV must be supported
    VerifyOrReturnError((HasFeature(Feature::kNumericMeasurement) || HasFeature(Feature::kLevelIndication)),
                        CHIP_ERROR_INCORRECT_STATE);

    // Register the object as attribute provider
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
}

bool Instance::HasFeature(Feature feature) const
{
    return (mFeature & to_underlying(feature)) != 0;
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

    case Attributes::Level::Id:
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

} // namespace ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
