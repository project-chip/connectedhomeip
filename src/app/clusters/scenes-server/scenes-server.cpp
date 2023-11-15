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
using ScenesServer      = chip::app::Clusters::Scenes::ScenesServer;

namespace chip {
namespace app {
namespace Clusters {
namespace Scenes {

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

/// @brief Generate and add a response to a command handler context depending on an EmberAfStatus
/// @tparam ResponseType Type of response, depends on the command
/// @param ctx Command Handler context where to add reponse
/// @param resp Response to add in ctx
/// @param status Status to verify
/// @return EmberAfStatus -> CHIP_ERROR
template <typename ResponseType>
CHIP_ERROR AddResponseOnError(CommandHandlerInterface::HandlerContext & ctx, ResponseType & resp, EmberAfStatus status)
{
    return AddResponseOnError(ctx, resp, StatusIB(ToInteractionModelStatus(status)).ToChipError());
}

template <typename ResponseType>
CHIP_ERROR UpdateLastConfiguredBy(HandlerContext & ctx, ResponseType resp)
{
    Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    EmberAfStatus status                 = EMBER_ZCL_STATUS_SUCCESS;

    if (AuthMode::kCase == descriptor.authMode)
    {
        status = Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject);
    }
    else
    {
        status = Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId);
    }

    // LastConfiguredBy is optional, so we don't want to fail the command if it fails to update
    VerifyOrReturnValue(!(EMBER_ZCL_STATUS_SUCCESS == status || EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE == status), CHIP_NO_ERROR);
    return AddResponseOnError(ctx, resp, status);
}

ScenesServer ScenesServer::mInstance;

ScenesServer & ScenesServer::Instance()
{
    return mInstance;
}
void ReportAttributeOnAllEndpoints(AttributeId attribute) {}

class ScenesClusterFabricDelegate : public chip::FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        SceneTable * sceneTable = scenes::GetSceneTableImpl();
        VerifyOrReturn(nullptr != sceneTable);
        // The implementation of SceneTable::RemoveFabric() must not call back into the FabricTable
        sceneTable->RemoveFabric(fabricIndex);
    }
};

static ScenesClusterFabricDelegate gFabricDelegate;

CHIP_ERROR ScenesServer::Init()
{
    // Prevents re-initializing
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    mGroupProvider = Credentials::GetGroupDataProvider();

    SceneTable * sceneTable = scenes::GetSceneTableImpl();
    ReturnErrorOnFailure(sceneTable->Init(&chip::Server::GetInstance().GetPersistentStorage()));
    ReturnErrorOnFailure(chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate));

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

void ScenesServer::Shutdown()
{
    chip::app::InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);

    mGroupProvider = nullptr;
    mIsInitialized = false;
}

template <typename CommandData, typename ResponseType>
void AddSceneParse(CommandHandlerInterface::HandlerContext & ctx, const CommandData & req, GroupDataProvider * groupProvider)
{
    ResponseType response;
    uint16_t endpointTableSize = 0;

    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

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

    uint32_t featureMap = 0;
    ReturnOnFailure(AddResponseOnError(ctx, response, Attributes::FeatureMap::Get(ctx.mRequestPath.mEndpointId, &featureMap)));

    SceneData storageData(CharSpan(), transitionTimeMs);
    if (featureMap & to_underlying(Feature::kSceneNames))
    {
        storageData.SetName(req.sceneName);
    }

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
                ReturnOnFailure(AddResponseOnError(
                    ctx, response, handler.SerializeAdd(ctx.mRequestPath.mEndpointId, fieldSetIter.GetValue(), buff_span)));
                break;
            }
        }

        static_assert(sizeof(tempEFS.mBytesBuffer) <= UINT8_MAX, "Serialized EFS number of bytes must fit in a uint8");
        tempEFS.mUsedBytes = static_cast<uint8_t>(buff_span.size());

        if (!tempEFS.IsEmpty())
        {
            storageData.mExtensionFieldSets.InsertFieldSet(tempEFS);
        }
    }
    ReturnOnFailure(AddResponseOnError(ctx, response, fieldSetIter.GetStatus()));

    // Create scene from data and ID
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID), storageData);

    // Get Capacity
    VerifyOrReturn(nullptr != sceneTable);
    uint8_t capacity = 0;
    ReturnOnFailure(AddResponseOnError(ctx, response,
                                       sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    if (0 == capacity)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    //  Insert in table
    ReturnOnFailure(
        AddResponseOnError(ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));

    // Update Attributes
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::SceneCount::Id);
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::RemainingCapacity::Id);

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

template <typename CommandData, typename ResponseType>
void ViewSceneParse(HandlerContext & ctx, const CommandData & req, GroupDataProvider * groupProvider)
{
    ResponseType response;

    uint16_t endpointTableSize = 0;

    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

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

    //  Gets the scene from the table
    ReturnOnFailure(AddResponseOnError(ctx, response,
                                       sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                                      SceneStorageId(req.sceneID, req.groupID), scene)));

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
                ReturnOnFailure(AddResponseOnError(
                    ctx, response,
                    handler.Deserialize(ctx.mRequestPath.mEndpointId, tempField.mID, efsSpan, responseEFSBuffer[i])));
                deserializedEFSCount++;
                break;
            }
        }
    }

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);

    // Verifies how to convert transition time
    if (Commands::ViewScene::Id == ctx.mRequestPath.mCommandId)
    {
        response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 1000));
    }
    else if (Commands::EnhancedViewScene::Id == ctx.mRequestPath.mCommandId)
    {
        response.transitionTime.SetValue(static_cast<uint16_t>(scene.mStorageData.mSceneTransitionTimeMs / 100));
    }

    response.sceneName.SetValue(CharSpan(scene.mStorageData.mName, scene.mStorageData.mNameLength));
    Span<Structs::ExtensionFieldSet::Type> responseEFSSpan(responseEFSBuffer, deserializedEFSCount);
    response.extensionFieldSets.SetValue(responseEFSSpan);

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                           const SceneId & sceneID, GroupDataProvider * groupProvider)
{
    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(endpointID, &endpointTableSize))).ToChipError());

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(endpointID, endpointTableSize);

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
        uint32_t featureMap = 0;
        ReturnErrorOnFailure(
            StatusIB(ToInteractionModelStatus(Attributes::FeatureMap::Get(endpointID, &featureMap))).ToChipError());
        // Check if we still support scenes name in case an OTA changed that, if we don't, set name to empty
        if (!(featureMap & to_underlying(Feature::kSceneNames)))
        {
            scene.mStorageData.SetName(CharSpan());
        }
        scene.mStorageData.mExtensionFieldSets.Clear();
    }

    // Gets the EFS
    ReturnErrorOnFailure(sceneTable->SceneSaveEFS(scene));
    // Insert in Scene Table
    ReturnErrorOnFailure(sceneTable->SetSceneTableEntry(fabricIdx, scene));

    // Update size attributes
    MatterReportingAttributeChangeCallback(endpointID, Id, Attributes::SceneCount::Id);
    MatterReportingAttributeChangeCallback(endpointID, Id, Attributes::RemainingCapacity::Id);

    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentScene::Set(endpointID, sceneID))).ToChipError());
    ReturnErrorOnFailure(StatusIB(ToInteractionModelStatus(Attributes::CurrentGroup::Set(endpointID, groupID))).ToChipError());

    return CHIP_NO_ERROR;
}

CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                            const SceneId & sceneID, const Optional<DataModel::Nullable<uint16_t>> & transitionTime,
                            GroupDataProvider * groupProvider)
{
    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(endpointID, &endpointTableSize))).ToChipError());

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(endpointID, endpointTableSize);

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
    switch (ctxt.mRequestPath.mCommandId)
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
    uint8_t value              = 0;
    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(
        StatusIB(ToInteractionModelStatus(Attributes::SceneTableSize::Get(aPath.mEndpointId, &endpointTableSize))).ToChipError());

    // Get Scene Table Instance
    SceneTable * sceneTable;

    switch (aPath.mAttributeId)
    {
    case Attributes::SceneCount::Id:
        sceneTable = scenes::GetSceneTableImpl(aPath.mEndpointId, endpointTableSize);
        ReturnErrorOnFailure(sceneTable->GetEndpointSceneCount(value));
        return aEncoder.Encode(value);
    case Attributes::RemainingCapacity::Id:
        sceneTable = scenes::GetSceneTableImpl(aPath.mEndpointId, endpointTableSize);
        ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(aEncoder.AccessingFabricIndex(), value));
        return aEncoder.Encode(value);
    default:
        return CHIP_NO_ERROR;
    }
}

void ScenesServer::GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId)
{
    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);
    VerifyOrReturn(nullptr != sceneTable);

    chip::GroupId currentGroup;
    Attributes::CurrentGroup::Get(aEndpointId, &currentGroup);

    // If currentGroup is what is being removed, we can't possibly still have a valid scene,
    // because the scene we have (if any) will also be removed.
    if (aGroupId == currentGroup)
    {
        MakeSceneInvalid(aEndpointId);
    }

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

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

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
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    //  Gets the scene from the table
    ReturnOnFailure(AddResponseOnError(
        ctx, response, sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

    // Remove the scene from the scene table
    ReturnOnFailure(AddResponseOnError(
        ctx, response, sceneTable->RemoveSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId)));

    // Update Attributes
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::SceneCount::Id);
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::RemainingCapacity::Id);

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req)
{
    Commands::RemoveAllScenesResponse::Type response;

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

    // Response data
    response.groupID = req.groupID;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != req.groupID &&
        !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, ctx.mRequestPath.mEndpointId))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    ReturnOnFailure(AddResponseOnError(
        ctx, response, sceneTable->DeleteAllScenesInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID)));

    // Update Attributes
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::SceneCount::Id);
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::RemainingCapacity::Id);

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req)
{
    Commands::StoreSceneResponse::Type response;

    // Scene Valid is false when this command begins
    ReturnOnFailure(AddResponseOnError(ctx, response, Attributes::SceneValid::Set(ctx.mRequestPath.mEndpointId, false)));

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    CHIP_ERROR err = StoreSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), ctx.mRequestPath.mEndpointId, req.groupID,
                                     req.sceneID, mGroupProvider);

    if (CHIP_NO_ERROR == err)
    {
        ReturnOnFailure(AddResponseOnError(ctx, response, Attributes::SceneValid::Set(ctx.mRequestPath.mEndpointId, true)));

        ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));
    }

    response.status = to_underlying(StatusIB(err).mStatus);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req)
{
    // Scene Valid is false when this command begins
    EmberAfStatus status = Attributes::SceneValid::Set(ctx.mRequestPath.mEndpointId, false);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, ToInteractionModelStatus(status));
        return;
    }

    CHIP_ERROR err = RecallSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), ctx.mRequestPath.mEndpointId, req.groupID,
                                      req.sceneID, req.transitionTime, mGroupProvider);

    if (CHIP_NO_ERROR == err)
    {
        status = Attributes::SceneValid::Set(ctx.mRequestPath.mEndpointId, true);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, ToInteractionModelStatus(status));
        return;
    }

    if (CHIP_ERROR_NOT_FOUND == err)
    {
        // TODO : implement proper mapping between CHIP_ERROR and IM Status
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, StatusIB(err).mStatus);
}

void ScenesServer::HandleGetSceneMembership(HandlerContext & ctx, const Commands::GetSceneMembership::DecodableType & req)
{
    Commands::GetSceneMembershipResponse::Type response;

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

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
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    uint8_t capacity = 0;
    // Get Capacity
    ReturnOnFailure(AddResponseOnError(ctx, response,
                                       sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));
    response.capacity.SetNonNull(capacity);

    // populate scene list
    ReturnOnFailure(AddResponseOnError(
        ctx, response, sceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupID, sceneList)));

    response.sceneList.SetValue(sceneList);

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
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

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

    // Response data
    response.groupIdentifierFrom = req.groupIdentifierFrom;
    response.sceneIdentifierFrom = req.sceneIdentifierFrom;

    // Verify Endpoint in group
    VerifyOrReturn(nullptr != mGroupProvider);
    if ((0 != req.groupIdentifierFrom &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom,
                                      ctx.mRequestPath.mEndpointId)) ||
        (0 != req.groupIdentifierTo &&
         !mGroupProvider->HasEndpoint(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierTo,
                                      ctx.mRequestPath.mEndpointId)))
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::InvalidCommand);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    uint8_t capacity = 0;
    // Get Capacity
    ReturnOnFailure(AddResponseOnError(ctx, response,
                                       sceneTable->GetRemainingCapacity(ctx.mCommandHandler.GetAccessingFabricIndex(), capacity)));

    // Checks if we copy a single scene or all of them
    if (req.mode.GetField(app::Clusters::Scenes::CopyModeBitmap::kCopyAllScenes))
    {
        // Scene Table interface data
        SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
        Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);

        // populate scene list
        ReturnOnFailure(AddResponseOnError(
            ctx, response,
            sceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, sceneList)));

        if (0 == capacity)
        {
            response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
            return;
        }

        for (auto & sceneId : sceneList)
        {
            SceneTableEntry scene(SceneStorageId(sceneId, req.groupIdentifierFrom));
            //  Insert in table
            ReturnOnFailure(AddResponseOnError(
                ctx, response,
                sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

            scene.mStorageId = SceneStorageId(sceneId, req.groupIdentifierTo);

            ReturnOnFailure(AddResponseOnError(
                ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));
        }

        // Update Attributes
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::SceneCount::Id);
        MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::RemainingCapacity::Id);

        ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

        response.status = to_underlying(Protocols::InteractionModel::Status::Success);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    SceneTableEntry scene(SceneStorageId(req.sceneIdentifierFrom, req.groupIdentifierFrom));
    ReturnOnFailure(AddResponseOnError(
        ctx, response, sceneTable->GetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene.mStorageId, scene)));

    scene.mStorageId = SceneStorageId(req.sceneIdentifierTo, req.groupIdentifierTo);

    ReturnOnFailure(
        AddResponseOnError(ctx, response, sceneTable->SetSceneTableEntry(ctx.mCommandHandler.GetAccessingFabricIndex(), scene)));

    // Update Attributes
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::SceneCount::Id);
    MatterReportingAttributeChangeCallback(ctx.mRequestPath.mEndpointId, Id, Attributes::RemainingCapacity::Id);

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

} // namespace Scenes
} // namespace Clusters
} // namespace app
} // namespace chip

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Scenes;

void emberAfScenesClusterServerInitCallback(EndpointId endpoint)
{
    uint32_t featureMap  = 0;
    EmberAfStatus status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        // According to spec, bit 7 MUST match feature bit 0 (SceneNames)
        BitMask<NameSupportBitmap> nameSupport = (featureMap & to_underlying(Feature::kSceneNames))
            ? BitMask<NameSupportBitmap>(NameSupportBitmap::kSceneNames)
            : BitMask<NameSupportBitmap>();
        status                                 = Attributes::NameSupport::Set(endpoint, nameSupport);
        if (EMBER_ZCL_STATUS_SUCCESS != status)
        {
            ChipLogDetail(Zcl, "ERR: setting NameSupport on Endpoint %hu Status: %x", endpoint, status);
        }
    }
    else
    {
        ChipLogDetail(Zcl, "ERR: getting the scenes FeatureMap on Endpoint %hu Status: %x", endpoint, status);
    }

    // Explicit AttributeValuePairs and TableSize features are mandatory for matter so we force-set them here
    featureMap |= (to_underlying(Feature::kExplicit) | to_underlying(Feature::kTableSize));
    status = Attributes::FeatureMap::Set(endpoint, featureMap);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "ERR: setting the scenes FeatureMap on Endpoint %hu Status: %x", endpoint, status);
    }

    status = Attributes::LastConfiguredBy::SetNull(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "ERR: setting LastConfiguredBy on Endpoint %hu Status: %x", endpoint, status);
    }
}

void MatterScenesClusterServerShutdownCallback(EndpointId endpoint)
{
    uint16_t endpointTableSize = 0;
    ReturnOnFailure(Attributes::SceneTableSize::Get(endpoint, &endpointTableSize));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(endpoint, endpointTableSize);
    sceneTable->RemoveEndpoint();
}

void MatterScenesPluginServerInitCallback()
{
    CHIP_ERROR err = ScenesServer::Instance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ScenesServer::Instance().Init() error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
