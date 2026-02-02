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
#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/storage/FabricTableImpl.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Pool.h>

namespace chip {
namespace scenes {

/**
 * @brief Implementation of a storage in nonvolatile storage of the scene table.
 *
 * DefaultSceneTableImpl is an implementation that allows to store scenes using PersistentStorageDelegate.
 * It handles the storage of scenes by their ID, GroupId and EnpointId over multiple fabrics.
 * It is meant to be used exclusively when the scene cluster is enable for at least one endpoint
 * on the device.
 */
using SceneTableBase = SceneTable<scenes::ExtensionFieldSetsImpl>;
class DefaultSceneTableImpl : public SceneTableBase,
                              public app::Storage::FabricTableImpl<SceneTableBase::SceneStorageId, SceneTableBase::SceneData>
{
public:
    using Super = app::Storage::FabricTableImpl<SceneTableBase::SceneStorageId, SceneTableBase::SceneData>;

    DefaultSceneTableImpl() : Super(kMaxScenesPerFabric, kMaxScenesPerEndpoint) {}
    ~DefaultSceneTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate & storage, app::DataModel::Provider & dataModel) override;
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
    CHIP_ERROR RemoveEndpoint() override;

    uint16_t GetTableSize() const override { return mCurrentTableSize; }

    void SetTableSize(uint16_t endpointSceneTableSize);

protected:
    // This constructor is meant for test purposes, it allows to change the defined max for scenes per fabric and global, which
    // allows to simulate OTA where this value was changed
    DefaultSceneTableImpl(uint16_t maxScenesPerFabric, uint16_t maxScenesPerEndpoint) :
        FabricTableImpl(maxScenesPerFabric, maxScenesPerEndpoint)
    {}

    // Endpoint scene count
    CHIP_ERROR SetEndpointSceneCount(const uint8_t & scene_count);

    // List clusters available on the current endpoint. Virtual to allow for more unit test logic.
    //
    // TODO: control over this SHOULD be done via the datamodel provider instead (use that for testing).
    virtual CHIP_ERROR ServerClusters(ReadOnlyBufferBuilder<app::DataModel::ServerClusterEntry> & builder);

private:
    app::DataModel::Provider * mDataModel = nullptr;
    uint16_t mCurrentTableSize            = kMaxScenesPerEndpoint;
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
