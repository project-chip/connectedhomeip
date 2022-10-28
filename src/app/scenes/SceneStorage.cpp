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

#include <app/PersistentData.h>
#include <app/scenes/SceneStorage.h>
#include <lib/support/CodeUtils.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace scenes {

struct FabricData : public PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagFirstEndpoint() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagEndpointCount() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(3); }

    FabricIndex fabric_index  = kUndefinedFabricIndex;
    EndpointId first_endpoint = kRootEndpointId;
    uint16_t group_count      = 0;
    FabricIndex next          = kUndefinedFabricIndex;

    FabricData() = default;
    FabricData(FabricIndex fabric) : fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        // TODO: fill in logic to update storage key
    }

    void Clear() override
    {
        // TODO: fill in logic to clear the fabric data linked list
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        // TODO: fill in logic for serialize function
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // TODO: fill in logic for deserialize function
    }

    // Register the fabric in the fabrics' linked-list
    CHIP_ERROR Register(PersistentStorageDelegate * storage)
    {
        FabricList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            // New fabric list
            fabric_list.first_fabric = fabric_index;
            fabric_list.fabric_count = 1;
            return fabric_list.Save(storage);
        }
        ReturnErrorOnFailure(err);

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_fabric);
        for (size_t i = 0; i < fabric_list.fabric_count; i++)
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
        this->next               = fabric_list.first_fabric;
        fabric_list.first_fabric = this->fabric_index;
        fabric_list.fabric_count++;
        return fabric_list.Save(storage);
    }

    // Remove the fabric from the fabrics' linked list
    CHIP_ERROR Unregister(PersistentStorageDelegate * storage) const
    {
        FabricList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_fabric);
        FabricData prev;

        for (size_t i = 0; i < fabric_list.fabric_count; i++)
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
                    fabric_list.first_fabric = this->next;
                }
                else
                {
                    // Remove intermediate fabric
                    prev.next = this->next;
                    ReturnErrorOnFailure(prev.Save(storage));
                }
                VerifyOrReturnError(fabric_list.fabric_count > 0, CHIP_ERROR_INTERNAL);
                fabric_list.fabric_count--;
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
        FabricList fabric_list;
        ReturnErrorOnFailure(fabric_list.Load(storage));

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_fabric);

        for (size_t i = 0; i < fabric_list.fabric_count; i++)
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

struct EndpointData : public SceneStorage::FabricEndpoint, PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagFirstScene() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagSceneCount() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(3); }

    EndpointData() = default;
    EndpointData(SceneId scene, GroupId group, EndpointId endpoint) : endpoint_id(endpoint) {}

    EndpointId endpoint_id = kInvalidEndpointId;

    bool operator==(const EndpointData & other) const { return this->endpoint_id == other.endpoint_id; }

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        // TODO: fill in logic to update storage key
    }

    void Clear() override
    {
        // TODO: fill in logic to clear the endpoint data linked list
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        // TODO: fill in logic for serialize function
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // TODO: fill in logic for deserialize function
    }
};

struct SceneTable : public SceneStorage::SceneTableEntry, PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagSceneGroupID() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagSceneID() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagSceneName() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagSceneTransitionTime() { return TLV::ContextTag(4); }
    static constexpr TLV::Tag TagExtensionFieldSets() { return TLV::ContextTag(5); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(5); }

    FabricIndex fabric_index = kUndefinedFabricIndex;
    EndpointId endpoint_id   = kInvalidEndpointId;
    uint16_t endpoint_count  = 0;
    uint16_t index           = 0;
    SceneId next             = 0;
    SceneId prev             = 0;
    bool first               = true;

    SceneId first_scene  = kUndefinedSceneId;
    uint16_t scene_count = 0;

    SceneTable() : SceneTableEntry(nullptr) {}
    SceneTable(FabricIndex fabric, EndpointId endpoint) : fabric_index(fabric), endpoint_id(endpoint) {}
    SceneTable(FabricIndex fabric, EndpointId endpoint, SceneId scene, SceneGroupID groupId) :
        SceneTableEntry(scene, groupId, nullptr), fabric_index(fabric), endpoint_id(endpoint)
    {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        // TODO: fill in logic to update storage key
    }

    void Clear() override
    {
        // TODO: fill in logic to clear the Scene Table
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        // TODO: fill in logic for serialize function
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // TODO: fill in logic for deserialize function
    }

    bool Find(PersistentStorageDelegate * storage, const FabricData & fabric, const EndpointData & endpoint, SceneId target_scene)
    {
        // TODO: fill in logic to fin a scene by fabric and endpoint based on the sceneID
    }
};

SceneStorage::SceneStorage(){
    // TODO: fill in logic for default creator
};

SceneStorage::SceneStorage(uint16_t maxScenesPerFabric){
    // TODO: fill in logic for creator with max scene verification
};

void SceneStorage::SetStorageDelegate(PersistentStorageDelegate * storage)
{
    VerifyOrDie(storage != nullptr);
    mStorage = storage;
}

CHIP_ERROR SceneStorage::Init()
{
    // TODO: fill in logic for initialization
}

void SceneStorage::Finish()
{
    // TODO: fill in logic for clearing the scene storage object
}

CHIP_ERROR SceneStorage::SetSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, const SceneTableEntry & entry)
{
    // TODO: fill in logic to store a scene entry in the storage
}
CHIP_ERROR SceneStorage::GetSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, SceneId scene_id,
                                            SceneTableEntry & entry)
{
    // TODO: fill in logic to get a scene entry from storage
}
CHIP_ERROR SceneStorage::RemoveSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, SceneId scene_id)
{
    // TODO: fill in logic to remove Scene from Table
}

SceneStorage::EndpointIterator * SceneStorage::IterateEndpoint(FabricIndex fabric_index)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mEndpointIterators.CreateObject(*this, fabric_index);
}

SceneStorage::EndpointIterator::EndpointIterator(SceneStorage & provider, FabricIndex fabric_index) :
    mProvider(provider), mFabric(fabric_index)
{
    // TODO: fill in logic to create Endpoint
}

size_t SceneStorage::EndpointIterator::Count()
{
    // TODO: fill in logic to count through endpoint iterator
}

bool SceneStorage::EndpointIterator::Next(FabricEndpoint & output)
{
    // TODO: fill in logic to iterate though endpoints
}

void SceneStorage::EndpointIterator::Release()
{
    // TODO: fill in logic to release iterator
}

SceneStorage::SceneEntryIterator * SceneStorage::IterateSceneEntry(EndpointId endpoint_id)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mSceneEntryIterators.CreateObject(*this, endpoint_id);
}

SceneStorage::SceneEntryIterator::SceneEntryIterator(SceneStorage & provider, FabricIndex fabric_index, EndpointId endpoint_id) :
    mProvider(provider), mFabric(fabric_index), mEndpoint(endpoint_id)
{
    SceneTable scene(fabric_index, endpoint_id);
    if (CHIP_NO_ERROR == scene.Load(provider.mStorage))
    {
        mNextSceneId = scene.first_scene;
        mTotalScene  = scene.scene_count;
        mSceneCount  = 0;
    }
}

size_t SceneStorage::SceneEntryIterator::Count()
{
    return mTotalScene;
}

bool SceneStorage::SceneEntryIterator::Next(SceneTableEntry & output) {}

void SceneStorage::SceneEntryIterator::Release()
{
    // TODO: fill in logic to release iterator
}

} // namespace scenes
} // namespace chip