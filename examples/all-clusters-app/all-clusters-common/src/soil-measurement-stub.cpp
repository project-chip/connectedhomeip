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

#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <soil-measurement-stub.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace {

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

LazyRegisteredServerCluster<SoilMeasurementCluster> gServer;

constexpr EndpointId kEndpointWithSoilMeasurement = 1;

bool ValidEndpointForSoilMeasurement(EndpointId endpoint)
{
    if (endpoint != kEndpointWithSoilMeasurement)
    {
        ChipLogError(AppServer, "SoilMeasurement cluster invalid endpoint");
        return false;
    }
    return true;
}

} // namespace

void emberAfSoilMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(ValidEndpointForSoilMeasurement(endpoint));

    gServer.Create(endpoint, kDefaultSoilMoistureMeasurementLimits);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SoilMeasurement cluster error registration");
        return;
    }

    // Set initial measured value to min since all clusters does not perform any actual measurements
    TEMPORARY_RETURN_IGNORED gServer.Cluster().SetSoilMoistureMeasuredValue(kDefaultSoilMoistureMeasurementLimits.minMeasuredValue);
}

void emberAfSoilMeasurementClusterShutdownCallback(EndpointId endpoint)
{
    VerifyOrReturn(ValidEndpointForSoilMeasurement(endpoint));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SoilMeasurement unregister error");
    }

    gServer.Destroy();
}

namespace chip::app::Clusters::SoilMeasurement {

CHIP_ERROR
SetSoilMoistureMeasuredValue(const SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    return gServer.Cluster().SetSoilMoistureMeasuredValue(soilMoistureMeasuredValue);
}

} // namespace chip::app::Clusters::SoilMeasurement
