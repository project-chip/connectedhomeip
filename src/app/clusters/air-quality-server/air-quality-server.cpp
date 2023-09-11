/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app-common/zap-generated/ids/Clusters.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/air-quality-server/air-quality-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace AirQuality {

Instance::Instance(EndpointId aEndpointId, BitMask<Feature> aFeature) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id), mEndpointId(aEndpointId), mFeature(aFeature)
{}

Instance::~Instance()
{
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mEndpointId, Id) == true);

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

Protocols::InteractionModel::Status Instance::UpdateAirQuality(AirQualityEnum aNewAirQuality)
{
    // Check that the value in is valid according to the enabled features.
    switch (aNewAirQuality)
    {
    case AirQualityEnum::kFair: {
        if (!HasFeature(Feature::kFair))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kModerate: {
        if (!HasFeature(Feature::kModerate))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kVeryPoor: {
        if (!HasFeature(Feature::kVeryPoor))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kExtremelyPoor: {
        if (!HasFeature(Feature::kExtremelyPoor))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kUnknown:
    case AirQualityEnum::kGood:
    case AirQualityEnum::kPoor:
        break;
    default: {
        return Protocols::InteractionModel::Status::InvalidValue;
    }
    }

    mAirQuality = aNewAirQuality;
    MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, Id, Attributes::AirQuality::Id));
    return Protocols::InteractionModel::Status::Success;
}

AirQualityEnum Instance::GetAirQuality()
{
    return mAirQuality;
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::AirQuality::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mAirQuality));
        break;
    case Attributes::FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature.Raw()));
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace AirQuality
} // namespace Clusters
} // namespace app
} // namespace chip
