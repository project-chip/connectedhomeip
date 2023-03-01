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

enum SceneImplTLVTag
{
    kTagSceneCount = 1,
    kTagNext,
};

using SceneTableEntry = DefaultSceneTableImpl::SceneTableEntry;
using SceneStorageId  = DefaultSceneTableImpl::SceneStorageId;
using SceneData       = DefaultSceneTableImpl::SceneData;

struct FabricHavingSceneList : public CommonPersistentData::FabricList
{
    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        key = DefaultStorageKeyAllocator::SceneFabricList();
        return CHIP_NO_ERROR;
    }
};

static constexpr size_t kPersistentBufferMax = 256;

struct SceneTableData : public SceneTableEntry, PersistentData<kPersistentBufferMax>
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
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(mStorageId.Serialize(writer));
        ReturnErrorOnFailure(mStorageData.Serialize(writer));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(mStorageId.Deserialize(reader));
        ReturnErrorOnFailure(mStorageData.Deserialize(reader));

        return reader.ExitContainer(container);
    }
};

/**
 * @brief Linked list of all scenes in a fabric, stored in persistent memory
 *
 * FabricSceneData is an access to a linked list of scenes
 */
struct FabricSceneData : public PersistentData<kPersistentBufferMax>
{
    FabricIndex fabric_index = kUndefinedFabricIndex;
    uint8_t scene_count      = 0;
    SceneStorageId scene_map[kMaxScenePerFabric];
    FabricIndex next = kUndefinedFabricIndex;

    FabricSceneData() = default;
    FabricSceneData(FabricIndex fabric) : fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricScenesKey(fabric_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        scene_count = 0;
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            scene_map[i].Clear();
        }
        next = kUndefinedFabricIndex;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagSceneCount), static_cast<uint16_t>(scene_count)));
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            ReturnErrorOnFailure(scene_map[i].Serialize(writer));
        }
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagNext), static_cast<uint16_t>(next)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagSceneCount)));
        ReturnErrorOnFailure(reader.Get(scene_count));
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            ReturnErrorOnFailure(scene_map[i].Deserialize(reader));
        }
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagNext)));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    // Register the fabric in the list of fabrics having scenes
    CHIP_ERROR Register(PersistentStorageDelegate * storage)
    {
        FabricHavingSceneList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            // New fabric list
            fabric_list.first_entry = fabric_index;
            fabric_list.entry_count = 1;
            return fabric_list.Save(storage);
        }
        ReturnErrorOnFailure(err);

        // Existing fabric list, search for existing entry
        FabricSceneData fabric(fabric_list.first_entry);
        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            err = fabric.Load(storage);
            if (CHIP_NO_ERROR != err)
            {
                break;
            }
            if (fabric.fabric_index == this->fabric_index)
            {
                // Fabric already registered
                return CHIP_NO_ERROR;
            }
            fabric.fabric_index = fabric.next;
        }
        // Add this fabric to the fabric list
        this->next              = fabric_list.first_entry;
        fabric_list.first_entry = this->fabric_index;
        fabric_list.entry_count++;
        return fabric_list.Save(storage);
    }

    // Remove the fabric from the fabrics' linked list
    CHIP_ERROR Unregister(PersistentStorageDelegate * storage) const
    {
        FabricHavingSceneList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

        // Existing fabric list, search for existing entry
        FabricSceneData fabric(fabric_list.first_entry);
        FabricSceneData prev;

        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            err = fabric.Load(storage);
            if (CHIP_NO_ERROR != err)
            {
                break;
            }
            if (fabric.fabric_index == this->fabric_index)
            {
                // Fabric found
                if (i == 0)
                {
                    // Remove first fabric
                    fabric_list.first_entry = this->next;
                }
                else
                {
                    // Remove intermediate fabric
                    prev.next = this->next;
                    ReturnErrorOnFailure(prev.Save(storage));
                }
                VerifyOrReturnError(fabric_list.entry_count > 0, CHIP_ERROR_INTERNAL);
                fabric_list.entry_count--;
                return fabric_list.Save(storage);
            }
            prev                = fabric;
            fabric.fabric_index = fabric.next;
        }
        // Fabric not in the list
        return CHIP_ERROR_NOT_FOUND;
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
    CHIP_ERROR Save(PersistentStorageDelegate * storage) override
    {
        ReturnErrorOnFailure(Register(storage));
        return PersistentData::Save(storage);
    }

    CHIP_ERROR Delete(PersistentStorageDelegate * storage) override
    {
        ReturnErrorOnFailure(Unregister(storage));
        return PersistentData::Delete(storage);
    }

    CHIP_ERROR SaveScene(PersistentStorageDelegate * storage, const SceneTableEntry & entry)
    {
        CHIP_ERROR err;
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

            return scene.Save(storage);
        }

        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    CHIP_ERROR RemoveScene(PersistentStorageDelegate * storage, const SceneStorageId & scene_id)
    {
        SceneTableData scene(fabric_index, scene_id);
        // Look for empty storage space

        VerifyOrReturnError(this->Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(scene.Delete(storage));

        if (scene_count > 0)
        {
            scene_count--;
            scene_map[scene.index].Clear();
            ReturnErrorOnFailure(this->Save(storage));
        }

        return CHIP_NO_ERROR;
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

/// @brief This function is meant to provide a way to empty the scene table without knowing any specific scene Id. Outisde of this
/// specific use case, RemoveSceneTableEntry should be used.
/// @param fabric_index Fabric in which the scene belongs
/// @param scened_idx Position in the Scene Table
/// @return CHIP_NO_ERROR if removal was successful, errors if failed to remove the scene or to update the fabric after removing it
CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntryAtPosition(FabricIndex fabric_index, SceneIndex scened_idx)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene(fabric_index, scened_idx);

    ReturnErrorOnFailure(scene.Delete(mStorage));

    if (fabric.scene_count > 0)
    {
        fabric.scene_count--;
        ReturnErrorOnFailure(fabric.Save(mStorage));
    }

    return CHIP_NO_ERROR;
}

/// @brief Register a handler in the handler list
/// @param handler Cluster specific handler for extension field sets interaction
/// @return CHIP_NO_ERROR if handler was registered, CHIP_ERROR_NO_MEMORY if the handler list is full
CHIP_ERROR DefaultSceneTableImpl::RegisterHandler(SceneHandler * handler)
{
    CHIP_ERROR err             = CHIP_ERROR_NO_MEMORY;
    uint8_t idPosition         = kInvalidPosition;
    uint8_t fisrtEmptyPosition = kInvalidPosition;

    for (uint8_t i = 0; i < kMaxSceneHandlers; i++)
    {
        if (this->mHandlers[i] == handler)
        {
            idPosition = i;
            break;
        }
        if (this->mHandlers[i] == nullptr && fisrtEmptyPosition == kInvalidPosition)
        {
            fisrtEmptyPosition = i;
        }
    }

    // if found, insert at found position, otherwise at first free possition, otherwise return error
    if (idPosition < kMaxSceneHandlers)
    {
        this->mHandlers[idPosition] = handler;
        err                         = CHIP_NO_ERROR;
    }
    else if (fisrtEmptyPosition < kMaxSceneHandlers)
    {
        this->mHandlers[fisrtEmptyPosition] = handler;
        this->handlerNum++;
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR DefaultSceneTableImpl::UnregisterHandler(uint8_t pos)
{
    VerifyOrReturnError(pos < kMaxSceneHandlers, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(!this->HandlerListEmpty() && !(this->mHandlers[pos] == nullptr), CHIP_NO_ERROR);

    uint8_t nextPos = pos++;
    uint8_t moveNum = kMaxSceneHandlers - nextPos;

    // TODO: Implement general array management methods
    // Compress array after removal
    memmove(&this->mHandlers[pos], &this->mHandlers[nextPos], sizeof(SceneHandler *) * moveNum);

    this->handlerNum--;
    // Clear last occupied position
    this->mHandlers[handlerNum] = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::SceneSaveEFS(SceneTableEntry & scene, clusterId cluster)
{
    ExtensionFieldsSet EFS;
    MutableByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, kMaxFieldsPerCluster);
    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < this->handlerNum; i++)
        {
            if (this->mHandlers[i] != nullptr)
            {
                EFS.mID = cluster;
                ReturnErrorOnFailure(this->mHandlers[i]->SerializeSave(scene.mStorageId.mEndpointId, cluster, EFSSpan));
                EFS.mUsedBytes = (uint8_t) EFSSpan.size();
                ReturnErrorOnFailure(scene.mStorageData.mExtensionFieldsSets.InsertFieldSet(EFS));
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::SceneApplyEFS(FabricIndex fabric_index, const SceneStorageId & scene_id)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene(fabric_index);
    ExtensionFieldsSet EFS;
    TransitionTimeMs time;
    clusterId cluster;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(fabric.Find(scene_id, scene.index) == CHIP_NO_ERROR, CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(scene.Load(mStorage));

    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < scene.mStorageData.mExtensionFieldsSets.GetFieldNum(); i++)
        {
            scene.mStorageData.mExtensionFieldsSets.GetFieldSetAtPosition(EFS, i);
            cluster = EFS.mID;
            time    = scene.mStorageData.mSceneTransitionTime * 1000 +
                (scene.mStorageData.mTransitionTime100ms ? scene.mStorageData.mTransitionTime100ms * 10 : 0);
            ByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, EFS.mUsedBytes);

            if (!EFS.IsEmpty())
            {
                for (uint8_t j = 0; j < this->handlerNum; j++)
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

DefaultSceneTableImpl::SceneEntryIterator * DefaultSceneTableImpl::IterateSceneEntry(FabricIndex fabric_index)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mSceneEntryIterators.CreateObject(*this, fabric_index);
}

DefaultSceneTableImpl::SceneEntryIteratorImpl::SceneEntryIteratorImpl(DefaultSceneTableImpl & provider, FabricIndex fabric_index) :
    mProvider(provider), mFabric(fabric_index)
{
    FabricSceneData fabric(fabric_index);
    ReturnOnFailure(fabric.Load(provider.mStorage));
    mTotalScene = fabric.scene_count;
    mSceneIndex = 0;
}

size_t DefaultSceneTableImpl::SceneEntryIteratorImpl::Count()
{
    return mTotalScene;
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

namespace {

DefaultSceneTableImpl * gSceneTable = nullptr;

} // namespace

DefaultSceneTableImpl * GetSceneTable()
{
    return gSceneTable;
}

void SetSceneTable(DefaultSceneTableImpl * provider)
{
    gSceneTable = provider;
}

} // namespace scenes
} // namespace chip
