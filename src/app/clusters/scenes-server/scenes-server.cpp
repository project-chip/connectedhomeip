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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <tracing/macros.h>

using SceneTableEntry   = chip::scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneStorageId    = chip::scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData         = chip::scenes::DefaultSceneTableImpl::SceneData;
using HandlerContext    = chip::app::CommandHandlerInterface::HandlerContext;
using ExtensionFieldSet = chip::scenes::ExtensionFieldSet;
using GroupDataProvider = chip::Credentials::GroupDataProvider;
using SceneTable        = chip::scenes::SceneTable<chip::scenes::ExtensionFieldSetsImpl>;
using AuthMode          = chip::Access::AuthMode;
using ScenesServer      = chip::app::Clusters::ScenesManagement::ScenesServer;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ScenesManagement {

namespace {

Protocols::InteractionModel::Status ResponseStatus(CHIP_ERROR err)
{
    // TODO : Properly fix mapping between error types (issue https://github.com/project-chip/connectedhomeip/issues/26885)
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        return Protocols::InteractionModel::Status::NotFound;
    }
    if (CHIP_ERROR_NO_MEMORY == err)
    {
        return Protocols::InteractionModel::Status::ResourceExhausted;
    }
    if (CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute) == err)
    {
        // TODO: Confirm if we need to add UnsupportedAttribute status as a return for Scene Commands
        return Protocols::InteractionModel::Status::InvalidCommand;
    }
    return StatusIB(err).mStatus;
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
        resp.status = to_underlying(ResponseStatus(err));
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
    }
    return err;
}

/// @brief Generate and add a response to a command handler context depending on an InteractionModel::Status
/// @tparam ResponseType Type of response, depends on the command
/// @param ctx Command Handler context where to add reponse
/// @param resp Response to add in ctx
/// @param status Status to verify
/// @return InteractionModel::Status -> CHIP_ERROR
template <typename ResponseType>
CHIP_ERROR AddResponseOnError(CommandHandlerInterface::HandlerContext & ctx, ResponseType & resp, Status status)
{
    // TODO: this seems odd: we convert `status` to a CHIP_ERROR and then back to status. This seems
    //       potentially lossy and not ideal.
    return AddResponseOnError(ctx, resp, StatusIB(status).ToChipError());
}

Status SetLastConfiguredBy(HandlerContext & ctx)
{
    const Access::SubjectDescriptor descriptor = ctx.mCommandHandler.GetSubjectDescriptor();

    if (AuthMode::kCase == descriptor.authMode)
    {
        return Attributes::LastConfiguredBy::Set(ctx.mRequestPath.mEndpointId, descriptor.subject);
    }

    return Attributes::LastConfiguredBy::SetNull(ctx.mRequestPath.mEndpointId);
}

template <typename ResponseType>
CHIP_ERROR UpdateLastConfiguredBy(HandlerContext & ctx, ResponseType resp)
{
    Status status = SetLastConfiguredBy(ctx);

    // LastConfiguredBy is optional, so we don't want to fail the command if it fails to update
    VerifyOrReturnValue(!(Status::Success == status || Status::UnsupportedAttribute == status), CHIP_NO_ERROR);
    return AddResponseOnError(ctx, resp, status);
}

/// @brief Helper function to update the FabricSceneInfo attribute for a given Endpoint and fabric
/// @param endpoint Endpoint to update
/// @param fabric Fabric to update
/// @param group Group to update, if not provided, will be assigned 0 for a new SceneInfoStruct or keep previous value for an
/// existing one
/// @param scene Scene to update, if not provided, will be assigned 0 for a new SceneInfoStruct or keep previous value for an
/// existing one
/// @param sceneValid sceneValid status, if not provided, will be assigned false for a new SceneInfoStruct or keep previous
/// value for an existing one
/// @return
CHIP_ERROR UpdateFabricSceneInfo(EndpointId endpoint, FabricIndex fabric, Optional<GroupId> group, Optional<SceneId> scene,
                                 Optional<bool> sceneValid)
{
    VerifyOrReturnError(kInvalidEndpointId != endpoint, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    SceneTable * sceneTable                    = scenes::GetSceneTableImpl(endpoint);
    Structs::SceneInfoStruct::Type * sceneInfo = ScenesServer::Instance().GetSceneInfoStruct(endpoint, fabric);
    if (nullptr != sceneInfo)
    {
        if (group.HasValue())
        {
            sceneInfo->currentGroup = group.Value();
        }

        if (scene.HasValue())
        {
            sceneInfo->currentScene = scene.Value();
        }

        if (sceneValid.HasValue())
        {
            sceneInfo->sceneValid = sceneValid.Value();
        }

        ReturnErrorOnFailure(sceneTable->GetFabricSceneCount(fabric, sceneInfo->sceneCount));
        ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(fabric, sceneInfo->remainingCapacity));
    }
    else
    {
        // If we couldn't find a SceneInfoStruct for the fabric, create one
        Structs::SceneInfoStruct::Type newSceneInfo;
        newSceneInfo.fabricIndex = fabric;

        newSceneInfo.currentGroup = group.ValueOr(0);
        newSceneInfo.currentScene = scene.ValueOr(0);
        newSceneInfo.sceneValid   = sceneValid.ValueOr(false);

        ReturnErrorOnFailure(sceneTable->GetFabricSceneCount(fabric, newSceneInfo.sceneCount));
        ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(fabric, newSceneInfo.remainingCapacity));
        ReturnErrorOnFailure(ScenesServer::Instance().SetSceneInfoStruct(endpoint, fabric, newSceneInfo));
    }

    MatterReportingAttributeChangeCallback(endpoint, Id, Attributes::FabricSceneInfo::Id);
    return CHIP_NO_ERROR;
}

} // namespace

/// @brief Gets the SceneInfoStruct array associated to an endpoint
/// @param endpoint target endpoint
/// @return Optional with no value not found, Span of SceneInfoStruct
Span<Structs::SceneInfoStruct::Type> ScenesServer::FabricSceneInfo::GetFabricSceneInfo(EndpointId endpoint)
{
    size_t endpointIndex = 0;
    Span<Structs::SceneInfoStruct::Type> fabricSceneInfoSpan;
    CHIP_ERROR status = FindFabricSceneInfoIndex(endpoint, endpointIndex);
    if (CHIP_NO_ERROR == status)
    {
        fabricSceneInfoSpan =
            Span<Structs::SceneInfoStruct::Type>(&mSceneInfoStructs[endpointIndex][0], mSceneInfoStructsCount[endpointIndex]);
    }
    return fabricSceneInfoSpan;
}

/// @brief Gets the SceneInfoStruct for a specific fabric for a specific endpoint
/// @param endpoint target endpoint
/// @param fabric target fabric
/// @param index
/// @return Nullptr if not found, pointer to the SceneInfoStruct otherwise
Structs::SceneInfoStruct::Type * ScenesServer::FabricSceneInfo::GetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric)
{
    size_t endpointIndex = 0;
    VerifyOrReturnValue(CHIP_NO_ERROR == FindFabricSceneInfoIndex(endpoint, endpointIndex), nullptr);
    uint8_t sceneInfoStructIndex = 0;
    VerifyOrReturnValue(CHIP_NO_ERROR == FindSceneInfoStructIndex(fabric, endpointIndex, sceneInfoStructIndex), nullptr);

    return &mSceneInfoStructs[endpointIndex][sceneInfoStructIndex];
}

/// @brief Sets the SceneInfoStruct for a specific fabric for a specific endpoint
/// @param endpoint target endpoint
/// @param fabric target fabric
/// @param [in] sceneInfoStruct SceneInfoStruct to set
/// @return CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND if the endpoint is not found, CHIP_ERROR_NO_MEMORY if the number of fabrics is
/// exceeded, CHIP_ERROR_INVALID_ARGUMENT if invalid fabric or endpoint
CHIP_ERROR ScenesServer::FabricSceneInfo::SetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric,
                                                             Structs::SceneInfoStruct::Type & sceneInfoStruct)
{
    VerifyOrReturnError(kInvalidEndpointId != endpoint, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    size_t endpointIndex = 0;
    ReturnErrorOnFailure(FindFabricSceneInfoIndex(endpoint, endpointIndex));
    uint8_t sceneInfoStructIndex = 0;
    if (CHIP_ERROR_NOT_FOUND == FindSceneInfoStructIndex(fabric, endpointIndex, sceneInfoStructIndex))
    {
        VerifyOrReturnError(mSceneInfoStructsCount[endpointIndex] < MATTER_ARRAY_SIZE(mSceneInfoStructs[endpointIndex]),
                            CHIP_ERROR_NO_MEMORY);
        sceneInfoStructIndex = mSceneInfoStructsCount[endpointIndex];

        // Increment number of populated ScenesInfoStructs
        mSceneInfoStructsCount[endpointIndex]++;
    }
    mSceneInfoStructs[endpointIndex][sceneInfoStructIndex] = sceneInfoStruct;

    return CHIP_NO_ERROR;
}

/// @brief Clears the SceneInfoStruct associated to a fabric and compresses the array to leave uninitialised structs at the end
/// @param[in] endpoint target endpoint
/// @param[in] fabric target fabric
void ScenesServer::FabricSceneInfo::ClearSceneInfoStruct(EndpointId endpoint, FabricIndex fabric)
{
    size_t endpointIndex = 0;
    ReturnOnFailure(FindFabricSceneInfoIndex(endpoint, endpointIndex));
    uint8_t sceneInfoStructIndex = 0;
    ReturnOnFailure(FindSceneInfoStructIndex(fabric, endpointIndex, sceneInfoStructIndex));

    uint8_t nextIndex = static_cast<uint8_t>(sceneInfoStructIndex + 1);
    uint8_t moveNum   = static_cast<uint8_t>(MATTER_ARRAY_SIZE(mSceneInfoStructs[endpointIndex]) - nextIndex);
    // Compress the endpoint's SceneInfoStruct array
    if (moveNum)
    {
        for (size_t i = 0; i < moveNum; ++i)
        {
            mSceneInfoStructs[endpointIndex][sceneInfoStructIndex + i] = mSceneInfoStructs[endpointIndex][nextIndex + i];
        }
    }

    // Decrement the SceneInfoStruct count
    mSceneInfoStructsCount[endpointIndex]--;

    // Clear the last populated SceneInfoStruct
    mSceneInfoStructs[endpointIndex][mSceneInfoStructsCount[endpointIndex]].fabricIndex       = kUndefinedFabricIndex;
    mSceneInfoStructs[endpointIndex][mSceneInfoStructsCount[endpointIndex]].sceneCount        = 0;
    mSceneInfoStructs[endpointIndex][mSceneInfoStructsCount[endpointIndex]].currentScene      = 0;
    mSceneInfoStructs[endpointIndex][mSceneInfoStructsCount[endpointIndex]].currentGroup      = 0;
    mSceneInfoStructs[endpointIndex][mSceneInfoStructsCount[endpointIndex]].remainingCapacity = 0;
}

/// @brief Returns the index of the FabricSceneInfo associated to an endpoint
/// @param[in] endpoint target endpoint
/// @param[out] endpointIndex index of the corresponding FabricSceneInfo for an endpoint, corresponds to a row in the
/// mSceneInfoStructs array,
/// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid endpoint
CHIP_ERROR ScenesServer::FabricSceneInfo::FindFabricSceneInfoIndex(EndpointId endpoint, size_t & endpointIndex)
{
    VerifyOrReturnError(kInvalidEndpointId != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t index =
        emberAfGetClusterServerEndpointIndex(endpoint, ScenesManagement::Id, MATTER_DM_SCENES_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index < MATTER_ARRAY_SIZE(mSceneInfoStructs))
    {
        endpointIndex = index;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

/// @brief Returns the SceneInfoStruct associated to a fabric
/// @param[in] fabric target fabric index
/// @param[in] endpointIndex index of the corresponding FabricSceneInfo for an endpoint, corresponds to a row in the
/// mSceneInfoStructs array
/// @param[out] index index of the corresponding SceneInfoStruct if found, otherwise the index value will be invalid and
/// should not be used. This is safe to store in a uint8_t because the index is guaranteed to be smaller than
/// CHIP_CONFIG_MAX_FABRICS.
/// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid fabric or endpointIndex are provided
CHIP_ERROR ScenesServer::FabricSceneInfo::FindSceneInfoStructIndex(FabricIndex fabric, size_t endpointIndex, uint8_t & index)
{
    VerifyOrReturnError(endpointIndex < MATTER_ARRAY_SIZE(mSceneInfoStructs), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    index = 0;

    for (auto & info : mSceneInfoStructs[endpointIndex])
    {
        if (info.fabricIndex == fabric)
        {
            return CHIP_NO_ERROR;
        }
        index++;
    }

    return CHIP_ERROR_NOT_FOUND;
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

    ReturnErrorOnFailure(chip::app::CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    mGroupProvider = Credentials::GetGroupDataProvider();

    SceneTable * sceneTable = scenes::GetSceneTableImpl();
    ReturnErrorOnFailure(sceneTable->Init(&chip::Server::GetInstance().GetPersistentStorage()));
    ReturnErrorOnFailure(chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate));

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

void ScenesServer::Shutdown()
{
    chip::app::CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);

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
    if (req.transitionTime > scenes::kScenesMaxTransitionTime || req.sceneName.size() > scenes::kSceneNameMaxLength ||
        req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ConstraintError);
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

    uint32_t featureMap = 0;
    ReturnOnFailure(AddResponseOnError(ctx, response, Attributes::FeatureMap::Get(ctx.mRequestPath.mEndpointId, &featureMap)));

    SceneData storageData(CharSpan(), req.transitionTime);
    if (featureMap & to_underlying(Feature::kSceneNames))
    {
        storageData.SetName(req.sceneName);
    }

    auto fieldSetIter = req.extensionFieldSets.begin();
    uint8_t EFSCount  = 0;
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

    // Update FabricSceneInfo
    ReturnOnFailure(
        AddResponseOnError(ctx, response,
                           UpdateFabricSceneInfo(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                 Optional<GroupId>(), Optional<SceneId>(), Optional<bool>())));

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

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ConstraintError);
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
    response.transitionTime.SetValue(scene.mStorageData.mSceneTransitionTimeMs);

    response.sceneName.SetValue(CharSpan(scene.mStorageData.mName, scene.mStorageData.mNameLength));
    Span<Structs::ExtensionFieldSet::Type> responseEFSSpan(responseEFSBuffer, deserializedEFSCount);
    response.extensionFieldSets.SetValue(responseEFSSpan);

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

CHIP_ERROR StoreSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                           const SceneId & sceneID, GroupDataProvider * groupProvider)
{
    // Make the current fabric's SceneValid false before storing a scene
    ScenesServer::Instance().MakeSceneInvalid(endpointID, fabricIdx);

    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(StatusIB(Attributes::SceneTableSize::Get(endpointID, &endpointTableSize)).ToChipError());

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
        ReturnErrorOnFailure(StatusIB(Attributes::FeatureMap::Get(endpointID, &featureMap)).ToChipError());
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

    // Update SceneInfo Attribute
    ReturnErrorOnFailure(UpdateFabricSceneInfo(endpointID, fabricIdx, MakeOptional(groupID), MakeOptional(sceneID),
                                               MakeOptional(static_cast<bool>(true))));

    return CHIP_NO_ERROR;
}

CHIP_ERROR RecallSceneParse(const FabricIndex & fabricIdx, const EndpointId & endpointID, const GroupId & groupID,
                            const SceneId & sceneID, const Optional<DataModel::Nullable<uint32_t>> & transitionTime,
                            GroupDataProvider * groupProvider)
{
    // Make SceneValid false for all fabrics before recalling a scene
    ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpointID);

    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(StatusIB(Attributes::SceneTableSize::Get(endpointID, &endpointTableSize)).ToChipError());

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
            scene.mStorageData.mSceneTransitionTimeMs = transitionTime.Value().Value();
        }
    }

    ReturnErrorOnFailure(sceneTable->SceneApplyEFS(scene));

    // Update FabricSceneInfo, at this point the scene is considered valid
    ReturnErrorOnFailure(
        UpdateFabricSceneInfo(endpointID, fabricIdx, Optional<GroupId>(groupID), Optional<SceneId>(sceneID), Optional<bool>(true)));

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
    case Commands::CopyScene::Id:
        HandleCommand<Commands::CopyScene::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleCopyScene(ctx, req); });
        return;
    }
}

// AttributeAccessInterface
CHIP_ERROR ScenesServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    uint16_t endpointTableSize = 0;
    ReturnErrorOnFailure(StatusIB(Attributes::SceneTableSize::Get(aPath.mEndpointId, &endpointTableSize)).ToChipError());

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aPath.mEndpointId, endpointTableSize);

    switch (aPath.mAttributeId)
    {
    case Attributes::FabricSceneInfo::Id: {
        return aEncoder.EncodeList([&, sceneTable](const auto & encoder) -> CHIP_ERROR {
            Span<Structs::SceneInfoStruct::Type> fabricSceneInfoSpan = mFabricSceneInfo.GetFabricSceneInfo(aPath.mEndpointId);
            for (auto & info : fabricSceneInfoSpan)
            {
                // Update the SceneInfoStruct's Capacity in case it's capacity was limited by other fabrics
                sceneTable->GetRemainingCapacity(info.fabricIndex, info.remainingCapacity);
                ReturnErrorOnFailure(encoder.Encode(info));
            }
            return CHIP_NO_ERROR;
        });
    }
    default:
        return CHIP_NO_ERROR;
    }
}

Structs::SceneInfoStruct::Type * ScenesServer::GetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric)
{
    Structs::SceneInfoStruct::Type * sceneInfoStruct = mFabricSceneInfo.GetSceneInfoStruct(endpoint, fabric);
    return sceneInfoStruct;
}

CHIP_ERROR ScenesServer::SetSceneInfoStruct(EndpointId endpoint, FabricIndex fabric,
                                            Structs::SceneInfoStruct::Type & sceneInfoStruct)
{
    ReturnErrorOnFailure(mFabricSceneInfo.SetSceneInfoStruct(endpoint, fabric, sceneInfoStruct));
    return CHIP_NO_ERROR;
}

void ScenesServer::GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId)
{
    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);
    VerifyOrReturn(nullptr != sceneTable);

    Structs::SceneInfoStruct::Type * sceneInfo = mFabricSceneInfo.GetSceneInfoStruct(aEndpointId, aFabricIx);
    chip::GroupId currentGroup                 = (nullptr != sceneInfo) ? sceneInfo->currentGroup : 0x0000;

    // If currentGroup is what is being removed, we can't possibly still have a valid scene,
    // because the scene we have (if any) will also be removed.
    if (aGroupId == currentGroup)
    {
        MakeSceneInvalid(aEndpointId, aFabricIx);
    }

    VerifyOrReturn(nullptr != mGroupProvider);
    if (0 != aGroupId && !mGroupProvider->HasEndpoint(aFabricIx, aGroupId, aEndpointId))
    {
        return;
    }

    sceneTable->DeleteAllScenesInGroup(aFabricIx, aGroupId);
}

void ScenesServer::MakeSceneInvalid(EndpointId aEndpointId, FabricIndex aFabricIx)
{
    UpdateFabricSceneInfo(aEndpointId, aFabricIx, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>(false));
}

void ScenesServer::MakeSceneInvalidForAllFabrics(EndpointId aEndpointId)
{
    for (auto & info : chip::Server::GetInstance().GetFabricTable())
    {
        MakeSceneInvalid(aEndpointId, info.GetFabricIndex());
    }
}

void ScenesServer::StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    StoreSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, mGroupProvider);
}
void ScenesServer::RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId)
{
    Optional<DataModel::Nullable<uint32_t>> transitionTime;

    RecallSceneParse(aFabricIx, aEndpointId, aGroupId, aSceneId, transitionTime, mGroupProvider);
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

void ScenesServer::RemoveFabric(EndpointId aEndpointId, FabricIndex aFabricIndex)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(aEndpointId);
    sceneTable->RemoveFabric(aFabricIndex);
    mFabricSceneInfo.ClearSceneInfoStruct(aEndpointId, aFabricIndex);
}

void ScenesServer::HandleAddScene(HandlerContext & ctx, const Commands::AddScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("AddScene", "Scenes");
    AddSceneParse<Commands::AddScene::DecodableType, Commands::AddSceneResponse::Type>(ctx, req, mGroupProvider);
}

void ScenesServer::HandleViewScene(HandlerContext & ctx, const Commands::ViewScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("ViewScene", "Scenes");
    ViewSceneParse<Commands::ViewScene::DecodableType, Commands::ViewSceneResponse::Type>(ctx, req, mGroupProvider);
}

void ScenesServer::HandleRemoveScene(HandlerContext & ctx, const Commands::RemoveScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RemoveScene", "Scenes");
    Commands::RemoveSceneResponse::Type response;

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ConstraintError);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

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

    // Update SceneInfoStruct Attributes
    Structs::SceneInfoStruct::Type * sceneInfo =
        GetSceneInfoStruct(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex());
    Optional<bool> sceneValid;
    if (nullptr != sceneInfo && req.groupID == sceneInfo->currentGroup && req.sceneID == sceneInfo->currentScene)
    {
        sceneValid.Emplace(false);
    }

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));
    ReturnOnFailure(
        AddResponseOnError(ctx, response,
                           UpdateFabricSceneInfo(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                 Optional<GroupId>(), Optional<SceneId>(), sceneValid)));

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleRemoveAllScenes(HandlerContext & ctx, const Commands::RemoveAllScenes::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RemoveAllScenes", "Scenes");
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
    Structs::SceneInfoStruct::Type * sceneInfo =
        GetSceneInfoStruct(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex());

    Optional<bool> sceneValid;
    if (nullptr != sceneInfo && req.groupID == sceneInfo->currentGroup)
    {
        sceneValid.Emplace(false);
    }

    ReturnOnFailure(
        AddResponseOnError(ctx, response,
                           UpdateFabricSceneInfo(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                 Optional<GroupId>(), Optional<SceneId>(), sceneValid)));

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    // Write response
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleStoreScene(HandlerContext & ctx, const Commands::StoreScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("StoreScene", "Scenes");
    Commands::StoreSceneResponse::Type response;

    // Response data
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ConstraintError);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    CHIP_ERROR err = StoreSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), ctx.mRequestPath.mEndpointId, req.groupID,
                                     req.sceneID, mGroupProvider);

    ReturnOnFailure(AddResponseOnError(ctx, response, err));

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));
    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ScenesServer::HandleRecallScene(HandlerContext & ctx, const Commands::RecallScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RecallScene", "Scenes");

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    CHIP_ERROR err = RecallSceneParse(ctx.mCommandHandler.GetAccessingFabricIndex(), ctx.mRequestPath.mEndpointId, req.groupID,
                                      req.sceneID, req.transitionTime, mGroupProvider);

    if (CHIP_NO_ERROR == err)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
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
    MATTER_TRACE_SCOPE("GetSceneMembership", "Scenes");
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

void ScenesServer::HandleCopyScene(HandlerContext & ctx, const Commands::CopyScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("CopyScene", "Scenes");
    Commands::CopySceneResponse::Type response;

    uint16_t endpointTableSize = 0;
    ReturnOnFailure(
        AddResponseOnError(ctx, response, Attributes::SceneTableSize::Get(ctx.mRequestPath.mEndpointId, &endpointTableSize)));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(ctx.mRequestPath.mEndpointId, endpointTableSize);

    // Response data
    response.groupIdentifierFrom = req.groupIdentifierFrom;
    response.sceneIdentifierFrom = req.sceneIdentifierFrom;

    // Verify the attributes are respecting constraints
    if (req.sceneIdentifierFrom == scenes::kUndefinedSceneId || req.sceneIdentifierTo == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

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

    if (0 == capacity)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        return;
    }

    // Checks if we copy a single scene or all of them
    if (req.mode.GetField(app::Clusters::ScenesManagement::CopyModeBitmap::kCopyAllScenes))
    {
        // Scene Table interface data
        SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
        Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);

        // populate scene list
        ReturnOnFailure(AddResponseOnError(
            ctx, response,
            sceneTable->GetAllSceneIdsInGroup(ctx.mCommandHandler.GetAccessingFabricIndex(), req.groupIdentifierFrom, sceneList)));

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

            // Update SceneInfoStruct Attributes after each insert in case we hit max capacity in the middle of the loop
            ReturnOnFailure(AddResponseOnError(
                ctx, response,
                UpdateFabricSceneInfo(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                      Optional<GroupId>(), Optional<SceneId>(), Optional<bool>() /* = sceneValid*/)));
        }

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
    ReturnOnFailure(
        AddResponseOnError(ctx, response,
                           UpdateFabricSceneInfo(ctx.mRequestPath.mEndpointId, ctx.mCommandHandler.GetAccessingFabricIndex(),
                                                 Optional<GroupId>(), Optional<SceneId>(), Optional<bool>())));

    ReturnOnFailure(UpdateLastConfiguredBy(ctx, response));

    response.status = to_underlying(Protocols::InteractionModel::Status::Success);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ScenesManagement;

void emberAfScenesManagementClusterServerInitCallback(EndpointId endpoint)
{
    Status status = Attributes::LastConfiguredBy::SetNull(endpoint);
    if (Status::Success != status)
    {
        ChipLogDetail(Zcl, "ERR: setting LastConfiguredBy on Endpoint %hu Status: %x", endpoint, to_underlying(status));
    }

    // Initialize the FabricSceneInfo by getting the number of scenes and the remaining capacity for storing fabric scene data
    for (auto & info : chip::Server::GetInstance().GetFabricTable())
    {
        auto fabric = info.GetFabricIndex();
        UpdateFabricSceneInfo(endpoint, fabric, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>());
    }
}

void MatterScenesManagementClusterServerShutdownCallback(EndpointId endpoint)
{
    uint16_t endpointTableSize = 0;
    VerifyOrReturn(Status::Success == Attributes::SceneTableSize::Get(endpoint, &endpointTableSize));

    // Get Scene Table Instance
    SceneTable * sceneTable = scenes::GetSceneTableImpl(endpoint, endpointTableSize);
    sceneTable->RemoveEndpoint();
}

void MatterScenesManagementPluginServerInitCallback()
{
    CHIP_ERROR err = ScenesServer::Instance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ScenesServer::Instance().Init() error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
