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
    kGlobalSceneCount = 1,
    kSceneCount,
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

// Currently takes 5 Bytes to serialize Container and value in a TLV: 1 byte start struct, 2 bytes control + tag for the value, 1
// byte value, 1 byte end struct. 8 Bytes leaves space for potential increase in count_value size.
static constexpr size_t kPersistentBufferSceneCountBytes = 8;

struct GlobalSceneCount : public PersistentData<kPersistentBufferSceneCountBytes>
{
    uint8_t count_value = 0;

    GlobalSceneCount(uint8_t count = 0) : count_value(count) {}
    ~GlobalSceneCount() {}

    void Clear() override { count_value = 0; }

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        key = DefaultStorageKeyAllocator::GlobalSceneCountKey();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGlobalSceneCount), count_value));
        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGlobalSceneCount)));
        ReturnErrorOnFailure(reader.Get(count_value));
        return reader.ExitContainer(container);
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) override
    {
        CHIP_ERROR err = PersistentData::Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            count_value = 0;
        }

        return CHIP_NO_ERROR;
    }
};

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
    FabricIndex fabric_index;
    uint8_t scene_count = 0;
    uint8_t max_scenes_per_fabric;
    uint8_t max_scenes_global;
    SceneStorageId scene_map[kMaxScenesPerFabric];

    FabricSceneData(FabricIndex fabric = kUndefinedFabricIndex, uint8_t maxScenesPerFabric = kMaxScenesPerFabric,
                    uint8_t maxScenesGlobal = kMaxScenesGlobal) :
        fabric_index(fabric),
        max_scenes_per_fabric(maxScenesPerFabric), max_scenes_global(maxScenesGlobal)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricSceneDataKey(fabric_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        scene_count = 0;
        for (uint8_t i = 0; i < max_scenes_per_fabric; i++)
        {
            scene_map[i].Clear();
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType fabricSceneContainer;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, fabricSceneContainer));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneCount), scene_count));
        TLV::TLVType sceneMapContainer;
        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagScene::kStorageIDArray), TLV::kTLVType_Array, sceneMapContainer));

        // Storing the scene map
        for (uint8_t i = 0; i < max_scenes_per_fabric; i++)
        {
            TLV::TLVType sceneIdContainer;
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, sceneIdContainer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kEndpointID), (scene_map[i].mEndpointId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGroupID), (scene_map[i].mGroupId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kSceneID), (scene_map[i].mSceneId)));
            ReturnErrorOnFailure(writer.EndContainer(sceneIdContainer));
        }
        ReturnErrorOnFailure(writer.EndContainer(sceneMapContainer));
        return writer.EndContainer(fabricSceneContainer);
    }

    /// @brief This Deserialize method is implemented only to allow compilation. It is not used throughout the code.
    /// @param reader TLV reader
    /// @return CHIP_NO_ERROR
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override { return CHIP_ERROR_INCORRECT_STATE; }

    /// @brief This Deserialize method checks that the recovered scenes from the deserialization fit in the current max and if
    /// there are too many scenes in nvm, it deletes them. The method sets the deleted_scenes output parameter to true if scenes
    /// were deleted so that the load function can know it needs to save the Fabric scene data to update the scene_count and the
    /// scene map in stored memory.
    /// @param reade [in] TLV reader, must be big enough to hold the scene map size
    /// @param storage [in] Persistent Storage Delegate, required to delete scenes if the number of scenes in storage is greater
    /// than the maximum allowed
    /// @param deleted_scenes_count [out] uint8_t letting the caller (in this case the load method) know how many scenes were
    /// deleted so it can adjust the fabric and global scene count accordingly. Even if Deserialize fails, this value will return
    /// the number of scenes deleted before the failure happened.
    /// @return CHIP_NO_ERROR on success, specific CHIP_ERROR otherwise
    CHIP_ERROR Deserialize(TLV::TLVReader & reader, PersistentStorageDelegate * storage, uint8_t & deleted_scenes_count)
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        TLV::TLVType fabricSceneContainer;
        ReturnErrorOnFailure(reader.EnterContainer(fabricSceneContainer));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneCount)));
        ReturnErrorOnFailure(reader.Get(scene_count));
        scene_count = min(scene_count, max_scenes_per_fabric);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagScene::kStorageIDArray)));
        TLV::TLVType sceneMapContainer;
        ReturnErrorOnFailure(reader.EnterContainer(sceneMapContainer));

        uint8_t i = 0;
        CHIP_ERROR err;
        deleted_scenes_count = 0;

        while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR)
        {
            TLV::TLVType sceneIdContainer;
            if (i < max_scenes_per_fabric)
            {
                ReturnErrorOnFailure(reader.EnterContainer(sceneIdContainer));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kEndpointID)));
                ReturnErrorOnFailure(reader.Get(scene_map[i].mEndpointId));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
                ReturnErrorOnFailure(reader.Get(scene_map[i].mGroupId));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneID)));
                ReturnErrorOnFailure(reader.Get(scene_map[i].mSceneId));
                ReturnErrorOnFailure(reader.ExitContainer(sceneIdContainer));
            }
            else
            {
                SceneTableData scene(fabric_index, i);
                ReturnErrorOnFailure(reader.EnterContainer(sceneIdContainer));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kEndpointID)));
                ReturnErrorOnFailure(reader.Get(scene.mStorageId.mEndpointId));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
                ReturnErrorOnFailure(reader.Get(scene.mStorageId.mGroupId));
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kSceneID)));
                ReturnErrorOnFailure(reader.Get(scene.mStorageId.mSceneId));
                ReturnErrorOnFailure(reader.ExitContainer(sceneIdContainer));
                ReturnErrorOnFailure(scene.Delete(storage));
                deleted_scenes_count++;
            }

            i++;
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(sceneMapContainer));
        return reader.ExitContainer(fabricSceneContainer);
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

        while (index < max_scenes_per_fabric)
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

        if (firstFreeIdx < max_scenes_per_fabric)
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
            // Update the global scene count
            GlobalSceneCount global_scene_count;
            ReturnErrorOnFailure(global_scene_count.Load(storage));
            VerifyOrReturnError(global_scene_count.count_value < max_scenes_global, CHIP_ERROR_NO_MEMORY);
            global_scene_count.count_value++;
            ReturnErrorOnFailure(global_scene_count.Save(storage));

            scene_count++;
            scene_map[scene.index] = scene.mStorageId;

            err = this->Save(storage);
            if (CHIP_NO_ERROR != err)
            {
                global_scene_count.count_value--;
                ReturnErrorOnFailure(global_scene_count.Save(storage));
                return err;
            }

            err = scene.Save(storage);

            // on failure to save the scene, undoes the changes to Fabric Scene Data
            if (CHIP_NO_ERROR != err)
            {
                global_scene_count.count_value--;
                ReturnErrorOnFailure(global_scene_count.Save(storage));

                scene_count--;
                scene_map[scene.index].Clear();
                ReturnErrorOnFailure(this->Save(storage));
                return err;
            }
        }

        return err;
    }

    /// @brief Removes a scene from the non-volatile memory and clears its index in the scene map. Decreases the number of scenes in
    /// the global scene count and in the scene fabric data if successful. As the scene map size is not compressed upon removal,
    /// this only clears the entry correpsonding to the scene from the scene map.
    /// @param storage Storage delegate to access the scene
    /// @param scene_id Scene to remove
    /// @return CHIP_NO_ERROR if successful, specific CHIP_ERROR otherwise
    CHIP_ERROR RemoveScene(PersistentStorageDelegate * storage, const SceneStorageId & scene_id)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        SceneTableData scene(fabric_index, scene_id);

        // Empty Scene Fabric Data returns CHIP_NO_ERROR on remove
        if (scene_count > 0)
        {
            // If Find doesn't return CHIP_NO_ERROR, the scene wasn't found, which doesn't return an error
            VerifyOrReturnValue(this->Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_NO_ERROR);

            // Update the global scene count
            GlobalSceneCount global_scene_count;
            ReturnErrorOnFailure(global_scene_count.Load(storage));
            global_scene_count.count_value--;
            ReturnErrorOnFailure(global_scene_count.Save(storage));

            scene_count--;
            scene_map[scene.index].Clear();
            err = this->Save(storage);

            // On failure to update the scene map, undo the global count modification
            if (CHIP_NO_ERROR != err)
            {
                global_scene_count.count_value++;
                ReturnErrorOnFailure(global_scene_count.Save(storage));
                return err;
            }

            err = scene.Delete(storage);

            // On failure to delete scene, undo the change to the Fabric Scene Data and the global scene count
            if (CHIP_NO_ERROR != err)
            {
                global_scene_count.count_value++;
                ReturnErrorOnFailure(global_scene_count.Save(storage));

                scene_count++;
                scene_map[scene.index] = scene.mStorageId;
                ReturnErrorOnFailure(this->Save(storage));
                return err;
            }
        }
        return err;
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) override
    {
        VerifyOrReturnError(nullptr != storage, CHIP_ERROR_INVALID_ARGUMENT);
        uint8_t deleted_scenes_count = 0;

        uint8_t buffer[kPersistentFabricBufferMax] = { 0 };
        StorageKeyName key                         = StorageKeyName::Uninitialized();

        // Set data to defaults
        Clear();

        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Load the serialized data
        uint16_t size  = static_cast<uint16_t>(sizeof(buffer));
        CHIP_ERROR err = storage->SyncGetKeyValue(key.KeyName(), buffer, size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(buffer, size);

        err = Deserialize(reader, storage, deleted_scenes_count);

        // If Deserialize sets the "deleted_scenes" variable, the table in flash memory held too many scenes (can happen
        // if max_scenes_per_fabric was reduced during an OTA) and was adjusted during deserailizing . The fabric data must then
        // be updated
        if (deleted_scenes_count)
        {
            GlobalSceneCount global_count;
            ReturnErrorOnFailure(global_count.Load(storage));
            global_count.count_value = static_cast<uint8_t>(global_count.count_value - deleted_scenes_count);
            ReturnErrorOnFailure(global_count.Save(storage));
            ReturnErrorOnFailure(this->Save(storage));
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

    // Verified the initialized parameter respect the maximum allowed values for scene capacity
    VerifyOrReturnError(mMaxScenesPerFabric <= kMaxScenesPerFabric && mMaxScenesGlobal <= kMaxScenesGlobal,
                        CHIP_ERROR_INVALID_INTEGER_VALUE);
    mStorage = storage;
    return CHIP_NO_ERROR;
}

void DefaultSceneTableImpl::Finish()
{
    UnregisterAllHandlers();
    mSceneEntryIterators.ReleaseAll();
}

CHIP_ERROR DefaultSceneTableImpl::GetGlobalSceneCount(uint8_t & scene_count)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    GlobalSceneCount global_count;

    ReturnErrorOnFailure(global_count.Load(mStorage));
    scene_count = global_count.count_value;

    return CHIP_NO_ERROR;
}
CHIP_ERROR DefaultSceneTableImpl::SetGlobalSceneCount(const uint8_t & scene_count)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    GlobalSceneCount global_count(scene_count);
    return global_count.Save(mStorage);
}

CHIP_ERROR DefaultSceneTableImpl::GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    uint8_t global_scene_count = 0;
    ReturnErrorOnFailure(GetGlobalSceneCount(global_scene_count));

    // If the global scene count is higher than the maximal Global scene capacity, this returns a capacity of 0 until enough scenes
    // have been deleted to bring the global number of scenes under the global maximum.
    if (global_scene_count > mMaxScenesGlobal)
    {
        capacity = 0;
        return CHIP_NO_ERROR;
    }
    uint8_t remaining_capacity_global = static_cast<uint8_t>(mMaxScenesGlobal - global_scene_count);
    uint8_t remaining_capacity_fabric = mMaxScenesPerFabric;

    FabricSceneData fabric(fabric_index);

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (err == CHIP_NO_ERROR)
    {
        remaining_capacity_fabric = static_cast<uint8_t>(mMaxScenesPerFabric - fabric.scene_count);
    }

    capacity = min(remaining_capacity_fabric, remaining_capacity_global);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    return fabric.SaveScene(mStorage, entry);
}

CHIP_ERROR DefaultSceneTableImpl::GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
    SceneTableData scene(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);

    CHIP_ERROR err = scene.Load(mStorage);

    // If scene.Load returns "buffer too small", the scene in memory is too big to be retrieve (this could happen if the
    // kMaxClustersPerScene was reduced by OTA) and therefore must be deleted as is is no longer considered accessible.
    if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        ReturnErrorOnFailure(this->RemoveSceneTableEntry(fabric_index, scene_id));
    }
    ReturnErrorOnFailure(err);

    entry.mStorageId   = scene.mStorageId;
    entry.mStorageData = scene.mStorageData;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    CHIP_ERROR err = CHIP_NO_ERROR;
    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
    SceneTableData scene(fabric_index, scene_idx);

    ReturnErrorOnFailure(fabric.Load(mStorage));
    err = scene.Load(mStorage);
    VerifyOrReturnValue(CHIP_ERROR_NOT_FOUND != err, CHIP_NO_ERROR);
    ReturnErrorOnFailure(err);

    return fabric.RemoveScene(mStorage, scene.mStorageId);
}

CHIP_ERROR DefaultSceneTableImpl::GetAllSceneIdsInGroup(FabricIndex fabric_index, GroupId group_id, Span<SceneId> & scene_list)
{
    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
    SceneTableData scene(fabric_index);

    auto * iterator = this->IterateSceneEntries(fabric_index);
    VerifyOrReturnError(nullptr != iterator, CHIP_ERROR_INTERNAL);
    SceneId * list      = scene_list.data();
    uint8_t scene_count = 0;

    while (iterator->Next(scene))
    {
        if (scene.mStorageId.mGroupId == group_id)
        {
            if (scene_count >= scene_list.size())
            {
                iterator->Release();
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            list[scene_count] = scene.mStorageId.mSceneId;
            scene_count++;
        }
    }
    scene_list.reduce_size(scene_count);
    iterator->Release();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::DeleteAllScenesInGroup(FabricIndex fabric_index, GroupId group_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
    SceneTableData scene(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));

    for (uint8_t i = 0; i < mMaxScenesPerFabric; i++)
    {
        if (fabric.scene_map[i].mGroupId == group_id)
        {
            // Removing each scene from the nvm and clearing their entry in the scene map
            ReturnErrorOnFailure(fabric.RemoveScene(mStorage, fabric.scene_map[i]));
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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricSceneData fabric(fabric_index);
    SceneIndex idx = 0;
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    while (idx < mMaxScenesPerFabric)
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
    return mSceneEntryIterators.CreateObject(*this, fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
}

DefaultSceneTableImpl::SceneEntryIteratorImpl::SceneEntryIteratorImpl(DefaultSceneTableImpl & provider, FabricIndex fabric_index,
                                                                      uint8_t maxScenesPerFabric, uint8_t maxScenesGlobal) :
    mProvider(provider),
    mFabric(fabric_index), mMaxScenesPerFabric(maxScenesPerFabric), mMaxScenesGlobal(maxScenesGlobal)
{
    FabricSceneData fabric(fabric_index, mMaxScenesPerFabric, mMaxScenesGlobal);
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
    while (mSceneIndex < mMaxScenesPerFabric)
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
