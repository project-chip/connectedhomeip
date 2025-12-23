/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <app/clusters/scenes-server/ExtensionFieldSets.h>
#include <app/clusters/scenes-server/ExtensionFieldSetsImpl.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ScenesManagement/AttributeIds.h>
#include <clusters/ScenesManagement/ClusterId.h>
#include <clusters/ScenesManagement/Commands.h>
#include <clusters/ScenesManagement/Structs.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>

#include <limits>

namespace chip::app::Clusters {

using ScenesManagementSceneTable = scenes::SceneTable<scenes::ExtensionFieldSetsImpl>;

/// Provides access to a single scene table. Specifically it allows a single object
/// re-use for scenes management
class ScenesManagementTableProvider
{
public:
    virtual ~ScenesManagementTableProvider()           = default;
    virtual ScenesManagementSceneTable * Take()        = 0;
    virtual void Release(ScenesManagementSceneTable *) = 0;
};

class ScenesManagementCluster : public DefaultServerCluster, public FabricTable::Delegate
{
public:
    static constexpr uint8_t kScenesServerMaxFabricCount = CHIP_CONFIG_MAX_FABRICS;

    using SceneInfoStructType = ScenesManagement::Structs::SceneInfoStruct::Type;

    class FabricSceneInfo
    {
    public:
        Span<SceneInfoStructType> GetFabricSceneInfo() { return { mSceneInfoStructs, mSceneInfoStructsCount }; }

        /// Gets the SceneInfoStruct for a specific fabric.
        ///
        /// returns nullptr if not found
        SceneInfoStructType * GetSceneInfoStruct(FabricIndex fabric);

        /// Sets the SceneInfoStruct for a specific fabric for a specific endpoint.
        CHIP_ERROR SetSceneInfoStruct(FabricIndex fabric, const SceneInfoStructType & sceneInfoStruct);

        /// Clears the SceneInfoStruct associated to a fabric and compresses the array to leave uninitialised structs at the end.
        void ClearSceneInfoStruct(FabricIndex fabric);

    private:
        static_assert(kScenesServerMaxFabricCount <= std::numeric_limits<uint8_t>::max());

        /// Returns the SceneInfoStruct associated to a fabric
        ///
        /// @param[in] fabric target fabric index
        /// @param[out] index index of the corresponding SceneInfoStruct if found, otherwise the index value will be invalid and
        /// should not be used. This is safe to store in a uint8_t because the index is guaranteed to be smaller than
        /// CHIP_CONFIG_MAX_FABRICS.
        ///
        /// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid fabric or endpoint
        CHIP_ERROR FindSceneInfoStructIndex(FabricIndex fabric, uint8_t & index);
        SceneInfoStructType mSceneInfoStructs[kScenesServerMaxFabricCount];
        uint8_t mSceneInfoStructsCount = 0;
    };

    /// Injected dependencies of this cluster
    struct Context
    {
        Credentials::GroupDataProvider * groupDataProvider;
        FabricTable * fabricTable;
        const BitMask<ScenesManagement::Feature> features;
        ScenesManagementTableProvider & sceneTableProvider;
        const bool supportsCopyScene;
    };

    ScenesManagementCluster(EndpointId endpointId, const Context & context) :
        DefaultServerCluster({ endpointId, ScenesManagement::Id }), mFeatures(context.features),
        mGroupProvider(context.groupDataProvider), mFabricTable{ context.fabricTable },
        mSceneTableProvider(context.sceneTableProvider), mSupportCopyScenes(context.supportsCopyScene)
    {}

    // ServerClusterInterface/DefaultServerCluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    // FabricTable::Delegate implementation
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    /// Removes the data persisted for this cluster
    ///
    /// MUST be called while started up.
    CHIP_ERROR ClearPersistentData();

    // Integration methods for other cluster integrations
    CHIP_ERROR GroupWillBeRemoved(FabricIndex aFabricIdx, GroupId aGroupId);
    CHIP_ERROR MakeSceneInvalid(FabricIndex aFabricIdx);
    CHIP_ERROR StoreCurrentScene(FabricIndex aFabricIx, GroupId aGroupId, SceneId aSceneId);
    CHIP_ERROR RecallScene(FabricIndex aFabricIx, GroupId aGroupId, SceneId aSceneId);
    CHIP_ERROR RemoveFabric(FabricIndex aFabricIndex);
    CHIP_ERROR MakeSceneInvalidForAllFabrics();

private:
    const BitMask<ScenesManagement::Feature> mFeatures;
    Credentials::GroupDataProvider * mGroupProvider = nullptr;
    FabricTable * mFabricTable                      = nullptr;
    ScenesManagementTableProvider & mSceneTableProvider;
    FabricSceneInfo mFabricSceneInfo;
    bool mSupportCopyScenes;

    SceneInfoStructType * GetSceneInfoStruct(FabricIndex fabric) { return mFabricSceneInfo.GetSceneInfoStruct(fabric); }

    CHIP_ERROR SetSceneInfoStruct(FabricIndex fabric, SceneInfoStructType & sceneInfoStruct)
    {
        return mFabricSceneInfo.SetSceneInfoStruct(fabric, sceneInfoStruct);
    }

    CHIP_ERROR UpdateFabricSceneInfo(FabricIndex fabric, Optional<GroupId> group, Optional<SceneId> scene,
                                     Optional<bool> sceneValid);

    CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const GroupId & groupID, const SceneId & sceneID);

    CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const GroupId & groupID, const SceneId & sceneID,
                                const Optional<DataModel::Nullable<uint32_t>> & transitionTime);

    //  Command handlers
    ScenesManagement::Commands::AddSceneResponse::Type
    HandleAddScene(FabricIndex fabricIndex, const ScenesManagement::Commands::AddScene::DecodableType & req);

    /// Handles view scene
    ///
    /// Returned type contains non-owned memory (spans) and the memory backing for these is offered
    /// by the input arguments:
    ///   - scene (for its name char span)
    ///   - responseEFSBuffer (for extensionFieldSetStructs)
    ScenesManagement::Commands::ViewSceneResponse::Type HandleViewScene(
        FabricIndex fabricIndex, const ScenesManagement::Commands::ViewScene::DecodableType & req,
        scenes::SceneTable<chip::scenes::ExtensionFieldSetsImpl>::SceneTableEntry & scene,
        std::array<ScenesManagement::Structs::ExtensionFieldSetStruct::Type, scenes::kMaxClustersPerScene> & responseEFSBuffer);

    ScenesManagement::Commands::RemoveSceneResponse::Type
    HandleRemoveScene(FabricIndex fabricIndex, const ScenesManagement::Commands::RemoveScene::DecodableType & req);

    ScenesManagement::Commands::RemoveAllScenesResponse::Type
    HandleRemoveAllScenes(FabricIndex fabricIndex, const ScenesManagement::Commands::RemoveAllScenes::DecodableType & req);

    ScenesManagement::Commands::StoreSceneResponse::Type
    HandleStoreScene(FabricIndex fabricIndex, const ScenesManagement::Commands::StoreScene::DecodableType & req);

    Protocols::InteractionModel::Status HandleRecallScene(FabricIndex fabricIndex,
                                                          const ScenesManagement::Commands::RecallScene::DecodableType & req);

    /// Handles getting the scene membership
    /// Returned type contains non-owned memory (spans) and the memory backing for these is offered
    /// by the input arguments:
    ///   - ScenesInGroup for the `sceneList` value of the response
    ScenesManagement::Commands::GetSceneMembershipResponse::Type
    HandleGetSceneMembership(FabricIndex fabricIndex, const ScenesManagement::Commands::GetSceneMembership::DecodableType & req,
                             std::array<SceneId, scenes::kMaxScenesPerFabric> & scenesInGroup);

    ScenesManagement::Commands::CopySceneResponse::Type
    HandleCopyScene(FabricIndex fabricIndex, const ScenesManagement::Commands::CopyScene::DecodableType & req);
};

} // namespace chip::app::Clusters
