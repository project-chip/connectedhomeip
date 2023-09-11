/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Nullable.h>
#include <credentials/GroupDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Scenes {

class ScenesServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    static ScenesServer & Instance();

    CHIP_ERROR Init();
    void Shutdown();

    // CommandHanlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // Callbacks
    void GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId);
    void MakeSceneInvalid(EndpointId aEndpointId);
    void StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);
    void RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);

    bool IsHandlerRegistered(EndpointId aEndpointId, scenes::SceneHandler * handler);
    void RegisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler);
    void UnregisterSceneHandler(EndpointId aEndpointId, scenes::SceneHandler * handler);

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
    void HandleEnhancedAddScene(HandlerContext & ctx, const Commands::EnhancedAddScene::DecodableType & req);
    void HandleEnhancedViewScene(HandlerContext & ctx, const Commands::EnhancedViewScene::DecodableType & req);
    void HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req);

    // Group Data Provider
    Credentials::GroupDataProvider * mGroupProvider = nullptr;

    // Instance
    static ScenesServer mInstance;
};

} // namespace Scenes
} // namespace Clusters
} // namespace app
} // namespace chip
