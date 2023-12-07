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

#include "ICDClientStorage.h"
#include <lib/core/CHIPCore.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>
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
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS;

    CHIP_ERROR Init(PersistentStorageDelegate * clientInfoStore, Crypto::SymmetricKeystore * keyStore);

    ICDClientInfoIterator * IterateICDClientInfo() override;

   /**
     * In order to add an ICD ClientInfo to the ClientInfoStore using its fabric index as the key and further iterate clientInfos in storage,
     * this function need to store fabric index in dedicated table.
     */
    CHIP_ERROR UpdateFabricList(FabricIndex fabricIndex);

    CHIP_ERROR SetKey(ICDClientInfo & clientInfo, const ByteSpan keyData) override;

    void RemoveKey(ICDClientInfo & clientInfo) override;

    CHIP_ERROR StoreEntry(const ICDClientInfo & clientInfo) override;

    CHIP_ERROR GetEntry(const ScopedNodeId & peerNode, ICDClientInfo & clientInfo) override;

    CHIP_ERROR DeleteEntry(const ScopedNodeId & peerNode) override;

    CHIP_ERROR DeleteAllEntries(FabricIndex fabricIndex) override;

    CHIP_ERROR ProcessCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo) override;

protected:
    enum class ClientInfoTag : uint8_t
    {
        kPeerNodeId       = 1,
        kFabricIndex      = 2,
        kStartICDCounter  = 3,
        kOffset           = 4,
        kMonitoredSubject = 5,
        kSharedKey        = 6
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
        return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex), sizeof(uint32_t), sizeof(uint32_t),
                                           sizeof(uint64_t), sizeof(Crypto::Symmetric128BitsKeyByteArray));
    }

    static constexpr size_t MaxICDCounterSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(size_t), sizeof(size_t));
    }

private:
    friend class ICDClientInfoIteratorImpl;
    CHIP_ERROR LoadFabricList();
    CHIP_ERROR LoadCounter(FabricIndex fabricIndex, size_t & count, size_t & clientInfoSize);

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
