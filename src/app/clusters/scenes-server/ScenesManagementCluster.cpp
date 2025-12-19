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

#include "ScenesManagementCluster.h"

#include <app/clusters/scenes-server/SceneTable.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ScenesManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

#include <optional>

using SceneTable        = chip::scenes::SceneTable<chip::scenes::ExtensionFieldSetsImpl>;
using SceneTableEntry   = SceneTable::SceneTableEntry;
using SceneStorageId    = SceneTable::SceneStorageId;
using SceneData         = SceneTable::SceneData;
using ExtensionFieldSet = chip::scenes::ExtensionFieldSet;
using GroupDataProvider = chip::Credentials::GroupDataProvider;
using AuthMode          = chip::Access::AuthMode;
using chip::Protocols::InteractionModel::Status;

using namespace chip::app::Clusters::ScenesManagement::Attributes;
using namespace chip::app::Clusters::ScenesManagement::Commands;
using namespace chip::app::Clusters::ScenesManagement::Structs;

namespace chip::app::Clusters {

namespace {

constexpr Protocols::InteractionModel::Status ResponseStatus(CHIP_ERROR err)
{
    // TODO : Properly fix mapping between error types (issue https://github.com/project-chip/connectedhomeip/issues/26885)
    if (CHIP_ERROR_NOT_FOUND == err)
    {
        return Status::NotFound;
    }
    if (CHIP_ERROR_NO_MEMORY == err)
    {
        return Status::ResourceExhausted;
    }
    if (CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute) == err)
    {
        // TODO: Confirm if we need to add UnsupportedAttribute status as a return for Scene Commands
        return Status::InvalidCommand;
    }
    return StatusIB(err).mStatus;
}

class ScopedSceneTable
{
public:
    ScopedSceneTable(ScenesManagementTableProvider & provider) : mProvider(provider), mTable(provider.Take()) {}
    ~ScopedSceneTable() { mProvider.Release(mTable); }

    SceneTable * operator->() { return mTable; }
    const SceneTable * operator->() const { return mTable; }

    operator bool() const { return mTable != nullptr; }

private:
    ScenesManagementTableProvider & mProvider;
    SceneTable * mTable;
};

/// A very common pattern of:
///   - if error (i.e. NOT CHIP_NO_ERROR), then set response status and return response
#define SuccessOrReturnWithFailureStatus(err_expr, response)                                                                       \
    if (CHIP_ERROR __err = err_expr; __err != CHIP_NO_ERROR)                                                                       \
    {                                                                                                                              \
        response.status = to_underlying(ResponseStatus(__err));                                                                    \
        return response;                                                                                                           \
    }

} // namespace

CHIP_ERROR ScenesManagementCluster::UpdateFabricSceneInfo(FabricIndex fabric, Optional<GroupId> group, Optional<SceneId> scene,
                                                          Optional<bool> sceneValid)
{
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    ScopedSceneTable sceneTable(mSceneTableProvider);
    VerifyOrReturnError(sceneTable, CHIP_ERROR_INTERNAL);
    SceneInfoStruct::Type * sceneInfo = GetSceneInfoStruct(fabric);
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
        SceneInfoStruct::Type newSceneInfo;
        newSceneInfo.fabricIndex = fabric;

        newSceneInfo.currentGroup = group.ValueOr(0);
        newSceneInfo.currentScene = scene.ValueOr(0);
        newSceneInfo.sceneValid   = sceneValid.ValueOr(false);

        ReturnErrorOnFailure(sceneTable->GetFabricSceneCount(fabric, newSceneInfo.sceneCount));
        ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(fabric, newSceneInfo.remainingCapacity));
        ReturnErrorOnFailure(SetSceneInfoStruct(fabric, newSceneInfo));
    }

    NotifyAttributeChanged(ScenesManagement::Attributes::FabricSceneInfo::Id);

    return CHIP_NO_ERROR;
}

SceneInfoStruct::Type * ScenesManagementCluster::FabricSceneInfo::GetSceneInfoStruct(FabricIndex fabric)
{
    uint8_t sceneInfoStructIndex = 0;
    VerifyOrReturnValue(CHIP_NO_ERROR == FindSceneInfoStructIndex(fabric, sceneInfoStructIndex), nullptr);
    return &mSceneInfoStructs[sceneInfoStructIndex];
}

/// @brief Sets the SceneInfoStruct for a specific fabric for a specific endpoint
/// @param endpoint target endpoint
/// @param fabric target fabric
/// @param [in] sceneInfoStruct SceneInfoStruct to set
/// @return CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND if the endpoint is not found, CHIP_ERROR_NO_MEMORY if the number of fabrics is
/// exceeded, CHIP_ERROR_INVALID_ARGUMENT if invalid fabric or endpoint
CHIP_ERROR ScenesManagementCluster::FabricSceneInfo::SetSceneInfoStruct(FabricIndex fabric,
                                                                        const SceneInfoStruct::Type & sceneInfoStruct)
{
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t sceneInfoStructIndex = 0;
    if (CHIP_ERROR_NOT_FOUND == FindSceneInfoStructIndex(fabric, sceneInfoStructIndex))
    {
        VerifyOrReturnError(mSceneInfoStructsCount < MATTER_ARRAY_SIZE(mSceneInfoStructs), CHIP_ERROR_NO_MEMORY);
        sceneInfoStructIndex = mSceneInfoStructsCount;
        mSceneInfoStructsCount++;
    }
    mSceneInfoStructs[sceneInfoStructIndex] = sceneInfoStruct;
    return CHIP_NO_ERROR;
}

void ScenesManagementCluster::FabricSceneInfo::ClearSceneInfoStruct(FabricIndex fabric)
{
    uint8_t sceneInfoStructIndex = 0;
    ReturnOnFailure(FindSceneInfoStructIndex(fabric, sceneInfoStructIndex));

    uint8_t nextIndex = static_cast<uint8_t>(sceneInfoStructIndex + 1);
    uint8_t moveNum   = static_cast<uint8_t>(MATTER_ARRAY_SIZE(mSceneInfoStructs) - nextIndex);
    // Compress the endpoint's SceneInfoStruct array
    if (moveNum)
    {
        for (size_t i = 0; i < moveNum; ++i)
        {
            mSceneInfoStructs[sceneInfoStructIndex + i] = mSceneInfoStructs[nextIndex + i];
        }
    }

    // Decrement the SceneInfoStruct count
    mSceneInfoStructsCount--;

    // Clear the last populated SceneInfoStruct
    mSceneInfoStructs[mSceneInfoStructsCount].fabricIndex       = kUndefinedFabricIndex;
    mSceneInfoStructs[mSceneInfoStructsCount].sceneCount        = 0;
    mSceneInfoStructs[mSceneInfoStructsCount].currentScene      = 0;
    mSceneInfoStructs[mSceneInfoStructsCount].currentGroup      = 0;
    mSceneInfoStructs[mSceneInfoStructsCount].remainingCapacity = 0;
}

CHIP_ERROR ScenesManagementCluster::FabricSceneInfo::FindSceneInfoStructIndex(FabricIndex fabric, uint8_t & index)
{
    VerifyOrReturnError(kUndefinedFabricIndex != fabric, CHIP_ERROR_INVALID_ARGUMENT);

    index = 0;

    for (auto & info : mSceneInfoStructs)
    {
        if (info.fabricIndex == fabric)
        {
            return CHIP_NO_ERROR;
        }
        index++;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ScenesManagementCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), {}, {});
}

CHIP_ERROR ScenesManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mSupportCopyScenes)
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        ReturnErrorOnFailure(builder.Append(CopyScene::kMetadataEntry));
    }

    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        AddScene::kMetadataEntry,           //
        ViewScene::kMetadataEntry,          //
        RemoveScene::kMetadataEntry,        //
        RemoveAllScenes::kMetadataEntry,    //
        StoreScene::kMetadataEntry,         //
        RecallScene::kMetadataEntry,        //
        GetSceneMembership::kMetadataEntry, //
    };

    return builder.ReferenceExisting(kCommands);
}

CHIP_ERROR ScenesManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (mSupportCopyScenes)
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        ReturnErrorOnFailure(builder.Append(CopySceneResponse::Id));
    }

    static constexpr CommandId kCommands[] = {
        AddSceneResponse::Id,           //
        ViewSceneResponse::Id,          //
        RemoveSceneResponse::Id,        //
        RemoveAllScenesResponse::Id,    //
        StoreSceneResponse::Id,         //
        GetSceneMembershipResponse::Id, //
    };

    return builder.ReferenceExisting(kCommands);
}

void ScenesManagementCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    ScopedSceneTable sceneTable(mSceneTableProvider);
    VerifyOrReturn(sceneTable);
    // The implementation of SceneTable::RemoveFabric() must not call back into the FabricTable
    TEMPORARY_RETURN_IGNORED sceneTable->RemoveFabric(fabricIndex);
}

CHIP_ERROR ScenesManagementCluster::StoreSceneParse(const FabricIndex & fabricIdx, const GroupId & groupID, const SceneId & sceneID)
{
    // Make the current fabric's SceneValid false before storing a scene
    ReturnErrorOnFailure(MakeSceneInvalid(fabricIdx));

    ScopedSceneTable sceneTable(mSceneTableProvider);

    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != mGroupProvider, CHIP_ERROR_INTERNAL);
    if (0 != groupID && !mGroupProvider->HasEndpoint(fabricIdx, groupID, mPath.mEndpointId))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(sceneTable, CHIP_ERROR_INTERNAL);
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
        // Check if we still support scenes name in case an OTA changed that, if we don't, set name to empty
        if (!mFeatures.Has(ScenesManagement::Feature::kSceneNames))
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
    return UpdateFabricSceneInfo(fabricIdx, MakeOptional(groupID), MakeOptional(sceneID), MakeOptional(static_cast<bool>(true)));
}

CHIP_ERROR ScenesManagementCluster::MakeSceneInvalidForAllFabrics()
{
    for (auto & info : *mFabricTable)
    {
        ReturnErrorOnFailure(MakeSceneInvalid(info.GetFabricIndex()));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ScenesManagementCluster::RecallSceneParse(const FabricIndex & fabricIdx, const GroupId & groupID,
                                                     const SceneId & sceneID,
                                                     const Optional<DataModel::Nullable<uint32_t>> & transitionTime)
{
    // Make SceneValid false for all fabrics before recalling a scene
    ReturnErrorOnFailure(MakeSceneInvalidForAllFabrics());

    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);

    // Verify Endpoint in group
    VerifyOrReturnError(nullptr != mGroupProvider, CHIP_ERROR_INTERNAL);
    if (0 != groupID && !mGroupProvider->HasEndpoint(fabricIdx, groupID, mPath.mEndpointId))
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(sceneID, groupID));

    VerifyOrReturnError(sceneTable, CHIP_ERROR_INTERNAL);
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
    return UpdateFabricSceneInfo(fabricIdx, Optional<GroupId>(groupID), Optional<SceneId>(sceneID), Optional<bool>(true));
}

std::optional<DataModel::ActionReturnStatus> ScenesManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                    chip::TLV::TLVReader & input_arguments,
                                                                                    CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case AddScene::Id: {
        AddScene::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path, HandleAddScene(handler->GetAccessingFabricIndex(), request_data));
        return std::nullopt;
    }
    case ViewScene::Id: {
        ViewScene::DecodableType request_data;
        SceneTableEntry scene;
        std::array<ExtensionFieldSetStruct::Type, scenes::kMaxClustersPerScene> responseEFSBuffer;

        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path,
                             HandleViewScene(handler->GetAccessingFabricIndex(), request_data, scene, responseEFSBuffer));
        return std::nullopt;
    }
    case RemoveScene::Id: {
        RemoveScene::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path, HandleRemoveScene(handler->GetAccessingFabricIndex(), request_data));
        return std::nullopt;
    }
    case RemoveAllScenes::Id: {
        RemoveAllScenes::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path, HandleRemoveAllScenes(handler->GetAccessingFabricIndex(), request_data));
        return std::nullopt;
    }
    case StoreScene::Id: {
        StoreScene::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path, HandleStoreScene(handler->GetAccessingFabricIndex(), request_data));
        return std::nullopt;
    }
    case RecallScene::Id: {
        RecallScene::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        return HandleRecallScene(handler->GetAccessingFabricIndex(), request_data);
    }
    case GetSceneMembership::Id: {
        GetSceneMembership::DecodableType request_data;
        std::array<SceneId, scenes::kMaxScenesPerFabric> scenesInGroup;

        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path,
                             HandleGetSceneMembership(handler->GetAccessingFabricIndex(), request_data, scenesInGroup));
        return std::nullopt;
    }
    case CopyScene::Id: {
        CopyScene::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, request.GetAccessingFabricIndex()));
        handler->AddResponse(request.path, HandleCopyScene(handler->GetAccessingFabricIndex(), request_data));
        return std::nullopt;
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ScenesManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(ScenesManagement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case SceneTableSize::Id: {
        ScopedSceneTable sceneTable(mSceneTableProvider);
        VerifyOrReturnError(sceneTable, Status::Failure);
        return encoder.Encode(sceneTable->GetTableSize());
    }
    case ScenesManagement::Attributes::FabricSceneInfo::Id: {
        ScopedSceneTable sceneTable(mSceneTableProvider);
        return encoder.EncodeList([&](const auto & listEncoder) -> CHIP_ERROR {
            Span<SceneInfoStruct::Type> fabricSceneInfoSpan = mFabricSceneInfo.GetFabricSceneInfo();
            for (auto & info : fabricSceneInfoSpan)
            {
                // Update the SceneInfoStruct's Capacity in case it's capacity was limited by other fabrics
                ReturnErrorOnFailure(sceneTable->GetRemainingCapacity(info.fabricIndex, info.remainingCapacity));
                ReturnErrorOnFailure(listEncoder.Encode(info));
            }
            return CHIP_NO_ERROR;
        });
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ScenesManagementCluster::Startup(ServerClusterContext & context)
{
    ScopedSceneTable sceneTable(mSceneTableProvider);

    // NOTE: this re-sets the storage delegate and provider on a SHARED GLOBAL member.
    //       Generally safe(the same values should be used within an entire cluster) but will not work well
    //       if multiple stacks work in parallel.
    ReturnErrorOnFailure(sceneTable->Init(context.storage, context.provider));
    ReturnErrorOnFailure(mFabricTable->AddFabricDelegate(this));

    for (const FabricInfo & info : *mFabricTable)
    {
        FabricIndex fabric = info.GetFabricIndex();
        LogErrorOnFailure(UpdateFabricSceneInfo(fabric, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>()));
    }

    return DefaultServerCluster::Startup(context);
}

CHIP_ERROR ScenesManagementCluster::ClearPersistentData()
{
    // For the persistent storage to be likely correct, we enfoce cluster to be started up
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ScopedSceneTable sceneTable(mSceneTableProvider);
    VerifyOrReturnError(sceneTable, CHIP_ERROR_INCORRECT_STATE);
    return sceneTable->RemoveEndpoint();
}

void ScenesManagementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mFabricTable->RemoveFabricDelegate(this);

    if (shutdownType == ClusterShutdownType::kPermanentRemove)
    {
        LogErrorOnFailure(ClearPersistentData());
    }

    DefaultServerCluster::Shutdown(shutdownType);
}

CHIP_ERROR ScenesManagementCluster::GroupWillBeRemoved(FabricIndex aFabricIdx, GroupId aGroupId)
{
    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);
    VerifyOrReturnError(sceneTable, CHIP_ERROR_INTERNAL);

    SceneInfoStruct::Type * sceneInfo = mFabricSceneInfo.GetSceneInfoStruct(aFabricIdx);
    chip::GroupId currentGroup        = (nullptr != sceneInfo) ? sceneInfo->currentGroup : 0x0000;

    // If currentGroup is what is being removed, we can't possibly still have a valid scene,
    // because the scene we have (if any) will also be removed.
    if (aGroupId == currentGroup)
    {
        ReturnErrorOnFailure(MakeSceneInvalid(aFabricIdx));
    }

    VerifyOrReturnError(nullptr != mGroupProvider, CHIP_ERROR_INCORRECT_STATE);
    if (0 != aGroupId && !mGroupProvider->HasEndpoint(aFabricIdx, aGroupId, mPath.mEndpointId))
    {
        return CHIP_NO_ERROR;
    }

    return sceneTable->DeleteAllScenesInGroup(aFabricIdx, aGroupId);
}

CHIP_ERROR ScenesManagementCluster::MakeSceneInvalid(FabricIndex aFabricIdx)
{
    return UpdateFabricSceneInfo(aFabricIdx, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>(false));
}

CHIP_ERROR ScenesManagementCluster::StoreCurrentScene(FabricIndex aFabricIx, GroupId aGroupId, SceneId aSceneId)
{
    return StoreSceneParse(aFabricIx, aGroupId, aSceneId);
}

CHIP_ERROR ScenesManagementCluster::RecallScene(FabricIndex aFabricIx, GroupId aGroupId, SceneId aSceneId)
{
    Optional<DataModel::Nullable<uint32_t>> transitionTime;
    return RecallSceneParse(aFabricIx, aGroupId, aSceneId, transitionTime);
}

CHIP_ERROR ScenesManagementCluster::RemoveFabric(FabricIndex aFabricIndex)
{
    ScopedSceneTable sceneTable(mSceneTableProvider);
    VerifyOrReturnError(sceneTable, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(sceneTable->RemoveFabric(aFabricIndex));
    mFabricSceneInfo.ClearSceneInfoStruct(aFabricIndex);
    return CHIP_NO_ERROR;
}

AddSceneResponse::Type ScenesManagementCluster::HandleAddScene(FabricIndex fabricIndex,
                                                               const ScenesManagement::Commands::AddScene::DecodableType & req)
{
    AddSceneResponse::Type response;
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Verify the attributes are respecting constraints
    if (req.transitionTime > scenes::kScenesMaxTransitionTime || req.sceneName.size() > scenes::kSceneNameMaxLength ||
        req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Status::ConstraintError);
        return response;
    }

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::UnsupportedCommand);
        return response;
    }

    if (0 != req.groupID && !mGroupProvider->HasEndpoint(fabricIndex, req.groupID, mPath.mEndpointId))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    SceneData storageData(CharSpan(), req.transitionTime);
    if (mFeatures.Has(ScenesManagement::Feature::kSceneNames))
    {
        storageData.SetName(req.sceneName);
    }

    auto fieldSetIter = req.extensionFieldSetStructs.begin();
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
            if (handler.SupportsCluster(mPath.mEndpointId, tempEFS.mID))
            {
                SuccessOrReturnWithFailureStatus(handler.SerializeAdd(mPath.mEndpointId, fieldSetIter.GetValue(), buff_span),
                                                 response);
                break;
            }
        }

        static_assert(sizeof(tempEFS.mBytesBuffer) <= UINT8_MAX, "Serialized EFS number of bytes must fit in a uint8");
        tempEFS.mUsedBytes = static_cast<uint8_t>(buff_span.size());

        if (!tempEFS.IsEmpty())
        {
            TEMPORARY_RETURN_IGNORED storageData.mExtensionFieldSets.InsertFieldSet(tempEFS);
        }
    }
    SuccessOrReturnWithFailureStatus(fieldSetIter.GetStatus(), response);

    // Create scene from data and ID
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID), storageData);

    // Get Capacity
    uint8_t capacity = 0;
    SuccessOrReturnWithFailureStatus(sceneTable->GetRemainingCapacity(fabricIndex, capacity), response);

    if (0 == capacity)
    {
        response.status = to_underlying(Status::ResourceExhausted);
        return response;
    }

    //  Insert in table
    SuccessOrReturnWithFailureStatus(sceneTable->SetSceneTableEntry(fabricIndex, scene), response);

    // Update FabricSceneInfo
    SuccessOrReturnWithFailureStatus(UpdateFabricSceneInfo(fabricIndex, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>()),
                                     response);

    // Write response
    response.status = to_underlying(Status::Success);
    return response;
}

ViewSceneResponse::Type ScenesManagementCluster::HandleViewScene(
    FabricIndex fabricIndex, const ScenesManagement::Commands::ViewScene::DecodableType & req, SceneTableEntry & scene,
    std::array<ScenesManagement::Structs::ExtensionFieldSetStruct::Type, scenes::kMaxClustersPerScene> & responseEFSBuffer)
{
    MATTER_TRACE_SCOPE("ViewScene", "Scenes");

    ViewSceneResponse::Type response;
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Status::ConstraintError);
        return response;
    }

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    if (0 != req.groupID && !mGroupProvider->HasEndpoint(fabricIndex, req.groupID, mPath.mEndpointId))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    // Gets the scene from the table
    SuccessOrReturnWithFailureStatus(sceneTable->GetSceneTableEntry(fabricIndex, SceneStorageId(req.sceneID, req.groupID), scene),
                                     response);

    // Response Extension Field Sets buffer
    uint8_t deserializedEFSCount = 0;

    // Adds extension field sets to the scene
    for (uint8_t i = 0; i < scene.mStorageData.mExtensionFieldSets.GetFieldSetCount(); i++)
    {
        // gets data from the field in the scene
        ExtensionFieldSet tempField;
        TEMPORARY_RETURN_IGNORED scene.mStorageData.mExtensionFieldSets.GetFieldSetAtPosition(tempField, i);
        ByteSpan efsSpan(tempField.mBytesBuffer, tempField.mUsedBytes);

        // This should only find one handle per cluster
        for (auto & handler : sceneTable->mHandlerList)
        {
            if (handler.SupportsCluster(mPath.mEndpointId, tempField.mID))
            {
                SuccessOrReturnWithFailureStatus(
                    handler.Deserialize(mPath.mEndpointId, tempField.mID, efsSpan, responseEFSBuffer[i]), response);
                deserializedEFSCount++;
                break;
            }
        }
    }

    response.status = to_underlying(Status::Success);
    response.transitionTime.SetValue(scene.mStorageData.mSceneTransitionTimeMs);
    response.sceneName.SetValue({ scene.mStorageData.mName, scene.mStorageData.mNameLength });
    response.extensionFieldSetStructs.SetValue({ responseEFSBuffer.data(), deserializedEFSCount });

    return response;
}

RemoveSceneResponse::Type
ScenesManagementCluster::HandleRemoveScene(FabricIndex fabricIndex,
                                           const ScenesManagement::Commands::RemoveScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RemoveScene", "Scenes");
    // Write response
    RemoveSceneResponse::Type response;

    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Status::ConstraintError);
        return response;
    }

    // Scene Table interface data
    SceneTableEntry scene(SceneStorageId(req.sceneID, req.groupID));

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }
    if (0 != req.groupID && !mGroupProvider->HasEndpoint(fabricIndex, req.groupID, mPath.mEndpointId))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    //  Gets the scene from the table
    SuccessOrReturnWithFailureStatus(sceneTable->GetSceneTableEntry(fabricIndex, scene.mStorageId, scene), response);

    // Remove the scene from the scene table
    SuccessOrReturnWithFailureStatus(sceneTable->RemoveSceneTableEntry(fabricIndex, scene.mStorageId), response);

    // Update SceneInfoStruct Attributes
    SceneInfoStruct::Type * sceneInfo = GetSceneInfoStruct(fabricIndex);
    Optional<bool> sceneValid;
    if (nullptr != sceneInfo && req.groupID == sceneInfo->currentGroup && req.sceneID == sceneInfo->currentScene)
    {
        sceneValid.Emplace(false);
    }

    SuccessOrReturnWithFailureStatus(UpdateFabricSceneInfo(fabricIndex, Optional<GroupId>(), Optional<SceneId>(), sceneValid),
                                     response);

    response.status = to_underlying(Status::Success);
    return response;
}

RemoveAllScenesResponse::Type
ScenesManagementCluster::HandleRemoveAllScenes(FabricIndex fabricIndex,
                                               const ScenesManagement::Commands::RemoveAllScenes::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RemoveAllScenes", "Scenes");
    // Response data
    RemoveAllScenesResponse::Type response;
    response.groupID = req.groupID;

    // Get Scene Table Instance
    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }
    if (0 != req.groupID && !mGroupProvider->HasEndpoint(fabricIndex, req.groupID, mPath.mEndpointId))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    SuccessOrReturnWithFailureStatus(sceneTable->DeleteAllScenesInGroup(fabricIndex, req.groupID), response);

    // Update Attributes
    SceneInfoStruct::Type * sceneInfo = GetSceneInfoStruct(fabricIndex);

    Optional<bool> sceneValid;
    if (nullptr != sceneInfo && req.groupID == sceneInfo->currentGroup)
    {
        sceneValid.Emplace(false);
    }

    SuccessOrReturnWithFailureStatus(UpdateFabricSceneInfo(fabricIndex, Optional<GroupId>(), Optional<SceneId>(), sceneValid),
                                     response);

    response.status = to_underlying(Status::Success);
    return response;
}

ScenesManagement::Commands::StoreSceneResponse::Type
ScenesManagementCluster::HandleStoreScene(FabricIndex fabricIndex,
                                          const ScenesManagement::Commands::StoreScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("StoreScene", "Scenes");
    StoreSceneResponse::Type response;
    response.groupID = req.groupID;
    response.sceneID = req.sceneID;

    // Verify the attributes are respecting constraints
    if (req.sceneID == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Status::ConstraintError);
        return response;
    }

    SuccessOrReturnWithFailureStatus(StoreSceneParse(fabricIndex, req.groupID, req.sceneID), response);

    response.status = to_underlying(Status::Success);
    return response;
}

Protocols::InteractionModel::Status
ScenesManagementCluster::HandleRecallScene(FabricIndex fabricIndex,
                                           const ScenesManagement::Commands::RecallScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("RecallScene", "Scenes");

    // Verify the attributes are respecting constraints
    VerifyOrReturnError(req.sceneID != scenes::kUndefinedSceneId, Status::ConstraintError);

    CHIP_ERROR err = RecallSceneParse(fabricIndex, req.groupID, req.sceneID, req.transitionTime);

    // TODO : implement proper mapping between CHIP_ERROR and IM Status
    VerifyOrReturnValue(CHIP_NO_ERROR != err, Status::Success);
    VerifyOrReturnValue(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);

    return StatusIB(err).mStatus;
}

ScenesManagement::Commands::GetSceneMembershipResponse::Type
ScenesManagementCluster::HandleGetSceneMembership(FabricIndex fabricIndex,
                                                  const ScenesManagement::Commands::GetSceneMembership::DecodableType & req,
                                                  std::array<SceneId, scenes::kMaxScenesPerFabric> & scenesInGroup)
{
    MATTER_TRACE_SCOPE("GetSceneMembership", "Scenes");
    GetSceneMembershipResponse::Type response;
    response.groupID = req.groupID;

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    if (0 != req.groupID && !mGroupProvider->HasEndpoint(fabricIndex, req.groupID, mPath.mEndpointId))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    uint8_t capacity = 0;
    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Get Capacity
    SuccessOrReturnWithFailureStatus(sceneTable->GetRemainingCapacity(fabricIndex, capacity), response);
    response.capacity.SetNonNull(capacity);

    // populate scene list
    auto sceneList = Span<SceneId>(scenesInGroup);
    SuccessOrReturnWithFailureStatus(sceneTable->GetAllSceneIdsInGroup(fabricIndex, req.groupID, sceneList), response);

    response.sceneList.SetValue(sceneList);

    // Write response
    response.status = to_underlying(Status::Success);
    return response;
}

ScenesManagement::Commands::CopySceneResponse::Type
ScenesManagementCluster::HandleCopyScene(FabricIndex fabricIndex, const ScenesManagement::Commands::CopyScene::DecodableType & req)
{
    MATTER_TRACE_SCOPE("CopyScene", "Scenes");
    CopySceneResponse::Type response;
    response.groupIdentifierFrom = req.groupIdentifierFrom;
    response.sceneIdentifierFrom = req.sceneIdentifierFrom;

    // Verify the attributes are respecting constraints
    if (req.sceneIdentifierFrom == scenes::kUndefinedSceneId || req.sceneIdentifierTo == scenes::kUndefinedSceneId)
    {
        response.status = to_underlying(Status::ConstraintError);
        return response;
    }

    // Verify Endpoint in group
    if (nullptr == mGroupProvider)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    if ((0 != req.groupIdentifierFrom && !mGroupProvider->HasEndpoint(fabricIndex, req.groupIdentifierFrom, mPath.mEndpointId)) ||
        (0 != req.groupIdentifierTo && !mGroupProvider->HasEndpoint(fabricIndex, req.groupIdentifierTo, mPath.mEndpointId)))
    {
        response.status = to_underlying(Status::InvalidCommand);
        return response;
    }

    ScopedSceneTable sceneTable(mSceneTableProvider);
    if (!sceneTable)
    {
        response.status = to_underlying(Status::Failure);
        return response;
    }

    // Copying a scene over an existing one should be ok. At this point we:
    //  - check if we overwrite a scene (then capacity is ok)
    //  - adding a new slot (in this case we have to check cpacity)
    // Check if the destination scene already exists
    SceneTableEntry destScene(SceneStorageId(req.sceneIdentifierTo, req.groupIdentifierTo));
    CHIP_ERROR err = sceneTable->GetSceneTableEntry(fabricIndex, destScene.mStorageId, destScene);

    if (err == CHIP_ERROR_NOT_FOUND)
    {
        // Only check capacity if the destination scene does not exist, as an overwrite doesn't consume a new slot.
        uint8_t capacity = 0;
        SuccessOrReturnWithFailureStatus(sceneTable->GetRemainingCapacity(fabricIndex, capacity), response);
        if (0 == capacity)
        {
            response.status = to_underlying(Status::ResourceExhausted);
            return response;
        }
    }
    else if (err != CHIP_NO_ERROR)
    {
        // Some other error occurred when trying to check if the scene exists.
        response.status = to_underlying(ResponseStatus(err));
        return response;
    }

    // Checks if we copy a single scene or all of them
    if (req.mode.GetField(app::Clusters::ScenesManagement::CopyModeBitmap::kCopyAllScenes))
    {
        // Scene Table interface data
        SceneId scenesInGroup[scenes::kMaxScenesPerFabric];
        Span<SceneId> sceneList = Span<SceneId>(scenesInGroup);

        // populate scene list
        SuccessOrReturnWithFailureStatus(sceneTable->GetAllSceneIdsInGroup(fabricIndex, req.groupIdentifierFrom, sceneList),
                                         response);

        for (auto & sceneId : sceneList)
        {
            SceneTableEntry scene(SceneStorageId(sceneId, req.groupIdentifierFrom));
            //  Insert in table
            SuccessOrReturnWithFailureStatus(sceneTable->GetSceneTableEntry(fabricIndex, scene.mStorageId, scene), response);

            scene.mStorageId = SceneStorageId(sceneId, req.groupIdentifierTo);

            SuccessOrReturnWithFailureStatus(sceneTable->SetSceneTableEntry(fabricIndex, scene), response);

            // Update SceneInfoStruct Attributes after each insert in case we hit max capacity in the middle of the loop
            SuccessOrReturnWithFailureStatus(
                UpdateFabricSceneInfo(fabricIndex, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>() /* = sceneValid*/),
                response);
        }

        response.status = to_underlying(Status::Success);
        return response;
    }

    SceneTableEntry scene(SceneStorageId(req.sceneIdentifierFrom, req.groupIdentifierFrom));
    SuccessOrReturnWithFailureStatus(sceneTable->GetSceneTableEntry(fabricIndex, scene.mStorageId, scene), response);

    scene.mStorageId = SceneStorageId(req.sceneIdentifierTo, req.groupIdentifierTo);

    SuccessOrReturnWithFailureStatus(sceneTable->SetSceneTableEntry(fabricIndex, scene), response);

    // Update Attributes
    SuccessOrReturnWithFailureStatus(UpdateFabricSceneInfo(fabricIndex, Optional<GroupId>(), Optional<SceneId>(), Optional<bool>()),
                                     response);
    response.status = to_underlying(Status::Success);
    return response;
}

} // namespace chip::app::Clusters
