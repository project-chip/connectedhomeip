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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using SceneTableEntry   = chip::scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneStorageId    = chip::scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData         = chip::scenes::DefaultSceneTableImpl::SceneData;
using HandlerContext    = chip::app::CommandHandlerInterface::HandlerContext;
using ExtensionFieldSet = chip::scenes::ExtensionFieldSet;
using GroupDataProvider = chip::Credentials::GroupDataProvider;
using SceneTable        = chip::scenes::SceneTable<chip::scenes::ExtensionFieldSetsImpl>;

namespace chip {
namespace app {
namespace Clusters {
namespace Scenes {

ScenesServer ScenesServer::mInstance;

ScenesServer & ScenesServer::Instance()
{
    return mInstance;
}

CHIP_ERROR ScenesServer::Init()
{
    // Prevents re-initializing
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    mSceneTable = chip::scenes::GetSceneTableImpl();
    ReturnErrorOnFailure(mSceneTable->Init(&chip::Server::GetInstance().GetPersistentStorage()));

    mGroupProvider = Credentials::GetGroupDataProvider();

    mFeatureFlags.Set(ScenesFeature::kSceneNames);
    // The bit of 7 the NameSupport attribute indicates whether or not scene names are supported
    //
    // According to spec, bit 7 (Scene Names) MUST match feature bit 0 (Scene Names)
    mNameSupport = 0x80;

    // TODO: Make modifications to the CommandHandler and Transport layer to be able to forward the NodeId to clusters
    mLastConfiguredBy.SetNull();
    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

template <typename CommandData, typename ResponseType>
void AddSceneParse(CommandHandlerInterface::HandlerContext & ctx, const CommandData & req, SceneTable * sceneTable,
                   GroupDataProvider * groupProvider)
{
    ResponseType response;
    CHIP_ERROR err;
    uint8_t capacity          = 0;
    uint32_t transitionTimeMs = 0;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider);
    if (!groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId) &&
        req.groupID != 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    if (ctx.mRequestPath.mCommandId == Commands::AddScene::Id)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime * 1000);
    }
    else if (ctx.mRequestPath.mCommandId == Commands::EnhancedAddScene::Id)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime * 10000);
    }

    auto fieldSetIter = req.extensionFieldSets.begin();

    uint8_t EFSCount = 0;
    SceneData StorageData(req.sceneName, transitionTimeMs);

    // Goes through all EFS in command
    while (fieldSetIter.Next() && EFSCount < scenes::kMaxClustersPerScene)
    {
        Structs::ExtensionFieldSet::DecodableType EFS = fieldSetIter.GetValue();
        scenes::ExtensionFieldSet tempEFS;
        tempEFS.mID = EFS.clusterID;

        MutableByteSpan buff_span(tempEFS.mBytesBuffer);

        // Check if a handler is registered for the EFS's cluster
        for (auto & handler : sceneTable->mHandlerList)
        {
            if (handler.SupportsCluster(ctx.mRequestPath.mEndpointId, tempEFS.mID))
            {
                handler.SerializeAdd(ctx.mRequestPath.mEndpointId, EFS, buff_span);
                break;
            }
        }
        tempEFS.mUsedBytes = static_cast<uint8_t>(buff_span.size());

        if (!tempEFS.IsEmpty())
        {
            StorageData.mExtensionFieldSets.InsertFieldSet(tempEFS);
        }
    }

    // Create scene from data and ID
    SceneTableEntry scene(SceneStorageId(ctx.mRequestPath.mEndpointId, req.sceneID, req.groupID), StorageData);

    // Get Capacity
    VerifyOrReturn(nullptr != sceneTable);
    err = sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    if (capacity == 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    //  Insert in table
    err = sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    // Write response
    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

template <typename CommandData, typename ResponseType>
void ViewSceneParse(HandlerContext & ctx, const CommandData & req, SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    ResponseType response;
    CHIP_ERROR err;
    // Response Extension Field Sets buffer
    Structs::ExtensionFieldSet::Type mResponseEFSBuffer[CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE];

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider);
    if (!groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId) &&
        req.groupID != 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    SceneTableEntry scene;

    //  Gets the scene form the table
    VerifyOrReturn(nullptr != sceneTable);
    err = sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(),
                                         SceneStorageId(ctx.mRequestPath.mEndpointId, req.sceneID, req.groupID), scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::NotFound);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        }
        else
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        }
        return;
    }

    // Verifies how to convert transition time
    if (scene.mStorageData.mSceneTransitionTimeMs)
    {
        if (ctx.mRequestPath.mCommandId == Commands::ViewScene::Id)
        {
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 1000));
        }
        else if (ctx.mRequestPath.mCommandId == Commands::EnhancedViewScene::Id)
        {
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 10000));
        }
    }

    // Verify if name was saved in the scene
    if (scene.mStorageData.mNameLength)
    {
        response.sceneName.SetValue(CharSpan(scene.mStorageData.mName));
    }

    Structs::ExtensionFieldSet::Type responseEFSBuffer[scenes::kMaxClustersPerScene];
    uint8_t DeserializedEFSCount = 0;

    // Verify that the scene had extension field sets
    if (!scene.mStorageData.mExtensionFieldSets.IsEmpty())
    {
        for (uint8_t i = 0; i < scene.mStorageData.mExtensionFieldSets.GetFieldSetCount(); i++)
        {
            // gets data from the field in the scene
            ExtensionFieldSet tempField;
            scene.mStorageData.mExtensionFieldSets.GetFieldSetAtPosition(tempField, i);
            ByteSpan efsSpan(tempField.mBytesBuffer, tempField.mUsedBytes);

            // This should only find one handle per cluster
            for (auto & handler : sceneTable->mHandlerList)
            {
                if (handler.SupportsCluster(ctx.mRequestPath.mEndpointId, tempField.mID))
                {
                    handler.Deserialize(ctx.mRequestPath.mEndpointId, tempField.mID, efsSpan, responseEFSBuffer[i]);
                    DeserializedEFSCount++;
                    break;
                }
            }
        }

        Span<Structs::ExtensionFieldSet::Type> responseEFSSpan(mResponseEFSBuffer, DeserializedEFSCount);
        response.extensionFieldSets.SetValue(responseEFSSpan);
    }
    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                           const SceneId & sceneID, SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (!groupProvider->HasEndpoint(fabricIdx, groupID, endpointID) && groupID != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(endpointID, sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    CHIP_ERROR err = sceneTable->GetSceneTableEntry(fabricIdx, scene.mStorageId, scene);
    if (err != CHIP_NO_ERROR && err != CHIP_ERROR_NOT_FOUND)
    {
        return err;
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
    ReturnErrorOnFailure(sceneTable->SceneSaveEFS(scene));
    // Insert in Scene Table
    return sceneTable->SetSceneTableEntry(fabricIdx, scene);
}

CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                            const SceneId & sceneID, const Optional<DataModel::Nullable<uint16_t>> & transitionTime,
                            SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (!groupProvider->HasEndpoint(fabricIdx, groupID, endpointID) && groupID != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(endpointID, sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(sceneTable->GetSceneTableEntry(fabricIdx, scene.mStorageId, scene));

    // Check for optional
    if (transitionTime.HasValue())
    {
        // Check for nullable
        if (transitionTime.Value().HasValidValue())
        {
            scene.mStorageData.mSceneTransitionTimeMs = static_cast<uint32_t>(transitionTime.Value().Value() * 100);
        }
    }

    return sceneTable->SceneApplyEFS(scene);
}

// CommandHanlerInterface
void ScenesServer::InvokeCommand(HandlerContext & ctx)
{
    mPath = ctx.mRequestPath;

    switch (mPath.mCommandId)
    {
    case Commands::AddScene::Id:
        HandleCommand<Commands::AddScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleAddScene(ctx, req); });
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
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleRemoveAllScenes(ctx, req); });
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
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleEnhancedAddScene(ctx, req); });
        return;
    case Commands::EnhancedViewScene::Id:
        HandleCommand<Commands::EnhancedViewScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleEnhancedViewScene(ctx, req); });
        return;
    case Commands::CopyScene::Id:
        HandleCommand<Commands::CopyScene::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleCopyScene(ctx, req); });
        return;
    }
}

CHIP_ERROR ScenesServer::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::Scenes::Commands;

    constexpr CommandId acceptedCommandsList[] = {
        AddScene::Id,    ViewScene::Id,          RemoveScene::Id,      RemoveAllScenes::Id,   StoreScene::Id,
        RecallScene::Id, GetSceneMembership::Id, EnhancedAddScene::Id, EnhancedViewScene::Id, CopyScene::Id,
    };

    for (const auto & cmd : acceptedCommandsList)
    {
        if (callback(cmd, context) != Loop::Continue)
        {
            break;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ScenesServer::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{

    using namespace Clusters::Scenes::Commands;

    constexpr CommandId generatedCommandsList[] = {
        AddSceneResponse::Id,         ViewSceneResponse::Id,         RemoveSceneResponse::Id,
        RemoveAllScenesResponse::Id,  StoreSceneResponse::Id,        GetSceneMembershipResponse::Id,
        EnhancedAddSceneResponse::Id, EnhancedViewSceneResponse::Id, CopySceneResponse::Id,
    };

    for (const auto & cmd : generatedCommandsList)
    {
        if (callback(cmd, context) != Loop::Continue)
        {
            break;
        }
    }
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR ScenesServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::SceneCount::Id:
        ReturnErrorOnFailure(mSceneTable->GetGlobalSceneCount(mSceneCount));
        return aEncoder.Encode(mSceneCount);
    case Attributes::CurrentScene::Id:
        return aEncoder.Encode(mCurrentScene);
    case Attributes::CurrentGroup::Id:
        return aEncoder.Encode(mCurrentGroup);
    case Attributes::SceneValid::Id:
        return aEncoder.Encode(mSceneValid);
    case Attributes::NameSupport::Id:
        return aEncoder.Encode(mNameSupport);
    case Attributes::LastConfiguredBy::Id:
        if (mLastConfiguredBy.IsNull())
        {
            return aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.Encode(mLastConfiguredBy);
        }
    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeatureFlags);
    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR ScenesServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    // No writeable attributes in the Scene Cluster
    switch (aPath.mAttributeId)
    {
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
}

void ScenesServer::OnGroupRemoved(const FabricIndex & aFabricIx, const EndpointId & aEndpointId, const GroupId & aGroupId)
{
    VerifyOrReturn(nullptr != mGroupProvider);
    if (!mGroupProvider->HasEndpoint(aFabricIx, aGroupId, aEndpointId) && aGroupId != 0)
    {
        return;
    }

    mSceneTable->DeleteAllScenesInGroup(aFabricIx, aGroupId);
}

void ScenesServer::OnMakeInvalid()
{
    mSceneValid = false;
}

void ScenesServer::OnStoreCurrentScene(const FabricIndex & aFabricIx, const EndpointId & aEndpointId, const GroupId & aGroupId,
                                       const SceneId & aSceneId)
{
    mSceneValid = false;

    if (CHIP_NO_ERROR == StoreSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, mSceneTable, mGroupProvider))
    {
        mSceneValid = true;
    }
}
void ScenesServer::OnRecallScene(const FabricIndex & aFabricIx, const EndpointId & aEndpointId, const GroupId & aGroupId,
                                 const SceneId & aSceneId)
{
    mSceneValid = false;
    Optional<DataModel::Nullable<uint16_t>> transitionTime;

    if (CHIP_NO_ERROR == RecallSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, transitionTime, mSceneTable, mGroupProvider))
    {
        mSceneValid = true;
    }
}

void ScenesServer::HandleAddScene(HandlerContext & ctx, const Commands::AddScene::DecodableType & req)
{
    AddSceneParse<Commands::AddScene::DecodableType, Commands::AddSceneResponse::Type>(ctx, req, mSceneTable, mGroupProvider);
}

void ScenesServer::HandleViewScene(HandlerContext & ctx, const Commands::ViewScene::DecodableType & req)
{
    ViewSceneParse<Commands::ViewScene::DecodableType, Commands::ViewSceneResponse::Type>(ctx, req, mSceneTable, mGroupProvider);
}

void ScenesServer::HandleRemoveScene(HandlerContext & ctx, const Commands::RemoveScene::DecodableType & req)
{
    Commands::RemoveSceneResponse::Type response;
    CHIP_ERROR err;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(ctx.mRequestPath.mEndpointId, req.sceneID, req.groupID));

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (!mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId) &&
        req.groupID != 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    //  Gets the scene form the table
    err = mSceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::NotFound);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        else
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        return;
    }

    // Remove the scene from the scene table
    err = mSceneTable->RemoveSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Write response
    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req)
{
    Commands::RemoveAllScenesResponse::Type response;
    CHIP_ERROR err;

    // Response data
    response.groupID = req.groupID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (!mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId) &&
        req.groupID != 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    err = mSceneTable->DeleteAllScenesInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Write response
    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req)
{
    // Scene Valid is false when this command begins
    mSceneValid = false;

    Commands::StoreSceneResponse::Type response;

    // Update Attributes
    mCurrentScene = req.sceneID;
    mCurrentGroup = req.groupID;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    CHIP_ERROR err = StoreSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), mPath.mEndpointId, req.groupID, req.sceneID,
                                     mSceneTable, mGroupProvider);

    switch (err.AsInteger())
    {
    case CHIP_NO_ERROR.AsInteger():
        mSceneValid     = true;
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    case CHIP_ERROR_INVALID_ARGUMENT.AsInteger():
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    case CHIP_ERROR_NO_MEMORY.AsInteger():
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    default:
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }
}

void ScenesServer::HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req)
{
    // Scene Valid is false when this command begins
    mSceneValid = false;

    // Update Attributes
    mCurrentScene = req.sceneID;
    mCurrentGroup = req.groupID;

    CHIP_ERROR err = RecallSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), mPath.mEndpointId, req.groupID, req.sceneID,
                                      req.transitionTime, mSceneTable, mGroupProvider);

    switch (err.AsInteger())
    {
    case CHIP_NO_ERROR.AsInteger():
        mSceneValid = true;
        ctx.mCommandHandler.AddStatus(mPath, Protocols::InteractionModel::Status::Success);
        return;
    case CHIP_ERROR_INVALID_ARGUMENT.AsInteger():
        ctx.mCommandHandler.AddStatus(mPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    case CHIP_ERROR_NOT_FOUND.AsInteger():
        ctx.mCommandHandler.AddStatus(mPath, Protocols::InteractionModel::Status::NotFound);
        return;
    default:
        ctx.mCommandHandler.AddStatus(mPath, Protocols::InteractionModel::Status::Failure);
        return;
    }
}

void ScenesServer::HandleGetSceneMembership(HandlerContext & ctx, const Commands::GetSceneMembership::DecodableType & req)
{
    Commands::GetSceneMembershipResponse::Type response;
    CHIP_ERROR err;
    uint8_t capacity = 0;

    // Response data
    response.groupID = req.groupID;

    // Scene Table interface data
    SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
    Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);
    SceneTableEntry scene;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (!mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId) &&
        req.groupID != 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Get Capacity
    err = mSceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }
    response.capacity.SetNonNull(capacity);

    if (capacity == 0)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // populate scene list
    err = mSceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, sceneList);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    response.sceneList.SetValue(sceneList);

    // Write response
    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleEnhancedAddScene(HandlerContext & ctx, const Commands::EnhancedAddScene::DecodableType & req)
{
    AddSceneParse<Commands::EnhancedAddScene::DecodableType, Commands::EnhancedAddSceneResponse::Type>(ctx, req, mSceneTable,
                                                                                                       mGroupProvider);
}

void ScenesServer::HandleEnhancedViewScene(HandlerContext & ctx, const Commands::EnhancedViewScene::DecodableType & req)
{
    ViewSceneParse<Commands::EnhancedViewScene::DecodableType, Commands::EnhancedViewSceneResponse::Type>(ctx, req, mSceneTable,
                                                                                                          mGroupProvider);
}
void ScenesServer::HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req)
{
    Commands::CopySceneResponse::Type response;
    CHIP_ERROR err;
    uint8_t capacity;

    // Response data
    response.groupIdentifierFrom = req.groupIdentifierFrom;
    response.sceneIdentifierFrom = req.sceneIdentifierFrom;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if ((!mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, mPath.mEndpointId) &&
         req.groupIdentifierFrom != 0) ||
        (!mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierTo, mPath.mEndpointId) &&
         req.groupIdentifierTo != 0))
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Get Capacity
    err = mSceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Checks if we copy a single scene or all of them
    if (req.mode.GetField(app::Clusters::Scenes::ScenesCopyMode::kCopyAllScenes))
    {
        // Scene Table interface data
        SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
        Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);

        // populate scene list
        err = mSceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, sceneList);
        if (err != CHIP_NO_ERROR)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
            ctx.mCommandHandler.AddResponse(mPath, response);
            return;
        }

        if (capacity == 0)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(mPath, response);
            return;
        }

        for (auto & SceneId : sceneList)
        {
            SceneTableEntry scene(SceneStorageId(mPath.mEndpointId, SceneId, req.groupIdentifierFrom));
            //  Insert in table
            err = mSceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene);
            if (err != CHIP_NO_ERROR)
            {
                response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
                ctx.mCommandHandler.AddResponse(mPath, response);
                return;
            }

            scene.mStorageId = SceneStorageId(mPath.mEndpointId, SceneId, req.groupIdentifierTo);

            err = mSceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
            if (err != CHIP_NO_ERROR)
            {
                if (err == CHIP_ERROR_NO_MEMORY)
                {
                    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
                    ctx.mCommandHandler.AddResponse(mPath, response);
                }
                else
                {
                    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
                    ctx.mCommandHandler.AddResponse(mPath, response);
                }
                return;
            }
        }

        response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    SceneTableEntry scene(SceneStorageId(mPath.mEndpointId, req.sceneIdentifierFrom, req.groupIdentifierFrom));
    err = mSceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::NotFound);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        else
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        return;
    }

    scene.mStorageId = SceneStorageId(mPath.mEndpointId, req.sceneIdentifierTo, req.groupIdentifierTo);

    err = mSceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NO_MEMORY)
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        else
        {
            response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Failure);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        return;
    }

    response.status = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

} // namespace Scenes
} // namespace Clusters
} // namespace app
} // namespace chip

// EMBER AF CALLBACKS:
// Those have been added to avoid linker issue with zap and should never get called

bool emberAfScenesClusterAddSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                          chip::app::Clusters::Scenes::Commands::AddScene::DecodableType const &)
{
    return false;
}

bool emberAfScenesClusterViewSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                           chip::app::Clusters::Scenes::Commands::ViewScene::DecodableType const &)
{
    return false;
}
bool emberAfScenesClusterServerInitCallback(unsigned short)
{
    return false;
}
bool emberAfScenesClusterStoreSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                            chip::app::Clusters::Scenes::Commands::StoreScene::DecodableType const &)
{
    return false;
}
bool emberAfScenesClusterRecallSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                             chip::app::Clusters::Scenes::Commands::RecallScene::DecodableType const &)
{
    return false;
}
bool emberAfScenesClusterRemoveSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                             chip::app::Clusters::Scenes::Commands::RemoveScene::DecodableType const &)
{
    return false;
}
bool emberAfScenesClusterRemoveAllScenesCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                                 chip::app::Clusters::Scenes::Commands::RemoveAllScenes::DecodableType const &)
{
    return false;
}

bool emberAfScenesClusterGetSceneMembershipCallback(
    chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
    chip::app::Clusters::Scenes::Commands::GetSceneMembership::DecodableType const &)
{
    return false;
}

bool emberAfScenesClusterEnhancedAddSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                                  chip::app::Clusters::Scenes::Commands::EnhancedAddScene::DecodableType const &)
{
    return false;
}

bool emberAfScenesClusterEnhancedViewSceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                                   chip::app::Clusters::Scenes::Commands::EnhancedViewScene::DecodableType const &)
{
    return false;
}

bool emberAfScenesClusterCopySceneCallback(chip::app::CommandHandler *, chip::app::ConcreteCommandPath const &,
                                           chip::app::Clusters::Scenes::Commands::CopyScene::DecodableType const &)
{
    return false;
}

void MatterScenesPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in ScenesServer::Init()
}
