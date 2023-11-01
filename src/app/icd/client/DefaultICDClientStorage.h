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

    CHIP_ERROR Init(ICDStorageKeyDelegate * apICDStorageKeyDelegate);

    ICDClientInfoIterator * IterateICDClientInfo() override;

    CHIP_ERROR SetKey(ICDClientInfo & aClientInfo, const ByteSpan aKeyData) override;

    CHIP_ERROR StoreICDInfoEntry(ICDClientInfo & aICDClientInfo) override;

    CHIP_ERROR DeleteEntry(ScopedNodeId aPeerNodeId) override;

    CHIP_ERROR DeleteAllEntries(FabricIndex aFabricIndex) override;

    /**
     * Add the storage instance in the persistent storage manager
     * When new fabric is created, the new ICD storage would be put in storage managemment.
     *
     * @param[in] aStorage the storage instance to be added
     */
    CHIP_ERROR AddStorage(ICDStorage && aStorage);

    /**
     * Removes the storage instance associated with the specified fabric index from
     * the persistent storage manager.
     * Called When fabricIndex is removed, the corresponding storage is removed.
     *
     * @param[in] aFabricIndex the index of the fabric for which to remove storage instance
     */
    void RemoveStorage(FabricIndex aFabricIndex);

    bool ValidateCheckInPayload(const ByteSpan & aPayload, ICDClientInfo & aClientInfo) override;

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

    CHIP_ERROR Save(TLV::TLVWriter & aWriter, const ICDClientInfo & aICDClientInfo);

    class ICDClientInfoIteratorImpl : public ICDClientInfoIterator
    {
    public:
        ICDClientInfoIteratorImpl(DefaultICDClientStorage & aManager);
        size_t Count() override;
        bool Next(ICDClientInfo & aOutput) override;
        void Release() override;

    private:
        DefaultICDClientStorage & mManager;
        size_t mStorageIndex    = 0;
        size_t mClientInfoIndex = 0;
    };

    static constexpr size_t MaxICDClientInfoSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(uint32_t), sizeof(uint32_t), sizeof(uint64_t),
                                           sizeof(Crypto::Aes128KeyByteArray));
    }

private:
    friend class ICDClientInfoIteratorImpl;
    CHIP_ERROR Delete(ICDStorage & aStorage, size_t aIndex, Crypto::Aes128KeyHandle & aSharedKey);
    CHIP_ERROR DeleteKey(Crypto::SymmetricKeystore * apKeyStore, Crypto::Aes128KeyHandle & aSharedKey);
    CHIP_ERROR Load(ICDStorage & aStorage, size_t aIndex, ICDClientInfo & aICDClientInfo);
    CHIP_ERROR SetClientInfoKeyHandle(Crypto::SymmetricKeystore * apKeyStore, ICDClientInfo & aClientInfo,
                                      const ByteSpan & aKeyData);
    ICDStorage * FindStorage(FabricIndex aFabricIndex);

    ObjectPool<ICDClientInfoIteratorImpl, kIteratorsMax> mICDClientInfoIterators;
    std::vector<ICDStorage> mStorages;
    ICDStorageKeyDelegate * mpICDStorageKeyDelegate = nullptr;
};
} // namespace app
} // namespace chip
