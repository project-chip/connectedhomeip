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
#include <app/clusters/scenes-server/ExtensionFieldSetsImpl.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Pool.h>

namespace chip {
namespace scenes {

static constexpr uint16_t kMaxScenesPerFabric   = CHIP_CONFIG_SCENES_MAX_PER_FABRIC;
static constexpr uint16_t kMaxScenesPerEndpoint = CHIP_CONFIG_MAX_SCENES_PER_ENDPOINT;

using clusterId = chip::ClusterId;

/// @brief Default implementation of handler, handle EFS from add scene and view scene commands for any cluster
///        The implementation of SerializeSave and ApplyScene were omitted and must be implemented in a way that
///        is compatible with the SerializeAdd output in order to function with the Default Scene Handler.
///        It is worth noting that this implementation is very memory consuming. In the current worst case,
///        (Color control cluster), the Extension Field Set's value pair list TLV occupies 99 bytes of memory
class DefaultSceneHandlerImpl : public scenes::SceneHandler
{

    template <typename T>
    using List = chip::app::DataModel::List<T>;

    template <typename T>
    using DecodableList = chip::app::DataModel::DecodableList<T>;

    using AttributeValuePairType          = chip::app::Clusters::Scenes::Structs::AttributeValuePair::Type;
    using AttributeValuePairDecodableType = chip::app::Clusters::Scenes::Structs::AttributeValuePair::DecodableType;
    using ExtensionFieldSetDecodableType  = chip::app::Clusters::Scenes::Structs::ExtensionFieldSet::DecodableType;
    using ExtensionFieldSetType           = chip::app::Clusters::Scenes::Structs::ExtensionFieldSet::Type;

public:
    static constexpr uint8_t kMaxAvPair = CHIP_CONFIG_SCENES_MAX_AV_PAIRS_EFS;

    DefaultSceneHandlerImpl() = default;
    ~DefaultSceneHandlerImpl() override{};

    /// @brief Encodes an attribute value list into a TLV structure and resizes the buffer to the size of the encoded data
    /// @param aVlist[in] Attribute value list to encode
    /// @param serializedBytes[out] Buffer to fill from the Attribute value list in a TLV format
    /// @return CHIP_ERROR
    virtual CHIP_ERROR EncodeAttributeValueList(const List<AttributeValuePairType> & aVlist, MutableByteSpan & serializedBytes);

    /// @brief Decodes an attribute value list from a TLV structure and ensure it fits the member pair buffer
    /// @param serializedBytes [in] Buffer to read from
    /// @param aVlist [out] Attribute value list to fill from the TLV structure.  Only valid while the buffer backing
    /// serializedBytes exists and its contents are not modified.
    /// @return CHIP_ERROR
    virtual CHIP_ERROR DecodeAttributeValueList(const ByteSpan & serializedBytes,
                                                DecodableList<AttributeValuePairDecodableType> & aVlist);

    /// @brief From command AddScene, allows handler to filter through clusters in command to serialize only the supported ones.
    /// @param endpoint[in] Endpoint ID
    /// @param extensionFieldSet[in] ExtensionFieldSets provided by the AddScene Command, pre initialized
    /// @param serializedBytes[out] Buffer to fill from the ExtensionFieldSet in command
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR_INVALID_ARGUMENT if the cluster is not supported, CHIP_ERROR value
    /// otherwise
    virtual CHIP_ERROR SerializeAdd(EndpointId endpoint, const ExtensionFieldSetDecodableType & extensionFieldSet,
                                    MutableByteSpan & serializedBytes) override;

    /// @brief Simulates taking data from nvm and loading it in a command object if the cluster is supported by the endpoint
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes data to deserialize into EFS
    /// @return CHIP_NO_ERROR if Extension Field Set was successfully populated, CHIP_ERROR_INVALID_ARGUMENT if the cluster is not
    /// supported, specific CHIP_ERROR otherwise
    virtual CHIP_ERROR Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                   ExtensionFieldSetType & extensionFieldSet) override;

private:
    AttributeValuePairType mAVPairs[kMaxAvPair];
};

/**
 * @brief Implementation of a storage in nonvolatile storage of the scene table.
 *
 * DefaultSceneTableImpl is an implementation that allows to store scenes using PersistentStorageDelegate.
 * It handles the storage of scenes by their ID, GroupID and EnpointID over multiple fabrics.
 * It is meant to be used exclusively when the scene cluster is enable for at least one endpoint
 * on the device.
 */
class DefaultSceneTableImpl : public SceneTable<scenes::ExtensionFieldSetsImpl>
{
public:
    DefaultSceneTableImpl() {}
    ~DefaultSceneTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate * storage) override;
    void Finish() override;

    // Scene count
    CHIP_ERROR GetEndpointSceneCount(uint8_t & scene_count) override;
    CHIP_ERROR GetFabricSceneCount(FabricIndex fabric_index, uint8_t & scene_count) override;

    // Data
    CHIP_ERROR GetRemainingCapacity(FabricIndex fabric_index, uint8_t & capacity) override;
    CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry) override;
    CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry) override;
    CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id) override;
    CHIP_ERROR RemoveSceneTableEntryAtPosition(EndpointId endpoint, FabricIndex fabric_index, SceneIndex scene_idx) override;

    // Groups
    CHIP_ERROR GetAllSceneIdsInGroup(FabricIndex fabric_index, GroupId group_id, Span<SceneId> & scene_list) override;
    CHIP_ERROR DeleteAllScenesInGroup(FabricIndex fabric_index, GroupId group_id) override;

    // SceneHandlers
    void RegisterHandler(SceneHandler * handler) override;
    void UnregisterHandler(SceneHandler * handler) override;
    void UnregisterAllHandlers() override;

    // Extension field sets operation
    CHIP_ERROR SceneSaveEFS(SceneTableEntry & scene) override;
    CHIP_ERROR SceneApplyEFS(const SceneTableEntry & scene) override;

    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index) override;

    // Iterators
    SceneEntryIterator * IterateSceneEntries(FabricIndex fabric_index) override;

    void SetEndpoint(EndpointId endpoint);
    void SetTableSize(uint16_t endpointSceneTableSize);
    bool IsInitialized() { return (mStorage != nullptr); }

protected:
    // This constructor is meant for test purposes, it allows to change the defined max for scenes per fabric and global, which
    // allows to simulate OTA where this value was changed
    DefaultSceneTableImpl(uint16_t maxScenesPerFabric, uint16_t maxScenesPerEndpoint) :
        mMaxScenesPerFabric(maxScenesPerFabric), mMaxScenesPerEndpoint(maxScenesPerEndpoint)
    {}

    // Global scene count
    CHIP_ERROR SetEndpointSceneCount(const uint8_t & scene_count);

    // wrapper function around emberAfGetClustersFromEndpoint to allow override when testing
    virtual uint8_t GetClustersFromEndpoint(ClusterId * clusterList, uint8_t listLen);

    // wrapper function around emberAfGetClusterCountForEndpoint to allow override when testing
    virtual uint8_t GetClusterCountFromEndpoint();

    class SceneEntryIteratorImpl : public SceneEntryIterator
    {
    public:
        SceneEntryIteratorImpl(DefaultSceneTableImpl & provider, FabricIndex fabricIdx, EndpointId endpoint,
                               uint16_t maxScenesPerFabric, uint16_t maxScenesPerEndpoint);
        size_t Count() override;
        bool Next(SceneTableEntry & output) override;
        void Release() override;

    protected:
        DefaultSceneTableImpl & mProvider;
        FabricIndex mFabric  = kUndefinedFabricIndex;
        EndpointId mEndpoint = kInvalidEndpointId;
        SceneIndex mNextSceneIdx;
        SceneIndex mSceneIndex = 0;
        uint8_t mTotalScenes   = 0;
        uint16_t mMaxScenesPerFabric;
        uint16_t mMaxScenesPerEndpoint;
    };

    uint16_t mMaxScenesPerFabric               = kMaxScenesPerFabric;
    uint16_t mMaxScenesPerEndpoint             = kMaxScenesPerEndpoint;
    EndpointId mEndpointId                     = kInvalidEndpointId;
    chip::PersistentStorageDelegate * mStorage = nullptr;
    ObjectPool<SceneEntryIteratorImpl, kIteratorsMax> mSceneEntryIterators;
}; // class DefaultSceneTableImpl

/// @brief Gets a pointer to the instance of Scene Table Impl, providing EndpointId and Table Size for said endpoint
/// @param endpoint Endpoint ID, optional only if getting the Table to initialize it, any storage action will require an endpoint
/// different than kInvalidEndpoint ID and WILL fail if an endpoint wasn't provided here.
/// @param endpointTableSize Size of the scene table for a specific endpoint. Must be lower than restriction set in CHIPConfig to
/// match device restrictions, if it is higher than permitted by CHIPConfig, will be capped at kMaxScenesPerEndpoint
/// @return Pointer to the instance of the Scene Table
DefaultSceneTableImpl * GetSceneTableImpl(EndpointId endpoint        = kInvalidEndpointId,
                                          uint16_t endpointTableSize = kMaxScenesPerEndpoint);

} // namespace scenes
} // namespace chip
