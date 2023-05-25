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
#include <app/util/error-mapping.h>
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
using AuthMode          = chip::Access::AuthMode;

namespace chip {
namespace app {
namespace Clusters {
namespace Scenes {

/// @brief Generate and add a response to a command handler context depending on an EmberAfStatus
/// @tparam ResponseType Type of response, depends on the command
/// @param ctx Command Handler context where to add reponse
/// @param resp Response to add in ctx
/// @param status Status to verify
/// @return EmberAfStatus -> CHIP_ERROR
template <typename ResponseType>
CHIP_ERROR AddResponseOnError(CommandHandlerInterface::HandlerContext & ctx, ResponseType & resp, EmberAfStatus status)
{
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        resp.status = to_underlying(ToInteractionModelStatus(status));
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
    }
    return StatusIB(ToInteractionModelStatus(status)).ToChipError();
}

/// @brief Generate and add a response to a command handler context if err parameter is not CHIP_NO_ERROR
/// @tparam ResponseType Type of response, depends on the command
/// @param ctx Command Handler context where to add reponse
/// @param resp Response to add in ctx
/// @param status Status to verify
/// @return CHIP_ERROR
template <typename ResponseType>
CHIP_ERROR AddResponseOnError(CommandHandlerInterface::HandlerContext & ctx, ResponseType & resp, CHIP_ERROR err)
{
    if (CHIP_NO_ERROR != err)
    {
        resp.status = to_underlying(StatusIB(err).mStatus);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
    }
    return err;
}

ScenesServer ScenesServer::mInstance;

ScenesServer & ScenesServer::Instance()
{
    return mInstance;
}
void ReportAttributeOnAllEndpoints(AttributeId attribute) {}

CHIP_ERROR ScenesServer::Init()
{
    // Prevents re-initializing
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    mGroupProvider = Credentials::GetGroupDataProvider();

    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable = chip::scenes::GetSceneTableImpl();
    ReturnErrorOnFailure(sceneTable->Init(&chip::Server::GetInstance().GetPersistentStorage()));

    for (auto endpoint : EnabledEndpointsWithServerCluster(Id))
    {
        EmberAfStatus status = Attributes::FeatureMap::Set(endpoint, to_underlying(Feature::kSceneNames));
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogDetail(Zcl, "ERR: setting feature map on Endpoint %hu Status: %x", endpoint, status);
        }
        //  The bit of 7 the NameSupport attribute indicates whether or not scene names are supported
        //
        //  According to spec, bit 7 (Scene Names) MUST match feature bit 0 (Scene Names)
        status = Attributes::NameSupport::Set(endpoint, 0x80);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogDetail(Zcl, "ERR: setting NameSupport on Endpoint %hu Status: %x", endpoint, status);
        }
        status = Attributes::LastConfiguredBy::SetNull(endpoint);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogDetail(Zcl, "ERR: setting LastConfiguredBy on Endpoint %hu Status: %x", endpoint, status);
        }
    }

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

void ScenesServer::Shutdown()
{
    chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);

    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable = chip::scenes::GetSceneTableImpl();
    sceneTable->Finish();
    mGroupProvider = nullptr;
    mIsInitialized = false;
}

template <typename CommandData, typename ResponseType>
void AddSceneParse(CommandHandlerInterface::HandlerContext & ctx, const CommandData & req, GroupDataProvider * groupProvider)
{
    ResponseType response;
    uint16_t endpoinTableSize = 0;

    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

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
    if (0 != req.groupID &&
        !groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    uint32_t transitionTimeMs = 0;
    if (Commands::AddScene::Id == ctx.mRequestPath.mCommandId)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime) * 1000u;
    }
    else if (Commands::EnhancedAddScene::Id == ctx.mRequestPath.mCommandId)
    {
        transitionTimeMs = static_cast<uint32_t>(req.transitionTime) * 100u;
    }

    auto fieldSetIter = req.extensionFieldSets.begin();

    uint8_t EFSCount = 0;
    SceneData storageData(req.sceneName, transitionTimeMs);

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
                ReturnOnFailure(AddResponseOnError<ResponseType>(
                    ctx, response, handler.SerializeAdd(ctx.mRequestPath.mEndpointId, fieldSetIter.GetValue(), buff_span)));
                break;
            }
        }

        ReturnOnFailure(AddResponseOnError<ResponseType>(ctx, response, fieldSetIter.GetStatus()));

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
    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    if (0 == capacity)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    //  Insert in table
    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));

    // Update Attributes
    uint8_t scene_count = 0;
    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, sceneTable->GetFabricSceneCount(ctx.mCommandHandler.GetAccessingFabricIndex(), scene_count)));

    ReturnOnFailure(
        AddResponseOnError<ResponseType>(ctx, response, Attributes::SceneCount::Set(ctx.mRequestPath.mEndpointId, scene_count)));

    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    ReturnOnFailure(AddResponseOnError<ResponseType>(ctx, response,
                                                     Attributes::RemainingCapacity::Set(ctx.mRequestPath.mEndpointId, capacity)));

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<ResponseType>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(
            AddResponseOnError<ResponseType>(ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

template <typename CommandData, typename ResponseType>
void ViewSceneParse(HandlerContext & ctx, const CommandData & req, GroupDataProvider * groupProvider)
{
    ResponseType response;

    uint16_t endpoinTableSize = 0;

    ReturnOnFailure(AddResponseOnError<ResponseType>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != groupProvider);
    if (0 != req.groupID &&
        !groupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    SceneTableEntry scene;

    //  Gets the scene form the table
    ReturnOnFailure(
        AddResponseOnError<ResponseType>(ctx, response,
                                         sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                                        SceneStorageId(req.sceneID, req.groupID), scene)));

    // Verifies how to convert transition time
    if (scene.mStorageData.mSceneTransitionTimeMs)
    {
        if (Commands::ViewScene::Id == ctx.mRequestPath.mCommandId)
        {
            response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 1000));
        }
        else if (Commands::EnhancedViewScene::Id == ctx.mRequestPath.mCommandId)
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
                ReturnOnFailure(AddResponseOnError<ResponseType>(
                    ctx, response,
                    handler.Deserialize(ctx.mRequestPath.mEndpointId, tempField.mID, efsSpan, responseEFSBuffer[i])));
                deserializedEFSCount++;
                break;
            }
        }
    }

    Span<Structs::ExtensionFieldSet::Type> responseEFSSpan(responseEFSBuffer, deserializedEFSCount);
    response.extensionFieldSets.SetValue(responseEFSSpan);

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<ResponseType>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(
            AddResponseOnError<ResponseType>(ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
    }

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                           const SceneId & sceneID, GroupDataProvider * groupProvider)
{
    uint16_t endpoinTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(endpointID, &endpoinTableSize))).ToChipError());

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable = chip::scenes::GetSceneTableImpl(endpointID, endpoinTableSize);

    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (0 != groupID && !groupProvider->HasEndpoint(fabricIdx, groupID, endpointID))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    CHIP_ERROR err = sceneTable->GetSceneTableEntry(fabricIdx, scene.mStorageId, scene);
    if (CHIP_NO_ERROR != err && CHIP_ERROR_NOT_FOUND != err)
    {
        return err;
    }

    if (CHIP_ERROR_NOT_FOUND == err)
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
    ReturnErrorOnFailure(sceneTable->SetSceneTableEntry(fabricIdx, scene));

    // Update size attributes
    uint8_t scene_count = 0;
    ReturnErrorOnFailure(sceneTable->GetFabricSceneCount(fabricIdx, scene_count));

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::SceneCount::Set(endpointID, scene_count))).ToChipError());

    uint8_t capacity = 0;
    ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(fabricIdx, capacity));

    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::RemainingCapacity::Set(endpointID, capacity))).ToChipError());

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentScene::Set(endpointID, sceneID))).ToChipError());

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentGroup::Set(endpointID, groupID))).ToChipError());

    return CHIP_NO_ERROR;
}

CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                            const SceneId & sceneID, const Optional<DataModel::Nullable<uint16_t>> & transitionTime,
                            GroupDataProvider * groupProvider)
{
    uint16_t endpoinTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(endpointID, &endpoinTableSize))).ToChipError());

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable = chip::scenes::GetSceneTableImpl(endpointID, endpoinTableSize);

    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != groupProvider, CHIP_ERROR_INTERNAL);
    if (0 != groupID && !groupProvider->HasEndpoint(fabricIdx, groupID, endpointID))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(nullptr != sceneTable, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(sceneTable->GetSceneTableEntry(fabricIdx, scene.mStorageId, scene));

    // Check for optional
    if (transitionTime.HasValue())
    {
        // Check for nullable
        if (!transitionTime.Value().IsNull())
        {
            scene.mStorageData.mSceneTransitionTimeMs = static_cast<uint32_t>(transitionTime.Value().Value() * 100);
        }
    }

    ReturnErrorOnFailure(sceneTable->SceneApplyEFS(scene));

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentScene::Set(endpointID, sceneID))).ToChipError());

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentGroup::Set(endpointID, groupID))).ToChipError());

    return CHIP_NO_ERROR;
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
    uint8_t value             = 0;
    uint16_t endpoinTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(aPath.mEndpointId, &endpoinTableSize))).ToChipError());

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable;

    switch (aPath.mAttributeId)
    {
    case Attributes::SceneCount::Id:
        sceneTable = chip::scenes::GetSceneTableImpl(aPath.mEndpointId, endpoinTableSize);
        ReturnErrorOnFailure(sceneTable->GetEndpointSceneCount(value));
        return aEncoder.Encode(value);
    case Attributes::RemainingCapacity::Id:
        sceneTable = chip::scenes::GetSceneTableImpl(aPath.mEndpointId, endpoinTableSize);
        ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(aEncoder.AccessingFabricIndex(), value));
        return aEncoder.Encode(value);
    default:
        return CHIP_NO_ERROR;
    }
}

void ScenesServer::GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId)
{
    uint16_t endpoinTableSize = 0;
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == Attributes::SceneTableSize::Get(aEndpointId, &endpoinTableSize));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(aEndpointId, endpoinTableSize);
    VerifyOrReturn(nullptr != sceneTable);

    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == Attributes::SceneValid::Set(aEndpointId, false));

    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != aGroupId && !mGroupProvider->HasEndpoint(aFabricIx, aGroupId, aEndpointId))
    {
        return;
    }

    sceneTable->DeleteAllScenesInGroup(aFabricIx, aGroupId);
}

void ScenesServer::MakeSceneInvalid(EndpointId aEndpointId)
{
    Attributes::SceneValid::Set(aEndpointId, false);
}

void ScenesServer::StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    if (CHIP_NO_ERROR == StoreSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, mGroupProvider))
    {
        Attributes::SceneValid::Set(aEndpointId, true);
    }
}
void ScenesServer::RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == Attributes::SceneValid::Set(aEndpointId, false));

    Optional<DataModel::Nullable<uint16_t>> transitionTime;

    if (CHIP_NO_ERROR == RecallSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, transitionTime, mGroupProvider))
    {
        Attributes::SceneValid::Set(aEndpointId, true);
    }
}

void ScenesServer::HandleAddScene(HandlerContext & ctx, const Commands::AddScene::DecodableType & req)
{
    AddSceneParse<Commands::AddScene::DecodableType, Commands::AddSceneResponse::Type>(ctx, req, mGroupProvider);
}

void ScenesServer::HandleViewScene(HandlerContext & ctx, const Commands::ViewScene::DecodableType & req)
{
    ViewSceneParse<Commands::ViewScene::DecodableType, Commands::ViewSceneResponse::Type>(ctx, req, mGroupProvider);
}

void ScenesServer::HandleRemoveScene(HandlerContext & ctx, const Commands::RemoveScene::DecodableType & req)
{
    Commands::RemoveSceneResponse::Type response;

    uint16_t endpoinTableSize = 0;
    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID));

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != req.groupID &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    //  Gets the scene from the table
    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

    // Remove the scene from the scene table
    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, sceneTable->RemoveSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId)));

    // Update Attributes
    uint8_t scene_count = 0;
    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, sceneTable->GetFabricSceneCount(ctx.mCommandHandler.GetAccessingFabricIndex(), scene_count)));

    Attributes::SceneCount::Set(ctx.mRequestPath.mEndpointId, scene_count);

    uint8_t capacity = 0;
    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
        ctx, response, Attributes::RemainingCapacity::Set(ctx.mRequestPath.mEndpointId, capacity)));

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(AddResponseOnError<Commands::RemoveSceneResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req)
{
    Commands::RemoveAllScenesResponse::Type response;

    uint16_t endpoinTableSize = 0;
    ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

    // Response data
    response.groupID = req.groupID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != req.groupID &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
        ctx, response, sceneTable->DeleteAllScenesInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID)));

    // Update Attributes
    ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
        ctx, response, Attributes::SceneCount::Set(ctx.mRequestPath.mEndpointId, 0)));

    ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
        ctx, response, Attributes::RemainingCapacity::Set(ctx.mRequestPath.mEndpointId, 0)));

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(AddResponseOnError<Commands::RemoveAllScenesResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req)
{
    Commands::StoreSceneResponse::Type response;

    // Scene Valid is false when this command begins
    ReturnOnFailure(AddResponseOnError<Commands::StoreSceneResponse::Type>(ctx, response,
                                                                           Attributes::SceneValid::Set(mPath.mEndpointId, false)));

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    CHIP_ERROR err =
        StoreSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), mPath.mEndpointId, req.groupID, req.sceneID, mGroupProvider);

    if (CHIP_NO_ERROR == err)
    {
        ReturnOnFailure(AddResponseOnError<Commands::StoreSceneResponse::Type>(
            ctx, response, Attributes::SceneValid::Set(mPath.mEndpointId, true)));
        Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
        if (AuthMode::kCase == descriptor.authMode)
        {
            ReturnOnFailure(AddResponseOnError<Commands::StoreSceneResponse::Type>(
                ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
        }
        else
        {
            ReturnOnFailure(AddResponseOnError<Commands::StoreSceneResponse::Type>(
                ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
        }
    }

    response.status = to_underlying(StatusIB(err).mStatus);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req)
{
    // Scene Valid is false when this command begins
    EmberAfStatus status = Attributes::SceneValid::Set(mPath.mEndpointId, false);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ctx.mCommandHandler.AddStatus(mPath, ToInteractionModelStatus(status));
        return;
    }

    CHIP_ERROR err = RecallSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), mPath.mEndpointId, req.groupID, req.sceneID,
                                      req.transitionTime, mGroupProvider);

    if (CHIP_NO_ERROR == err)
    {
        status = Attributes::SceneValid::Set(mPath.mEndpointId, true);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ctx.mCommandHandler.AddStatus(mPath, ToInteractionModelStatus(status));
            return;
        }

        Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
        if (AuthMode::kCase == descriptor.authMode)
        {
            status = Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject);
            if (EMBER_ZCL_STATUS_SUCCESS != status)
            {
                ctx.mCommandHandler.AddStatus(mPath, ToInteractionModelStatus(status));
                return;
            }
        }
        else
        {
            status = Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId);
            if (EMBER_ZCL_STATUS_SUCCESS != status)
            {
                ctx.mCommandHandler.AddStatus(mPath, ToInteractionModelStatus(status));
                return;
            }
        }
    }

    ctx.mCommandHandler.AddStatus(mPath, StatusIB(err).mStatus);
}

void ScenesServer::HandleGetSceneMembership(HandlerContext & ctx, const Commands::GetSceneMembership::DecodableType & req)
{
    Commands::GetSceneMembershipResponse::Type response;

    uint16_t endpoinTableSize = 0;
    ReturnOnFailure(AddResponseOnError<Commands::GetSceneMembershipResponse::Type>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

    // Response data
    response.groupID = req.groupID;

    // Scene Table interface data
    SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
    Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);
    SceneTableEntry scene;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != req.groupID &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    uint8_t capacity = 0;
    // Get Capacity
    ReturnOnFailure(AddResponseOnError<Commands::GetSceneMembershipResponse::Type>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));
    response.capacity.SetNonNull(capacity);

    // populate scene list
    ReturnOnFailure(AddResponseOnError<Commands::GetSceneMembershipResponse::Type>(
        ctx, response, sceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, sceneList)));

    response.sceneList.SetValue(sceneList);

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<Commands::GetSceneMembershipResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(AddResponseOnError<Commands::GetSceneMembershipResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
    }

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(mPath, response);
}

void ScenesServer::HandleEnhancedAddScene(HandlerContext & ctx, const Commands::EnhancedAddScene::DecodableType & req)
{
    AddSceneParse<Commands::EnhancedAddScene::DecodableType, Commands::EnhancedAddSceneResponse::Type>(ctx, req, mGroupProvider);
}

void ScenesServer::HandleEnhancedViewScene(HandlerContext & ctx, const Commands::EnhancedViewScene::DecodableType & req)
{
    ViewSceneParse<Commands::EnhancedViewScene::DecodableType, Commands::EnhancedViewSceneResponse::Type>(ctx, req, mGroupProvider);
}
void ScenesServer::HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req)
{
    Commands::CopySceneResponse::Type response;

    uint16_t endpoinTableSize = 0;
    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpoinTableSize)));

    // Get Scene Table Instance
    scenes::SceneTable<scenes::ExtensionFieldSetsImpl> * sceneTable =
        chip::scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpoinTableSize);

    // Response data
    response.groupIdentifierFrom = req.groupIdentifierFrom;
    response.sceneIdentifierFrom = req.sceneIdentifierFrom;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if ((0 != req.groupIdentifierFrom &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, mPath.mEndpointId)) ||
        (0 != req.groupIdentifierTo &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierTo, mPath.mEndpointId)))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    uint8_t capacity = 0;
    // Get Capacity
    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    // Checks if we copy a single scene or all of them
    if (req.mode.GetField(app::Clusters::Scenes::ScenesCopyMode::kCopyAllScenes))
    {
        // Scene Table interface data
        SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
        Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);

        // populate scene list
        ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
            ctx, response,
            sceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, sceneList)));

        if (0 == capacity)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(mPath, response);
            return;
        }

        for (auto & sceneId : sceneList)
        {
            SceneTableEntry scene(SceneStorageId(sceneId, req.groupIdentifierFrom));
            //  Insert in table
            ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
                ctx, response,
                sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

            scene.mStorageId = SceneStorageId(sceneId, req.groupIdentifierTo);

            ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
                ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));
        }

        response.status = to_underlying(Protocols::InteractionModel::Status::Success);
        ctx.mCommandHandler.AddResponse(mPath, response);
        return;
    }

    SceneTableEntry scene(SceneStorageId(req.sceneIdentifierFrom, req.groupIdentifierFrom));
    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

    scene.mStorageId = SceneStorageId(req.sceneIdentifierTo, req.groupIdentifierTo);

    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));

    // Update Attributes
    uint8_t scene_count = 0;
    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, sceneTable->GetFabricSceneCount(ctx.mCommandHandler.GetAccessingFabricIndex(), scene_count)));

    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, Attributes::SceneCount::Set(ctx.mRequestPath.mEndpointId, scene_count)));

    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
        ctx, response, Attributes::RemainingCapacity::Set(ctx.mRequestPath.mEndpointId, capacity)));

    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    if (AuthMode::kCase == descriptor.authMode)
    {
        ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject)));
    }
    else
    {
        ReturnOnFailure(AddResponseOnError<Commands::CopySceneResponse::Type>(
            ctx, response, Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId)));
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
