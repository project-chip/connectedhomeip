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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/pressure-measurement-server/CodegenIntegration.h>
#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>
#include <app/static-cluster-config/PressureMeasurement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PressureMeasurement;
using namespace chip::app::Clusters::PressureMeasurement::Attributes;

namespace {

constexpr size_t kPressureMeasurementFixedClusterCount = PressureMeasurement::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kPressureMeasurementMaxClusterCount =
    kPressureMeasurementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<PressureMeasurementCluster> gServers[kPressureMeasurementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        using namespace chip::Protocols::InteractionModel;

        PressureMeasurementCluster::Config config;

        if (MinMeasuredValue::Get(endpointId, config.minMeasuredValue) != Status::Success)
        {
            config.minMeasuredValue.SetNull();
        }

        if (MaxMeasuredValue::Get(endpointId, config.maxMeasuredValue) != Status::Success)
        {
            config.maxMeasuredValue.SetNull();
        }

        // If both values are non-null but form an invalid range, treat both as null
        if (!config.minMeasuredValue.IsNull() && !config.maxMeasuredValue.IsNull() &&
            config.maxMeasuredValue.Value() < config.minMeasuredValue.Value() + 1)
        {
            config.minMeasuredValue.SetNull();
            config.maxMeasuredValue.SetNull();
        }

        PressureMeasurementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        if (optionalAttributeSet.IsSet(Tolerance::Id))
        {
            uint16_t tolerance{};
            VerifyOrDie(Tolerance::Get(endpointId, &tolerance) == Status::Success);
            config.WithTolerance(tolerance);
        }

        // EXT feature: ScaledValue, MinScaledValue, MaxScaledValue, Scale are mandated by Extended feature
        BitFlags<PressureMeasurement::Feature> features(featureMap);
        if (features.Has(PressureMeasurement::Feature::kExtended))
        {
            DataModel::Nullable<int16_t> minScaledValue;
            DataModel::Nullable<int16_t> maxScaledValue;
            int8_t scale = 0;

            MinScaledValue::Get(endpointId, minScaledValue);
            MaxScaledValue::Get(endpointId, maxScaledValue);
            Scale::Get(endpointId, &scale);

            config.WithExtendedFeature(minScaledValue, maxScaledValue, scale);

            if (optionalAttributeSet.IsSet(ScaledTolerance::Id))
            {
                uint16_t scaledTolerance{};
                VerifyOrDie(ScaledTolerance::Get(endpointId, &scaledTolerance) == Status::Success);
                config.WithScaledTolerance(scaledTolerance);
            }
        }

        gServers[clusterInstanceIndex].Create(endpointId, config);
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

void MatterPressureMeasurementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PressureMeasurement::Id,
            .fixedClusterInstanceCount = kPressureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kPressureMeasurementMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterPressureMeasurementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PressureMeasurement::Id,
            .fixedClusterInstanceCount = kPressureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kPressureMeasurementMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::PressureMeasurement {

PressureMeasurementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * pressureMeasurement = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = PressureMeasurement::Id,
            .fixedClusterInstanceCount = kPressureMeasurementFixedClusterCount,
            .maxClusterInstanceCount   = kPressureMeasurementMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<PressureMeasurementCluster *>(pressureMeasurement);
}

CHIP_ERROR SetMeasuredValue(EndpointId endpointId, DataModel::Nullable<int16_t> measuredValue)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetMeasuredValue(measuredValue);
}

CHIP_ERROR SetScaledValue(EndpointId endpointId, DataModel::Nullable<int16_t> scaledValue)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetScaledValue(scaledValue);
}

} // namespace chip::app::Clusters::PressureMeasurement
