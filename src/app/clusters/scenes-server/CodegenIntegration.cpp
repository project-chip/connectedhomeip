/**
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

#include <app/clusters/scenes-server/CodegenIntegration.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/ScenesManagement.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

// Cluster configuration sets values based on this. Ensure config is valid.
// This means it is NOT sufficient to just set the SCENES_MANAGEMENT_TABLE_SIZE in ZAP, but rather
// the CHIP_CONFIG must be updated.

#if defined(SCENES_MANAGEMENT_TABLE_SIZE) && SCENES_MANAGEMENT_TABLE_SIZE
static_assert(chip::scenes::kMaxScenesPerEndpoint >= SCENES_MANAGEMENT_TABLE_SIZE,
              "ZAP configuration for scenes table should be at most CHIP_CONFIG_MAX_SCENES_TABLE_SIZE");
#endif

using SceneTable = chip::scenes::SceneTable<chip::scenes::ExtensionFieldSetsImpl>;

namespace chip::app::Clusters::ScenesManagement {

namespace {

class DefaultScenesManagementTableProvider : public ScenesManagementTableProvider
{
public:
    void SetParameters(EndpointId endpointId, uint16_t endpointTableSize)
    {
        mEndpointId        = endpointId;
        mEndpointTableSize = endpointTableSize;
    }
    EndpointId GetEndpointId() const { return mEndpointId; }

    ScenesManagementSceneTable * Take() override { return scenes::GetSceneTableImpl(mEndpointId, mEndpointTableSize); }
    void Release(ScenesManagementSceneTable *) override {}

private:
    EndpointId mEndpointId      = kInvalidEndpointId;
    uint16_t mEndpointTableSize = scenes::kMaxScenesPerEndpoint;
};

constexpr size_t kScenesManagementFixedClusterCount = StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kScenesManagementMaxClusterCount = kScenesManagementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ScenesManagementCluster> gServers[kScenesManagementMaxClusterCount];
DefaultScenesManagementTableProvider gTableProviders[kScenesManagementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        uint16_t endpointTableSize = 0;
        VerifyOrDie(Attributes::SceneTableSize::Get(endpointId, &endpointTableSize) ==
                    Protocols::InteractionModel::Status::Success);

        // Guaranteed non-null: if CreateRegistration is called, this means the ember cluster
        // was found (because we have a valid clusterInstanceIndex)
        const EmberAfCluster * cluster = emberAfFindServerCluster(endpointId, ScenesManagement::Id);

        bool supportsCopyScene = false;
        // NOTE: acceptedCommandList is KNOWN to not be nullptr because ScenesManagement should have mandatory commands
        //       as such we iterate here directly without a 'empty list' (i.e. nullptr) check.
        for (const CommandId * cmd = cluster->acceptedCommandList; *cmd != kInvalidCommandId; cmd++)
        {
            if (*cmd == ScenesManagement::Commands::CopyScene::Id)
            {
                supportsCopyScene = true;
                break;
            }
        }

        gTableProviders[clusterInstanceIndex].SetParameters(endpointId, endpointTableSize);
        gServers[clusterInstanceIndex].Create(endpointId,
                                              ScenesManagementCluster::Context{
                                                  .groupDataProvider  = Credentials::GetGroupDataProvider(),
                                                  .fabricTable        = &Server::GetInstance().GetFabricTable(),
                                                  .features           = BitMask<ScenesManagement::Feature>(featureMap),
                                                  .sceneTableProvider = gTableProviders[clusterInstanceIndex],
                                                  .supportsCopyScene  = supportsCopyScene,
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

ScenesManagementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ScenesManagement::Id,
            .fixedClusterInstanceCount = kScenesManagementFixedClusterCount,
            .maxClusterInstanceCount   = kScenesManagementMaxClusterCount,
        },
        integrationDelegate);
    return static_cast<ScenesManagementCluster *>(cluster);
}

ScenesServer & ScenesServer::Instance()
{
    static ScenesServer gInstance;
    return gInstance;
}

bool ScenesServer::IsHandlerRegistered(EndpointId aEndpointId, scenes::SceneHandler * handler)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);
    return sceneTable->mHandlerList.Contains(handler);
}

void ScenesServer::RegisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);

    if (!IsHandlerRegistered(aEndpointId, handler))
    {
        sceneTable->RegisterHandler(handler);
    }
}

void ScenesServer::UnregisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);

    if (IsHandlerRegistered(aEndpointId, handler))
    {
        sceneTable->UnregisterHandler(handler);
    }
}

void ScenesServer::GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);

    TEMPORARY_RETURN_IGNORED cluster->GroupWillBeRemoved(aFabricIx, aGroupId);
}

void ScenesServer::MakeSceneInvalid(EndpointId aEndpointId, FabricIndex aFabricIx)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);

    TEMPORARY_RETURN_IGNORED cluster->MakeSceneInvalid(aFabricIx);
}

void ScenesServer::MakeSceneInvalidForAllFabrics(EndpointId aEndpointId)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);
    TEMPORARY_RETURN_IGNORED cluster->MakeSceneInvalidForAllFabrics();
}

void ScenesServer::StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);
    TEMPORARY_RETURN_IGNORED cluster->StoreCurrentScene(aFabricIx, aGroupId, aSceneId);
}

void ScenesServer::RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);
    TEMPORARY_RETURN_IGNORED cluster->RecallScene(aFabricIx, aGroupId, aSceneId);
}

void ScenesServer::RemoveFabric(EndpointId aEndpointId, FabricIndex aFabricIndex)
{
    ScenesManagementCluster * cluster = FindClusterOnEndpoint(aEndpointId);
    VerifyOrReturn(cluster != nullptr);
    TEMPORARY_RETURN_IGNORED cluster->RemoveFabric(aFabricIndex);
}

} // namespace chip::app::Clusters::ScenesManagement

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ScenesManagement;

void MatterScenesManagementClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ScenesManagement::Id,
            .fixedClusterInstanceCount = kScenesManagementFixedClusterCount,
            .maxClusterInstanceCount   = kScenesManagementMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterScenesManagementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ScenesManagement::Id,
            .fixedClusterInstanceCount = kScenesManagementFixedClusterCount,
            .maxClusterInstanceCount   = kScenesManagementMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterScenesManagementPluginServerInitCallback() {}
