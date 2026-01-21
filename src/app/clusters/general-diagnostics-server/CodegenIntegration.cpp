/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/InteractionModelEngine.h>
#include <app/clusters/general-diagnostics-server/CodegenIntegration.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <app/static-cluster-config/GeneralDiagnostics.h>
#include <app/util/config.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;

// for fixed endpoint, this file is ever only included IF general diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

// Determine if the configurable version of the general diagnostics cluster with additonal command options is needed
#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER) || defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
LazyRegisteredServerCluster<GeneralDiagnosticsClusterFullConfigurable> gServer;
#else
LazyRegisteredServerCluster<GeneralDiagnosticsCluster> gServer;
#endif

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        InteractionModelEngine * interactionModel = InteractionModelEngine::GetInstance();

#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER) || defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
        const GeneralDiagnosticsFunctionsConfig functionsConfig
        {
            /*
            Only consider real time if time sync cluster is actually enabled. If it's not
            enabled, this avoids likelihood of frequently reporting unusable unsynched time.
            */
#if defined(ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER)
            .enablePosixTime = true,
#else
            .enablePosixTime       = false,
#endif
#if defined(GENERAL_DIAGNOSTICS_ENABLE_PAYLOAD_TEST_REQUEST_CMD)
            .enablePayloadSnapshot = true,
#else
            .enablePayloadSnapshot = false,
#endif
        };
        gServer.Create(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(featureMap), interactionModel, functionsConfig);
#else
        gServer.Create(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(featureMap), interactionModel);
#endif
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterGeneralDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = GeneralDiagnostics::Id,
            .fixedClusterInstanceCount = GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterGeneralDiagnosticsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = GeneralDiagnostics::Id,
            .fixedClusterInstanceCount = GeneralDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate, shutdownType);
}

void MatterGeneralDiagnosticsPluginServerInitCallback() {}

void MatterGeneralDiagnosticsPluginServerShutdownCallback() {}

namespace chip::app::Clusters::GeneralDiagnostics {
void GlobalNotifyDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnDeviceReboot(bootReason);
    }
}

void GlobalNotifyHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                      const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnHardwareFaultsDetect(previous, current);
    }
}

void GlobalNotifyRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                                   const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnRadioFaultsDetect(previous, current);
    }
}

void GlobalNotifyNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                                     const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current)
{
    if (gServer.IsConstructed())
    {
        gServer.Cluster().OnNetworkFaultsDetect(previous, current);
    }
}
} // namespace chip::app::Clusters::GeneralDiagnostics
