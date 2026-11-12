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
#include <app/clusters/meter-identification-server/CodegenIntegration.h>
#include <app/clusters/meter-identification-server/MeterIdentificationCluster.h>
#include <app/static-cluster-config/MeterIdentification.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;

namespace {

constexpr size_t kMeterIdentificationFixedClusterCount =
    MeterIdentification::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kMeterIdentificationMaxClusterCount =
    kMeterIdentificationFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<MeterIdentificationCluster> gServers[kMeterIdentificationMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        MeterIdentificationCluster::Config config;
        config.WithFeatures(BitFlags<Feature>(featureMap));
        config.WithOptionalAttributes(MeterIdentificationCluster::OptionalAttributeSet(optionalAttributeBits));

        if (config.mOptionalAttributes.IsSet(PowerThreshold::Id))
        {
            config.WithFeature(Feature::kPowerThreshold);
        }

        MeterType::GetDefaultOr(endpointId, config.mMeterType, DataModel::NullNullable);
        PointOfDelivery::GetDefaultOr(endpointId, config.mPointOfDelivery, DataModel::NullNullable);
        MeterSerialNumber::GetDefaultOr(endpointId, config.mMeterSerialNumber, DataModel::NullNullable);
        if (config.mOptionalAttributes.IsSet(ProtocolVersion::Id))
        {
            DataModel::Nullable<CharSpan> protocolVersion;
            ProtocolVersion::GetDefaultOr(endpointId, protocolVersion, DataModel::NullNullable);
            config.WithProtocolVersion(protocolVersion);
        }

        gServers[clusterInstanceIndex].Create(endpointId, config);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gServers[clusterInstanceIndex].Destroy();
    }
};

} // namespace

void MatterMeterIdentificationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = MeterIdentification::Id,
            .fixedClusterInstanceCount = kMeterIdentificationFixedClusterCount,
            .maxClusterInstanceCount   = kMeterIdentificationMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterMeterIdentificationClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = MeterIdentification::Id,
            .fixedClusterInstanceCount = kMeterIdentificationFixedClusterCount,
            .maxClusterInstanceCount   = kMeterIdentificationMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

// Legacy callback stubs
void MatterMeterIdentificationPluginServerInitCallback() {}
void MatterMeterIdentificationPluginServerShutdownCallback() {}

namespace chip::app::Clusters::MeterIdentification {

MeterIdentificationCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = MeterIdentification::Id,
            .fixedClusterInstanceCount = kMeterIdentificationFixedClusterCount,
            .maxClusterInstanceCount   = kMeterIdentificationMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<MeterIdentificationCluster *>(cluster);
}

} // namespace chip::app::Clusters::MeterIdentification
