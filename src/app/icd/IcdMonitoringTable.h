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

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentArray.h>
#include <stddef.h>

namespace chip {

struct IcdMonitoringEntry
{
    static constexpr size_t kKeyMaxSize = 16;

    IcdMonitoringEntry(NodeId nodeId = kUndefinedNodeId)
    {
        this->checkInNodeID    = nodeId;
        this->monitoredSubject = nodeId;
    }

    bool Compare(const IcdMonitoringEntry & other);
    CHIP_ERROR Copy(const IcdMonitoringEntry & other);
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader);

    chip::NodeId checkInNodeID = static_cast<chip::NodeId>(0);
    uint64_t monitoredSubject  = static_cast<uint64_t>(0);
    uint8_t key[kKeyMaxSize];
};

constexpr size_t kIcdMonitoringEntrySize =
    sizeof(chip::NodeId) + sizeof(uint64_t) + sizeof(size_t) + IcdMonitoringEntry::kKeyMaxSize + 2; // +StartTag(1) +EndTag(1)

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
    : public PersistentArray<CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC, kIcdMonitoringEntrySize, IcdMonitoringEntry>
{
    IcdMonitoringTable(PersistentStorageDelegate & storage, FabricIndex fabric) :
        PersistentArray<CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC, kIcdMonitoringEntrySize, IcdMonitoringEntry>(&storage),
        mFabric(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override;

    /**
     * @brief Search the registered clients for an entry on the fabric whose checkInNodeID matches the given id.
     * @param id    NodeId to match.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given node ID.
     *  If found, entry.index contains the position of the entry in the table.
     *  If CHIP_ERROR_NOT_FOUND is returned, entry.index contains the total number of entries in the table.
     * @return CHIP_NO_ERROR if found, CHIP_ERROR_NOT_FOUND if no checkInNodeID matches the provided id.
     */
    CHIP_ERROR Find(NodeId id, IcdMonitoringEntry & entry);

private:
    FabricIndex mFabric = 0;
};

} // namespace chip
