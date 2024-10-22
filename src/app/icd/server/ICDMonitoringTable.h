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

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/ClusterEnums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentData.h>
#include <stddef.h>

namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
}
} // namespace chip

namespace chip {

static constexpr size_t MaxICDMonitoringEntrySize()
{
    // All the fields added together
    return TLV::EstimateStructOverhead(sizeof(NodeId) /*checkInNodeID*/, sizeof(uint64_t) /*monitoredSubject*/,
                                       sizeof(Crypto::Symmetric128BitsKeyByteArray) /*aes_key_handle*/,
                                       sizeof(Crypto::Symmetric128BitsKeyByteArray) /*hmac_key_handle*/,
                                       sizeof(uint8_t) /*client_type*/) *
        // Provide 50% extra space to make a firmware upgrade that starts storing
        // more data followed by a downgrade work easily and reliably.
        // The 50% number is chosen fairly randomly; storage increases larger than that are
        // possible but need to be staged carefully.
        3 / 2;
}

inline constexpr size_t kICDMonitoringBufferSize = MaxICDMonitoringEntrySize();

struct ICDMonitoringEntry : public PersistentData<kICDMonitoringBufferSize>
{
    ICDMonitoringEntry(FabricIndex fabric = kUndefinedFabricIndex, NodeId nodeId = kUndefinedNodeId)
    {
        this->fabricIndex      = fabric;
        this->checkInNodeID    = nodeId;
        this->monitoredSubject = nodeId;
    }

    ICDMonitoringEntry(Crypto::SymmetricKeystore * keyStore, FabricIndex fabric = kUndefinedFabricIndex,
                       NodeId nodeId = kUndefinedNodeId)
    {
        this->fabricIndex       = fabric;
        this->checkInNodeID     = nodeId;
        this->monitoredSubject  = nodeId;
        this->symmetricKeystore = keyStore;
    }

    CHIP_ERROR UpdateKey(StorageKeyName & key) override;
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override;
    void Clear() override;
    /**
     * @brief Set the Key object
     *        This method will create a new keyHandle. The key handle might contain either
     *        the raw key or a keyID depending on which Crypto implementation is used.
     *        To avoid leaking keys, API consumers must either call the DeleteKey method
     *        or save the entry within the ICDMonitoring Table before this object goes out of scope.
     *
     *        A new entry object should be used for each key when adding entries to the ICDMonitoring
     *        table.
     *
     * @param keyData A byte span containing the raw key
     * @return CHIP_ERROR CHIP_NO_ERROR     success
     *         CHIP_ERROR_INVALID_ARGUMENT  wrong size of the raw key
     *         CHIP_ERROR_INTERNAL          No KeyStore for the entry or Key Handle already present
     *         CHIP_ERROR_XXX               Crypto API related failure
     */
    CHIP_ERROR SetKey(ByteSpan keyData);
    CHIP_ERROR DeleteKey(void);
    inline bool IsValid()
    {
        return (symmetricKeystore != nullptr && keyHandleValid && fabricIndex != kUndefinedFabricIndex &&
                checkInNodeID != kUndefinedNodeId);
    }

    ICDMonitoringEntry & operator=(const ICDMonitoringEntry & icdMonitoringEntry);

    /**
     * @brief Implement the key verification needed by the ICDManagement Server.
     *        Since for some key implementations we cannot retrieve the key from the AES128KeyHandle
     *        we must implement a way to deduce whether the verification key
     *        received is the same or at least works as the same way as the one stored.
     *
     *        This method will produce a random number and then encrypt it with the keyData.
     *        It will then decrypt it with the key stored in the entry. If the resulting decrypted
     *        challenge matches the randomly generated number, then we can safely assume that both key are interchangeable.
     *        This method cannot guarantee a perfect match since the probability of two keys generating the same output in AES128 is
     *        not 0 but 1/2^128 which is small enough for our purposes.
     *
     * @param keyData
     * @return bool True if the key is equivalent to the one stored, otherwise false
     */
    bool IsKeyEquivalent(ByteSpan keyData);

    chip::FabricIndex fabricIndex                           = kUndefinedFabricIndex;
    chip::NodeId checkInNodeID                              = kUndefinedNodeId;
    uint64_t monitoredSubject                               = static_cast<uint64_t>(0);
    app::Clusters::IcdManagement::ClientTypeEnum clientType = app::Clusters::IcdManagement::ClientTypeEnum::kPermanent;
    Crypto::Aes128KeyHandle aesKeyHandle                    = Crypto::Aes128KeyHandle();
    Crypto::Hmac128KeyHandle hmacKeyHandle                  = Crypto::Hmac128KeyHandle();
    bool keyHandleValid                                     = false;
    uint16_t index                                          = 0;
    Crypto::SymmetricKeystore * symmetricKeystore           = nullptr;
};

/**
 * @brief ICDMonitoringTable exists to manage the persistence of entries in the IcdManagement Cluster.
 *        To access persisted data with the ICDMonitoringTable class, instantiate an instance of this class
 *        and call the LoadFromStorage function.
 *
 *        This class can only manage one fabric at a time. The flow is load a fabric, execute necessary operations,
 *        save it if there are any changes and load another fabric.
 *
 *        Issue to refactor the class to use one entry for the entire table
 *        https://github.com/project-chip/connectedhomeip/issues/24288
 */

struct ICDMonitoringTable
{
    ICDMonitoringTable(PersistentStorageDelegate & storage, FabricIndex fabric, uint16_t limit,
                       Crypto::SymmetricKeystore * symmetricKeystore) :
        mStorage(&storage),
        mFabric(fabric), mLimit(limit), mSymmetricKeystore(symmetricKeystore)
    {}

    /**
     * @brief Returns the MonitoringRegistrationStruct entry at the given position.
     * @param index Zero-based position within the RegisteredClients table.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given index.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if index is greater than the index of the last entry on the table.
     */
    CHIP_ERROR Get(uint16_t index, ICDMonitoringEntry & entry) const;

    /**
     * @brief Stores the MonitoringRegistrationStruct entry at the given position,
     *        overwriting any existing entry.
     * @param index Zero-based position within the RegisteredClients table.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given index.
     * @return CHIP_NO_ERROR on success
     */
    CHIP_ERROR Set(uint16_t index, const ICDMonitoringEntry & entry);

    /**
     * @brief Search the registered clients for an entry on the fabric whose checkInNodeID matches the given id.
     * @param id    NodeId to match.
     * @param entry On success, contains the MonitoringRegistrationStruct matching the given node ID.
     *  If found, entry.index contains the position of the entry in the table.
     *  If CHIP_ERROR_NOT_FOUND is returned, entry.index contains the total number of entries in the table.
     * @return CHIP_NO_ERROR if found, CHIP_ERROR_NOT_FOUND if no checkInNodeID matches the provided id.
     */
    CHIP_ERROR Find(NodeId id, ICDMonitoringEntry & entry);

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
     * @brief Check if the table is empty
     * @return True when there is no entry in the table. False if there is at least one
     */
    bool IsEmpty();

    /**
     * @return Maximum number of entries allowed in the RegisteredClients table.
     */
    uint16_t Limit() const;

private:
    PersistentStorageDelegate * mStorage;
    FabricIndex mFabric;
    uint16_t mLimit                                = 0;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
};

} // namespace chip
