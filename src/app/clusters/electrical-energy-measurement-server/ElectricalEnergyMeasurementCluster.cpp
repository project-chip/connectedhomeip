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
#include "ElectricalEnergyMeasurementCluster.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueEncoder.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/cluster-enums.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

using namespace chip;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

CHIP_ERROR ElectricalEnergyMeasurementAttrAccess::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void ElectricalEnergyMeasurementAttrAccess::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR ElectricalEnergyMeasurementAttrAccess::Read(const app::ConcreteReadAttributePath & aPath,
                                                       app::AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == app::Clusters::ElectricalEnergyMeasurement::Id);

    MeasurementData * data = MeasurementDataForEndpoint(aPath.mEndpointId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case Accuracy::Id:
        if (data == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        return aEncoder.Encode(data->measurementAccuracy);
    case CumulativeEnergyImported::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) &&
                HasFeature(ElectricalEnergyMeasurement::Feature::kImportedEnergy),
            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Energy Measurement: can not get CumulativeEnergyImported, feature is not supported"));
        if ((data == nullptr) || !data->cumulativeImported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->cumulativeImported.Value());
    case CumulativeEnergyExported::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) &&
                HasFeature(ElectricalEnergyMeasurement::Feature::kExportedEnergy),
            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Energy Measurement: can not get CumulativeEnergyExported, feature is not supported"));
        if ((data == nullptr) || !data->cumulativeExported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->cumulativeExported.Value());
    case PeriodicEnergyImported::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) &&
                HasFeature(ElectricalEnergyMeasurement::Feature::kImportedEnergy),
            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Energy Measurement: can not get PeriodicEnergyImported, feature is not supported"));
        if ((data == nullptr) || !data->periodicImported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->periodicImported.Value());
    case PeriodicEnergyExported::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) &&
                HasFeature(ElectricalEnergyMeasurement::Feature::kExportedEnergy),
            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Energy Measurement: can not get PeriodicEnergyExported, feature is not supported"));
        if ((data == nullptr) || !data->periodicExported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->periodicExported.Value());
    case CumulativeEnergyReset::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Energy Measurement: can not get CumulativeEnergyReset, feature is not supported"));

        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        if ((data == nullptr) || !data->cumulativeReset.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->cumulativeReset.Value());
    }

    return CHIP_NO_ERROR;
}

bool ElectricalEnergyMeasurementAttrAccess::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

bool ElectricalEnergyMeasurementAttrAccess::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
