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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/scenes/ExtensionFieldSetsImpl.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/CommonPersistentData.h>
#include <lib/support/Span.h>

/**
 * @brief Indicates the absence of a Scene table entry.
 */
#ifndef CHIP_CONFIG_SCENES_TABLE_NULL_INDEX
#define CHIP_CONFIG_SCENES_TABLE_NULL_INDEX 0xFF
#endif

/**
 * @brief The group identifier for the global scene.
 */
#ifndef CHIP_CONFIG_SCENES_GLOBAL_SCENE_GROUP_ID
#define CHIP_CONFIG_SCENES_GLOBAL_SCENE_GROUP_ID 0x0000
#endif

/**
 * @brief The scene identifier for the global scene.
 */
#ifndef CHIP_CONFIG_SCENES_GLOBAL_SCENE_SCENE_ID
#define CHIP_CONFIG_SCENES_GLOBAL_SCENE_SCENE_ID 0x00
#endif

namespace chip {
namespace scenes {

typedef uint32_t TransitionTimeMs;
typedef uint16_t SceneTransitionTime;
typedef uint8_t TransitionTime100ms;

constexpr SceneGroupID kGlobalGroupSceneId  = CHIP_CONFIG_SCENES_GLOBAL_SCENE_GROUP_ID;
constexpr SceneIndex kUndefinedSceneIndex   = 0xff;
constexpr SceneId kUndefinedSceneId         = CHIP_CONFIG_SCENES_TABLE_NULL_INDEX;
static constexpr uint8_t kMaxScenePerFabric = CHIP_CONFIG_SCENES_MAX_PER_FABRIC;
static constexpr uint8_t kMaxSceneHandlers  = CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES;
static constexpr size_t kIteratorsMax       = CHIP_CONFIG_MAX_SCENES_CONCURRENT_ITERATORS;
static constexpr size_t kSceneNameMax       = CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH;

/// @brief Abstract class allowing different Endpoints interactions with the ExtensionFieldSets added to and retrieved from the
/// scene Table
class SceneHandler
{
public:
    SceneHandler(){};
    virtual ~SceneHandler() = default;

    /// @brief Copies the list of supported clusters for an endpoint in a Span and resizes the span to fit the actual number of
    /// supported clusters
    /// @param endpoint target endpoint
    /// @param clusterBuffer Buffer to hold the supported cluster IDs, cannot hold more than
    /// CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES, the function shall use the reduce_size() method in the event it is supporting
    /// less than CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENES clusters
    virtual void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) = 0;

    /// @brief Returns whether or not a cluster for scenes is supported on an endpoint

    /// @param endpoint Target Endpoint ID
    /// @param cluster Target Cluster ID
    /// @return true if supported, false if not supported
    virtual bool SupportsCluster(EndpointId endpoint, ClusterId cluster) = 0;

    /// @brief From command AddScene, allows handler to filter through clusters in command to serialize only the supported ones.
    /// @param endpoint Endpoint ID
    /// @param cluster  Cluster ID to fetch from command
    /// @param serialisedBytes Buffer for ExtensionFieldSet in command
    /// @param extensionFieldSet ExtensionFieldSets provided by the AddScene Command
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    virtual CHIP_ERROR SerializeAdd(EndpointId endpoint, ClusterId & cluster, MutableByteSpan & serialisedBytes,
                                    app::Clusters::Scenes::Structs::ExtensionFieldSet::DecodableType & extensionFieldSet) = 0;

    /// @brief From command StoreScene, retrieves ExtensionField from currently active values, it is the function's responsibility to

    /// place the serialized data in serializedBytes as described below.

    /// @param endpoint Target Endpoint
    /// @param cluster Target Cluster
    /// @param serializedBytes Output buffer, data needs to be writen in there and size adjusted to the size of the data written.

    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    virtual CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) = 0;


    /// @brief From stored scene (e.g. ViewScene), deserialize ExtensionFieldSet into a cluster object
    /// @param endpoint Endpoint ID
    /// @param cluster Cluster ID to save
    /// @param serializedBytes ExtensionFieldSet stored in NVM

    /// @param extensionFieldSet ExtensionFieldSet in command format
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    virtual CHIP_ERROR Deserialize(EndpointId endpoint, ClusterId cluster, ByteSpan & serialisedBytes,
                                   app::Clusters::Scenes::Structs::ExtensionFieldSet::Type & extensionFieldSet) = 0;

    /// @brief From stored scene (e.g RecallScene), applies EFS values to cluster at transition time
    /// @param endpoint Endpoint ID
    /// @param cluster Cluster ID
    /// @param serialisedBytes ExtensionFieldSet stored in NVM
    /// @param timeMs Transition time in ms to apply the scene
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    virtual CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, ByteSpan & serialisedBytes, TransitionTimeMs timeMs) = 0;
};

template <class EFStype>
class SceneTable
{
public:
    /// @brief struct used to identify a scene in storage by 3 ids, endpoint, group and scene
    struct SceneStorageId
    {
        // Identifies endpoint to which this scene applies
        EndpointId mEndpointId = kInvalidEndpointId;
        // Identifies group within the scope of the given fabric
        SceneGroupID mGroupId = kGlobalGroupSceneId;
        SceneId mSceneId      = kUndefinedSceneId;

        SceneStorageId() = default;
        SceneStorageId(EndpointId endpoint, SceneId id, SceneGroupID groupId = kGlobalGroupSceneId) :
            mEndpointId(endpoint), mGroupId(groupId), mSceneId(id)
        {}

        void Clear()
        {
            mEndpointId = kInvalidEndpointId;
            mGroupId    = kGlobalGroupSceneId;
            mSceneId    = kUndefinedSceneId;
        }

        bool operator==(const SceneStorageId & other)
        {
            return (this->mEndpointId == other.mEndpointId && this->mGroupId == other.mGroupId && this->mSceneId == other.mSceneId);
        }
    };

    /// @brief struct used to store data held in a scene
    struct SceneData
    {
        char mName[kSceneNameMax]                       = { 0 };
        size_t mNameLength                              = 0;
        SceneTransitionTime mSceneTransitionTimeSeconds = 0;
        EFStype mExtensionFieldSets;
        TransitionTime100ms mTransitionTime100ms = 0;

        SceneData(const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0, TransitionTime100ms time100ms = 0) :
            mSceneTransitionTimeSeconds(time), mTransitionTime100ms(time100ms)
        {
            this->SetName(sceneName);
        }
        SceneData(EFStype fields, const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0,
                  TransitionTime100ms time100ms = 0) :
            mSceneTransitionTimeSeconds(time),
            mTransitionTime100ms(time100ms)
        {
            this->SetName(sceneName);
            mExtensionFieldSets = fields;
        }
        SceneData(const SceneData & other) :
            mSceneTransitionTimeSeconds(other.mSceneTransitionTimeSeconds), mTransitionTime100ms(other.mTransitionTime100ms)
        {
            this->SetName(CharSpan(other.mName, other.mNameLength));
            mExtensionFieldSets = other.mExtensionFieldSets;
        }
        ~SceneData(){};

        void SetName(const CharSpan & sceneName)
        {
            if (nullptr == sceneName.data())
            {
                mName[0]    = 0;
                mNameLength = 0;
            }
            else
            {
                size_t maxChars = std::min(sceneName.size(), kSceneNameMax);
                memcpy(mName, sceneName.data(), maxChars);
                mNameLength = maxChars;
            }
        }

        void Clear()
        {
            this->SetName(CharSpan());
            mSceneTransitionTimeSeconds = 0;
            mTransitionTime100ms        = 0;
            mExtensionFieldSets.Clear();
        }

        bool operator==(const SceneData & other)
        {
            return (!memcmp(this->mName, other.mName, this->mNameLength) &&
                    (this->mSceneTransitionTimeSeconds == other.mSceneTransitionTimeSeconds) &&
                    (this->mTransitionTime100ms == other.mTransitionTime100ms) &&
                    (this->mExtensionFieldSets == other.mExtensionFieldSets));
        }

        void operator=(const SceneData & other)
        {
            this->SetName(CharSpan(other.mName, other.mNameLength));
            this->mExtensionFieldSets         = other.mExtensionFieldSets;
            this->mSceneTransitionTimeSeconds = other.mSceneTransitionTimeSeconds;
            this->mTransitionTime100ms        = other.mTransitionTime100ms;
        }
    };

    /// @brief Struct combining both ID and data of a table entry
    struct SceneTableEntry
    {

        // ID
        SceneStorageId mStorageId;

        // DATA
        SceneData mStorageData;

        SceneTableEntry() = default;
        SceneTableEntry(SceneStorageId id) : mStorageId(id) {}
        SceneTableEntry(const SceneStorageId id, const SceneData data) : mStorageId(id), mStorageData(data) {}

        bool operator==(const SceneTableEntry & other)
        {
            return (this->mStorageId == other.mStorageId && this->mStorageData == other.mStorageData);
        }

        void operator=(const SceneTableEntry & other)
        {
            this->mStorageId   = other.mStorageId;
            this->mStorageData = other.mStorageData;
        }
    };

    SceneTable(){};

    virtual ~SceneTable() = default;

    // Not copyable
    SceneTable(const SceneTable &) = delete;

    SceneTable & operator=(const SceneTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate * storage) = 0;
    virtual void Finish()                                        = 0;

    // Data
    virtual CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)                    = 0;
    virtual CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry) = 0;
    virtual CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)                       = 0;
    virtual CHIP_ERROR RemoveSceneTableEntryAtPosition(FabricIndex fabric_index, SceneIndex scened_idx)               = 0;

    // SceneHandlers
    virtual CHIP_ERROR RegisterHandler(SceneHandler * handler)   = 0;
    virtual CHIP_ERROR UnregisterHandler(SceneHandler * handler) = 0;
    virtual CHIP_ERROR UnregisterAllHandler()                    = 0;

    // Extension field sets operation
    virtual CHIP_ERROR SceneSaveEFS(SceneTableEntry & scene)                                    = 0;
    virtual CHIP_ERROR SceneApplyEFS(FabricIndex fabric_index, const SceneStorageId & scene_id) = 0;

    // Fabrics
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;

    // Iterators
    using SceneEntryIterator = CommonIterator<SceneTableEntry>;

    virtual SceneEntryIterator * IterateSceneEntry(FabricIndex fabric_index) = 0;

    // Handlers
    virtual bool HandlerListEmpty() { return (mNumHandlers == 0); }
    virtual bool HandlerListFull() { return (mNumHandlers >= kMaxSceneHandlers); }
    virtual uint8_t GetHandlerNum() { return this->mNumHandlers; }

    SceneHandler * mHandlers[kMaxSceneHandlers] = { nullptr };
    uint8_t mNumHandlers                        = 0;
};

} // namespace scenes
} // namespace chip
