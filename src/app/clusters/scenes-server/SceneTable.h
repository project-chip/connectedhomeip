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
#include <app/clusters/scenes-server/ExtensionFieldSets.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Span.h>

namespace chip {
namespace scenes {

// Storage index for scenes in nvm
typedef uint16_t SceneIndex;

typedef uint32_t TransitionTimeMs;
typedef uint32_t SceneTransitionTime;

inline constexpr GroupId kGlobalGroupSceneId     = 0x0000;
inline constexpr SceneIndex kUndefinedSceneIndex = 0xffff;
inline constexpr SceneId kUndefinedSceneId       = 0xff;

static constexpr size_t kIteratorsMax            = CHIP_CONFIG_MAX_SCENES_CONCURRENT_ITERATORS;
static constexpr size_t kSceneNameMaxLength      = CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH;
static constexpr size_t kScenesMaxTransitionTime = 60'000'000u;

/// @brief SceneHandlers are meant as interface between various clusters and the Scene table.
/// When a scene command involving extension field sets is received, the Scene Table will go through
/// the list of handlers to either retrieve, populate or apply those extension field sets.
///
/// Generally, for each specific <endpoint, cluster> pair there should be one and only one handler
/// registered with the scene table that claims to handle that pair.
///
/// A SceneHandler can handle a single <endpoint, cluster> pair, or many such pairs.
///
/// @note If more than one handler claims to handle a specific <endpoint, cluster> pair, only one of
/// those handlers will get called when executing actions related to extension field sets on the scene
/// table.  It is not defined which handler will be selected.

class SceneHandler : public IntrusiveListNodeBase<>
{
public:
    SceneHandler(){};
    virtual ~SceneHandler() = default;

    /// @brief Copies the list of supported clusters for an endpoint in a Span and resizes the span to fit the actual number of
    /// supported clusters
    /// @param endpoint target endpoint
    /// @param clusterBuffer Buffer to hold the supported cluster IDs, cannot hold more than
    /// CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE. The function shall use the reduce_size() method in the event it is supporting
    /// less than CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE clusters.
    virtual void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) = 0;

    /// @brief Returns whether or not a cluster for scenes is supported on an endpoint
    ///
    /// @param endpoint Target Endpoint ID
    /// @param cluster Target Cluster ID
    /// @return true if supported, false if not supported
    virtual bool SupportsCluster(EndpointId endpoint, ClusterId cluster) = 0;

    /// @brief Called when handling AddScene.  Allows the handler to filter through the clusters in the command to serialize only
    /// the supported ones.
    ///
    /// @param endpoint[in] Endpoint ID
    /// @param extensionFieldSet[in] ExtensionFieldSets provided by the AddScene Command, pre initialized
    /// @param serialisedBytes[out] Buffer to fill from the ExtensionFieldSet in command
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    /// @note Only gets called after the scene-cluster has previously verified that the endpoint,cluster pair is supported by
    /// the handler. It is therefore the implementation's reponsibility to also implement the SupportsCluster method.
    virtual CHIP_ERROR
    SerializeAdd(EndpointId endpoint,
                 const app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType & extensionFieldSet,
                 MutableByteSpan & serialisedBytes) = 0;

    /// @brief Called when handling StoreScene, and only if the handler supports the given endpoint and cluster.
    ///
    /// The implementation must write the actual scene data to store to serializedBytes as described below.
    ///
    /// @param endpoint[in] Target Endpoint
    /// @param cluster[in] Target Cluster
    /// @param serializedBytes[out] Output buffer, data needs to be writen in there and size adjusted to the size of the data
    /// written.
    ///
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    virtual CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) = 0;

    /// @brief Deserialize an ExtensionFieldSet into a cluster object (e.g. when handling ViewScene).
    ///
    /// @param endpoint[in] Endpoint ID
    /// @param cluster[in] Cluster ID
    /// @param serializedBytes[in] ExtensionFieldSet stored in NVM
    ///
    /// @param extensionFieldSet[out] ExtensionFieldSet in command format
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    /// @note Only gets called for handlers for which SupportsCluster() is true for the given endpoint and cluster.
    virtual CHIP_ERROR Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,

                                   app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type & extensionFieldSet) = 0;

    /// @brief Restore a stored scene for the given cluster instance, over timeMs milliseconds (e.g. when handling RecallScene)
    ///
    /// @param endpoint[in] Endpoint ID
    /// @param cluster[in] Cluster ID
    /// @param serializedBytes[in] ExtensionFieldSet stored in NVM
    ///
    /// @param timeMs[in] Transition time in ms to apply the scene
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR value otherwise
    /// @note Only gets called for handlers for which SupportsCluster() is true for the given endpoint and cluster.
    virtual CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                  TransitionTimeMs timeMs) = 0;
};

template <class EFStype>
class SceneTable
{
public:
    /// @brief struct used to identify a scene in storage by 3 ids, endpoint, group and scene
    struct SceneStorageId
    {
        // Identifies group within the scope of the given fabric
        GroupId mGroupId = kGlobalGroupSceneId;
        SceneId mSceneId = kUndefinedSceneId;

        SceneStorageId() = default;
        SceneStorageId(SceneId id, GroupId groupId = kGlobalGroupSceneId) : mGroupId(groupId), mSceneId(id) {}

        void Clear()
        {
            mGroupId = kGlobalGroupSceneId;
            mSceneId = kUndefinedSceneId;
        }

        bool IsValid() { return (mSceneId != kUndefinedSceneId); }

        bool operator==(const SceneStorageId & other) const { return (mGroupId == other.mGroupId && mSceneId == other.mSceneId); }
    };

    /// @brief struct used to store data held in a scene
    /// Members:
    /// mName: char buffer holding the name of the scene, only serialized when mNameLenght is greater than 0
    /// mNameLength: lentgh of the name if a name was provided at scene creation
    /// mSceneTransitionTimeSeconds: Time in seconds it will take a cluster to change to the scene
    /// mExtensionFieldSets: class holding the different field sets of each cluster values to store with the scene
    /// mTransitionTime100ms: Transition time in tenths of a second, allows for more precise transition when combiened with
    /// mSceneTransitionTimeSeconds in enhanced scene commands
    struct SceneData
    {
        char mName[kSceneNameMaxLength]            = { 0 };
        size_t mNameLength                         = 0;
        SceneTransitionTime mSceneTransitionTimeMs = 0;
        EFStype mExtensionFieldSets;

        SceneData(const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0) : mSceneTransitionTimeMs(time)
        {
            SetName(sceneName);
        }
        SceneData(EFStype fields, const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0) :
            mSceneTransitionTimeMs(time)
        {
            SetName(sceneName);

            mExtensionFieldSets = fields;
        }
        SceneData(const SceneData & other) : mSceneTransitionTimeMs(other.mSceneTransitionTimeMs)
        {
            SetName(CharSpan(other.mName, other.mNameLength));

            mExtensionFieldSets = other.mExtensionFieldSets;
        }
        ~SceneData(){};

        bool operator==(const SceneData & other) const
        {
            return ((CharSpan(mName, mNameLength).data_equal(CharSpan(other.mName, other.mNameLength))) &&
                    (mSceneTransitionTimeMs == other.mSceneTransitionTimeMs) && (mExtensionFieldSets == other.mExtensionFieldSets));
        }

        void SetName(const CharSpan & sceneName)
        {
            if (nullptr == sceneName.data())
            {
                mName[0]    = 0;
                mNameLength = 0;
            }
            else
            {
                size_t maxChars = std::min(sceneName.size(), kSceneNameMaxLength);
                memcpy(mName, sceneName.data(), maxChars);
                mNameLength = maxChars;
            }
        }

        void Clear()
        {
            SetName(CharSpan());
            mSceneTransitionTimeMs = 0;
            mExtensionFieldSets.Clear();
        }

        void operator=(const SceneData & other)
        {
            SetName(CharSpan(other.mName, other.mNameLength));
            mExtensionFieldSets    = other.mExtensionFieldSets;
            mSceneTransitionTimeMs = other.mSceneTransitionTimeMs;
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

        bool operator==(const SceneTableEntry & other) const
        {
            return (mStorageId == other.mStorageId && mStorageData == other.mStorageData);
        }

        void operator=(const SceneTableEntry & other)
        {
            mStorageId   = other.mStorageId;
            mStorageData = other.mStorageData;
        }
    };

    SceneTable(){};

    virtual ~SceneTable(){};

    // Not copyable
    SceneTable(const SceneTable &) = delete;

    SceneTable & operator=(const SceneTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate * storage) = 0;
    virtual void Finish()                                        = 0;

    // Global scene count
    virtual CHIP_ERROR GetEndpointSceneCount(uint8_t & scene_count)                         = 0;
    virtual CHIP_ERROR GetFabricSceneCount(FabricIndex fabric_index, uint8_t & scene_count) = 0;

    // Data
    virtual CHIP_ERROR GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity)                                   = 0;
    virtual CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)                          = 0;
    virtual CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry)       = 0;
    virtual CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)                             = 0;
    virtual CHIP_ERROR RemoveSceneTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index, SceneIndex scene_idx) = 0;

    // Groups
    virtual CHIP_ERROR GetAllSceneIdsInGroup(FabricIndex fabric_index, GroupId group_id, Span<SceneId> & scene_list) = 0;
    virtual CHIP_ERROR DeleteAllScenesInGroup(FabricIndex fabric_index, GroupId group_id)                            = 0;

    // SceneHandlers
    virtual void RegisterHandler(SceneHandler * handler)   = 0;
    virtual void UnregisterHandler(SceneHandler * handler) = 0;
    virtual void UnregisterAllHandlers()                   = 0;

    // Extension field sets operation
    virtual CHIP_ERROR SceneSaveEFS(SceneTableEntry & scene)        = 0;
    virtual CHIP_ERROR SceneApplyEFS(const SceneTableEntry & scene) = 0;

    // Fabrics

    /**
     * @brief Removes all scenes associated with a fabric index and the stored FabricSceneData that maps them
     * @param fabric_index Fabric index to remove
     * @return CHIP_ERROR, CHIP_NO_ERROR if successful or if the Fabric was not found, specific CHIP_ERROR otherwise
     * @note This function is meant to be used after a fabric is removed from the device, the implementation MUST ensure that it
     * won't interact with the actual fabric table as it will be removed beforehand.
     */
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;
    virtual CHIP_ERROR RemoveEndpoint()                       = 0;

    // Iterators
    using SceneEntryIterator = CommonIterator<SceneTableEntry>;

    virtual SceneEntryIterator * IterateSceneEntries(FabricIndex fabric_index) = 0;

    // Handlers
    virtual bool HandlerListEmpty() { return mHandlerList.Empty(); }

    IntrusiveList<SceneHandler> mHandlerList;
};

} // namespace scenes
} // namespace chip
