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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

static const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

static const Globals::Structs::MeasurementAccuracyStruct::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

Instance::Instance(EndpointId aEndpointId) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id), mEndpointId(aEndpointId)
{}

Instance::~Instance()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::Init()
{
    VerifyOrDie(emberAfContainsServer(mEndpointId, Id) == true);

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    // Initialize the soil moisture measurement limits to default values
    mSoilMeasurementData.soilMoistureMeasurementLimits = kDefaultSoilMoistureMeasurementLimits;
    mSoilMeasurementData.soilMoistureMeasuredValue.SetNull();

    return CHIP_NO_ERROR;
}

// This function is intended for the application to set the soil measurement accuracy limits to the proper values during init.
// Given the limits are fixed, it is not intended to be changes at runtime, hence why this function does not report the change.
// The application should call this function only once during init.
CHIP_ERROR
Instance::SetSoilMeasurementAccuracy(const Globals::Structs::MeasurementAccuracyStruct::Type & measurementLimits, bool reportChange)
{
    mSoilMeasurementData.soilMoistureMeasurementLimits = measurementLimits;

    if (reportChange)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, SoilMeasurement::Id, SoilMoistureMeasurementLimits::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
Instance::SetSoilMeasuredValue(const Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    mSoilMeasurementData.soilMoistureMeasuredValue = soilMoistureMeasuredValue;

    MatterReportingAttributeChangeCallback(mEndpointId, SoilMeasurement::Id, SoilMoistureMeasuredValue::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == SoilMeasurement::Id);

    switch (aPath.mAttributeId)
    {
    case SoilMoistureMeasurementLimits::Id: {
        return aEncoder.Encode(mSoilMeasurementData.soilMoistureMeasurementLimits);
    }
    case SoilMoistureMeasuredValue::Id: {
        return aEncoder.Encode(mSoilMeasurementData.soilMoistureMeasuredValue);
    }
    case ClusterRevision::Id: {
        return aEncoder.Encode(kClusterRevision);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
