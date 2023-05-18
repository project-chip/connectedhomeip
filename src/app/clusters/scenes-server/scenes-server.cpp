/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
#include <app/reporting/reporting.h>
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
void ReportAttributeOnAllEndpoints(AttributeId attribute)
{
    for (auto endpoint : EnabledEndpointsWithServerCluster(Id))
    {
        MatterReportingAttributeChangeCallback(endpoint, Id, attribute);
    }
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

    mLastConfiguredBy.SetNull();
    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

void ScenesServer::Shutdown()
{
    chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    mSceneTable->Finish();
    mGroupProvider = nullptr;
    mIsInitialized = false;
}

template <typename CommandData, typename ResponseType>
void AddSceneParse(CommandHandlerInterface::HandlerContext & ctx, const CommandData & req, SceneTable * sceneTable,
                   GroupDataProvider * groupProvider)
{
    ResponseType response;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify the attributes are respecting constraints
    if (req.transitionTime > scenes::kScenesMaxTransitionTimeS || req.sceneName.size() > scenes::kSceneNameMaxLength)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider);
    if (req.groupID != 0 &&
        !groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    uint32_t transitionTimeMs = 0;
    if (ctx.mRequestPath.mCommandId == Commands::AddScene::Id)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime) * 1000u;
    }
    else if (ctx.mRequestPath.mCommandId == Commands::EnhancedAddScene::Id)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime) * 100u;
    }

    auto fieldSetIter = req.extensionFieldSets.begin();

    uint8_t EFSCount = 0;
    SceneData storageData(req.sceneName, transitionTimeMs);
    CHIP_ERROR err;

    // Goes through all EFS in command
    while (fieldSetIter.Next() && EFSCount < scenes::kMaxClustersPerScene)
    {
        scenes::ExtensionFieldSet tempEFS;
        tempEFS.mID = fieldSetIter.GetValue().clusterID;

        MutableByteSpan buff_span(tempEFS.mBytesBuffer);

        // Check if a handler is registered for the EFS's cluster
        for (auto & handler : sceneTable->mHandlerList)
        {
            if (handler.SupportsCluster(ctx.mRequestPath.mEndpointId, tempEFS.mID))
            {
                err = handler.SerializeAdd(ctx.mRequestPath.mEndpointId, fieldSetIter.GetValue(), buff_span);
                if (err != CHIP_NO_ERROR)
                {
                    response.status = to_underlying(StatusIB(err).mStatus);
                    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                    return;
                }
                break;
            }
        }

        err = fieldSetIter.GetStatus();
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(StatusIB(err).mStatus);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return;
        }

        static_assert(sizeof(buff_span.size()) <= UINT8_MAX, "Serialized EFS number of bytes must fit in a uint8");
        tempEFS.mUsedBytes = static_cast<uint8_t>(buff_span.size());

        if (!tempEFS.IsEmpty())
        {
            storageData.mExtensionFieldSets.InsertFieldSet(tempEFS);
        }
    }

    // Create scene from data and ID
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID), storageData);

    // Get Capacity
    VerifyOrReturn(nullptr != sceneTable);
    uint8_t capacity = 0;
    err = sceneTable->GetRemainingCapacity(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    if (capacity == 0)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    //  Insert in table
    err = sceneTable->SetSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);

    // TODO : Handle dirty marking of attributes
}

template <typename CommandData, typename ResponseType>
void ViewSceneParse(HandlerContext & ctx, const CommandData & req, SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    ResponseType response;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider);
    if (req.groupID != 0 &&
        !groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    SceneTableEntry scene;

    //  Gets the scene form the table
    VerifyOrReturn(nullptr != sceneTable);
    CHIP_ERROR err = sceneTable->GetSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                    SceneStorageId(req.sceneID, req.groupID), scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::NotFound);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        }
        else
        {
            response.status = to_underlying(StatusIB(err).mStatus);
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
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 100));
        }
    }

    // Verify if name was saved in the scene
    if (scene.mStorageData.mNameLength)
    {
        response.sceneName.SetValue(CharSpan(scene.mStorageData.mName, scenes::kSceneNameMaxLength));
    }

    // Response Extension Field Sets buffer
    Structs::ExtensionFieldSet::Type responseEFSBuffer[scenes::kMaxClustersPerScene];
    uint8_t deserializedEFSCount = 0;

    // Adds extension field sets to the scene
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
                err = handler.Deserialize(ctx.mRequestPath.mEndpointId, tempField.mID, efsSpan, responseEFSBuffer[i]);
                if (err != CHIP_NO_ERROR)
                {
                    response.status = to_underlying(StatusIB(err).mStatus);
                    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
                    return;
                }
                deserializedEFSCount++;
                break;
            }
        }
    }

    Span<Structs::ExtensionFieldSet::Type> responseEFSSpan(responseEFSBuffer, deserializedEFSCount);
    response.extensionFieldSets.SetValue(responseEFSSpan);

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                           const SceneId & sceneID, SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (groupID != 0 && !groupProvider->HasEndpoint(fabricIdx, groupID, endpointID))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    CHIP_ERROR err = sceneTable->GetSceneTableEntry(endpointID, fabricIdx, scene.mStorageId, scene);
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
    ReturnErrorOnFailure(sceneTable->SceneSaveEFS(endpointID, scene));
    // Insert in Scene Table
    return sceneTable->SetSceneTableEntry(endpointID, fabricIdx, scene);
}

CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                            const SceneId & sceneID, const Optional<DataModel::Nullable<uint16_t>> & transitionTime,
                            SceneTable * sceneTable, GroupDataProvider * groupProvider)
{
    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (groupID != 0 && !groupProvider->HasEndpoint(fabricIdx, groupID, endpointID))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(sceneTable->GetSceneTableEntry(endpointID, fabricIdx, scene.mStorageId, scene));

    // Check for optional
    if (transitionTime.HasValue())
    {
        // Check for nullable
        if (!transitionTime.Value().IsNull())
        {
            scene.mStorageData.mSceneTransitionTimeMs = static_cast<uint32_t>(transitionTime.Value().Value() * 100);
        }
    }

    return sceneTable->SceneApplyEFS(endpointID, scene);
}

// CommandHanlerInterface
void ScenesServer::InvokeCommand(HandlerContext & ctxt)
{
    mPath = ctxt.mRequestPath;

    switch (mPath.mCommandId)
    {
    case Commands::AddScene::Id:
        HandleCommand<Commands::AddScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddScene(ctx, req); });
        return;
    case Commands::ViewScene::Id:
        HandleCommand<Commands::ViewScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleViewScene(ctx, req); });
        return;
    case Commands::RemoveScene::Id:
        HandleCommand<Commands::RemoveScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveScene(ctx, req); });
        return;
    case Commands::RemoveAllScenes::Id:
        HandleCommand<Commands::RemoveAllScenes::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveAllScenes(ctx, req); });
        return;
    case Commands::StoreScene::Id:
        HandleCommand<Commands::StoreScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleStoreScene(ctx, req); });
        return;
    case Commands::RecallScene::Id:
        HandleCommand<Commands::RecallScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRecallScene(ctx, req); });
        return;
    case Commands::GetSceneMembership::Id:
        HandleCommand<Commands::GetSceneMembership::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleGetSceneMembership(ctx, req); });
        return;
    case Commands::EnhancedAddScene::Id:
        HandleCommand<Commands::EnhancedAddScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleEnhancedAddScene(ctx, req); });
        return;
    case Commands::EnhancedViewScene::Id:
        HandleCommand<Commands::EnhancedViewScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleEnhancedViewScene(ctx, req); });
        return;
    case Commands::CopyScene::Id:
        HandleCommand<Commands::CopyScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleCopyScene(ctx, req); });
        return;
    }
}

// AttributeAccessInterface
CHIP_ERROR ScenesServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    uint8_t value = 0;
    // TODO: handle endpoint attribute scoping and refactor scene table implementation for a per endpoint scope.
    switch (aPath.mAttributeId)
    {
    case Attributes::SceneCount::Id:
        ReturnErrorOnFailure(mSceneTable->GetEndpointSceneCount(aPath.mEndpointId, value));
        return aEncoder.Encode(value);
    case Attributes::CurrentScene::Id:
        return aEncoder.Encode(mCurrentScene);
    case Attributes::CurrentGroup::Id:
        return aEncoder.Encode(mCurrentGroup);
    case Attributes::SceneValid::Id:
        return aEncoder.Encode(mSceneValid);
    case Attributes::NameSupport::Id:
        return aEncoder.Encode(mNameSupport);
    case Attributes::LastConfiguredBy::Id:
        return aEncoder.Encode(mLastConfiguredBy);
    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeatureFlags);
    case Attributes::SceneTableSize::Id:
        return aEncoder.Encode(scenes::kMaxScenesPerEndpoint);
    case Attributes::RemainingCapacity::Id:
        ReturnErrorOnFailure(mSceneTable->GetRemainingCapacity(aPath.mEndpointId, aEncoder.AccessingFabricIndex(), value));
        return aEncoder.Encode(value);
    default:
        return CHIP_NO_ERROR;
    }
}

void ScenesServer::GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId)
{
    VerifyOrReturn(nullptr != mGroupProvider);
    if (aGroupId != 0 && !mGroupProvider->HasEndpoint(aFabricIx, aGroupId, aEndpointId))
    {
        return;
    }

    mSceneTable->DeleteAllScenesInGroup(aEndpointId, aFabricIx, aGroupId);
}

void ScenesServer::MakeSceneInvalid()
{
    mSceneValid = false;
}

void ScenesServer::StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    mSceneValid = false;

    if (CHIP_NO_ERROR == StoreSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, mSceneTable, mGroupProvider))
    {
        mSceneValid = true;
    }
}
void ScenesServer::RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
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
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID));

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (req.groupID != 0 &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    //  Gets the scene from the table
    err = mSceneTable->GetSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                          scene.mStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Remove the scene from the scene table
    err = mSceneTable->RemoveSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                             scene.mStorageId);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
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
    if (req.groupID != 0 &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    err = mSceneTable->DeleteAllScenesInGroup(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                              req.groupID);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
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

    if (err == CHIP_NO_ERROR)
    {
        mSceneValid = true;
    }

    response.status = to_underlying(StatusIB(err).mStatus);
    ctx.mCommandHandler.AddResponse(mPath, response);
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

    if (err == CHIP_NO_ERROR)
    {
        mSceneValid = true;
    }

    ctx.mCommandHandler.AddStatus(mPath, StatusIB(err).mStatus);
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
    if (req.groupID != 0 &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Get Capacity
    err = mSceneTable->GetRemainingCapacity(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }
    response.capacity.SetNonNull(capacity);

    // populate scene list
    err = mSceneTable->GetAllSceneIdsInGroup(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                             req.groupID, sceneList);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    response.sceneList.SetValue(sceneList);

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
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
    if ((req.groupIdentifierFrom != 0 &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, mPath.mEndpointId)) ||
        (req.groupIdentifierTo != 0 &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierTo, mPath.mEndpointId)))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    // Get Capacity
    err = mSceneTable->GetRemainingCapacity(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), capacity);
    if (err != CHIP_NO_ERROR)
    {
        response.status = to_underlying(StatusIB(err).mStatus);
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
        err = mSceneTable->GetAllSceneIdsInGroup(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                 req.groupIdentifierFrom, sceneList);
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(StatusIB(err).mStatus);
            ctx.mCommandHandler.AddResponse(mPath, response);
            return;
        }

        if (capacity == 0)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(mPath, response);
            return;
        }

        for (auto & sceneId : sceneList)
        {
            SceneTableEntry scene(SceneStorageId(sceneId, req.groupIdentifierFrom));
            //  Insert in table
            err = mSceneTable->GetSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                  scene.mStorageId, scene);
            if (err != CHIP_NO_ERROR)
            {
                response.status = to_underlying(StatusIB(err).mStatus);
                ctx.mCommandHandler.AddResponse(mPath, response);
                return;
            }

            scene.mStorageId = SceneStorageId(sceneId, req.groupIdentifierTo);

            err = mSceneTable->SetSceneTableEntry(mPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
            if (err != CHIP_NO_ERROR)
            {
                if (err == CHIP_ERROR_NO_MEMORY)
                {
                    response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
                    ctx.mCommandHandler.AddResponse(mPath, response);
                }
                else
                {
                    response.status = to_underlying(StatusIB(err).mStatus);
                    ctx.mCommandHandler.AddResponse(mPath, response);
                }
                return;
            }
        }

        response.status = to_underlying(Protocols::InteractionModel::Status::Success);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    SceneTableEntry scene(SceneStorageId(req.sceneIdentifierFrom, req.groupIdentifierFrom));
    err = mSceneTable->GetSceneTableEntry(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                          scene.mStorageId, scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::NotFound);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        else
        {
            response.status = to_underlying(StatusIB(err).mStatus);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        return;
    }

    scene.mStorageId = SceneStorageId(req.sceneIdentifierTo, req.groupIdentifierTo);

    err = mSceneTable->SetSceneTableEntry(mPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(), scene);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_NO_MEMORY)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        else
        {
            response.status = to_underlying(StatusIB(err).mStatus);
            ctx.mCommandHandler.AddResponse(mPath, response);
        }
        return;
    }

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

} // namespace Scenes
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterScenesPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in ScenesServer::Init()
}
