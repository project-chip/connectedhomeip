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
#include <app/clusters/time-synchronization-server/CodegenIntegration.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/static-cluster-config/ValveConfigurationAndControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
using namespace chip::app::Clusters::ValveConfigurationAndControl::Attributes;

namespace {

constexpr size_t kValveConfigurationAndControlFixedClusterCount =
    ValveConfigurationAndControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kValveConfigurationAndControlMaxClusterCount =
    kValveConfigurationAndControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ValveConfigurationAndControlCluster> gServers[kValveConfigurationAndControlMaxClusterCount];

class CodegenTimeSyncTracker : public TimeSyncTracker
{
public:
    bool IsTimeSyncClusterSupported() override { return (nullptr != TimeSynchronization::GetClusterInstance()); }

    bool IsValidUTCTime() override
    {
        if (IsTimeSyncClusterSupported())
        {
            return TimeSynchronization::GetClusterInstance()->GetGranularity() !=
                TimeSynchronization::GranularityEnum::kNoTimeGranularity;
        }
        return false;
    }
};

CodegenTimeSyncTracker codegenTracker;
class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // Create OptionalAttributeSet from optionalAttributeBits
        ValveConfigurationAndControlCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        gServers[clusterInstanceIndex].Create(endpointId, BitFlags<ValveConfigurationAndControl::Feature>(featureMap),
                                              optionalAttributeSet, &codegenTracker);
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

void MatterValveConfigurationAndControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterValveConfigurationAndControlClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);
}

void MatterValveConfigurationAndControlPluginServerInitCallback() {}

void MatterValveConfigurationAndControlPluginServerShutdownCallback() {}

namespace chip::app::Clusters::ValveConfigurationAndControl {

void SetDelegate(EndpointId endpointId, Delegate * delegate)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * interface = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);

    VerifyOrReturn(interface != nullptr);

    static_cast<ValveConfigurationAndControlCluster *>(interface)->SetDelegate(delegate);
}

ValveConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * valveConfigurationAndControl = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ValveConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kValveConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kValveConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ValveConfigurationAndControlCluster *>(valveConfigurationAndControl);
}

} // namespace chip::app::Clusters::ValveConfigurationAndControl
