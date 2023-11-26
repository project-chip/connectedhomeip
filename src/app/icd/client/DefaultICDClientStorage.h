/*
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include "ICDClientStorageDelegate.h"
#include "ICDStorage.h"
#include <lib/core/CHIPCore.h>

#include <lib/core/TLV.h>
#include <lib/support/Pool.h>
#include <vector>

namespace chip {
namespace app {

/**
 * An DefaultICDClientStorage using ICDClientStorageDelegate as its backend.
 */
class DefaultICDClientStorage : public ICDClientStorageDelegate
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS;

    /**
     * @brief Retrieve the singleton DefaultICDClientStorage.
     *
     *  @return  A pointer to the shared DefaultICDClientStorage
     *
     */
    static DefaultICDClientStorage * GetInstance(void);

    ICDClientInfoIterator * IterateICDClientInfo() override;

    CHIP_ERROR SetKey(ICDClientInfo & clientInfo, const ByteSpan keyData) override;

    CHIP_ERROR StoreEntry(ICDClientInfo & clientInfo) override;

    CHIP_ERROR DeleteEntry(const ScopedNodeId & peerNodeId) override;

    /**
     * Add the storage instance in the persistent storage manager
     * When new fabric is created, the new ICD storage would be put in storage managemment.
     *
     * @param[in] aStorage the storage instance to be added
     */
    CHIP_ERROR AddStorage(ICDStorage && storage);

    /**
     * Removes the storage instance associated with the specified fabric index from
     * the persistent storage manager.
     * Called When fabricIndex is removed, the corresponding storage is removed.
     *
     * @param[in] aFabricIndex the index of the fabric for which to remove storage instance
     */
    void RemoveStorage(FabricIndex aFabricIndex);

    CHIP_ERROR DeleteAllEntries(FabricIndex aFabricIndex) override;

    bool ValidateCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo) override;

    size_t Size();

protected:
    enum class Tag : uint8_t
    {
        kPeerNodeId       = 1,
        kFabricIndex      = 2,
        kStartICDCounter  = 3,
        kOffset           = 4,
        kMonitoredSubject = 5,
        kSharedKey        = 6
    };

    CHIP_ERROR Save(TLV::TLVWriter & writer, const std::vector<ICDClientInfo> & clientInfoVector);

    class ICDClientInfoIteratorImpl : public ICDClientInfoIterator
    {
    public:
        ICDClientInfoIteratorImpl(DefaultICDClientStorage & manager);
        size_t Count() override;
        bool Next(ICDClientInfo & info) override;
        void Release() override;

    private:
        DefaultICDClientStorage & mManager;
        size_t mStorageIndex    = 0;
        size_t mClientInfoIndex = 0;
        std::vector<ICDClientInfo> mClientInfoVector;
    };

    static constexpr size_t MaxICDClientInfoSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex), sizeof(uint32_t), sizeof(uint32_t),
                                           sizeof(uint64_t), sizeof(Crypto::Aes128KeyByteArray));
    }

private:
    friend class ICDClientInfoIteratorImpl;
    CHIP_ERROR UpdateCounter(ICDStorage & storage, bool increase);
    CHIP_ERROR DeleteCounter(ICDStorage & storage);
    CHIP_ERROR DeleteClientInfo(ICDStorage & storage);
    CHIP_ERROR DeleteKey(ICDStorage & storage, Crypto::Aes128KeyHandle & sharedKey);
    CHIP_ERROR Load(ICDStorage & storage, std::vector<ICDClientInfo> & clientInfoVector);
    ICDStorage * FindStorage(FabricIndex fabricIndex);

    ObjectPool<ICDClientInfoIteratorImpl, kIteratorsMax> mICDClientInfoIterators;
    std::vector<ICDStorage> mStorages;
};
} // namespace app
} // namespace chip
