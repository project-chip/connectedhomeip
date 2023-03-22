/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "scenes-server.h"
#include <app/clusters/scenes/SceneTable.h>
#include <app/server/Server.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Scenes {

CHIP_ERROR ScenesServer::Init() {}
void ScenesServer::Shutdown() {}

template <typename CommandData, typname ResponseType>
void AddSceneParse(HandlerContext & ctx, const CommandData & commandData)
{
    ResponseType response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable           = chip::server::GetInstance->GetSceneTable();
    chip::Credentials::GroupDataProvider * groupProvider = GetGroupDataProvider();

    // Command data
    GroupId & groupId         = commandData.groupID;
    SceneId & sceneId         = commandData.sceneID;
    EndpointId & endpoint     = commandPath.mEndpointId;
    FabricIndex fabricIndex   = ctx.mCommandHandler.GetAccessingFabricIndex();
    auto & transitionTime     = commandData.transitionTime;
    auto & sceneName          = commandData.sceneName;
    auto & extensionFieldSets = commandData.extensionFieldSets;
    uint32_t transitionTimeMs = 0;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {

        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    if (transitionTime.HasValue())
    {
        if (commandData.GetCommandId() == Commands::AddScene::Id)
        {
            transitionTimeMs = static_cast<uint32_t>(transitionTime.Value() * 1000);
        }
        else if (commandData.GetCommandId() == Commands::EnhancedAddScene::Id)
        {
            transitionTimeMs = static_cast<uint32_t>(transitionTime.Value() * 10000);
        }
    }

    auto fieldSetIter = extensionFieldSets.begin();

    ClusterId tempCluster = 0;
    uint8_t buffer[kMaxFieldsPerCluster];
    uint8_t EFSCount = 0;

    scenes::SceneStorageId StorageId(endpoint, sceneId, groupId);
    scenes::SceneStorageData StorageData(sceneName, sceneId, transitionTimeMs, 0);

    // Goes through all EFS in command
    while (fieldSetIter.Next() && EFSCount < kMaxClusterPerScenes)
    {
        Structs::ExtensionFieldSet::DecodableType EFS = fieldSetIter.GetValue();
        chip::scene::ExtensionFieldsSet tempEFS;

        MutableByteSpan buff_span(tempEFS.mBytesBuffer);

        // Check if a handler is registered for the EFS's cluster
        for (uint8_t i = 0; i < sceneTable.handlerNum; i++)
        {
            sceneTable.mHandlers[i].SerializeAdd(endpoint, tempEFS.mID, tempEFS.mBytesBuffer, EFS);
        }
        tempEFS.mUsedBytes = buff_span.size();

        if (!tempEFS.IsEmpty())
        {
            StorageData.mExtensionFieldSet.InsertFieldSet(tempEFS);
        }
    }

    // Create scene from data and ID
    scenes::SceneTableEntry scene(StorageId, StorageData);

    //  Insert in table
    err = sceneTable.SetSceneTableEntry(fabricIndex, scene);

    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_INVALID_LIST_LENGTH)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ResourceExhausted);
        }
        else
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        }
        return;
    }

    // Write response
    response.status  = Protocols::InteractionModel::Status::Success;
    reponse.groupID  = groupId;
    response.sceneID = sceneId;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

template <typename CommandData, typname ResponseType>
void ViewSceneParse(HandlerContext & ctx, const CommandData & commandData)
{
    ResponseType response;
    CHIP_ERROR err;

    scenes::SceneTable * sceneTable                = chip::server::GetInstance->GetSceneTable();
    Credentials::GroupDataProvider * groupProvider = Credentials::GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    SceneId & sceneId       = commandData.sceneID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    scenes::SceneStorageId StorageId(endpoint, sceneId, groupId);
    scenes::SceneTableEntry scene();

    //  Gets the scene form the table
    err = sceneTable->GetSceneTableEntry(fabricIndex, StorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound);
        }
        else
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        }
        return;
    }

    // Write response
    response.status  = Protocols::InteractionModel::Status::Success;
    reponse.groupID  = groupId;
    response.sceneID = sceneId;
    if (scene.mStorageData.mSceneTransitionTimeMs)
    {
        if (commandData.GetCommandId() == Commands::AddScene::Id)
        {
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 1000));
        }
        else if (commandData.GetCommandId() == Commands::AddScene::Id)
        {
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 10000));
        }
    }
    if (!scene.mNameSpan.empty())
    {
        response.sceneName.SetValue(scene.mNameSpan);
    }
    if (!scene.mExtensionFieldSets.IsEmpty())
    {
        for (uint8_t i = 0; i < scene.mExtensionFieldSets.GetFieldNum(); i++)
        {
            // gets data from the field in the scene
            ExtensionFieldsSet tempField;
            scene.mExtensionFieldSets.GetFieldSetAtPosition(tempField, i);
            ByteSpan efsSpan(tempField.mBytesBuffer, tempField.mUsedBytes);

            // This should only find one handle per cluster
            for (uint8_t j = 0; j < sceneTable.mNumHandlers; j++)
            {
                CHIP_ERROR found;
                found = sceneTable->mHandlers[j].Deserialize(endpoint, tempField.mID, efsSpan, mResponseEFSBuffer[i]);

                // Leaves loop once the handler has deserialized its cluster
                if (found == CHIP_NO_ERROR)
                {
                    break;
                }
            }
        }

        response.extensionFieldSets.SetValue(mResponseEFSBuffer);
        response.extensionFieldSets.reduce_size(scene.mExtensionFieldSets.GetFieldNum());
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

// CommandHanlerInterface
void ScenesServer::InvokeCommand(HandlerContext & ctx)
{
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctx.SetCommandHandled();
        return;
    }

    // Since mPath is used for building the response command, and we have checked that we are not pending the response of another
    // command above. So it is safe to set the mPath here and not clear it when return.
    mPath = ctx.mRequestPath;

    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::AddScene::Id:
        HandleCommand<Commands::AddScene::DecodableType>(ctx, [this](HandlerContext & ctx, const auto & req) {
            HandleAddScene<Commands::AddScene::DecodableType, Commands::AddSceneResponse::Type>(ctx, req);
        });
        return;
    case Commands::ViewScene::Id:
        HandleCommand<Commands::ViewScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleViewScene(ctx, req); });
        return;
    case Commands::RemoveScene::Id:
        HandleCommand<Commands::RemoveScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleRemoveScene(ctx, req); });
        return;
    case Commands::RemoveAllScenes::Id:
        HandleCommand<Commands::RemoveAllScenes::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleRemoveAllScenese(ctx, req); });
        return;
    case Commands::StoreScene::Id:
        HandleCommand<Commands::StoreScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleStoreScene(ctx, req); });
        return;
    case Commands::RecallScene::Id:
        HandleCommand<Commands::RecallScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleRecallScene(ctx, req); });
        return;
    case Commands::GetSceneMembership::Id:
        HandleCommand<Commands::GetSceneMembership::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleGetSceneMembership(ctx, req); });
        return;
    case Commands::EnhancedAddScene::Id:
        HandleCommand<Commands::EnhancedAddScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleAddScene(ctx, req); });
        return;
    case Commands::EnhancedViewScene::Id:
        HandleCommand<Commands::EnhancedViewScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleEnhancedAddScene(ctx, req); });
        return;
    case Commands::CopyScene::Id:
        HandleCommand<Commands::CopyScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleCopyScene(ctx, req); });
        return;
    }
}

CHIP_ERROR ScenesServer::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{}
CHIP_ERROR ScenesServer::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{}

void ScenesServer::HandleAddScene(HandlerContext & ctx, const Commands::EnhancedAddSceneResponse::Type & commandData)
{
    AddSceneParse<Commands::AddScene::DecodableType, Commands::AddSceneResponse::Type>(ctx, commandData);
}

void ScenesServer::HandleViewScene(HandlerContext & ctx, const CommandData & commandData)
{
    ViewSceneParse<Commands::ViewScene::DecodableType, Commands::ViewSceneResponse::Type>(ctx, commandData);
}

void ScenesServer::HandleRemoveScene(HandlerContext & ctx, const Commands::RemoveScene::DecodableType & req)
{
    Commands::RemoveSceneResponse::Type response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable     = chip::server::GetInstance->GetSceneTable();
    Credentials::GroupDataProvider * groupProvider = Credentials::GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    SceneId & sceneId       = commandData.sceneID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Scene Table interface data
    scenes::SceneStorageId sceneStorageId(endpoint, sceneId, groupId);
    scenes::SceneTableEntry scene(sceneStorageId);

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // TODO : confirm that either using get is ok or refactor remove scene table entry to return error on removal of unexisting
    // scene
    //  Gets the scene form the table
    err = sceneTable->GetSceneTableEntry(fabricIndex, sceneStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound);
        }
        else
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        }
        return;
    }
    // Remove the scene from the scene table
    err = sceneTable->RemoveSceneTableEntry(fabricIndex, sceneStorageId);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    // Write response
    response.status  = Protocols::InteractionModel::Status::Success;
    reponse.groupID  = groupId;
    response.sceneID = sceneId;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}
void ScenesServer::HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req)
{
    Commands::RemoveAllScenesResponse::Type response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable     = chip::server::GetInstance->GetSceneTable();
    Credentials::GroupDataProvider * groupProvider = Credentials::GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();
    scenes::SceneTableEntry scene();

    // TODO: Add removal all group scene to Scene Table

    // Write response
    response.status = Protocols::InteractionModel::Status::Success;
    reponse.groupID = groupId;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req)
{
    Commands::StoreSceneResponse::Type response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable     = chip::server::GetInstance->GetSceneTable();
    Credentials::GroupDataProvider * groupProvider = Credentials::GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    SceneId & sceneId       = commandData.sceneID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Scene Table interface data
    scenes::SceneStorageId sceneStorageId(endpoint, sceneId, groupId);
    scenes::SceneTableEntry scene(sceneStorageId);

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // TODO: Add get available scene space left function and return RESOURCE_EXHAUSTED on check failure

    err = sceneTable->GetSceneTableEntry(fabricIndex, sceneStorageId, scene);
    if (err != CHIP_NO_ERROR || err != CHIP_ERROR_NOT_FOUND)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    if (err == CHIP_ERROR_NOT_FOUND)
    {
        scene.mStorageData.SetName(CharSpan());
        scene.mStorageData.mSceneTransitionTimeMs = 0;
    }
    else
    {
        scene.mStorageData.mExtensionFieldSets.Clear();
    }

    // Gets the EFS
    err = SceneSaveEFS(scene);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    // Write response
    response.status  = Protocols::InteractionModel::Status::Success;
    reponse.groupID  = groupId;
    response.sceneID = sceneId;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req)
{
    Commands::RecallSceneResponse::Type response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable           = chip::server::GetInstance->GetSceneTable();
    chip::Credentials::GroupDataProvider * groupProvider = GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    SceneId & sceneId       = commandData.sceneID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();
    auto & transitionTime   = commandData.transitionTime;

    // Scene Table interface data
    scenes::SceneStorageId sceneStorageId(endpoint, sceneId, groupId);
    scenes::SceneTableEntry scene(sceneStorageId);

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    err = sceneTable->GetSceneTableEntry(fabricIndex, sceneStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound);
        }
        else
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        }
        return;
    }

    if (transitionTime.HasValue())
    {
        scene.mStorageData.mTransitionTimeMs += transitionTime.Value() * 100;
    }

    err = SceneApplyEFS(scene);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
    return;
}

void ScenesServer::HandleGetSceneMembership(HandlerContext & ctx, const Commands::GetSceneMembership::DecodableType & req)
{
    Commands::GetSceneMembershipResponse::Type response;
    CHIP_ERROR err;
    uint8_t capacity        = 0;
    uint8_t sceneGroupCount = 0;

    scenes::DefaultSceneTableImpl * sceneTable           = chip::server::GetInstance->GetSceneTable();
    chip::Credentials::GroupDataProvider * groupProvider = GetGroupDataProvider();

    // Command data
    GroupId & groupId       = commandData.groupID;
    EndpointId & endpoint   = commandPath.mEndpointId;
    FabricIndex fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Scene Table interface data
    SceneId scenesInGroup[kMaxScenePerFabric];
    ByteSpan sceneList(scenesInGroup);
    scenes::SceneTableEntry scene;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if (!groupProvider->HasEndpoint(fabricIndex, groupId, endpoint) && groupId != 0)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // TODO: implement a get space left in Fabric and in Scene Table
    // Get Capacity

    // Iterate through fabrics scene to populate scene list
    auto iterator = sceneTable->IterateSceneEntries(fabricIndex);

    while (iterator->Next(scene))
    {
        if (scene.mGroupId == groupId)
        {
            scenesInGroup[sceneGroupCount] = scene.mStorageId.mSceneId;
            sceneGroupCount++;
        }
    }
    sceneList.reduce_size(sceneGroupCount);

    // Write response
    response.status = Protocols::InteractionModel::Status::Success;
    response.Capacity.SetValue(capacity);
    response.groupID   = groupId;
    response.sceneList = sceneList;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleEnhancedAddScene(HandlerContext & ctx, const Commands::EnhancedAddScene::DecodableType & req)
{
    AddSceneParse<Commands::EnhancedAddScene::DecodableType, Commands::EnhancedAddSceneResponse::Type>(ctx, commandData);
}

void ScenesServer::HandleEnhancedViewScene(HandlerContext & ctx, const Commands::EnhancedViewScene::DecodableType & req)
{
    ViewSceneParse<Commands::EnhancedViewScene::DecodableType, Commands::EnhancedViewSceneResponse::Type>(ctx, commandData);
}
void ScenesServer::HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req)
{
    Commands::CopySceneResponse::Type response;
    CHIP_ERROR err;

    scenes::DefaultSceneTableImpl * sceneTable           = chip::server::GetInstance->GetSceneTable();
    chip::Credentials::GroupDataProvider * groupProvider = GetGroupDataProvider();

    // Command data
    GroupId & groupFrom                  = commandData.groupIdentifierFrom;
    GroupId & groupTo                    = commandData.groupIdentifierTo;
    SceneId & sceneFrom                  = commandData.sceneIdentifierFrom;
    SceneId & sceneTo                    = commandData.sceneIdentifierTo;
    EndpointId & endpoint                = commandPath.mEndpointId;
    FabricIndex fabricIndex              = ctx.mCommandHandler.GetAccessingFabricIndex();
    chip::BitMask<ScenesCopyMode> & mode = commandPath.mode;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider, false);
    if ((!groupProvider->HasEndpoint(fabricIndex, groupFrom, endpoint) && groupFrom != 0) ||
        (!groupProvider->HasEndpoint(fabricIndex, groupTo, endpoint) && groupTo != 0))
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // Scene Table interface data
    scenes::SceneStorageId sceneStorageFrom(endpoint, sceneFrom, groupFrom);
    scenes::SceneStorageId sceneStorageTo(endpoint, sceneTo, groupTo);
    scenes::SceneTableEntry sceneFrom();
    scenes::SceneTableEntry sceneTo();

    err = sceneTable->GetSceneTableEntry(fabricIndex, sceneStorageFrom, scene);
    if (err != CHIP_NO_ERROR || err != CHIP_ERROR_NOT_FOUND)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound);
        }
        else
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
        }

        return;
    }

    // TODO: Add get available scene space left function and return RESOURCE_EXHAUSTED on check failure
    // TODO: Implement a get scene num in group methodu (or check with groupd data provider?)
}

} // namespace Scenes
} // namespace Clusters
} // namespace app
} // namespace chip
