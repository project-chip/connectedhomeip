/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <stdlib.h>

using namespace chip;
using namespace chip::scenes;
using namespace chip::app::Storage;

using SceneTableEntry = DefaultSceneTableImpl::SceneTableEntry;
using SceneStorageId  = DefaultSceneTableImpl::SceneStorageId;
using SceneData       = DefaultSceneTableImpl::SceneData;
using Serializer      = DefaultSerializer<SceneStorageId, SceneData>;

template <>
StorageKeyName Serializer::EndpointEntryCountKey(EndpointId endpoint_id)
{
    return DefaultStorageKeyAllocator::EndpointSceneCountKey(endpoint_id);
}

template <>
StorageKeyName Serializer::FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx)
{
    return DefaultStorageKeyAllocator::FabricSceneKey(fabric, endpoint, idx);
}

template <>
StorageKeyName Serializer::FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint)
{
    return DefaultStorageKeyAllocator::FabricSceneDataKey(fabric, endpoint);
}

// Worst case tested: Add Scene Command with EFS using the default SerializeAdd Method. This yielded a serialized scene of 175 bytes
// when using the OnOff, Level Control and Color Control as well as the maximal name length of 16 bytes. Putting 256 gives some
// slack in case different clusters are used. Value obtained by using writer.GetLengthWritten at the end of the SceneTableData
// Serialize method.
template <>
constexpr size_t Serializer::kEntryMaxBytes()
{
    return CHIP_CONFIG_SCENES_MAX_SERIALIZED_SCENE_SIZE_BYTES;
}

template <>
constexpr uint16_t Serializer::kMaxPerFabric()
{
    return kMaxScenesPerFabric;
}

template <>
constexpr uint16_t Serializer::kMaxPerEndpoint()
{
    return kMaxScenesPerEndpoint;
}

// A Full fabric serialized TLV length is 88 bytes, 128 bytes gives some slack.  Tested by running writer.GetLengthWritten at the
// end of the Serialize method of FabricSceneData
template <>
constexpr size_t Serializer::kFabricMaxBytes()
{
    return 128;
}

#include <app/storage/FabricTableImpl.ipp>

namespace {
/// @brief Tags Used to serialize Scenes so they can be stored in flash memory.
/// kGroupId: Tag for GroupID if the Scene is a Group Scene
/// kSceneId: Tag for the scene ID. Together with kGroupId, forms the SceneStorageId
/// kName: Tag for the name of the scene
/// kTransitionTime: Tag for the transition time of the scene in miliseconds
enum class TagScene : uint8_t
{
    kGroupId = static_cast<uint8_t>(TagEntry::kFabricTableFirstSpecializationReservedTag),
    kSceneId,
    kName,
    kTransitionTimeMs,
};
} // namespace

using SceneTableData = TableEntryData<SceneStorageId, SceneData, Serializer::kEntryMaxBytes()>;
using FabricSceneData =
    FabricEntryData<SceneStorageId, SceneData, Serializer::kEntryMaxBytes(), Serializer::kFabricMaxBytes(), kMaxScenesPerFabric>;

template class chip::app::Storage::FabricTableImpl<SceneTableBase::SceneStorageId, SceneTableBase::SceneData, kIteratorsMax>;

CHIP_ERROR DefaultSceneTableImpl::Init(PersistentStorageDelegate * storage)
{
    return FabricTableImpl::Init(storage);
}

void DefaultSceneTableImpl::Finish()
{
    UnregisterAllHandlers();
    FabricTableImpl::Finish();
}

CHIP_ERROR DefaultSceneTableImpl::GetFabricSceneCount(FabricIndex fabric_index, uint8_t & scene_count)
{
    return this->GetFabricEntryCount(fabric_index, scene_count);
}

CHIP_ERROR DefaultSceneTableImpl::GetEndpointSceneCount(uint8_t & scene_count)
{
    return this->GetEndpointEntryCount(scene_count);
}

CHIP_ERROR DefaultSceneTableImpl::SetEndpointSceneCount(const uint8_t & scene_count)
{
    return this->SetEndpointEntryCount(scene_count);
}

CHIP_ERROR DefaultSceneTableImpl::GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity)
{
    return FabricTableImpl::GetRemainingCapacity(fabric_index, capacity);
}

CHIP_ERROR DefaultSceneTableImpl::SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)
{
    return this->SetTableEntry(fabric_index, entry);
}

CHIP_ERROR DefaultSceneTableImpl::GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry)
{
    return this->GetTableEntry(fabric_index, scene_id, entry);
}

CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)
{
    return this->RemoveTableEntry(fabric_index, scene_id);
}

CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index,
                                                                  SceneIndex scene_idx)
{
    return this->RemoveTableEntryAtPosition(endpoint, fabric_index, scene_idx);
}

CHIP_ERROR DefaultSceneTableImpl::GetAllSceneIdsInGroup(FabricIndex fabric_index, GroupId group_id, Span<SceneId> & scene_list)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);
    SceneTableData scene(mEndpointId, fabric_index);

    uint8_t scene_count = 0;
    CHIP_ERROR err      = fabric.Load(this->mStorage);
    if (CHIP_ERROR_NOT_FOUND != err)
    {
        ReturnErrorOnFailure(err);
        SceneId * list = scene_list.data();

        for (uint16_t i = 0; i < mMaxPerFabric; i++)
        {
            if (fabric.entry_map[i].mGroupId != group_id)
            {
                continue;
            }

            if (scene_count >= scene_list.size())
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            list[scene_count] = fabric.entry_map[i].mSceneId;
            scene_count++;
        }
    }

    scene_list.reduce_size(scene_count);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::DeleteAllScenesInGroup(FabricIndex fabric_index, GroupId group_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(mEndpointId, fabric_index, mMaxPerFabric, mMaxPerEndpoint);
    SceneTableData scene(mEndpointId, fabric_index);

    CHIP_ERROR err = fabric.Load(this->mStorage);
    VerifyOrReturnValue(CHIP_ERROR_NOT_FOUND != err, CHIP_NO_ERROR);
    ReturnErrorOnFailure(err);

    for (uint16_t i = 0; i < mMaxPerFabric; i++)
    {
        if (fabric.entry_map[i].mGroupId == group_id)
        {
            // Removing each scene from the nvm and clearing their entry in the scene map
            ReturnErrorOnFailure(fabric.RemoveEntry(mStorage, fabric.entry_map[i]));
        }
    }

    return CHIP_NO_ERROR;
}

/// @brief Register a handler in the handler linked list
/// @param handler Cluster specific handler for extension field sets interaction
void DefaultSceneTableImpl::RegisterHandler(SceneHandler * handler)
{
    mHandlerList.PushFront(handler);
}

void DefaultSceneTableImpl::UnregisterHandler(SceneHandler * handler)
{
    // Verify list is populated and handler is not null
    VerifyOrReturn(!HandlerListEmpty() && !(handler == nullptr));
    mHandlerList.Remove(handler);
}

void DefaultSceneTableImpl::UnregisterAllHandlers()
{
    while (!mHandlerList.Empty())
    {
        IntrusiveList<SceneHandler>::Iterator foo = mHandlerList.begin();
        SceneHandler * handle                     = &(*foo);
        mHandlerList.Remove(handle);
    }
}

/// @brief Gets the field sets for the clusters implemented on a specific endpoint and store them in an EFS (extension field set).
/// Does so by going through the SceneHandler list and calling the first handler the list find for each specific clusters.
/// @param scene Scene in which the EFS gets populated
CHIP_ERROR DefaultSceneTableImpl::SceneSaveEFS(SceneTableEntry & scene)
{
    if (!HandlerListEmpty())
    {
        // TODO : Once zap supports the scenable quality, implement a GetSceneableClusterCountFromEndpointType function to avoid
        // over-allocation
        uint8_t clusterCount = GetClusterCountFromEndpoint();
        chip::Platform::ScopedMemoryBuffer<ClusterId> cBuffer;
        VerifyOrReturnError(cBuffer.Calloc(clusterCount), CHIP_ERROR_NO_MEMORY);
        clusterCount = GetClustersFromEndpoint(cBuffer.Get(), clusterCount);

        Span<ClusterId> cSpan(cBuffer.Get(), clusterCount);
        for (ClusterId cluster : cSpan)
        {
            ExtensionFieldSet EFS;
            MutableByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, kMaxFieldBytesPerCluster);
            EFS.mID                 = cluster;

            for (auto & handler : mHandlerList)
            {
                if (handler.SupportsCluster(mEndpointId, cluster))
                {
                    ReturnErrorOnFailure(handler.SerializeSave(mEndpointId, EFS.mID, EFSSpan));
                    EFS.mUsedBytes = static_cast<uint8_t>(EFSSpan.size());
                    ReturnErrorOnFailure(scene.mStorageData.mExtensionFieldSets.InsertFieldSet(EFS));
                    break;
                }
            }
        }
    }

    return CHIP_NO_ERROR;
}

/// @brief Retrieves the values of extension field sets on a scene and applies them to each cluster on the endpoint of the scene.
/// Does so by iterating through mHandlerList for each cluster in the EFS and calling the FIRST handler found that supports the
/// cluster. Does so by going through the SceneHandler list and calling the first handler the list find for each specific clusters.
/// @param scene Scene providing the EFSs (extension field sets)
CHIP_ERROR DefaultSceneTableImpl::SceneApplyEFS(const SceneTableEntry & scene)
{
    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < scene.mStorageData.mExtensionFieldSets.GetFieldSetCount(); i++)
        {
            ExtensionFieldSet EFS;
            scene.mStorageData.mExtensionFieldSets.GetFieldSetAtPosition(EFS, i);
            ByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, EFS.mUsedBytes);

            if (!EFS.IsEmpty())
            {
                for (auto & handler : mHandlerList)
                {
                    if (handler.SupportsCluster(mEndpointId, EFS.mID))
                    {
                        ReturnErrorOnFailure(
                            handler.ApplyScene(mEndpointId, EFS.mID, EFSSpan, scene.mStorageData.mSceneTransitionTimeMs));
                        break;
                    }
                }
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::RemoveFabric(FabricIndex fabric_index)
{
    return FabricTableImpl::RemoveFabric(fabric_index);
}

CHIP_ERROR DefaultSceneTableImpl::RemoveEndpoint()
{
    return FabricTableImpl::RemoveEndpoint();
}

/// @brief wrapper function around emberAfGetClustersFromEndpoint to allow testing, shimmed in test configuration because
/// emberAfGetClusterFromEndpoint relies on <app/util/attribute-storage.h>, which relies on zap generated files
uint8_t DefaultSceneTableImpl::GetClustersFromEndpoint(ClusterId * clusterList, uint8_t listLen)
{
    return emberAfGetClustersFromEndpoint(mEndpointId, clusterList, listLen, true);
}

/// @brief wrapper function around emberAfGetClusterCountForEndpoint to allow testing enforcing a specific count, shimmed in test
/// configuration because emberAfGetClusterCountForEndpoint relies on <app/util/attribute-storage.h>, which relies on zap generated
/// files
uint8_t DefaultSceneTableImpl::GetClusterCountFromEndpoint()
{
    return emberAfGetClusterCountForEndpoint(mEndpointId);
}

void DefaultSceneTableImpl::SetTableSize(uint16_t endpointSceneTableSize)
{
    FabricTableImpl::SetTableSize(endpointSceneTableSize, static_cast<uint16_t>((endpointSceneTableSize - 1) / 2));
}

namespace {

static DefaultSceneTableImpl gSceneTableImpl;

} // namespace

template <>
CHIP_ERROR Serializer::SerializeId(TLV::TLVWriter & writer, const SceneStorageId & id)
{
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGroupId), id.mGroupId));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneId), id.mSceneId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::DeserializeId(TLV::TLVReader & reader, SceneStorageId & id)
{
    // Scene ID
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupId)));
    ReturnErrorOnFailure(reader.Get(id.mGroupId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneId)));
    ReturnErrorOnFailure(reader.Get(id.mSceneId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::SerializeData(TLV::TLVWriter & writer, const SceneData & data)
{
    CharSpan nameSpan(data.mName, data.mNameLength);
    // A length of 0 means the name wasn't used so it won't get stored
    if (!nameSpan.empty())
    {
        ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(TagScene::kName), nameSpan));
    }

    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kTransitionTimeMs), data.mSceneTransitionTimeMs));
    ReturnErrorOnFailure(data.mExtensionFieldSets.Serialize(writer));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::DeserializeData(TLV::TLVReader & reader, SceneData & data)
{
    ReturnErrorOnFailure(reader.Next());
    TLV::Tag currTag = reader.GetTag();
    VerifyOrReturnError(TLV::ContextTag(TagScene::kName) == currTag || TLV::ContextTag(TagScene::kTransitionTimeMs) == currTag,
                        CHIP_ERROR_INVALID_TLV_TAG);

    CharSpan nameSpan;
    // A name may or may not have been stored.  Check whether it was.
    if (currTag == TLV::ContextTag(TagScene::kName))
    {
        ReturnErrorOnFailure(reader.Get(nameSpan));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kTransitionTimeMs)));
    }
    // Empty name will be initialized if the name wasn't stored
    data.SetName(nameSpan);
    ReturnErrorOnFailure(reader.Get(data.mSceneTransitionTimeMs));

    ReturnErrorOnFailure(data.mExtensionFieldSets.Deserialize(reader));
    return CHIP_NO_ERROR;
}

/// @brief Instance getter for the default global scene table implementation
/// @note This API should always be called prior to using the scene Table and the return pointer should never be cached. As per
/// issue: https://github.com/project-chip/connectedhomeip/issues/26878, this API is currently not thread
/// safe and calls to it should be made thread safe in the event of using multiple endpoints at once.
/// @return Default global scene table implementation
DefaultSceneTableImpl * chip::scenes::GetSceneTableImpl(EndpointId endpoint, uint16_t endpointTableSize)
{
    gSceneTableImpl.SetEndpoint(endpoint);
    gSceneTableImpl.SetTableSize(endpointTableSize);

    return &gSceneTableImpl;
}
