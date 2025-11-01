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

#include <app/clusters/soil-measurement-server/CodegenIntegration.h>
#include <app/clusters/soil-measurement-server/soil-measurement-cluster.h>
#include <app/static-cluster-config/SoilMeasurement.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace {

constexpr size_t kSoilMeasurementFixedClusterCount = SoilMeasurement::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kSoilMeasurementMaxClusterCount   = kSoilMeasurementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<SoilMeasurementCluster> gServer;

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

constexpr EndpointId kEndpointWithSoilMeasurement = 1;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId, kDefaultSoilMoistureMeasurementLimits);
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterSoilMeasurementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = SoilMeasurement::Id,
            .fixedClusterInstanceCount = kSoilMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kSoilMeasurementMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);

    // Set initial measured value to min since all clusters does not perform any actual measurements
    gServer.Cluster().SetSoilMoistureMeasuredValue(kDefaultSoilMoistureMeasurementLimits.minMeasuredValue);
}

void MatterSoilMeasurementClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = SoilMeasurement::Id,
            .fixedClusterInstanceCount = kSoilMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kSoilMeasurementMaxClusterCount,
        },
        integrationDelegate);
}

namespace chip::app::Clusters::SoilMeasurement {

CHIP_ERROR
SetSoilMoistureMeasuredValue(const SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    return gServer.Cluster().SetSoilMoistureMeasuredValue(soilMoistureMeasuredValue);
}

} // namespace chip::app::Clusters::SoilMeasurement
