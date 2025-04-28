/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "soil-measurement-server.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <zap-generated/gen_config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;
using namespace chip::app::Clusters::SoilMeasurement::Structs;

MeasurementData gMeasurements[MATTER_DM_SOIL_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];

MeasurementAccuracyRangeStruct::Type soilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

CHIP_ERROR SoilMeasurementAttrAccess::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    // Initialize the soil moisture measurement limits to default values
    for (auto & measurement : gMeasurements)
    {
        measurement.soilMoistureMeasurementLimits = { .measurementType  = MeasurementTypeEnum::kSoilMoisture,
                                                      .measured         = true,
                                                      .minMeasuredValue = 0,
                                                      .maxMeasuredValue = 100,
                                                      .accuracyRanges = DataModel::List<const MeasurementAccuracyRangeStruct::Type>(
                                                          soilMoistureMeasurementLimitsAccuracyRange) };
    }

    return CHIP_NO_ERROR;
}

void SoilMeasurementAttrAccess::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR SoilMeasurementAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == SoilMeasurement::Id);

    MeasurementData * data = SoilMeasurementDataForEndpoint(aPath.mEndpointId);

    switch (aPath.mAttributeId)
    {
    case Attributes::SoilMoistureMeasurementLimits::Id: {
        return aEncoder.Encode(data->soilMoistureMeasurementLimits);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

MeasurementData * SoilMeasurementDataForEndpoint(EndpointId endpointId)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpointId, app::Clusters::SoilMeasurement::Id,
                                                      MATTER_DM_SOIL_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    if (index >= MATTER_ARRAY_SIZE(gMeasurements))
    {
        ChipLogError(NotSpecified, "Internal error: invalid/unexpected energy measurement index.");
        return nullptr;
    }
    return &gMeasurements[index];
}

// This function is intended for the application to set the soil measurement accuracy limits to the proper values during init.
// Given the limits are fixed, it is not intended to be changes at runtime, hence why this function does not report the change.
// The application should call this function only once during init.
CHIP_ERROR SetSoilMeasurementAccuracy(EndpointId endpointId, const MeasurementAccuracyStruct::Type & measurementLimits)
{
    MeasurementData * data = SoilMeasurementDataForEndpoint(endpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    data->soilMoistureMeasurementLimits = measurementLimits;

    return CHIP_NO_ERROR;
}

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
