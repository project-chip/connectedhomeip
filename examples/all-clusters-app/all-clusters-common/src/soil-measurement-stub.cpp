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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <soil-measurement-stub.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;

namespace {
LazyRegisteredServerCluster<SoilMeasurementCluster> gServer;

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};
} // namespace

void emberAfSoilMeasurementClusterServerInitCallback(EndpointId endpoint) {}

void emberAfSoilMeasurementClusterInitCallback(EndpointId endpoint)
{
    // This cluster is only enabled for endpoint 1.
    VerifyOrReturn(endpoint == 1);

    gServer.Create(endpoint);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SoilMeasurement cluster error registration");
    }
    else
    {
        VerifyOrDie(emberAfContainsServer(endpoint, SoilMeasurement::Id) == true);

        gServer.Cluster().Init(kDefaultSoilMoistureMeasurementLimits);
    }
}

void emberAfSoilMeasurementClusterShutdownCallback(EndpointId endpoint)
{
    // This cluster is only enabled for endpoint 1.
    VerifyOrReturn(endpoint == 1);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SoilMeasurement unregister error");
    }

    gServer.Destroy();
}

Protocols::InteractionModel::Status
MatterSoilMeasurementClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                              EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{

    return Protocols::InteractionModel::Status::Success;
}

void MatterSoilMeasurementPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace SoilMeasurement {

CHIP_ERROR
SetSoilMoistureMeasuredValue(EndpointId endpoint,
                             const Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    // This cluster is only enabled for endpoint 1.
    VerifyOrReturnError(endpoint == 1, CHIP_ERROR_INVALID_ARGUMENT);

    return gServer.Cluster().SetSoilMoistureMeasuredValue(endpoint, soilMoistureMeasuredValue);
}

} // namespace SoilMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
