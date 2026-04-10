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

#include <app/clusters/temperature-measurement-server/CodegenIntegration.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <app/static-cluster-config/TemperatureMeasurement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureMeasurement;
using namespace chip::app::Clusters::TemperatureMeasurement::Attributes;

namespace {

constexpr size_t kTemperatureMeasurementFixedClusterCount =
    TemperatureMeasurement::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kTemperatureMeasurementMaxClusterCount =
    kTemperatureMeasurementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<TemperatureMeasurementCluster> gServers[kTemperatureMeasurementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        using namespace chip::Protocols::InteractionModel;

        // Try to read the default value for these mandatory attributes but do not fail if the operation is not successful.
        // This is because not all apps are setting a default value for them in ember.
        DataModel::Nullable<int16_t> minMeasuredValue{};
        if (MinMeasuredValue::Get(endpointId, minMeasuredValue) != Status::Success)
        {
            minMeasuredValue.SetNull();
        }

        DataModel::Nullable<int16_t> maxMeasuredValue{};
        if (MaxMeasuredValue::Get(endpointId, maxMeasuredValue) != Status::Success)
        {
            maxMeasuredValue.SetNull();
        }

        uint16_t tolerance{};
        if (optionalAttributeSet.IsSet(Tolerance::Id))
        {
            VerifyOrDie(Tolerance::Get(endpointId, &tolerance) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet,
                                              TemperatureMeasurementCluster::StartupConfiguration{
                                                  .minMeasuredValue = minMeasuredValue,
                                                  .maxMeasuredValue = maxMeasuredValue,
                                                  .tolerance        = tolerance,
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

void MatterTemperatureMeasurementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureMeasurement::Id,
            .fixedClusterInstanceCount = kTemperatureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureMeasurementMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterTemperatureMeasurementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureMeasurement::Id,
            .fixedClusterInstanceCount = kTemperatureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureMeasurementMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::TemperatureMeasurement {

TemperatureMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * temperatureMeasurement = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureMeasurement::Id,
            .fixedClusterInstanceCount = kTemperatureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureMeasurementMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<TemperatureMeasurementCluster *>(temperatureMeasurement);
}

CHIP_ERROR SetMeasuredValue(EndpointId endpointId, DataModel::Nullable<int16_t> measuredValue)
{
    auto temperatureMeasurement = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(temperatureMeasurement != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return temperatureMeasurement->SetMeasuredValue(measuredValue);
}

CHIP_ERROR SetMeasuredValueRange(EndpointId endpointId, DataModel::Nullable<int16_t> minMeasuredValue,
                                 DataModel::Nullable<int16_t> maxMeasuredValue)
{
    auto temperatureMeasurement = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(temperatureMeasurement != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return temperatureMeasurement->SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue);
}

} // namespace chip::app::Clusters::TemperatureMeasurement
