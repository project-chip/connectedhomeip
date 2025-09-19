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

#include <app/clusters/icd-management-server/icd-management-cluster.h>
#include <app/static-cluster-config/IcdManagement.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <zap-generated/gen_config.h>

namespace {
 using namespace chip;
 using namespace chip::app;
 using namespace chip::app::Clusters;
 using namespace chip::app::Clusters::IcdManagement;
 
 static_assert((IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
                IcdManagement::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
               IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size() == 0);
 
LazyRegisteredServerCluster<ICDManagementCluster> gServer;

#if CHIP_CONFIG_ENABLE_ICD_CIP
static IcdManagementFabricDelegate gFabricDelegate;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

constexpr chip::BitMask<OptionalCommands> kEnabledCommands()
{
    chip::BitMask<OptionalCommands> result;
#ifdef ICD_MANAGEMENT_STAY_ACTIVE_REQUEST_COMMAND
    result.Set(kStayActiveRequest);
#endif
#ifdef ICD_MANAGEMENT_STAY_ACTIVE_RESPONSE_COMMAND
    result.Set(kStayActiveResponse);
#endif
    return result;
}

 class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ICDManagementCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        const auto enabledCommands = kEnabledCommands();
        gServer.Create(Server::GetInstance().GetPersistentStorage(), *Server::GetInstance().GetSessionKeystore(), Server::GetInstance().GetFabricTable(), ICDConfigurationData::GetInstance().GetInstance(),
        BitFlags<IcdManagement::Feature>(featureMap), optionalAttributeSet, enabledCommands);
        return gServer.Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned clusterInstanceIndex) override
    {
        return gServer.Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};
} // namespace

 void MatterIcdManagementPluginServerInitCallback()
{
    PersistentStorageDelegate & storage           = Server::GetInstance().GetPersistentStorage();
    Crypto::SymmetricKeystore * symmetricKeystore = Server::GetInstance().GetSessionKeystore();
    FabricTable & fabricTable                     = Server::GetInstance().GetFabricTable();
    ICDConfigurationData & icdConfigurationData   = ICDConfigurationData::GetInstance().GetInstance();

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                        = kRootEndpointId,
            .clusterId                         = IcdManagement::Id,
            .fixedClusterServerEndpointCount   = IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxEndpointCount                  = 1, // only root-node functionality supported by this implementation
            .fetchFeatureMap                   = true,
            .fetchOptionalAttributes           = true,
        },
        integrationDelegate);

#if CHIP_CONFIG_ENABLE_ICD_CIP
    // Configure and register Fabric delegate
    gFabricDelegate.Init(storage, symmetricKeystore, icdConfigurationData);
    fabricTable.AddFabricDelegate(&gFabricDelegate);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP


}

void MatterIcdManagementPluginServerShutdownCallback()
{
#if CHIP_CONFIG_ENABLE_ICD_CIP
    FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    fabricTable.RemoveFabricDelegate(&gFabricDelegate);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                        = kRootEndpointId,
            .clusterId                         = IcdManagement::Id,
            .fixedClusterServerEndpointCount   = IcdManagement::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxEndpointCount                  = 1, // only root-node functionality supported by this implementation
        },
        integrationDelegate);
}