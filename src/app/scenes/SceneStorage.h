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
#include <app/PersistentData.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/SceneId.h>
#include <lib/support/Pool.h>

namespace chip {
namespace scenes {

class SceneStorage
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

    struct FabricEndpoint
    {
        FabricEndpoint() = default;
        FabricEndpoint(EndpointId endpoint) : endpoint_id(endpoint) {}

        // Endpoint on the fabric where the scene message has been sent
        EndpointId endpoint_id = kInvalidEndpointId;

        bool operator==(const FabricEndpoint & other) const { return this->endpoint_id == other.endpoint_id; }
    };

    struct SceneTableEntry
    {
        static constexpr size_t kSceneNameMax = ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH;

        // Identifies group within the scope of the given Fabric
        SceneGroupID sceneGroupId = kGlobalGroupSceneId;
        SceneId sceneId           = kUndefinedSceneId;
        // Lastest group name written for a given GroupId on any Endpoint via the Groups cluster
        char name[kSceneNameMax + 1]            = { 0 };
        SceneTransitionTime sceneTransitionTime = 0;
        ExtensionFieldsSets extentsionFieldsSets;

        SceneTableEntry() { SetName(nullptr); }
        SceneTableEntry(const char * sceneName) { SetName(sceneName); }
        SceneTableEntry(const CharSpan & sceneName) { SetName(sceneName); }
        SceneTableEntry(SceneId id, SceneGroupID groupId) : sceneGroupId(groupId), sceneId(id) { SetName(nullptr); }
        SceneTableEntry(SceneId id, SceneGroupID groupId, const char * sceneName) : sceneGroupId(groupId), sceneId(id)
        {
            SetName(sceneName);
        }
        SceneTableEntry(SceneId id, SceneGroupID groupId, const CharSpan & sceneName) : sceneGroupId(groupId), sceneId(id)
        {
            SetName(sceneName);
        }
        void SetName(const char * sceneName)
        {
            if (nullptr == sceneName)
            {
                name[0] = 0;
            }
            else
            {
                Platform::CopyString(name, sceneName);
            }
        }
        void SetName(const CharSpan & sceneName)
        {
            if (nullptr == sceneName.data())
            {
                name[0] = 0;
            }
            else
            {
                Platform::CopyString(name, sceneName);
            }
        }
        bool operator==(const SceneTableEntry & other)
        {
            return (this->sceneId == other.sceneId) &&
                !strncmp(this->name, other.name, kSceneNameMax && this->sceneGroupId == other.sceneGroupId);
        }
    };

    class EndpointIterator : public Iterator<FabricEndpoint>
    {
    public:
        EndpointIterator(SceneStorage & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(FabricEndpoint & output) override;
        void Release() override;

    protected:
        SceneStorage & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        EndpointId mFirstEndpoint;
        size_t mEndpointCount  = 0;
        size_t mEndpointIndex  = 0;
        size_t mNextEndpointID = 0;
        bool mFirst            = true;
    };

    class SceneEntryIterator : public Iterator<SceneTableEntry>
    {
    public:
        SceneEntryIterator(SceneStorage & provider, FabricIndex fabric_index, EndpointId endpoint_id);
        size_t Count() override;
        bool Next(SceneTableEntry & output) override;
        void Release() override;

    protected:
        SceneStorage & mProvider;
        FabricIndex mFabric   = kUndefinedFabricIndex;
        EndpointId mEndpoint  = kUnusedEndpointId;
        SceneId mFirstScene   = kUndefinedSceneId;
        size_t mSceneCount    = 0;
        uint16_t mNextSceneId = 0;
        size_t mTotalScene    = 0;
    };

    SceneStorage() = default;
    SceneStorage(uint16_t maxScenesPerFabric);

    ~SceneStorage() = default;
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
    CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, const SceneTableEntry & entry);
    CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, SceneId scene_id, SceneTableEntry & entry);
    CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, EndpointId endpoint_id, SceneId scene_id);

    // Iterators
    EndpointIterator * IterateEndpoint(FabricIndex fabric_index);
    SceneEntryIterator * IterateSceneEntry(EndpointId endpoint_id);

protected:
    bool IsInitialized() { return (mStorage != nullptr); }

    chip::PersistentStorageDelegate * mStorage = nullptr;
    ObjectPool<SceneEntryIterator, kIteratorsMax> mSceneEntryIterators;
    ObjectPool<EndpointIterator, kIteratorsMax> mEndpointIterators;
}; // class SceneStorage
} // namespace scenes
} // namespace chip