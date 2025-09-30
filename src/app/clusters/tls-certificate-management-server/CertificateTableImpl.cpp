/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/storage/FabricTableImpl.ipp>
#include <app/storage/TableEntry.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <stdlib.h>

using namespace chip;
using namespace chip::app::DataModel;
using namespace chip::app::Storage;
using namespace chip::app::Storage::Data;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement::Structs;
using chip::TLV::EstimateStructOverhead;

using RootSerializer   = DefaultSerializer<CertificateId, CertificateTable::RootCertStruct>;
using ClientSerializer = DefaultSerializer<CertificateId, CertificateTable::ClientCertWithKey>;

typedef uint8_t IntermediateCertIndex;

namespace {
/// @brief Tags Used to serialize certificates so they can be stored in flash memory;
/// the field IDs from TlsCertificateManagement::Structs are generally used directly
/// kCertificateId: Tag for the certificate ID
/// kClientCertificateCount: Tag for the number of client certificates stored
enum class TagCertificate : uint8_t
{
    kCertificateId,
    kCertificatePayload,
    kClientCertificateNextId,
    kRootCertificateNextId,
    kStoredFabricIndex,
    kRootCertMapping,
    kClientCertMapping,
    kClientCertDetail
};

enum class NocsrElements : uint8_t
{
    kCsr      = 1,
    kCsrNonce = 2,
};

enum class CertificateType : uint8_t
{
    kClient,
    kRoot
};

struct StoredCertificate
{
    CertificateId id;
    FabricIndex fabric = kUndefinedFabricIndex;
};

static constexpr size_t kPersistentBufferNextIdBytes =
    EstimateStructOverhead(sizeof(uint16_t), // mNextClientId
                           sizeof(uint16_t), // mNextRootId,
                           EstimateStructOverhead(sizeof(CertificateId), sizeof(FabricIndex)) *
                               (kMaxRootCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS), // mRootCertMapping
                           EstimateStructOverhead(sizeof(CertificateId), sizeof(FabricIndex)) *
                               (kMaxClientCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS)); // mClientCertMapping

struct GlobalCertificateData : public PersistentData<kPersistentBufferNextIdBytes>
{
    uint16_t mNextClientId = 0;
    uint16_t mNextRootId   = 0;
    EndpointId mEndpointId = kInvalidEndpointId;
    std::array<StoredCertificate, kMaxRootCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS> mRootCertMapping;
    size_t mRootCertMappingCount = 0;
    std::array<StoredCertificate, kMaxClientCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS> mClientCertMapping;
    size_t mClientCertMappingCount = 0;

    GlobalCertificateData(EndpointId endpoint) : mEndpointId(endpoint) {}
    ~GlobalCertificateData() {}

    void Clear() override
    {
        mNextClientId = 0;
        mNextRootId   = 0;

        for (auto & cert : mRootCertMapping)
        {
            cert.fabric = kUndefinedFabricIndex;
            cert.id.Clear();
        }
        for (auto & cert : mClientCertMapping)
        {
            cert.fabric = kUndefinedFabricIndex;
            cert.id.Clear();
        }
    }

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kInvalidEndpointId != mEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
        key = DefaultStorageKeyAllocator::TlsEndpointGlobalDataKey(mEndpointId);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kClientCertificateNextId), mNextClientId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kRootCertificateNextId), mNextRootId));

        // Storing the cert map
        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagCertificate::kRootCertMapping), TLV::kTLVType_Array, entryMapContainer));
        ReturnErrorOnFailure(SerializeMapping(writer, mRootCertMappingCount, mRootCertMapping.data()));
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));

        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagCertificate::kClientCertMapping), TLV::kTLVType_Array, entryMapContainer));
        ReturnErrorOnFailure(SerializeMapping(writer, mClientCertMappingCount, mClientCertMapping.data()));
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));

        return writer.EndContainer(container);
    }

    CHIP_ERROR SerializeMapping(TLV::TLVWriter & writer, const size_t & count, const StoredCertificate * source) const
    {
        for (size_t i = 0; i < count; i++)
        {
            const auto & stored = source[i];
            if (stored.fabric == kUndefinedFabricIndex)
            {
                continue;
            }
            TLV::TLVType entryIdContainer;
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryIdContainer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kCertificateId), stored.id.mCertificateId));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kStoredFabricIndex), stored.fabric));
            ReturnErrorOnFailure(writer.EndContainer(entryIdContainer));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kClientCertificateNextId)));
        ReturnErrorOnFailure(reader.Get(mNextClientId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kRootCertificateNextId)));
        ReturnErrorOnFailure(reader.Get(mNextRootId));

        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagCertificate::kRootCertMapping)));
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));
        ReturnErrorOnFailure(DeserializeMapping(reader, mRootCertMapping.size(), mRootCertMappingCount, mRootCertMapping.data()));
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));

        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagCertificate::kClientCertMapping)));
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));
        ReturnErrorOnFailure(
            DeserializeMapping(reader, mClientCertMapping.size(), mClientCertMappingCount, mClientCertMapping.data()));
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));

        return reader.ExitContainer(container);
    }

    CHIP_ERROR DeserializeMapping(TLV::TLVReader & reader, const size_t max, size_t & count, StoredCertificate * target)
    {
        size_t i = 0;
        CHIP_ERROR err;
        while ((err = reader.Next(TLV::AnonymousTag())) == CHIP_NO_ERROR)
        {
            TLV::TLVType entryIdContainer;
            if (i >= max)
            {
                // In-memory is not allowed to be smaller than what we've stored in persistence,
                // due to bindings to FabricTableImpl
                return CHIP_ERROR_INTERNAL;
            }
            auto & stored = target[i];
            ReturnErrorOnFailure(reader.EnterContainer(entryIdContainer));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kCertificateId)));
            ReturnErrorOnFailure(reader.Get(stored.id.mCertificateId));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kStoredFabricIndex)));
            ReturnErrorOnFailure(reader.Get(stored.fabric));
            ReturnErrorOnFailure(reader.ExitContainer(entryIdContainer));

            if (stored.fabric != kUndefinedFabricIndex)
            {
                ++i;
            }
        }
        count = i;
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) override
    {
        CHIP_ERROR err = PersistentData::Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            Clear();
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetNextClientCertificateId(FabricIndex fabric, TLSCCDID & id)
    {
        VerifyOrReturnError(mClientCertMappingCount < mClientCertMapping.size(), CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(
            GetNextCertificateId(mNextClientId, kMaxClientCertId, mClientCertMappingCount, mClientCertMapping.data()));
        return ConsumeId(fabric, id, mNextClientId, mClientCertMappingCount, mClientCertMapping.data());
    }

    CHIP_ERROR GetNextRootCertificateId(FabricIndex fabric, TLSCAID & id)
    {
        VerifyOrReturnError(mRootCertMappingCount < mRootCertMapping.size(), CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(GetNextCertificateId(mNextRootId, kMaxRootCertId, mRootCertMappingCount, mRootCertMapping.data()));
        return ConsumeId(fabric, id, mNextRootId, mRootCertMappingCount, mRootCertMapping.data());
    }

    CHIP_ERROR RemoveClientCertificate(PersistentStorageDelegate & storage, ClientCertificateTable & table, FabricIndex fabric,
                                       TLSCCDID id)
    {
        return DoRemoval(storage, table, fabric, id, mClientCertMappingCount, mClientCertMapping.data());
    }

    CHIP_ERROR RemoveRootCertificate(PersistentStorageDelegate & storage, RootCertificateTable & table, FabricIndex fabric,
                                     TLSCAID id)
    {
        return DoRemoval(storage, table, fabric, id, mRootCertMappingCount, mRootCertMapping.data());
    }

    CHIP_ERROR RemoveAllClientCertificates(PersistentStorageDelegate & storage, FabricIndex fabric)
    {
        return DoRemovalAll(storage, fabric, mClientCertMappingCount, mClientCertMapping.data());
    }

    CHIP_ERROR RemoveAllRootCertificates(PersistentStorageDelegate & storage, FabricIndex fabric)
    {
        return DoRemovalAll(storage, fabric, mRootCertMappingCount, mRootCertMapping.data());
    }

private:
    template <class CertificateTable>
    CHIP_ERROR DoRemoval(PersistentStorageDelegate & storage, CertificateTable & table, FabricIndex fabric, uint16_t id,
                         size_t count, StoredCertificate * source)
    {
        // Find the entry in the global mapping
        StoredCertificate * foundEntry = nullptr;
        for (size_t i = 0; i < count; ++i)
        {
            auto & entry = source[i];
            if (entry.fabric == fabric && entry.id == id)
            {
                foundEntry = &entry;
                break;
            }
        }
        if (foundEntry == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        foundEntry->fabric = kUndefinedFabricIndex;
        foundEntry->id.Clear();
        ReturnErrorOnFailure(this->Save(&storage));

        CertificateId localId(id);
        auto removeResult = table.RemoveTableEntry(fabric, localId);
        if (removeResult == CHIP_ERROR_NOT_FOUND)
        {
            // Index is out of sync; keep it updated & return NOT_FOUND
            return CHIP_ERROR_NOT_FOUND;
        }

        if (removeResult != CHIP_NO_ERROR)
        {
            // Failed to remove, re-add
            foundEntry->fabric = fabric;
            foundEntry->id     = localId;
            ReturnErrorOnFailure(this->Save(&storage));
        }
        return removeResult;
    }

    CHIP_ERROR DoRemovalAll(PersistentStorageDelegate & storage, FabricIndex fabric, size_t count, StoredCertificate * source)
    {
        // Find the entry in the global mapping
        uint16_t foundCount = 0;
        for (size_t i = 0; i < count; ++i)
        {
            auto & entry = source[i];
            if (entry.fabric == fabric)
            {
                entry.fabric = kUndefinedFabricIndex;
                entry.id.Clear();
                ++foundCount;
            }
        }
        if (foundCount == 0)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        return this->Save(&storage);
    }

    CHIP_ERROR GetNextCertificateId(uint16_t & nextId, uint16_t maxId, size_t count, const StoredCertificate * source)
    {
        bool looped = false;
        bool taken;
        uint16_t outOfIdCheck = nextId;
        do
        {
            if (nextId == outOfIdCheck)
            {
                VerifyOrReturnError(!looped, CHIP_ERROR_ENDPOINT_POOL_FULL);
                looped = true;
            }
            if (nextId == maxId)
            {
                nextId = 0;
            }
            taken = false;
            for (size_t i = 0; i < count; i++)
            {
                if (source[i].id == nextId)
                {
                    taken = true;
                    ++nextId;
                    break;
                }
            }
        } while (taken);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ConsumeId(FabricIndex fabric, uint16_t & id, uint16_t & nextId, size_t & count, StoredCertificate * stored)
    {
        id          = nextId++;
        auto & used = stored[count++];
        used.id     = id;
        used.fabric = fabric;
        return CHIP_NO_ERROR;
    }
};
} // namespace

//
// CertificateTable::RootCertStruct storage template specialization
//

template <>
StorageKeyName RootSerializer::EndpointEntryCountKey(EndpointId endpoint_id)
{
    return DefaultStorageKeyAllocator::TlsRootCertEndpointCountKey(endpoint_id);
}

template <>
StorageKeyName RootSerializer::FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint)
{
    return DefaultStorageKeyAllocator::TlsRootCertFabricDataKey(fabric, endpoint);
}

template <>
StorageKeyName RootSerializer::FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx)
{
    return DefaultStorageKeyAllocator::TlsRootCertEntityKey(fabric, endpoint, idx);
}

template <>
constexpr size_t RootSerializer::kEntryMaxBytes()
{
    return CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES;
}

template <>
constexpr uint16_t RootSerializer::kMaxPerFabric()
{
    return kMaxRootCertificatesPerFabric;
}

template <>
constexpr uint16_t RootSerializer::kMaxPerEndpoint()
{
    return kMaxCertificatesPerEndpoint;
}

// Serialized TLV length is 88 bytes, 128 bytes gives some slack.  Calculated using
// writer.GetLengthWritten after calling Serialize method of RootCertFabricData
template <>
constexpr size_t RootSerializer::kFabricMaxBytes()
{
    return 128;
}

template <>
CHIP_ERROR RootSerializer::SerializeId(TLV::TLVWriter & writer, const CertificateId & id)
{
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kCertificateId), id.mCertificateId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR RootSerializer::DeserializeId(TLV::TLVReader & reader, CertificateId & id)
{
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kCertificateId)));
    ReturnErrorOnFailure(reader.Get(id.mCertificateId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR RootSerializer::SerializeData(TLV::TLVWriter & writer, const CertificateTable::RootCertStruct & data)
{
    return data.EncodeForWrite(writer, TLV::ContextTag(TagCertificate::kCertificatePayload));
}

template <>
CHIP_ERROR RootSerializer::DeserializeData(TLV::TLVReader & reader, CertificateTable::RootCertStruct & data)
{
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(TagCertificate::kCertificatePayload)));
    return data.Decode(reader);
}

template <>
void RootSerializer::Clear(CertificateTable::RootCertStruct & data)
{
    new (&data) CertificateTable::RootCertStruct();
}

template class chip::app::Storage::FabricTableImpl<CertificateId, CertificateTable::RootCertStruct>;
using RootCertFabricData = FabricEntryData<CertificateId, CertificateTable::RootCertStruct, RootSerializer::kEntryMaxBytes(),
                                           RootSerializer::kFabricMaxBytes(), kMaxRootCertificatesPerFabric>;

//
// CertificateTable::ClientCertStruct storage template specialization
//

template <>
StorageKeyName ClientSerializer::EndpointEntryCountKey(EndpointId endpoint_id)
{
    return DefaultStorageKeyAllocator::TlsClientCertEndpointCountKey(endpoint_id);
}

template <>
StorageKeyName ClientSerializer::FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint)
{
    return DefaultStorageKeyAllocator::TlsClientCertFabricDataKey(fabric, endpoint);
}

template <>
StorageKeyName ClientSerializer::FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx)
{
    return DefaultStorageKeyAllocator::TlsClientCertEntityKey(fabric, endpoint, idx);
}

template <>
constexpr size_t ClientSerializer::kEntryMaxBytes()
{
    return CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES;
}

template <>
constexpr uint16_t ClientSerializer::kMaxPerFabric()
{
    return kMaxClientCertificatesPerFabric;
}

template <>
constexpr uint16_t ClientSerializer::kMaxPerEndpoint()
{
    return kMaxCertificatesPerEndpoint;
}

// Serialized TLV length is 88 bytes, 128 bytes gives some slack.  Calculated using
// writer.GetLengthWritten after calling Serialize method of ClientCertFabricData
template <>
constexpr size_t ClientSerializer::kFabricMaxBytes()
{
    return 128;
}

template <>
CHIP_ERROR ClientSerializer::SerializeId(TLV::TLVWriter & writer, const CertificateId & id)
{
    return writer.Put(TLV::ContextTag(TagCertificate::kCertificateId), id.mCertificateId);
}

template <>
CHIP_ERROR ClientSerializer::DeserializeId(TLV::TLVReader & reader, CertificateId & id)
{
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kCertificateId)));
    return reader.Get(id.mCertificateId);
}

template <>
CHIP_ERROR ClientSerializer::SerializeData(TLV::TLVWriter & writer, const CertificateTable::ClientCertWithKey & data)
{
    // TLSClientCertificateDetailStruct has an array, doesn't implement Encode; copy-pasted here
    // from TLSClientCertificateDetailStruct::Type::Encode
    using chip::app::Clusters::TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::Fields;
    TLV::TLVType container;
    const auto & detail = data.detail;
    ReturnErrorOnFailure(
        writer.StartContainer(TLV::ContextTag(TagCertificate::kClientCertDetail), TLV::kTLVType_Structure, container));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kCcdid), detail.ccdid));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kClientCertificate), detail.clientCertificate));
    ReturnErrorOnFailure(
        DataModel::Encode(writer, TLV::ContextTag(Fields::kIntermediateCertificates), detail.intermediateCertificates));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kFabricIndex), detail.fabricIndex));
    ReturnErrorOnFailure(writer.EndContainer(container));

    return DataModel::Encode(writer, TLV::ContextTag(TagCertificate::kCertificatePayload), data.key.Span());
}

template <>
CHIP_ERROR ClientSerializer::DeserializeData(TLV::TLVReader & reader, CertificateTable::ClientCertWithKey & data)
{
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kClientCertDetail)));
    ReturnErrorOnFailure(data.detail.Decode(reader));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kCertificatePayload)));
    ByteSpan key;
    ReturnErrorOnFailure(reader.Get(key));
    data.key.SetLength(key.size());
    MutableByteSpan keyAsSpan(data.key.Bytes(), data.key.Length());
    return CopySpanToMutableSpan(key, keyAsSpan);
}

template <>
void ClientSerializer::Clear(CertificateTable::ClientCertWithKey & data)
{
    new (&data) CertificateTable::ClientCertWithKey();
}

template class chip::app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertWithKey>;
using ClientCertFabricData = FabricEntryData<CertificateId, CertificateTable::ClientCertWithKey, ClientSerializer::kEntryMaxBytes(),
                                             ClientSerializer::kFabricMaxBytes(), kMaxClientCertificatesPerFabric>;

//
// CertificateTableImpl implementation
//
CHIP_ERROR CertificateTableImpl::Init(PersistentStorageDelegate & storage)
{
    mStorage = &storage;
    ReturnErrorOnFailure(mRootCertificates.Init(storage));
    ReturnErrorOnFailure(mClientCertificates.Init(storage));
    return CHIP_NO_ERROR;
}

void CertificateTableImpl::Finish()
{
    mRootCertificates.Finish();
    mClientCertificates.Finish();
}

CHIP_ERROR CertificateTableImpl::SetEndpoint(EndpointId endpoint)
{
    mEndpointId = endpoint;
    mRootCertificates.SetEndpoint(endpoint);
    mRootCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxRootCertificatesPerFabric);
    mClientCertificates.SetEndpoint(endpoint);
    mClientCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxClientCertificatesPerFabric);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CertificateTableImpl::UpsertRootCertificateEntry(FabricIndex fabric, Optional<TLSCAID> & id, RootBuffer & buffer,
                                                            const ByteSpan & certificate)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    TLSCAID localId = 0;
    if (id.HasValue())
    {
        localId = id.Value();
    }
    else
    {
        // Find a usable ID
        GlobalCertificateData globalData(mEndpointId);
        ReturnErrorOnFailure(globalData.Load(mStorage));
        ReturnErrorOnFailure(globalData.GetNextRootCertificateId(fabric, localId));
        ReturnErrorOnFailure(globalData.Save(mStorage));
    }

    CertificateId tableId(localId);
    const RootCertStruct updatedEntry = { localId, MakeOptional(certificate), fabric };
    ReturnErrorOnFailure(mRootCertificates.SetTableEntry(fabric, tableId, updatedEntry, buffer));
    id.SetValue(localId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CertificateTableImpl::GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID id, BufferedRootCert & entry)
{
    CertificateId localId(id);
    return mRootCertificates.GetTableEntry(fabric_index, localId, entry.mCert, GetBuffer(entry));
}

CHIP_ERROR CertificateTableImpl::HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID id)
{
    CertificateId localId(id);
    EntryIndex unused;
    return mRootCertificates.FindTableEntry(fabric_index, id, unused);
}

CHIP_ERROR CertificateTableImpl::IterateRootCertificates(FabricIndex fabric, BufferedRootCert & store,
                                                         IterateRootCertFnType iterateFn)
{
    return mRootCertificates.IterateEntries(fabric, GetBuffer(store), [&](auto & iter) {
        TableEntryDataConvertingIterator<CertificateId, RootCertStruct> innerIter(iter);
        return iterateFn(innerIter);
    });
}

CHIP_ERROR CertificateTableImpl::RemoveRootCertificate(FabricIndex fabric, TLSCAID id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    GlobalCertificateData globalData(mEndpointId);
    ReturnErrorOnFailure(globalData.Load(mStorage));
    return globalData.RemoveRootCertificate(*mStorage, mRootCertificates, fabric, id);
}

CHIP_ERROR CertificateTableImpl::GetRootCertificateCount(FabricIndex fabric, uint8_t & outCount)
{
    return mRootCertificates.GetFabricEntryCount(fabric, outCount);
}

CHIP_ERROR CertificateTableImpl::PrepareClientCertificate(FabricIndex fabric, const ByteSpan & nonce, ClientBuffer & buffer,
                                                          Optional<TLSCCDID> & id, MutableByteSpan & csr,
                                                          MutableByteSpan & nonceSignature)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    // TODO(gmarcosb): Use an interface like PSAKeyAllocator instead, but not bound to PSA

    Crypto::P256Keypair keyPair;

    if (id.HasValue())
    {
        ClientCertWithKey certWithKey;
        CertificateId localId(id.Value());
        ReturnErrorOnFailure(mClientCertificates.GetTableEntry(fabric, localId, certWithKey, buffer));
        ReturnErrorOnFailure(keyPair.Deserialize(certWithKey.key));
    }
    else
    {
        ReturnErrorOnFailure(keyPair.Initialize(Crypto::ECPKeyTarget::ECDSA));
    }

    size_t csrLen = csr.size();
    ReturnErrorOnFailure(keyPair.NewCertificateSigningRequest(csr.data(), csrLen));
    csr.reduce_size(csrLen);

    Crypto::P256ECDSASignature signatureBuffer;
    ReturnErrorOnFailure(keyPair.ECDSA_sign_msg(nonce.data(), nonce.size(), signatureBuffer));
    ReturnErrorOnFailure(CopySpanToMutableSpan(signatureBuffer.Span(), nonceSignature));

    if (id.HasValue())
    {
        return CHIP_NO_ERROR;
    }

    // Find a usable ID
    GlobalCertificateData globalData(mEndpointId);
    ReturnErrorOnFailure(globalData.Load(mStorage));

    // Update the next ID
    TLSCCDID localId = 0;
    ReturnErrorOnFailure(globalData.GetNextClientCertificateId(fabric, localId));
    ReturnErrorOnFailure(globalData.Save(mStorage));
    id.SetValue(localId);

    ClientCertWithKey certWithKey;
    certWithKey.detail.ccdid = localId;
    certWithKey.detail.clientCertificate.SetValue(Nullable<ByteSpan>());

    ReturnErrorOnFailure(keyPair.Serialize(certWithKey.key));
    CertificateId certId(localId);
    return mClientCertificates.SetTableEntry(fabric, certId, certWithKey, buffer);
}

CHIP_ERROR CertificateTableImpl::UpdateClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, ClientBuffer & buffer,
                                                              const ClientCertStruct & entry)
{
    ClientCertWithKey certWithKey;
    CertificateId localId(id);
    ReturnErrorOnFailure(mClientCertificates.GetTableEntry(fabric_index, localId, certWithKey, buffer));
    Crypto::P256PublicKey certPubKey;
    ReturnErrorOnFailure(Crypto::ExtractPubkeyFromX509Cert(entry.clientCertificate.Value().Value(), certPubKey));
    Crypto::P256Keypair keyPair;
    ReturnErrorOnFailure(keyPair.Deserialize(certWithKey.key));
    auto & storedPubKey = keyPair.Pubkey();
    if (!ByteSpan(storedPubKey, storedPubKey.Length()).data_equal(ByteSpan(certPubKey, certPubKey.Length())))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    certWithKey.detail       = entry;
    certWithKey.detail.ccdid = id;

    return mClientCertificates.SetTableEntry(fabric_index, localId, certWithKey, buffer);
}

CHIP_ERROR CertificateTableImpl::GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, BufferedClientCert & entry)
{
    CertificateId localId(id);
    return mClientCertificates.GetTableEntry(fabric_index, localId, entry.mCertWithKey, GetBuffer(entry));
}

CHIP_ERROR CertificateTableImpl::HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id)
{
    CertificateId localId(id);
    EntryIndex unused;
    return mClientCertificates.FindTableEntry(fabric_index, localId, unused);
}

CHIP_ERROR CertificateTableImpl::IterateClientCertificates(FabricIndex fabric, BufferedClientCert & store,
                                                           IterateClientCertFnType iterateFn)
{
    return mClientCertificates.IterateEntries(fabric, GetBuffer(store), [&](auto & iter) {
        TableEntryDataConvertingIterator<CertificateId, ClientCertWithKey> innerIter(iter);
        return iterateFn(innerIter);
    });
}

CHIP_ERROR CertificateTableImpl::RemoveClientCertificate(FabricIndex fabric, TLSCCDID id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    GlobalCertificateData globalData(mEndpointId);
    ReturnErrorOnFailure(globalData.Load(mStorage));
    return globalData.RemoveClientCertificate(*mStorage, mClientCertificates, fabric, id);
}

CHIP_ERROR CertificateTableImpl::GetClientCertificateCount(FabricIndex fabric, uint8_t & outCount)
{
    return mClientCertificates.GetFabricEntryCount(fabric, outCount);
}

CHIP_ERROR CertificateTableImpl::RemoveFabric(FabricIndex fabric)
{
    CHIP_ERROR result = mClientCertificates.RemoveFabric(fabric);
    ReturnErrorOnFailure(mRootCertificates.RemoveFabric(fabric));

    GlobalCertificateData globalData(mEndpointId);
    ReturnErrorOnFailure(globalData.Load(mStorage));
    if (result != CHIP_NO_ERROR)
    {
        result = globalData.RemoveAllClientCertificates(*mStorage, fabric);
    }

    if (result != CHIP_NO_ERROR)
    {
        result = globalData.RemoveAllRootCertificates(*mStorage, fabric);
    }

    return result;
}
