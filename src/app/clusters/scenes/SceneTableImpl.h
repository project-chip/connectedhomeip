/**
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

#pragma once
#include <app/clusters/scenes/SceneTable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Pool.h>

namespace chip {
namespace scenes {

/**
 * @brief Implementation of a storage in permanent memory of the scene table.
 *
 * SceneTableImpl is an implementation that allows to store scenes in a permenent manner.
 * It handles the storage of scenes by their ID, GroupID and EnpointID over multiple fabrics.
 * It is meant to be used exclusively when the scene cluster is enable for at least one endpoint
 * on the device.
 */
class SceneTableImpl : public SceneTable
{
public:
    SceneTableImpl() = default;

    ~SceneTableImpl() override {}
    /**
     * @brief Set the storage implementation used for non-volatile storage of configuration data.
     *        This method MUST be called before Init().
     *
     * @param storage Pointer to storage instance to set. Cannot be nullptr, will assert.
     */
    void SetStorageDelegate(PersistentStorageDelegate * storage);

    CHIP_ERROR Init();
    void Finish();

    //
    // Scene Data
    //

    // By id
    CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry);
    CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, SceneTableImpl::SceneStorageId scene_id, SceneTableEntry & entry);
    CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, SceneTableImpl::SceneStorageId scene_id);

    // Iterators
    SceneEntryIterator * IterateSceneEntry(FabricIndex fabric_index) override;

protected:
    class SceneEntryIteratorImpl : public SceneEntryIterator
    {
    public:
        SceneEntryIteratorImpl(SceneTableImpl & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(SceneTableEntry & output) override;
        void Release() override;

    protected:
        SceneTableImpl & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        SceneStorageId mNextSceneId;
        size_t mSceneCount = 0;
        size_t mTotalScene = 0;
    };
    bool IsInitialized() { return (mStorage != nullptr); }

    chip::PersistentStorageDelegate * mStorage = nullptr;
    ObjectPool<SceneEntryIteratorImpl, kIteratorsMax> mSceneEntryIterators;

    const uint8_t mMaxScenePerFabric = kMaxScenePerFabric;
}; // class SceneTableImpl

/**
 * Instance getter for the global SceneTable.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Scene Table
 */
SceneTableImpl * GetSceneTable();

/**
 * Instance setter for the global Scene Table.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * The `provider` can be set to nullptr if the owner is done with it fully.
 *
 * @param[in] provider pointer to the Scene Table global isntance to use
 */
void SetSceneTable(SceneTableImpl * provider);
} // namespace scenes
} // namespace chip
