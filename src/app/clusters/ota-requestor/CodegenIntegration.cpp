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

#include <app/clusters/ota-requestor/CodegenIntegration.h>

#include <app/clusters/ota-requestor/CodegenIntegrationInternal.h>
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

static_assert(kOtaRequestorFixedClusterCount == 0 || kOtaRequestorFixedClusterCount == 1,
              "OTA requestor is a node-scoped utility cluster, so only one may be created");

// The OTA requestor cluster may be registered by the application before the OTA requestor singleton instance is set.
// This class decouples the cluster and the singleton so the cluster doesn't need to know whether the singleton is set
// when handling commands.
class OTARequestorCommandForwarder : public OTARequestorCommandInterface
{
public:
    void
    HandleAnnounceOTAProvider(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) override
    {
        if (mDestination)
        {
            mDestination->HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
        }
    }

    void SetDestination(OTARequestorCommandInterface * destination) { mDestination = destination; }

private:
    OTARequestorCommandInterface * mDestination = nullptr;
};

// This class decouples the cluster and the DefaultOTARequestor singleton's event generation to allow the OTARequestorCluster to be
// created after DefaultOTARequestor::Init is called.
class OTAEventForwarder : public DefaultOTARequestorEventGenerator
{
public:
    ~OTAEventForwarder() override = default;

    CHIP_ERROR GenerateVersionAppliedEvent(const VersionAppliedEvent & event) override
    {
        VerifyOrReturnError(mDestination, CHIP_ERROR_INCORRECT_STATE);
        return mDestination->GenerateVersionAppliedEvent(event);
    }

    CHIP_ERROR GenerateDownloadErrorEvent(const DownloadErrorEvent & event) override
    {
        VerifyOrReturnError(mDestination, CHIP_ERROR_INCORRECT_STATE);
        return mDestination->GenerateDownloadErrorEvent(event);
    }

    void SetDestination(DefaultOTARequestorEventGenerator * destination) { mDestination = destination; }

private:
    DefaultOTARequestorEventGenerator * mDestination = nullptr;
};

OTARequestorAttributes gAttributes;
OTARequestorCommandForwarder gCommandForwarder;
OTAEventForwarder gEventForwarder;
LazyRegisteredServerCluster<OTARequestorCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId, gCommandForwarder, gAttributes);
        gEventForwarder.SetDestination(&gServer.Cluster());
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gEventForwarder.SetDestination(nullptr);
        gServer.Destroy();
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
            .maxClusterInstanceCount   = 1, // This is a node utility cluster so only 1 is supported.
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void UnregisterCluster(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = OtaSoftwareUpdateRequestor::Id,
            .fixedClusterInstanceCount = kOtaRequestorFixedClusterCount,
            .maxClusterInstanceCount   = 1, // This is a node utility cluster so only 1 is supported.
        },
        integrationDelegate, shutdownType);
}

void OnSetGlobalOtaRequestorInstance(OTARequestorInterface * instance)
{
    gCommandForwarder.SetDestination(instance);
}

} // namespace

namespace chip {

OTARequestorAttributes & GetOTARequestorAttributes()
{
    return gAttributes;
}

DefaultOTARequestorEventGenerator & GetDefaultOTARequestorEventGenerator()
{
    return gEventForwarder;
}

} // namespace chip

void MatterOtaSoftwareUpdateRequestorClusterInitCallback(EndpointId endpointId)
{
    gInternalOnSetRequestorInstance = OnSetGlobalOtaRequestorInstance;
    RegisterCluster(endpointId);
}

void MatterOtaSoftwareUpdateRequestorClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    UnregisterCluster(endpointId, shutdownType);
}

void MatterOtaSoftwareUpdateRequestorPluginServerInitCallback() {}
void MatterOtaSoftwareUpdateRequestorPluginServerShutdownCallback() {}
