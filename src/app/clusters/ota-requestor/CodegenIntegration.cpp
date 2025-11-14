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

/* This file contains the glue code for passing the incoming OTA Requestor cluster commands
 * to the OTA Requestor object that handles them
 */

#include <app/clusters/ota-requestor/OTARequestorCluster.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/static-cluster-config/OtaSoftwareUpdateRequestor.h>
#include <app/util/attribute-table.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kOtaRequestorFixedClusterCount =
    OtaSoftwareUpdateRequestor::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kOtaRequestorMaxClusterCount = kOtaRequestorFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// Uses the global singleton OTARequestorInterface as its data source.
class OTARequestorClusterUsingSingleton : public OTARequestorCluster
{
public:
    explicit OTARequestorClusterUsingSingleton(EndpointId endpointId) : OTARequestorCluster(endpointId, nullptr) {}

    OTARequestorInterface * OtaRequestorInstance() override { return GetRequestorInstance(); }
};

LazyRegisteredServerCluster<OTARequestorClusterUsingSingleton> gServers[kOtaRequestorMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId);
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

void MatterOtaSoftwareUpdateRequestorClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OtaSoftwareUpdateRequestor::Id,
            .fixedClusterInstanceCount = kOtaRequestorFixedClusterCount,
            .maxClusterInstanceCount   = kOtaRequestorMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterOtaSoftwareUpdateRequestorClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OtaSoftwareUpdateRequestor::Id,
            .fixedClusterInstanceCount = kOtaRequestorFixedClusterCount,
            .maxClusterInstanceCount   = kOtaRequestorMaxClusterCount,
        },
        integrationDelegate);
}

void MatterOtaSoftwareUpdateRequestorPluginServerInitCallback() {}
void MatterOtaSoftwareUpdateRequestorPluginServerShutdownCallback() {}
