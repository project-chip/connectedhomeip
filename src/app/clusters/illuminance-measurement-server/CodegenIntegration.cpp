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

#include "CodegenIntegration.h"
#include <app/clusters/illuminance-measurement-server/IlluminanceMeasurementCluster.h>
#include <app/static-cluster-config/IlluminanceMeasurement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IlluminanceMeasurement;
using namespace chip::app::Clusters::IlluminanceMeasurement::Attributes;

namespace {

constexpr size_t kIlluminanceMeasurementFixedClusterCount =
    IlluminanceMeasurement::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kIlluminanceMeasurementMaxClusterCount =
    kIlluminanceMeasurementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<IlluminanceMeasurementCluster> gServers[kIlluminanceMeasurementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        using namespace chip::Protocols::InteractionModel;

        DataModel::Nullable<uint16_t> minMeasuredValue{};
        VerifyOrDie(MinMeasuredValue::Get(endpointId, minMeasuredValue) == Status::Success);

        DataModel::Nullable<uint16_t> maxMeasuredValue{};
        VerifyOrDie(MaxMeasuredValue::Get(endpointId, maxMeasuredValue) == Status::Success);

        uint16_t tolerance{};
        if (optionalAttributeSet.IsSet(Tolerance::Id))
        {
            VerifyOrDie(Tolerance::Get(endpointId, &tolerance) == Status::Success);
        }

        DataModel::Nullable<LightSensorTypeEnum> lightSensorType{};
        if (optionalAttributeSet.IsSet(LightSensorType::Id))
        {
            VerifyOrDie(LightSensorType::Get(endpointId, lightSensorType) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet,
                                              IlluminanceMeasurementCluster::StartupConfiguration{
                                                  .minMeasuredValue = minMeasuredValue,
                                                  .maxMeasuredValue = maxMeasuredValue,
                                                  .tolerance        = tolerance,
                                                  .lightSensorType  = lightSensorType,
                                              });
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterIlluminanceMeasurementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = IlluminanceMeasurement::Id,
            .fixedClusterInstanceCount = kIlluminanceMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kIlluminanceMeasurementMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterIlluminanceMeasurementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = IlluminanceMeasurement::Id,
            .fixedClusterInstanceCount = kIlluminanceMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kIlluminanceMeasurementMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::IlluminanceMeasurement {

IlluminanceMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * illuminanceMeasurement = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = IlluminanceMeasurement::Id,
            .fixedClusterInstanceCount = kIlluminanceMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kIlluminanceMeasurementMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<IlluminanceMeasurementCluster *>(illuminanceMeasurement);
}

} // namespace chip::app::Clusters::IlluminanceMeasurement
