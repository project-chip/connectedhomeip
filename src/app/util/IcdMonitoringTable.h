/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentData.h>
#include <stddef.h>

namespace chip {

constexpr size_t kIcdMonitoringBufferSize = 40;

struct IcdMonitoringEntry : public PersistentData<kIcdMonitoringBufferSize>,
                            chip::app::Clusters::IcdManagement::Structs::MonitoringRegistrationStruct::Type
{
    static constexpr size_t kKeyMaxSize = 16;

    IcdMonitoringEntry(FabricIndex fabric = kUndefinedFabricIndex, NodeId nodeId = kUndefinedNodeId)
    {
        this->fabricIndex      = fabric;
        this->checkInNodeID    = nodeId;
        this->monitoredSubject = nodeId;
    }
    bool IsValid() { return this->checkInNodeID != kUndefinedNodeId && this->fabricIndex != kUndefinedFabricIndex; }

    CHIP_ERROR UpdateKey(StorageKeyName & key) override;
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override;
    void Clear() override;

    uint16_t index = 0;
};

/**
 * @brief IcdMonitoringTable exists to manage the persistence of entries in the IcdManagement Cluster.
 *        To access persisted data with the IcdMonitoringTable class, instantiate an instance of this class
 *        and call the LoadFromStorage function.
 *
 *        This class can only manage one fabric at a time. The flow is load a fabric, execute necessary operations,
 *        save it if there are any changes and load another fabric.
 *
 *        Issue to refactor the class to use one entry for the entire table
 *        https://github.com/project-chip/connectedhomeip/issues/24288
 */

struct IcdMonitoringTable
{
    IcdMonitoringTable(PersistentStorageDelegate & storage, FabricIndex fabric, uint16_t limit) :
        mStorage(&storage), mFabric(fabric), mLimit(limit)
    {}

    /**
     * @brief Returns the MonitoringRegistrationStruct entry at the given position.
     * @param index Zero-based position within the RegisteredClients table.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given index.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if index is greater than the index of the last entry on the table.
     */
    CHIP_ERROR Get(uint16_t index, IcdMonitoringEntry & entry) const;

    /**
     * @brief Stores the MonitoringRegistrationStruct entry at the given position,
     *        overwriting any existing entry.
     * @param index Zero-based position within the RegisteredClients table.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given index.
     * @return CHIP_NO_ERROR on success
     */
    CHIP_ERROR Set(uint16_t index, const IcdMonitoringEntry & entry);

    /**
     * @brief Search the registered clients for an entry on the fabric whose checkInNodeID matches the given id.
     * @param id    NodeId to match.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given node ID.
     *  If found, entry.index contains the position of the entry in the table.
     *  If CHIP_ERROR_NOT_FOUND is returned, entry.index contains the total number of entries in the table.
     * @return CHIP_NO_ERROR if found, CHIP_ERROR_NOT_FOUND if no checkInNodeID matches the provided id.
     */
    CHIP_ERROR Find(NodeId id, IcdMonitoringEntry & entry);

    /**
     * @brief Removes the MonitoringRegistrationStruct entry at the given position,
     *        shifting down the upper entries.
     * @param index Zero-based position within the RegisteredClients table.
     * @return CHIP_NO_ERROR on success
     */
    CHIP_ERROR Remove(uint16_t index);

    /**
     * @brief Removes all the entries for the current fabricIndex.
     * @return CHIP_NO_ERROR on success
     */
    CHIP_ERROR RemoveAll();

    /**
     * @return Maximum number of entries allowed in the RegisteredClients table.
     */
    uint16_t Limit() const;

private:
    PersistentStorageDelegate * mStorage;
    FabricIndex mFabric;
    uint16_t mLimit = 0;
};

} // namespace chip
