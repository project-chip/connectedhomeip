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

#include "CodegenIntegration.h"
#include <app/clusters/illuminance-measurement-server/illuminance-measurement-cluster.h>
#include <app/static-cluster-config/IlluminanceMeasurement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IlluminanceMeasurement;
using namespace chip::app::Clusters::IlluminanceMeasurement::Attributes;
using namespace chip::Protocols::InteractionModel;

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
        LightSensorTypeEnum lightSensorType = LightSensorTypeEnum::kPhotodiode;
        if (optionalAttributeSet.IsSet(LightSensorType::Id))
        {
            // VerifyOrDie(LightSensorType::Get(endpointId, &lightSensorType) == Status::Success); //revisar
        }

        Tolerance::TypeInfo::Type tolerance{};
        if (optionalAttributeSet.IsSet(Tolerance::Id))
        {
            // VerifyOrDie(Tolerance::Get(endpointId, &tolerance) == Status::Success); //revisar
        }
        gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet,
                                              IlluminanceMeasurementCluster::StartupConfiguration{
                                                  .lightSensorType = lightSensorType,
                                                  .tolerance       = tolerance,
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

void MatterIlluminanceMeasurementClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = IlluminanceMeasurement::Id,
            .fixedClusterInstanceCount = kIlluminanceMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kIlluminanceMeasurementMaxClusterCount,
        },
        integrationDelegate);
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
