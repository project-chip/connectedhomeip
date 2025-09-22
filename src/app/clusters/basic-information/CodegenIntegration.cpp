/*
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
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/static-cluster-config/BasicInformation.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation::Attributes;
using namespace chip::app::Clusters::BasicInformation::StaticApplicationConfig;
using chip::Protocols::InteractionModel::Status;

namespace {

// BasicInformationCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kBasicInformationFixedClusterCount = BasicInformation::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kBasicInformationFixedClusterCount == 0) ||
                  ((kBasicInformationFixedClusterCount == 1) &&
                   BasicInformation::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Basic Information cluster MUST be on endpoint 0");

ServerClusterRegistration gRegistration(BasicInformationCluster::Instance());

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {

        BasicInformationCluster::Instance().OptionalAttributes() =
            BasicInformationCluster::OptionalAttributesSet(optionalAttributeBits);

        return gRegistration;
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        return &BasicInformationCluster::Instance();
    }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned clusterInstanceIndex) override {}
};

} // namespace

void MatterBasicInformationClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = BasicInformation::Id,
            .fixedClusterInstanceCount = BasicInformation::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterBasicInformationClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = BasicInformation::Id,
            .fixedClusterInstanceCount = BasicInformation::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate);
}

void MatterBasicInformationPluginServerInitCallback() {}
void MatterBasicInformationPluginServerShutdownCallback() {}
