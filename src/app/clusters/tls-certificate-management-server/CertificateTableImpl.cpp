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
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <stdlib.h>

using namespace chip;
using namespace chip::app::Storage;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement::Structs;

using RootSerializer   = DefaultSerializer<CertificateId, CertificateTable::RootCertStruct>;
using ClientSerializer = DefaultSerializer<CertificateId, CertificateTable::ClientCertStruct>;

typedef uint8_t IntermediateCertIndex;

namespace {
/// @brief Tags Used to serialize certificates so they can be stored in flash memory;
/// the field IDs from TlsCertificateManagement::Structs are generally used directly
/// kCertificateId: Tag for the certificate ID
/// kClientCertificateCount: Tag for the number of client certificates stored
enum class TagCertificate : uint8_t
{
    kCertificateId,
    kClientCertificateCount
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
    return data.EncodeForWrite(writer, TLV::AnonymousTag());
}

template <>
CHIP_ERROR RootSerializer::DeserializeData(TLV::TLVReader & reader, CertificateTable::RootCertStruct & data)
{
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
CHIP_ERROR ClientSerializer::SerializeData(TLV::TLVWriter & writer, const CertificateTable::ClientCertStruct & data)
{
    // TLSClientCertificateDetailStruct has an array, doesn't implement Encode; copy-pasted here
    // from TLSClientCertificateDetailStruct::Type::Encode

    using chip::app::Clusters::TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::Fields;
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kCcdid), data.ccdid));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kClientCertificate), data.clientCertificate));
    ReturnErrorOnFailure(
        DataModel::Encode(writer, TLV::ContextTag(Fields::kIntermediateCertificates), data.intermediateCertificates));
    // Fields::kFabricIndex filled from table in GetClientCertificateEntry
    return writer.EndContainer(container);
}

template <>
CHIP_ERROR ClientSerializer::DeserializeData(TLV::TLVReader & reader, CertificateTable::ClientCertStruct & data)
{
    return data.Decode(reader);
}

template <>
void ClientSerializer::Clear(CertificateTable::ClientCertStruct & data)
{
    new (&data) CertificateTable::ClientCertStruct();
}

template class chip::app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertStruct>;
using ClientCertFabricData = FabricEntryData<CertificateId, CertificateTable::ClientCertStruct, ClientSerializer::kEntryMaxBytes(),
                                             ClientSerializer::kFabricMaxBytes(), kMaxClientCertificatesPerFabric>;

//
// CertificateTableImpl implementation
//
CHIP_ERROR CertificateTableImpl::Init(PersistentStorageDelegate & storage)
{
    ReturnErrorOnFailure(mRootCertificates.Init(storage));
    ReturnErrorOnFailure(mClientCertificates.Init(storage));
    return CHIP_NO_ERROR;
}

void CertificateTableImpl::Finish()
{
    mRootCertificates.Finish();
    mClientCertificates.Finish();
}

void CertificateTableImpl::SetEndpoint(EndpointId endpoint)
{
    mRootCertificates.SetEndpoint(endpoint);
    mRootCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxRootCertificatesPerFabric);
    mClientCertificates.SetEndpoint(endpoint);
    mClientCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxClientCertificatesPerFabric);
}

CHIP_ERROR CertificateTableImpl::GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id, BufferedRootCert & entry)
{
    CertificateId id(certificate_id);
    return mRootCertificates.GetTableEntry(fabric_index, id, entry.mCert, GetBuffer(entry));
}

CHIP_ERROR CertificateTableImpl::HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id)
{
    CertificateId id(certificate_id);
    EntryIndex unused;
    return mRootCertificates.FindTableEntry(fabric_index, id, unused);
}

CHIP_ERROR CertificateTableImpl::GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id,
                                                           BufferedClientCert & entry)
{
    CertificateId id(certificate_id);
    return mClientCertificates.GetTableEntry(fabric_index, id, entry.mCert, GetBuffer(entry));
}

CHIP_ERROR CertificateTableImpl::HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id)
{
    CertificateId id(certificate_id);
    EntryIndex unused;
    return mClientCertificates.FindTableEntry(fabric_index, id, unused);
}
