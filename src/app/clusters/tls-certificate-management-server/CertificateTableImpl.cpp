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
using namespace chip::scenes;
using namespace chip::app::Storage;
using namespace chip::app::Clusters::Tls;
using namespace chip:app::Clusters::TlsCertificateManagement::Structs;

using RootSerializer      = DefaultSerializer<TLSCAID, CertificateTable::RootCertStruct>;
using ClientSerializer    = DefaultSerializer<TLSCCDID, CertificateTable::ClientCertStruct>;

typdef uint8_t IntermediateCertIndex;

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

template class chip::app::Storage::FabricTableImpl<TLSCAID, CertificateTable::RootCertStruct, 0>;
using RootCertFabricData =
    FabricEntryData<TLSCAID, CertificateTable::RootCertStruct, RootSerializer::kEntryMaxBytes(), RootSerializer::kFabricMaxBytes(), kMaxScenesPerFabric>;

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
constexpr size_t RootSerializer::kEntryMaxBytes()
{
    return CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES;
}

template <>
constexpr uint16_t RootSerializer::kMaxPerFabric()
{
    return kMaxClientCertificatesPerFabric;
}

template <>
constexpr uint16_t RootSerializer::kMaxPerEndpoint()
{
    return kMaxCertificatesPerEndpoint;
}

// Serialized TLV length is 88 bytes, 128 bytes gives some slack.  Calculated using
// writer.GetLengthWritten after calling Serialize method of ClientCertFabricData
template <>
constexpr size_t RootSerializer::kFabricMaxBytes()
{
    return 128;
}

// Serializes & deserializes individual intermediate certificate entries from TLSClientCertificateDetailStruct
struct IntermediateCertificateEntry : DataAccessor
{
    EndpointId endpoint_id   = kInvalidEndpointId;
    FabricIndex fabric_index = kUndefinedFabricIndex;
    EntryIndex cert_index    = 0;
    IntermediateCertIndex intermediate_index = 0;
    bool first               = true;
    CertificateTable::ClientCertStruct& mEntry;

    IntermediateCertificateEntry(EndpointId endpoint, FabricIndex fabric, TableEntry& entry, EntryIndex idx = 0) :
        endpoint_id(endpoint), fabric_index(fabric), index(idx), mEntry(entry)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);
        key = DefaultStorageKeyAllocator::TlsClientCertEntityIntermediateKey(fabric_index, endpoint_id, cert_index, intermediate_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override { this->mEntry.mStorageData.Clear(); }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(Serializer::SerializeData(writer, this->mEntry.mStorageData));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(Serializer::DeserializeData(reader, this->mEntry.mStorageData));

        return reader.ExitContainer(container);
    }
};

template class chip::app::Storage::FabricTableImpl<TLSCCDID, CertificateTable::ClientCertStruct, 0>;
using ClientCertFabricData =
    FabricEntryData<TLSCCDID, CertificateTable::ClientCertStruct, RootSerializer::kEntryMaxBytes(), RootSerializer::kFabricMaxBytes(), kMaxScenesPerFabric>;

//
// CertificateTableImpl implementation
//
CHIP_ERROR CertificateTableImpl::Init(PersistentStorageDelegate & storage)
{
    ReturnErrorOnFailure(mRootCertificates.Init(storage));
    ReturnErrorOnFailure(mClientCertificates.Init(storage));
}

void CertificateTableImpl::Finish()
{
    mRootCertificates.Finish();
    mClientCertificates.Finish();
}


void CertificateTableImpl::SetEndpoint(EndpointId endpoint)
{
    mRootCertificates.SetEndpoint(endpoint);
    mClientCertificates.SetEndpoint(endpoint);
    mRootCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxRootCertificatesPerFabric);
    mClientCertificates.SetTableSize(kMaxCertificatesPerEndpoint, kMaxClientCertificatesPerFabric);
}

CHIP_ERROR CertificateTableImpl::GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id, CertificateTable::RootCertStruct & entry)
{
}

CHIP_ERROR CertificateTableImpl::HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id)
{
}

CHIP_ERROR CertificateTableImpl::GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id, CertificateTable::ClientCertStruct & entry)
{
}

CHIP_ERROR CertificateTableImpl::HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id)
{
}