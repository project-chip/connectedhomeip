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
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/static-cluster-config/OtaSoftwareUpdateRequestor.h>
#include <app/util/attribute-table.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kOtaRequestorFixedClusterCount =
    OtaSoftwareUpdateRequestor::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kOtaRequestorMaxClusterCount = kOtaRequestorFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// OTA requestor clusters may be registered by the application before the OTA requestor singleton instance is set.
// A fallback OTA requestor cluster is registered when the OTA requestor singleton hasn't been set. The fallback
// is unregistered when the singleton is set.

class FallbackOtaRequestorCluster : public DefaultServerCluster
{
public:
    FallbackOtaRequestorCluster(EndpointId endpointId) :
        DefaultServerCluster(ConcreteClusterPath(endpointId, OtaSoftwareUpdateRequestor::Id)) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id:
            return encoder.EncodeEmptyList();
        case OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id:
            return encoder.Encode(true);
        case OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id:
            return encoder.Encode(OtaSoftwareUpdateRequestor::UpdateStateEnum::kUnknown);
        case OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id:
            return encoder.EncodeNull();
        case OtaSoftwareUpdateRequestor::Attributes::FeatureMap::Id:
            return encoder.Encode<uint32_t>(0);
        case OtaSoftwareUpdateRequestor::Attributes::ClusterRevision::Id:
            return encoder.Encode(OtaSoftwareUpdateRequestor::kRevision);
        default:
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        AttributeListBuilder listBuilder(builder);
        return listBuilder.Append(Span(OtaSoftwareUpdateRequestor::Attributes::kMandatoryMetadata), {});
    }
};

LazyRegisteredServerCluster<OTARequestorCluster> gServers[kOtaRequestorMaxClusterCount];
LazyRegisteredServerCluster<FallbackOtaRequestorCluster> gFallbackServers[kOtaRequestorMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        if (GetRequestorInstance())
        {
            gServers[clusterInstanceIndex].Create(endpointId, *GetRequestorInstance());
            return gServers[clusterInstanceIndex].Registration();
        }
        else
        {
            gFallbackServers[clusterInstanceIndex].Create(endpointId);
            return gFallbackServers[clusterInstanceIndex].Registration();
        }
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        if (gServers[clusterInstanceIndex].IsConstructed())
        {
            return &gServers[clusterInstanceIndex].Cluster();
        }
        else if (gFallbackServers[clusterInstanceIndex].IsConstructed())
        {
            return &gFallbackServers[clusterInstanceIndex].Cluster();
        }
        return nullptr;
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        if (gServers[clusterInstanceIndex].IsConstructed())
        {
            gServers[clusterInstanceIndex].Destroy();
        }
        else
        {
            gFallbackServers[clusterInstanceIndex].Destroy();
        }
    }
};

void RegisterCluster(EndpointId endpointId)
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

void UnregisterCluster(EndpointId endpointId)
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

void OnSetGlobalOtaRequestorInstance(OTARequestorInterface * instance)
{
    // Re-register non-fallback servers first, or otherwise they would get re-registered a second time.
    for (auto & server : gServers)
    {
        if (server.IsConstructed())
        {
            EndpointId endpoint = server.Cluster().GetPaths()[0].mEndpointId;
            UnregisterCluster(endpoint);
            RegisterCluster(endpoint);
        }
    }
    for (auto & server : gFallbackServers)
    {
        if (server.IsConstructed())
        {
            EndpointId endpoint = server.Cluster().GetPaths()[0].mEndpointId;
            UnregisterCluster(endpoint);
            RegisterCluster(endpoint);
        }
    }
}

} // namespace

void MatterOtaSoftwareUpdateRequestorClusterInitCallback(EndpointId endpointId)
{
    internalOnSetRequestorInstance = OnSetGlobalOtaRequestorInstance;
    RegisterCluster(endpointId);
}

void MatterOtaSoftwareUpdateRequestorClusterShutdownCallback(EndpointId endpointId)
{
    UnregisterCluster(endpointId);
}

void MatterOtaSoftwareUpdateRequestorPluginServerInitCallback() {}
void MatterOtaSoftwareUpdateRequestorPluginServerShutdownCallback() {}
