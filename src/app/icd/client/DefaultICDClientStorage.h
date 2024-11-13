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

// Do not use the DefaultICDClientStorage class in settings where fabric indices are not stable.
// This class relies on the stability of fabric indices for efficient storage and retrieval of ICD client information.
// If fabric indices are not stable, the functionality of this class will be compromised and can lead to unexpected behavior.

#pragma once

#include <app/icd/client/ICDClientStorage.h>
#include <lib/core/CHIPCore.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>
#include <vector>

// TODO: SymmetricKeystore is an alias for SessionKeystore, replace the below when sdk supports SymmetricKeystore
namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
} // namespace Crypto
} // namespace chip

namespace chip {
namespace app {

/**
 * A DefaultICDClientStorage implementation of ICDClientStorage.
 */
class DefaultICDClientStorage : public ICDClientStorage
{
public:
    using ICDClientInfoIterator = CommonIterator<ICDClientInfo>;

    // ICDClientInfoIterator wrapper to release ICDClientInfoIterator when it is out of scope
    class ICDClientInfoIteratorWrapper
    {
    public:
        ICDClientInfoIteratorWrapper(ICDClientInfoIterator * apICDClientInfoIterator)
        {
            mpICDClientInfoIterator = apICDClientInfoIterator;
        }

        ~ICDClientInfoIteratorWrapper()
        {
            if (mpICDClientInfoIterator != nullptr)
            {
                mpICDClientInfoIterator->Release();
                mpICDClientInfoIterator = nullptr;
            }
        }

    private:
        ICDClientInfoIterator * mpICDClientInfoIterator = nullptr;
    };

    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS;

    CHIP_ERROR Init(PersistentStorageDelegate * clientInfoStore, Crypto::SymmetricKeystore * keyStore);

    /**
     * Iterate through persisted ICD Client Info
     *
     * @return A valid iterator on success. Use CommonIterator accessor to retrieve ICDClientInfo
     */
    ICDClientInfoIterator * IterateICDClientInfo();

    /**
     * When decrypting check-in messages, the system needs to iterate through all keys
     * from all ICD clientInfos. In DefaultICDClientStorage, ICDClientInfos for the same fabric are stored in
     * storage using the fabricIndex as the key. To retrieve all relevant ICDClientInfos
     * from storage, the system needs to know all fabricIndices in advance. The
     * `UpdateFabricList` function provides a way to inject newly created fabricIndices
     * into a dedicated table. It is recommended to call this function whenever a controller is created
     * with a new fabric index.
     *
     * @param[in] fabricIndex The newly created fabric index.
     */
    CHIP_ERROR UpdateFabricList(FabricIndex fabricIndex);

    CHIP_ERROR SetKey(ICDClientInfo & clientInfo, const ByteSpan keyData) override;

    void RemoveKey(ICDClientInfo & clientInfo) override;

    CHIP_ERROR StoreEntry(const ICDClientInfo & clientInfo) override;

    CHIP_ERROR DeleteEntry(const ScopedNodeId & peerNode) override;

    /**
     * Remove all ICDClient persistent information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     * When the whole fabric is removed, all entries from persistent storage in current fabric index are removed.
     *
     * @param[in] fabricIndex the index of the fabric for which to remove ICDClient persistent information
     */
    CHIP_ERROR DeleteAllEntries(FabricIndex fabricIndex);

    CHIP_ERROR ProcessCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo,
                                     Protocols::SecureChannel::CounterType & counter) override;

    /**
     * Shut down DefaultICDClientStorage
     *
     */
    void Shutdown();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    size_t GetFabricListSize() { return mFabricList.size(); }

    PersistentStorageDelegate * GetClientInfoStore() { return mpClientInfoStore; }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

protected:
    enum class ClientInfoTag : uint8_t
    {
        kPeerNodeId       = 1,
        kCheckInNodeId    = 2,
        kFabricIndex      = 3,
        kStartICDCounter  = 4,
        kOffset           = 5,
        kMonitoredSubject = 6,
        kAesKeyHandle     = 7,
        kHmacKeyHandle    = 8,
        kClientType       = 9,
    };

    enum class CounterTag : uint8_t
    {
        kCount = 1,
        kSize  = 2,
    };

    class ICDClientInfoIteratorImpl : public ICDClientInfoIterator
    {
    public:
        ICDClientInfoIteratorImpl(DefaultICDClientStorage & manager);
        size_t Count() override;
        bool Next(ICDClientInfo & info) override;
        void Release() override;

    private:
        DefaultICDClientStorage & mManager;
        size_t mFabricListIndex = 0;
        size_t mClientInfoIndex = 0;
        std::vector<ICDClientInfo> mClientInfoVector;
    };

    static constexpr size_t MaxICDClientInfoSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(
            sizeof(NodeId), sizeof(NodeId), sizeof(FabricIndex), sizeof(uint32_t) /*start_icd_counter*/,
            sizeof(uint32_t) /*offset*/, sizeof(uint64_t) /*monitored_subject*/,
            sizeof(Crypto::Symmetric128BitsKeyByteArray) /*aes_key_handle*/,
            sizeof(Crypto::Symmetric128BitsKeyByteArray) /*hmac_key_handle*/, sizeof(uint8_t) /*client_type*/);
    }

    static constexpr size_t MaxICDCounterSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(size_t), sizeof(size_t));
    }

private:
    friend class ICDClientInfoIteratorImpl;
    CHIP_ERROR StoreFabricList();
    CHIP_ERROR LoadFabricList();
    CHIP_ERROR LoadCounter(FabricIndex fabricIndex, size_t & count, size_t & clientInfoSize);

    bool FabricExists(FabricIndex fabricIndex);

    CHIP_ERROR IncreaseEntryCountForFabric(FabricIndex fabricIndex);
    CHIP_ERROR DecreaseEntryCountForFabric(FabricIndex fabricIndex);
    CHIP_ERROR UpdateEntryCountForFabric(FabricIndex fabricIndex, bool increase);

    CHIP_ERROR SerializeToTlv(TLV::TLVWriter & writer, const std::vector<ICDClientInfo> & clientInfoVector);
    CHIP_ERROR Load(FabricIndex fabricIndex, std::vector<ICDClientInfo> & clientInfoVector, size_t & clientInfoSize);

    ObjectPool<ICDClientInfoIteratorImpl, kIteratorsMax> mICDClientInfoIterators;

    PersistentStorageDelegate * mpClientInfoStore = nullptr;
    Crypto::SymmetricKeystore * mpKeyStore        = nullptr;
    std::vector<FabricIndex> mFabricList;
};
} // namespace app
} // namespace chip
