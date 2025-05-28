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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <clusters/SoilMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

Instance::Instance(EndpointId aEndpointId) :
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), SoilMeasurement::Id), mEndpointId(aEndpointId)
{}

Instance::~Instance()
{
    Shutdown();
}

CHIP_ERROR Instance::Init(const Globals::Structs::MeasurementAccuracyStruct::Type & measurementLimits)
{
    VerifyOrDie(emberAfContainsServer(mEndpointId, SoilMeasurement::Id) == true);

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    mSoilMeasurementData.soilMoistureMeasurementLimits = measurementLimits;
    mSoilMeasurementData.soilMoistureMeasuredValue.SetNull();

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR
Instance::SetSoilMeasuredValue(const Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    if (mSoilMeasurementData.soilMoistureMeasuredValue != soilMoistureMeasuredValue)
    {
        mSoilMeasurementData.soilMoistureMeasuredValue = soilMoistureMeasuredValue;

        MatterReportingAttributeChangeCallback(mEndpointId, SoilMeasurement::Id, SoilMoistureMeasuredValue::Id);
    }

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
        return aEncoder.Encode(kRevision);
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
