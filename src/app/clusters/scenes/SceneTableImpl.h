/**
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

#pragma once
#include <app/clusters/scenes/SceneTable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Pool.h>

namespace chip {
namespace scenes {

using clusterId = chip::ClusterId;

typedef CHIP_ERROR (*clusterFieldsHandle)(ExtensionFieldsSet & fields);

/// @brief Class to allow extension field sets to be handled by the scene table without any knowledge of the cluster or its
/// implementation
class SceneHandler
{
public:
    SceneHandler(ClusterId Id = kInvalidClusterId, clusterFieldsHandle getEFSHandle = nullptr,
                 clusterFieldsHandle setEFSHandle = nullptr)
    {
        if (getEFSHandle != nullptr && setEFSHandle != nullptr && Id != kInvalidClusterId)
        {
            getEFS      = getEFSHandle;
            setEFS      = setEFSHandle;
            cID         = Id;
            initialized = true;
        }
    };
    ~SceneHandler(){};

    void InitSceneHandler(ClusterId Id, clusterFieldsHandle getEFSHandle, clusterFieldsHandle setEFSHandle)
    {
        if (getEFSHandle != nullptr && setEFSHandle != nullptr && Id != kInvalidClusterId)
        {
            getEFS      = getEFSHandle;
            setEFS      = setEFSHandle;
            cID         = Id;
            initialized = true;
        }
    }

    void ClearSceneHandler()
    {
        getEFS      = nullptr;
        setEFS      = nullptr;
        cID         = kInvalidClusterId;
        initialized = false;
    }

    CHIP_ERROR GetClusterEFS(ExtensionFieldsSet & clusterFields)
    {
        if (this->IsInitialized())
        {
            ReturnErrorOnFailure(getEFS(clusterFields));
        }

        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetClusterEFS(ExtensionFieldsSet & clusterFields)
    {
        if (this->IsInitialized())
        {
            ReturnErrorOnFailure(setEFS(clusterFields));
        }

        return CHIP_NO_ERROR;
    }

    bool IsInitialized() const { return this->initialized; }

    ClusterId GetID() { return cID; }

    bool operator==(const SceneHandler & other)
    {
        return (this->getEFS == other.getEFS && this->setEFS == other.setEFS && this->cID == other.cID &&
                initialized == other.initialized);
    }
    void operator=(const SceneHandler & other)
    {
        this->getEFS      = other.getEFS;
        this->setEFS      = other.setEFS;
        this->cID         = other.cID;
        this->initialized = true;
    }

protected:
    clusterFieldsHandle getEFS = nullptr;
    clusterFieldsHandle setEFS = nullptr;
    ClusterId cID              = kInvalidClusterId;
    bool initialized           = false;
};

/**
 * @brief Implementation of a storage in nonvolatile storage of the scene table.
 *
 * DefaultSceneTableImpl is an implementation that allows to store scenes using PersistentStorageDelegate.
 * It handles the storage of scenes by their ID, GroupID and EnpointID over multiple fabrics.
 * It is meant to be used exclusively when the scene cluster is enable for at least one endpoint
 * on the device.
 */
class DefaultSceneTableImpl : public SceneTable
{
public:
    DefaultSceneTableImpl() = default;

    ~DefaultSceneTableImpl() override {}

    CHIP_ERROR Init(PersistentStorageDelegate * storage) override;
    void Finish() override;

    // Scene access by Id
    CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry) override;
    CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, DefaultSceneTableImpl::SceneStorageId scene_id,
                                  SceneTableEntry & entry) override;
    CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, DefaultSceneTableImpl::SceneStorageId scene_id) override;
    CHIP_ERROR RemoveSceneTableEntryAtPosition(FabricIndex fabric_index, SceneIndex scened_idx) override;

    // SceneHandlers
    CHIP_ERROR RegisterHandler(ClusterId ID, clusterFieldsHandle get_function, clusterFieldsHandle set_function);
    CHIP_ERROR UnregisterHandler(uint8_t position);

    // Extension field sets operation
    CHIP_ERROR EFSValuesFromCluster(ExtensionFieldsSetsImpl & fieldSets);
    CHIP_ERROR EFSValuesToCluster(ExtensionFieldsSetsImpl & fieldSets);

    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index) override;

    // Iterators
    SceneEntryIterator * IterateSceneEntry(FabricIndex fabric_index) override;

    bool HandlerListEmpty() { return (handlerNum == 0); }
    bool HandlerListFull() { return (handlerNum >= CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES); }
    uint8_t GetHandlerNum() { return this->handlerNum; }

protected:
    class SceneEntryIteratorImpl : public SceneEntryIterator
    {
    public:
        SceneEntryIteratorImpl(DefaultSceneTableImpl & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(SceneTableEntry & output) override;
        void Release() override;

    protected:
        DefaultSceneTableImpl & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        SceneIndex mNextSceneIdx;
        SceneIndex mSceneIndex = 0;
        uint8_t mTotalScene    = 0;
    };
    bool IsInitialized() { return (mStorage != nullptr); }

    chip::PersistentStorageDelegate * mStorage = nullptr;
    ObjectPool<SceneEntryIteratorImpl, kIteratorsMax> mSceneEntryIterators;
    SceneHandler handlers[CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES];
    uint8_t handlerNum = 0;
}; // class DefaultSceneTableImpl

/**
 * Instance getter for the global SceneTable.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Scene Table
 */
DefaultSceneTableImpl * GetSceneTable();

/**
 * Instance setter for the global Scene Table.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * The `provider` can be set to nullptr if the owner is done with it fully.
 *
 * @param[in] provider pointer to the Scene Table global instance to use
 */
void SetSceneTable(DefaultSceneTableImpl * provider);
} // namespace scenes
} // namespace chip
