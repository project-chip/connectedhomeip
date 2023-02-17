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

static constexpr size_t kPersistentBufferMax = 128;

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
    static constexpr TLV::Tag TagSceneCount() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagSceneMap() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(3); }

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

        ReturnErrorOnFailure(writer.Put(TagSceneCount(), static_cast<uint16_t>(scene_count)));
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            ReturnErrorOnFailure(scene_map[i].Serialize(writer));
        }
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TagSceneCount()));
        ReturnErrorOnFailure(reader.Get(scene_count));
        for (uint8_t i = 0; i < kMaxScenePerFabric; i++)
        {
            ReturnErrorOnFailure(scene_map[i].Deserialize(reader));
        }
        ReturnErrorOnFailure(reader.Next(TagNext()));
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
            if (scene_map[index].mSceneEndpointId == kInvalidEndpointId && firstFreeIdx == kUndefinedSceneIndex)
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

/// @brief Registers handle to get extension fields set for a specific cluster. If the handler is already present in the handler
/// array, it will be overwritten
/// @param ID ID of the cluster used to fill the extension fields set
/// @param get_function pointer to function to call to get the extension fiels set from the cluster
/// @param set_function pointer to function to call send an extension field to the cluster
/// @return CHIP_ERROR_NO_MEMORY if couldn't insert the handler, otherwise CHIP_NO_ERROR
CHIP_ERROR DefaultSceneTableImpl::RegisterHandler(ClusterId ID, clusterFieldsHandle get_function, clusterFieldsHandle set_function)
{
    CHIP_ERROR err     = CHIP_ERROR_NO_MEMORY;
    uint8_t idPosition = 0xff, fisrtEmptyPosition = 0xff;
    for (uint8_t i = 0; i < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; i++)
    {
        if (this->handlers[i].GetID() == ID)
        {
            idPosition = i;
            break;
        }
        if (!this->handlers[i].IsInitialized() && fisrtEmptyPosition == 0xff)
        {
            fisrtEmptyPosition = i;
        }
    }

    // if found, insert at found position, otherwise at first free possition, otherwise return error
    if (idPosition < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES)
    {
        this->handlers[idPosition].InitSceneHandler(ID, get_function, set_function);
        err = CHIP_NO_ERROR;
    }
    else if (fisrtEmptyPosition < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES)
    {
        this->handlers[fisrtEmptyPosition].InitSceneHandler(ID, get_function, set_function);
        this->handlerNum++;
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR DefaultSceneTableImpl::UnregisterHandler(uint8_t position)
{
    if (!HandlerListEmpty())
    {
        if (position < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES)
        {
            if (this->handlers[position].IsInitialized())
            {
                this->handlers[position].ClearSceneHandler();
                this->handlerNum--;
            }
        }
        else
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::EFSValuesFromCluster(ExtensionFieldsSetsImpl & fieldSets)
{
    ExtensionFieldsSet EFS;
    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; i++)
        {
            if (this->handlers[i].IsInitialized())
            {
                ReturnErrorOnFailure(this->handlers[i].GetClusterEFS(EFS));
                ReturnErrorOnFailure(fieldSets.InsertFieldSet(EFS));
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneTableImpl::EFSValuesToCluster(ExtensionFieldsSetsImpl & fieldSets)
{
    ExtensionFieldsSet EFS;
    if (!this->HandlerListEmpty())
    {
        for (uint8_t i = 0; i < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; i++)
        {
            fieldSets.GetFieldSetAtPosition(EFS, i);

            if (!EFS.IsEmpty())
            {
                for (uint8_t j = 0; j < CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES; j++)
                {
                    if (EFS.mID == this->handlers[j].GetID())
                    {
                        ReturnErrorOnFailure(this->handlers[j].SetClusterEFS(EFS));
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
        if (fabric.scene_map[mSceneIndex].mSceneEndpointId != kInvalidEndpointId)
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
