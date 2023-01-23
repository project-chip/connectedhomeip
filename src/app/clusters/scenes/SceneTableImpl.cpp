/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        https://urldefense.com/v3/__http://www.apache.org/licenses/LICENSE-2.0__;!!N30Cs7Jr!UgbMbEQ59BIK-1Xslc7QXYm0lQBh92qA3ElecRe1CF_9YhXxbwPOZa6j4plru7B7kCJ7bKQgHxgQrket3-Dnk268sIdA7Qb8$
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/scenes/SceneTableImpl.h>
#include <lib/support/CommonPersistentData.h>
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

/**
 * @brief Linked list of all scenes in a fabric, stored in persistent memory
 *
 * FabricSceneData is an access to a linked list of scenes
 */
struct FabricSceneData : public PersistentData<kPersistentBufferMax>
{
    // static constexpr TLV::Tag TagTableID() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagSceneCount() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(2); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    DefaultSceneTableImpl::SceneStorageId first_scene;
    uint16_t scene_count = 0;
    FabricIndex next     = kUndefinedFabricIndex;

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
        first_scene.Clear();
        scene_count = 0;
        next        = kUndefinedFabricIndex;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(first_scene.Serialize(writer));
        ReturnErrorOnFailure(writer.Put(TagSceneCount(), static_cast<uint16_t>(scene_count)));
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(first_scene.Deserialize(reader));
        ReturnErrorOnFailure(reader.Next(TagSceneCount()));
        ReturnErrorOnFailure(reader.Get(scene_count));
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

    // Check the fabric is registered in the fabrics' linked list
    CHIP_ERROR Validate(PersistentStorageDelegate * storage) const
    {
        FabricHavingSceneList fabric_list;
        ReturnErrorOnFailure(fabric_list.Load(storage));

        // Existing fabric list, search for existing entry
        FabricSceneData fabric(fabric_list.first_entry);

        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            ReturnErrorOnFailure(fabric.Load(storage));
            if (fabric.fabric_index == this->fabric_index)
            {
                return CHIP_NO_ERROR;
            }
            fabric.fabric_index = fabric.next;
        }
        // Fabric not in the list
        return CHIP_ERROR_NOT_FOUND;
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
};

struct SceneTableData : public SceneTableEntry, PersistentData<kPersistentBufferMax>
{
    FabricIndex fabric_index = kUndefinedFabricIndex;
    uint8_t index            = 0;
    SceneStorageId next;
    SceneStorageId prev;
    bool first = true;

    SceneTableData() : SceneTableEntry() {}
    SceneTableData(FabricIndex fabric) : fabric_index(fabric) {}
    SceneTableData(FabricIndex fabric, SceneStorageId storageId) : SceneTableEntry(storageId), fabric_index(fabric) {}
    SceneTableData(FabricIndex fabric, SceneStorageId storageId, SceneData data) :
        SceneTableEntry(storageId, data), fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricSceneKey(fabric_index, storageId.sceneEndpointId, storageId.sceneGroupId,
                                                         storageId.sceneId);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        storageData.Clear();
        next.Clear();
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(storageId.Serialize(writer));
        ReturnErrorOnFailure(storageData.Serialize(writer));
        ReturnErrorOnFailure(next.Serialize(writer));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(storageId.Deserialize(reader));
        ReturnErrorOnFailure(storageData.Deserialize(reader));
        ReturnErrorOnFailure(next.Deserialize(reader));

        return reader.ExitContainer(container);
    }

    bool Find(PersistentStorageDelegate * storage, const FabricSceneData & fabric,
              DefaultSceneTableImpl::SceneStorageId target_scene)
    {
        fabric_index = fabric.fabric_index;
        storageId    = fabric.first_scene;
        index        = 0;
        first        = true;

        while (index < fabric.scene_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (storageId == target_scene)
            {
                // Target index found
                return true;
            }
            first     = false;
            prev      = storageId;
            storageId = next;
            index++;
        }
        return false;
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
    SceneTableData scene;

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (scene.Find(mStorage, fabric, entry.storageId))
    {
        // Existing scene
        scene.storageData = entry.storageData;
        return scene.Save(mStorage);
    }

    scene.storageId   = entry.storageId;
    scene.storageData = entry.storageData;

    VerifyOrReturnError(fabric.scene_count < mMaxScenePerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);
    fabric.scene_count++;

    if (scene.first)
    {
        fabric.first_scene = scene.storageId;
    }
    else
    {
        // Update previous scene's next
        SceneTableData prev(fabric_index, scene.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = scene.storageId;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }

    ReturnErrorOnFailure(fabric.Save(mStorage));

    return scene.Save(mStorage);
}
CHIP_ERROR DefaultSceneTableImpl::GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(scene.Find(mStorage, fabric, scene_id), CHIP_ERROR_NOT_FOUND);

    entry.storageId   = scene.storageId;
    entry.storageData = scene.storageData;
    return CHIP_NO_ERROR;
}
CHIP_ERROR DefaultSceneTableImpl::RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)
{
    FabricSceneData fabric(fabric_index);
    SceneTableData scene;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(scene.Find(mStorage, fabric, scene_id), CHIP_ERROR_NOT_FOUND);

    ReturnErrorOnFailure(scene.Delete(mStorage));
    if (scene.first)
    {
        // Remove first scene
        fabric.first_scene = scene.next;
    }
    else
    {
        // Update previous scene's next
        SceneTableData prev(fabric_index, scene.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = scene.next;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    if (fabric.scene_count > 0)
    {
        fabric.scene_count--;
    }

    // Update fabric info
    ReturnErrorOnFailure(fabric.Save(mStorage));

    return CHIP_NO_ERROR;
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
    if (CHIP_NO_ERROR == fabric.Load(provider.mStorage))
    {

        mNextSceneId = fabric.first_scene;
        mTotalScene  = fabric.scene_count;
        mSceneCount  = 0;
    }
}

size_t DefaultSceneTableImpl::SceneEntryIteratorImpl::Count()
{
    return mTotalScene;
}

bool DefaultSceneTableImpl::SceneEntryIteratorImpl::Next(SceneTableEntry & output)
{
    VerifyOrReturnError(mSceneCount < mTotalScene, false);

    SceneTableData scene(mFabric, mNextSceneId);
    VerifyOrReturnError(CHIP_NO_ERROR == scene.Load(mProvider.mStorage), false);

    mSceneCount++;
    mNextSceneId       = scene.next;
    output.storageId   = scene.storageId;
    output.storageData = scene.storageData;
    return true;
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
