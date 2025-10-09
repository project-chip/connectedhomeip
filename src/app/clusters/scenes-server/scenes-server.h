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
 *    distributed under the License is distributed on an "AS IS" BASIS,¶
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <credentials/GroupDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ScenesManagement {

class ScenesServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    static constexpr size_t kScenesServerMaxEndpointCount =
        MATTER_DM_SCENES_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
    static_assert(kScenesServerMaxEndpointCount <= kEmberInvalidEndpointIndex, "Scenes endpoint count error");
    static constexpr uint8_t kScenesServerMaxFabricCount = CHIP_CONFIG_MAX_FABRICS;
    static constexpr SceneId kGlobalSceneId              = 0x00;
    static constexpr GroupId kGlobalSceneGroupId         = 0x0000;

    // FabricSceneInfo
    class FabricSceneInfo
    {
    public:
        Span<Structs::SceneInfoStruct::Type> GetFabricSceneInfo(EndpointId endpoint);
        Structs::SceneInfoStruct::Type * GetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric);
        CHIP_ERROR SetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric, Structs::SceneInfoStruct::Type & sceneInfoStruct);
        void ClearSceneInfoStruct(EndpointId endpoint, FabricIndex fabric);

    private:
        /// @brief Returns the index of the FabricSceneInfo associated to an endpoint
        /// @param[in] endpoint target endpoint
        /// @param[out] endpointIndex index of the corresponding FabricSceneInfo for an endpoint, corresponds to a row in the
        /// mSceneInfoStructs array,
        /// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid endpoint
        CHIP_ERROR FindFabricSceneInfoIndex(EndpointId endpoint, size_t & endpointIndex);

        /// @brief Returns the SceneInfoStruct associated to a fabric
        /// @param[in] fabric target fabric index
        /// @param[in] endpointIndex index of the corresponding FabricSceneInfo for an endpoint, corresponds to a row in the
        /// mSceneInfoStructs array
        /// @param[out] index index of the corresponding SceneInfoStruct if found, otherwise the index value will be invalid and
        /// should not be used. This is safe to store in a uint8_t because the index is guaranteed to be smaller than
        /// CHIP_CONFIG_MAX_FABRICS.
        /// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid fabric or endpoint
        CHIP_ERROR FindSceneInfoStructIndex(FabricIndex fabric, size_t endpointIndex, uint8_t & index);

        Structs::SceneInfoStruct::Type mSceneInfoStructs[kScenesServerMaxEndpointCount][kScenesServerMaxFabricCount];
        uint8_t mSceneInfoStructsCount[kScenesServerMaxEndpointCount] = { 0 };
    };

    static ScenesServer & Instance();

    CHIP_ERROR Init();
    void Shutdown();

    // CommandHanlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // SceneInfoStruct Accessors
    Structs::SceneInfoStruct::Type * GetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric);
    CHIP_ERROR SetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric, Structs::SceneInfoStruct::Type & sceneInfoStruct);

    // Callbacks
    void GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId);
    void MakeSceneInvalid(EndpointId aEndpointId, FabricIndex aFabricIx);
    void MakeSceneInvalidForAllFabrics(EndpointId aEndpointId);
    void StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);
    void RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);

    // Handlers for extension field sets
    bool IsHandlerRegistered(EndpointId aEndpointId, scenes::SceneHandler * handler);
    void RegisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler);
    void UnregisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler);

    // Fabric
    void RemoveFabric(EndpointId aEndpointId, FabricIndex aFabricIndex);

private:
    ScenesServer() : CommandHandlerInterface(Optional<EndpointId>(), Id), AttributeAccessInterface(Optional<EndpointId>(), Id) {}
    ~ScenesServer() { Shutdown(); }

    bool mIsInitialized = false;

    //  Command handlers
    void HandleAddScene(HandlerContext & ctx, const Commands::AddScene::DecodableType & req);
    void HandleViewScene(HandlerContext & ctx, const Commands::ViewScene::DecodableType & req);
    void HandleRemoveScene(HandlerContext & ctx, const Commands::RemoveScene::DecodableType & req);
    void HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req);
    void HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req);
    void HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req);
    void HandleGetSceneMembership(HandlerContext & ctx, const Commands::GetSceneMembership::DecodableType & req);
    void HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req);

    // Group Data Provider
    Credentials::GroupDataProvider * mGroupProvider = nullptr;

    // FabricSceneInfo
    FabricSceneInfo mFabricSceneInfo;

    // Instance
    static ScenesServer mInstance;
};

} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip
