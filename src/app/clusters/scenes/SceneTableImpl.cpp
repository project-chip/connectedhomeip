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
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <stdlib.h>

namespace chip {
namespace scenes {

/// @brief Tags Used to serialize Scene specific Fabric data
/// kSceneCount: Tag for the number of scenes in Fabric
/// kGroupID: Tag for the next Fabric
enum class TagSceneImpl : uint8_t
{
    kSceneCount = 1,
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
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kID), mStorageId.mSceneId));

        // Scene Data
        // A length of 0 means the name wasn't used so it won't get stored
        if (!nameSpan.empty())
        {
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(TagScene::kName), nameSpan));
        }

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kTransitionTimeMs), mStorageData.mSceneTransitionTimeMs));
        ReturnErrorOnFailure(mStorageData.mExtensionFieldSets.Serialize(writer));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        char nameBuffer[kSceneNameMax] = { 0 };
        CharSpan nameSpan(nameBuffer);

        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // Scene ID
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kEndpointID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mEndpointId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mGroupId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kID)));
        ReturnErrorOnFailure(reader.Get(mStorageId.mSceneId));

        // Scene Data
        ReturnErrorOnFailure(reader.Next());
        TLV::Tag currTag = reader.GetTag();
        VerifyOrReturnError(TLV::ContextTag(TagScene::kName) == currTag || TLV::ContextTag(TagScene::kTransitionTimeMs) == currTag,
                            CHIP_ERROR_WRONG_TLV_TYPE);

        // A name may or may not have been stored.  Check whether it was.
        if (currTag == TLV::ContextTag(TagScene::kName))
        {
            ReturnErrorOnFailure(reader.Get(nameSpan));
            mStorageData.SetName(nameSpan);
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kTransitionTimeMs)));
        }

        ReturnErrorOnFailure(reader.Get(mStorageData.mSceneTransitionTimeMs));
        ReturnErrorOnFailure(mStorageData.mExtensionFieldSets.Deserialize(reader));

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
    SceneStorageId scene_map[kMaxScenePerFabric];

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
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            scene_map[i].Clear();
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagSceneImpl::kSceneCount), (scene_count)));
        // Storing the scene map
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kEndpointID), (scene_map[i].mEndpointId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kGroupID), (scene_map[i].mGroupId)));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagScene::kID), (scene_map[i].mSceneId)));
        }

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagSceneImpl::kSceneCount)));
        ReturnErrorOnFailure(reader.Get(scene_count));

        uint8_t i = 0;
        while (reader.Next(TLV::ContextTag(TagScene::kEndpointID)) != CHIP_END_OF_TLV)
        {
            ReturnErrorOnFailure(reader.Get(scene_map[i].mEndpointId));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kGroupID)));
            ReturnErrorOnFailure(reader.Get(scene_map[i].mGroupId));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagScene::kID)));
            ReturnErrorOnFailure(reader.Get(scene_map[i].mSceneId));

            i++;
        }

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

        while (index < kMaxScenePerFabric)
        {
            if (scene_map[index] == target_scene)
            {
                idx = index;
                return CHIP_NO_ERROR; // return scene at current index if scene found
            }
            if (scene_map[index].mEndpointId == kInvalidEndpointId && firstFreeIdx == kUndefinedSceneIndex)
            {
                firstFreeIdx = index;
            }
            index++;
        }

        if (firstFreeIdx < kMaxScenePerFabric)
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

/// @brief Register a handler in the handler list
/// @param handler Cluster specific handler for extension field sets interaction
/// @return CHIP_NO_ERROR if handler was registered, CHIP_ERROR_NO_MEMORY if the handler list is full
CHIP_ERROR DefaultSceneTableImpl::RegisterHandler(SceneHandler * handler)
{
    uint8_t firstEmptyPosition = kInvalidPosition;

    for (uint8_t i = 0; i < kMaxSceneHandlers; i++)
    {
        if (this->mHandlers[i] == handler)
        {
            this->mHandlers[i] = handler;
            return CHIP_NO_ERROR;
        }
        if (this->mHandlers[i] == nullptr && firstEmptyPosition == kInvalidPosition)
        {
            firstEmptyPosition = i;
        }
    }

    // if found, insert at found position, otherwise at first free possition, otherwise return error
    if (firstEmptyPosition < kMaxSceneHandlers)
    {
        this->mHandlers[firstEmptyPosition] = handler;
        this->mNumHandlers++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR DefaultSceneTableImpl::UnregisterHandler(SceneHandler * handler)
{
    uint8_t position = kInvalidPosition;

    // Verify list is populated and handler is not null
    VerifyOrReturnValue(!this->HandlerListEmpty() && !(handler == nullptr), CHIP_NO_ERROR);

    // Finds the position of the Handler to unregister
    for (uint8_t i = 0; i < this->mNumHandlers; i++)
    {
        if (this->mHandlers[i] == handler)
        {
            position = i;
            break;
        }
    }

    // Verify Handler was found
    VerifyOrReturnValue(position < kMaxSceneHandlers, CHIP_NO_ERROR);

    uint8_t nextPos = static_cast<uint8_t>(position + 1);
    uint8_t moveNum = static_cast<uint8_t>(kMaxSceneHandlers - nextPos);

    // TODO: Implement general array management methods
    // Compress array after removal
    if (moveNum)
    {
        memmove(&this->mHandlers[position], &this->mHandlers[nextPos], sizeof(this->mHandlers[position]) * moveNum);
    }

    this->mNumHandlers--;
    // Clear last occupied position
    this->mHandlers[mNumHandlers] = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::UnregisterAllHandlers()
{
    for (uint8_t i = 0; i < this->mNumHandlers; i++)
    {
        ReturnErrorOnFailure(this->UnregisterHandler(this->mHandlers[0]));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::SceneSaveEFS(SceneTableEntry & scene)
{
    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < this->mNumHandlers; i++)
        {
            clusterId cArray[kMaxClusterPerScenes];
            Span<clusterId> cSpan(cArray);
            if (this->mHandlers[i] != nullptr)
            {
                this->mHandlers[i]->GetSupportedClusters(scene.mStorageId.mEndpointId, cSpan);
                for (clusterId cluster : cSpan)
                {
                    ExtensionFieldSet EFS;
                    MutableByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, kMaxFieldBytesPerCluster);

                    EFS.mID = cluster;
                    ReturnErrorOnFailure(this->mHandlers[i]->SerializeSave(scene.mStorageId.mEndpointId, EFS.mID, EFSSpan));
                    EFS.mUsedBytes = static_cast<uint8_t>(EFSSpan.size());
                    ReturnErrorOnFailure(scene.mStorageData.mExtensionFieldSets.InsertFieldSet(EFS));
                }
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::SceneApplyEFS(FabricIndex fabric_index, const SceneStorageId & scene_id)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene(fabric_index);
    ExtensionFieldSet EFS;
    TransitionTimeMs time;
    clusterId cluster;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(scene.Load(mStorage));

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
                for (uint8_t j = 0; j < this->mNumHandlers; j++)
                {
                    ReturnErrorOnFailure(this->mHandlers[j]->ApplyScene(scene.mStorageId.mEndpointId, cluster, EFSSpan, time));
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

    while (idx < kMaxScenePerFabric)
    {
        err = RemoveSceneTableEntryAtPosition(fabric_index, idx);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err, err);
        idx++;
    }

    // Remove fabric
    return fabric.Delete(mStorage);
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
    while (mSceneIndex < kMaxScenePerFabric)
    {
        if (fabric.scene_map[mSceneIndex].mEndpointId != kInvalidEndpointId)
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
