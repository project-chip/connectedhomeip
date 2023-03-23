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

#include <app/clusters/scenes/SceneTableImpl.h>
#include <app/util/attribute-storage.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <stdlib.h>

namespace chip {
namespace scenes {

/// @brief Tags Used to serialize Scenes so they can be stored in flash memory.
/// kSceneCount: Number of scenes in a Fabric
/// kStorageIDArray: Array of StorageID struct
/// kEndpointID: Tag for the Endpoint ID to which this scene applies to
/// kGroupID: Tag for GroupID if the Scene is a Group Scene
/// kSceneID: Tag for the scene ID together with the two previous tag, forms the SceneStorageID
/// kName: Tag for the name of the scene
/// kTransitionTime: Tag for the transition time of the scene in miliseconds
enum class TagScene : uint8_t
{
    kSceneCount = 1,
    kStorageIDArray,
    kEndpointID,
    kGroupID,
    kSceneID,
    kName,
    kTransitionTimeMs,
    kExtensionFieldSetsContainer,
};

using SceneTableEntry = DefaultSceneTableImpl::SceneTableEntry;
using SceneStorageId  = DefaultSceneTableImpl::SceneStorageId;
using SceneData       = DefaultSceneTableImpl::SceneData;

// Worst case tested: Add Scene Command with EFS using the default SerializeAdd Method. This yielded a serialized scene of 212bytes
// when using the OnOff, Level Control and Color Control as well as the maximal name length of 16 bytes. Putting 256 gives some
// slack in case different clusters are used. Value obtained by using writer.GetLengthWritten at the end of the SceneTableData
// Serialize method.
static constexpr size_t kPersistentSceneBufferMax = 256;

struct SceneTableData : public SceneTableEntry, PersistentData<kPersistentSceneBufferMax>
{
    FabricIndex fabric_index = kUndefinedFabricIndex;
    SceneIndex index         = 0;
    bool first               = true;

    SceneTableData() : SceneTableEntry() {}
    SceneTableData(FabricIndex fabric) : fabric_index(fabric) {}
    SceneTableData(FabricIndex fabric, SceneIndex idx) : fabric_index(fabric), index(idx) {}
    SceneTableData(FabricIndex fabric, SceneStorageId storageId) : SceneTableEntry(storageId), fabric_index(fabric) {}
    SceneTableData(FabricIndex fabric, SceneStorageId storageId, SceneData data) :
        SceneTableEntry(storageId, data), fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricSceneKey(fabric_index, index);
        return CHIP_NO_ERROR;
    }

    void Clear() override { mStorageData.Clear(); }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        CharSpan nameSpan(mStorageData.mName, mStorageData.mNameLength);
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        // Scene ID
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kEndpointID), mStorageId.mEndpointId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGroupID), mStorageId.mGroupId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneID), mStorageId.mSceneId));

        // Scene Data
        // A length of 0 means the name wasn't used so it won't get stored
        if (!nameSpan.empty())
        {
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(TagScene::kName), nameSpan));
        }

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kTransitionTimeMs), mStorageData.mSceneTransitionTimeMs));
        ReturnErrorOnFailure(
            mStorageData.mExtensionFieldSets.Serialize(writer, TLV::ContextTag(TagScene::kExtensionFieldSetsContainer)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // Scene ID
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kEndpointID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mEndpointId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mGroupId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mSceneId));

        // Scene Data
        ReturnErrorOnFailure(reader.Next());
        TLV::Tag currTag = reader.GetTag();
        VerifyOrReturnError(TLV::ContextTag(TagScene::kName) == currTag || TLV::ContextTag(TagScene::kTransitionTimeMs) == currTag,
                            CHIP_ERROR_WRONG_TLV_TYPE);

        CharSpan nameSpan;
        // A name may or may not have been stored.  Check whether it was.
        if (currTag == TLV::ContextTag(TagScene::kName))
        {
            ReturnErrorOnFailure(reader.Get(nameSpan));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kTransitionTimeMs)));
        }
        // Empty name will be initialized if the name wasn't stored
        mStorageData.SetName(nameSpan);

        ReturnErrorOnFailure(reader.Get(mStorageData.mSceneTransitionTimeMs));
        ReturnErrorOnFailure(
            mStorageData.mExtensionFieldSets.Deserialize(reader, TLV::ContextTag(TagScene::kExtensionFieldSetsContainer)));

        return reader.ExitContainer(container);
    }
};

// A Full fabric serialized TLV length is 88 bytes, 128 bytes gives some slack.  Tested by running writer.GetLengthWritten at the
// end of the Serialize method of FabricSceneData
static constexpr size_t kPersistentFabricBufferMax = 128;

/**
 * @brief Linked list of all scenes in a fabric, stored in persistent memory
 *
 * FabricSceneData is an access to a linked list of scenes
 */
struct FabricSceneData : public PersistentData<kPersistentFabricBufferMax>
{
    FabricIndex fabric_index = kUndefinedFabricIndex;
    uint8_t scene_count      = 0;
    SceneStorageId scene_map[kMaxScenesPerFabric];

    FabricSceneData() = default;
    FabricSceneData(FabricIndex fabric) : fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricSceneDataKey(fabric_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        scene_count = 0;
        for (uint8_t i = 0; i < kMaxScenesPerFabric; i++)
        {
            scene_map[i].Clear();
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneCount), scene_count));
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(TagScene::kStorageIDArray), TLV::kTLVType_Array, container));

        // Storing the scene map
        for (uint8_t i = 0; i < kMaxScenesPerFabric; i++)
        {
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kEndpointID), (scene_map[i].mEndpointId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGroupID), (scene_map[i].mGroupId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneID), (scene_map[i].mSceneId)));
            ReturnErrorOnFailure(writer.EndContainer(container));
        }
        ReturnErrorOnFailure(writer.EndContainer(container));
        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneCount)));
        ReturnErrorOnFailure(reader.Get(scene_count));
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagScene::kStorageIDArray)));
        ReturnErrorOnFailure(reader.EnterContainer(container));

        uint8_t i = 0;
        CHIP_ERROR err;
        while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR && i < kMaxScenesPerFabric)
        {
            ReturnErrorOnFailure(reader.EnterContainer(container));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kEndpointID)));
            ReturnErrorOnFailure(reader.Get(scene_map[i].mEndpointId));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
            ReturnErrorOnFailure(reader.Get(scene_map[i].mGroupId));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneID)));
            ReturnErrorOnFailure(reader.Get(scene_map[i].mSceneId));
            ReturnErrorOnFailure(reader.ExitContainer(container));

            i++;
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);

        ReturnErrorOnFailure(reader.ExitContainer(container));
        return reader.ExitContainer(container);
    }

    /// @brief Finds the index where to insert current scene by going through the whole table and looking if the scene is already in
    /// there. If the target is not in the table, sets idx to the first empty space
    /// @param target_scene Storage Id of scene to store
    /// @param idx Index where target or space is found
    /// @return CHIP_NO_ERROR if managed to find the target scene, CHIP_ERROR_NOT_FOUND if not found and space left
    ///         CHIP_ERROR_NO_MEMORY if target was not found and table is full
    CHIP_ERROR Find(SceneStorageId target_scene, SceneIndex & idx)
    {
        SceneIndex firstFreeIdx = kUndefinedSceneIndex; // storage index if scene not found
        uint8_t index           = 0;

        while (index < kMaxScenesPerFabric)
        {
            if (scene_map[index] == target_scene)
            {
                idx = index;
                return CHIP_NO_ERROR; // return scene at current index if scene found
            }
            if (!scene_map[index].IsValid() && firstFreeIdx == kUndefinedSceneIndex)
            {
                firstFreeIdx = index;
            }
            index++;
        }

        if (firstFreeIdx < kMaxScenesPerFabric)
        {
            idx = firstFreeIdx;
            return CHIP_ERROR_NOT_FOUND;
        }

        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR SaveScene(PersistentStorageDelegate * storage, const SceneTableEntry & entry)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        SceneTableData scene(fabric_index, entry.mStorageId, entry.mStorageData);
        // Look for empty storage space

        err = this->Find(entry.mStorageId, scene.index);

        if (CHIP_NO_ERROR == err)
        {
            return scene.Save(storage);
        }

        if (CHIP_ERROR_NOT_FOUND == err) // If not found, scene.index should be the first free index
        {
            scene_count++;
            scene_map[scene.index] = scene.mStorageId;
            ReturnErrorOnFailure(this->Save(storage));

            err = scene.Save(storage);

            // on failure to save the scene, undoes the changes to Fabric Scene Data
            if (err != CHIP_NO_ERROR)
            {
                scene_count--;
                scene_map[scene.index].Clear();
                ReturnErrorOnFailure(this->Save(storage));
            }

            return err;
        }

        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    CHIP_ERROR RemoveScene(PersistentStorageDelegate * storage, const SceneStorageId & scene_id)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        SceneTableData scene(fabric_index, scene_id);

        // Empty Scene Fabric Data returns CHIP_NO_ERROR on remove
        if (scene_count > 0)
        {
            // If Find doesn't return CHIP_NO_ERROR, the scene wasn't found, which doesn't return an error
            VerifyOrReturnValue(this->Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_NO_ERROR);

            scene_count--;
            scene_map[scene.index].Clear();
            ReturnErrorOnFailure(this->Save(storage));

            err = scene.Delete(storage);

            // On failure to delete scene, undoes the change to the Fabric Scene Data
            if (err != CHIP_NO_ERROR)
            {
                scene_count++;
                scene_map[scene.index] = scene.mStorageId;
                ReturnErrorOnFailure(this->Save(storage));
            }
        }
        return err;
    }
};

CHIP_ERROR DefaultSceneTableImpl::Init(PersistentStorageDelegate * storage)
{
    if (storage == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mStorage = storage;
    return CHIP_NO_ERROR;
}

void DefaultSceneTableImpl::Finish()
{
    UnregisterAllHandlers();
    mSceneEntryIterators.ReleaseAll();
}

CHIP_ERROR DefaultSceneTableImpl::SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index);

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    return fabric.SaveScene(mStorage, entry);
}

CHIP_ERROR DefaultSceneTableImpl::GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);

    ReturnErrorOnFailure(scene.Load(mStorage));
    entry.mStorageId   = scene.mStorageId;
    entry.mStorageData = scene.mStorageData;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)
{
    FabricSceneData fabric(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));

    return fabric.RemoveScene(mStorage, scene_id);
}

/// @brief This function is meant to provide a way to empty the scene table without knowing any specific scene Id. Outside of this
/// specific use case, RemoveSceneTableEntry should be used.
/// @param fabric_index Fabric in which the scene belongs
/// @param scened_idx Position in the Scene Table
/// @return CHIP_NO_ERROR if removal was successful, errors if failed to remove the scene or to update the fabric after removing it
CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntryAtPosition(FabricIndex fabric_index, SceneIndex scene_idx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    FabricSceneData fabric(fabric_index);
    SceneTableData scene(fabric_index, scene_idx);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    err = scene.Load(mStorage);
    VerifyOrReturnValue(CHIP_ERROR_NOT_FOUND != err, CHIP_NO_ERROR);
    ReturnErrorOnFailure(err);

    return fabric.RemoveScene(mStorage, scene.mStorageId);
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
        uint8_t clusterCount = 0;
        clusterId cArray[kMaxClustersPerScene];
        Span<clusterId> cSpan(cArray);
        clusterCount = GetClustersFromEndpoint(scene.mStorageId.mEndpointId, cArray, kMaxClustersPerScene);
        cSpan.reduce_size(clusterCount);
        for (clusterId cluster : cSpan)
        {
            ExtensionFieldSet EFS;
            MutableByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, kMaxFieldBytesPerCluster);
            EFS.mID                 = cluster;

            for (auto & handler : mHandlerList)
            {
                if (handler.SupportsCluster(scene.mStorageId.mEndpointId, cluster))
                {
                    ReturnErrorOnFailure(handler.SerializeSave(scene.mStorageId.mEndpointId, EFS.mID, EFSSpan));
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
    ExtensionFieldSet EFS;
    TransitionTimeMs time;
    clusterId cluster;

    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < scene.mStorageData.mExtensionFieldSets.GetFieldSetCount(); i++)
        {
            scene.mStorageData.mExtensionFieldSets.GetFieldSetAtPosition(EFS, i);
            cluster          = EFS.mID;
            time             = scene.mStorageData.mSceneTransitionTimeMs;
            ByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, EFS.mUsedBytes);

            if (!EFS.IsEmpty())
            {
                for (auto & handler : mHandlerList)
                {
                    if (handler.SupportsCluster(scene.mStorageId.mEndpointId, cluster))
                    {
                        ReturnErrorOnFailure(handler.ApplyScene(scene.mStorageId.mEndpointId, cluster, EFSSpan, time));
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
    FabricSceneData fabric(fabric_index);
    SceneIndex idx = 0;
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    while (idx < kMaxScenesPerFabric)
    {
        err = RemoveSceneTableEntryAtPosition(fabric_index, idx);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err, err);
        idx++;
    }

    // Remove fabric
    return fabric.Delete(mStorage);
}

/// @brief wrapper function around emberAfGetClustersFromEndpoint to allow testing, shimmed in test configuration because
/// emberAfGetClusterFromEndpoint relies on <app/util/attribute-storage.h>, which relies on zap generated files
uint8_t DefaultSceneTableImpl::GetClustersFromEndpoint(EndpointId endpoint, ClusterId * clusterList, uint8_t listLen)
{
    return emberAfGetClustersFromEndpoint(endpoint, clusterList, listLen, true);
}

DefaultSceneTableImpl::SceneEntryIterator * DefaultSceneTableImpl::IterateSceneEntries(FabricIndex fabric_index)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mSceneEntryIterators.CreateObject(*this, fabric_index);
}

DefaultSceneTableImpl::SceneEntryIteratorImpl::SceneEntryIteratorImpl(DefaultSceneTableImpl & provider, FabricIndex fabric_index) :
    mProvider(provider), mFabric(fabric_index)
{
    FabricSceneData fabric(fabric_index);
    ReturnOnFailure(fabric.Load(provider.mStorage));
    mTotalScenes = fabric.scene_count;
    mSceneIndex  = 0;
}

size_t DefaultSceneTableImpl::SceneEntryIteratorImpl::Count()
{
    return mTotalScenes;
}

bool DefaultSceneTableImpl::SceneEntryIteratorImpl::Next(SceneTableEntry & output)
{
    FabricSceneData fabric(mFabric);
    SceneTableData scene(mFabric);

    VerifyOrReturnError(fabric.Load(mProvider.mStorage) == CHIP_NO_ERROR, false);

    // looks for next available scene
    while (mSceneIndex < kMaxScenesPerFabric)
    {
        if (fabric.scene_map[mSceneIndex].IsValid())
        {
            scene.index = mSceneIndex;
            VerifyOrReturnError(scene.Load(mProvider.mStorage) == CHIP_NO_ERROR, false);
            output.mStorageId   = scene.mStorageId;
            output.mStorageData = scene.mStorageData;
            mSceneIndex++;

            return true;
        }

        mSceneIndex++;
    }

    return false;
}

void DefaultSceneTableImpl::SceneEntryIteratorImpl::Release()
{
    mProvider.mSceneEntryIterators.ReleaseObject(this);
}

} // namespace scenes
} // namespace chip
