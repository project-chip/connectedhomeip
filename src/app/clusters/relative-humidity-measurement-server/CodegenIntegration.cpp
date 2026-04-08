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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/static-cluster-config/RelativeHumidityMeasurement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RelativeHumidityMeasurement;
using namespace chip::app::Clusters::RelativeHumidityMeasurement::Attributes;

namespace {

constexpr size_t kRelativeHumidityFixedClusterCount =
    RelativeHumidityMeasurement::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kRelativeHumidityMaxClusterCount = kRelativeHumidityFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<RelativeHumidityMeasurementCluster> gServers[kRelativeHumidityMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        RelativeHumidityMeasurementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        using namespace chip::Protocols::InteractionModel;

        // Read default values from the Ember attribute store. Not all apps set
        // defaults, so failure is tolerated.
        DataModel::Nullable<uint16_t> minMeasuredValue{};
        if (MinMeasuredValue::Get(endpointId, minMeasuredValue) != Status::Success)
        {
            minMeasuredValue.SetNull();
        }

        DataModel::Nullable<uint16_t> maxMeasuredValue{};
        if (MaxMeasuredValue::Get(endpointId, maxMeasuredValue) != Status::Success)
        {
            maxMeasuredValue.SetNull();
        }

        // If both values are non-null but form an invalid range (e.g. ZAP defaults of 0/0),
        // treat both as null rather than crashing.
        if (!minMeasuredValue.IsNull() && !maxMeasuredValue.IsNull() && maxMeasuredValue.Value() < minMeasuredValue.Value() + 1)
        {
            minMeasuredValue.SetNull();
            maxMeasuredValue.SetNull();
        }

        uint16_t tolerance{};
        if (optionalAttributeSet.IsSet(Tolerance::Id))
        {
            VerifyOrDie(Tolerance::Get(endpointId, &tolerance) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(endpointId, optionalAttributeSet,
                                              RelativeHumidityMeasurementCluster::StartupConfiguration{
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

void MatterRelativeHumidityMeasurementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = RelativeHumidityMeasurement::Id,
            .fixedClusterInstanceCount = kRelativeHumidityFixedClusterCount,
            .maxClusterInstanceCount   = kRelativeHumidityMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterRelativeHumidityMeasurementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = RelativeHumidityMeasurement::Id,
            .fixedClusterInstanceCount = kRelativeHumidityFixedClusterCount,
            .maxClusterInstanceCount   = kRelativeHumidityMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::RelativeHumidityMeasurement {

RelativeHumidityMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = RelativeHumidityMeasurement::Id,
            .fixedClusterInstanceCount = kRelativeHumidityFixedClusterCount,
            .maxClusterInstanceCount   = kRelativeHumidityMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<RelativeHumidityMeasurementCluster *>(cluster);
}

CHIP_ERROR SetMeasuredValue(EndpointId endpointId, DataModel::Nullable<uint16_t> measuredValue)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetMeasuredValue(measuredValue);
}

CHIP_ERROR SetMeasuredValueRange(EndpointId endpointId, DataModel::Nullable<uint16_t> minMeasuredValue,
                                 DataModel::Nullable<uint16_t> maxMeasuredValue)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue);
}

} // namespace chip::app::Clusters::RelativeHumidityMeasurement
