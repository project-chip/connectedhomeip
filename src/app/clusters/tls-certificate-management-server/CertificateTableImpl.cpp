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

#include <app/InteractionModelEngine.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-certificate-management-server/IncrementingIdHelper.h>
#include <app/data-model-provider/Provider.h>
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

static constexpr size_t kPersistentBufferNextIdBytes =
    EstimateStructOverhead(sizeof(uint16_t), // mNextClientId
                           sizeof(uint16_t), // mNextRootId,
                           EstimateStructOverhead(sizeof(CertificateId), sizeof(FabricIndex)) *
                               (kMaxRootCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS), // mRootCertMapping
                           EstimateStructOverhead(sizeof(CertificateId), sizeof(FabricIndex)) *
                               (kMaxClientCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS)); // mClientCertMapping

class GlobalCertificateData : public PersistableData<kPersistentBufferNextIdBytes>
{
    IncrementingIdHelper<CertificateId, kMaxRootCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS> mRoot;
    IncrementingIdHelper<CertificateId, kMaxClientCertificatesPerFabric * CHIP_CONFIG_MAX_FABRICS> mClient;
    EndpointId mEndpointId = kInvalidEndpointId;

public:
    GlobalCertificateData(EndpointId endpoint) : mEndpointId(endpoint) {}
    ~GlobalCertificateData() {}

    void Clear() override
    {
        mRoot.Clear();
        mClient.Clear();
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
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kClientCertificateNextId), mClient.mNextId));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagCertificate::kRootCertificateNextId), mRoot.mNextId));

        // Storing the cert map
        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagCertificate::kRootCertMapping), TLV::kTLVType_Array, entryMapContainer));
        ReturnErrorOnFailure(SerializeMapping(writer, mRoot));
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));

        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagCertificate::kClientCertMapping), TLV::kTLVType_Array, entryMapContainer));
        ReturnErrorOnFailure(SerializeMapping(writer, mClient));
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));

        return writer.EndContainer(container);
    }

    template <size_t NumEntries>
    CHIP_ERROR SerializeMapping(TLV::TLVWriter & writer, const IncrementingIdHelper<CertificateId, NumEntries> & source) const
    {
        return source.SerializeMapping(writer, TLV::ContextTag(TagCertificate::kCertificateId),
                                       TLV::ContextTag(TagCertificate::kStoredFabricIndex));
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kClientCertificateNextId)));
        ReturnErrorOnFailure(reader.Get(mClient.mNextId));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagCertificate::kRootCertificateNextId)));
        ReturnErrorOnFailure(reader.Get(mRoot.mNextId));

        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagCertificate::kRootCertMapping)));
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));
        ReturnErrorOnFailure(DeserializeMapping(reader, mRoot));
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));

        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagCertificate::kClientCertMapping)));
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));
        ReturnErrorOnFailure(DeserializeMapping(reader, mClient));
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));

        return reader.ExitContainer(container);
    }

    template <size_t NumEntries>
    CHIP_ERROR DeserializeMapping(TLV::TLVReader & reader, IncrementingIdHelper<CertificateId, NumEntries> & source)
    {
        return source.DeserializeMapping(reader, TLV::ContextTag(TagCertificate::kCertificateId),
                                         TLV::ContextTag(TagCertificate::kStoredFabricIndex));
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) // NOLINT(bugprone-derived-method-shadowing-base-method)
    {
        CHIP_ERROR err = PersistableData::Load(storage);
        return err.NoErrorIf(CHIP_ERROR_NOT_FOUND); // NOT_FOUND is OK; DataAccessor::Load already called Clear()
    }

    CHIP_ERROR GetNextClientCertificateId(FabricIndex fabric, TLSCCDID & id)
    {
        ReturnErrorOnFailure(mClient.GetNextId(kMaxClientCertId));
        return mClient.ConsumeId(fabric, id);
    }

    CHIP_ERROR GetNextRootCertificateId(FabricIndex fabric, TLSCAID & id)
    {
        ReturnErrorOnFailure(mRoot.GetNextId(kMaxRootCertId));
        return mRoot.ConsumeId(fabric, id);
    }

    CHIP_ERROR RemoveClientCertificate(PersistentStorageDelegate & storage, ClientCertificateTable & table, FabricIndex fabric,
                                       TLSCCDID id)
    {
        return mClient.Remove(table, fabric, CertificateId(id), [&, this]() { return Save(&storage); });
    }

    CHIP_ERROR RemoveRootCertificate(PersistentStorageDelegate & storage, RootCertificateTable & table, FabricIndex fabric,
                                     TLSCAID id)
    {
        return mRoot.Remove(table, fabric, CertificateId(id), [&, this]() { return Save(&storage); });
    }

    CHIP_ERROR RemoveAllClientCertificates(PersistentStorageDelegate & storage, FabricIndex fabric)
    {
        return mClient.RemoveAll(fabric, [&, this]() { return Save(&storage); });
    }

    CHIP_ERROR RemoveAllRootCertificates(PersistentStorageDelegate & storage, FabricIndex fabric)
    {
        return mRoot.RemoveAll(fabric, [&, this]() { return Save(&storage); });
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
    ReturnErrorOnFailure(data.key.SetLength(key.size()));
    MutableByteSpan keyAsSpan(data.key.Bytes(), data.key.Length());
    return CopySpanToMutableSpan(key, keyAsSpan);
}

template <>
void ClientSerializer::Clear(CertificateTable::ClientCertWithKey & data)
{
    new (&data) CertificateTable::ClientCertWithKey();
}

template class chip::app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertWithKey>;

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
    // We want to release as many resources as possible; if anything fails,
    // hold on to the error until we've had a chance to try to free other resources
    DataModel::Provider * provider = InteractionModelEngine::GetInstance()->GetDataModelProvider();

    CHIP_ERROR clientResult = mClientCertificates.RemoveFabric(*provider, fabric).NoErrorIf(CHIP_ERROR_NOT_FOUND);
    CHIP_ERROR rootResult   = mRootCertificates.RemoveFabric(*provider, fabric).NoErrorIf(CHIP_ERROR_NOT_FOUND);

    GlobalCertificateData globalData(mEndpointId);
    CHIP_ERROR globalDataResult = globalData.Load(mStorage);
    if (globalDataResult == CHIP_ERROR_NOT_FOUND)
    {
        ReturnErrorOnFailure(clientResult);
        return rootResult;
    }
    ReturnErrorOnFailure(globalDataResult);

    if (rootResult == CHIP_NO_ERROR)
    {
        rootResult = globalData.RemoveAllRootCertificates(*mStorage, fabric).NoErrorIf(CHIP_ERROR_NOT_FOUND);
    }

    ReturnErrorOnFailure(globalData.RemoveAllClientCertificates(*mStorage, fabric).NoErrorIf(CHIP_ERROR_NOT_FOUND));
    ReturnErrorOnFailure(clientResult);
    return rootResult;
}
