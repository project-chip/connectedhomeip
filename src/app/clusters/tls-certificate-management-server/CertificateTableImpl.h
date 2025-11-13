/**
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

#pragma once
#include <app/clusters/tls-certificate-management-server/CertificateTable.h>
#include <app/storage/FabricTableImpl.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Pool.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Tls {

static constexpr uint16_t kMaxClientCertificatesPerFabric = CHIP_CONFIG_TLS_MAX_CLIENT_CERTS_PER_FABRIC_TABLE_SIZE;
static constexpr uint16_t kMaxRootCertificatesPerFabric   = CHIP_CONFIG_TLS_MAX_ROOT_PER_FABRIC_CERTS_TABLE_SIZE;

inline constexpr uint16_t kUndefinedCertificateId = 0xffff;

static_assert(kMaxClientCertificatesPerFabric >= 5, "Per spec, kMaxClientCertificatesPerFabric must be at least 5");
static_assert(kMaxRootCertificatesPerFabric >= 5, "Per spec, kMaxRootCertificatesPerFabric must be at least 5");
static_assert(kMaxClientCertificatesPerFabric <= 254, "Per spec, kMaxClientCertificatesPerFabric must be at most 254");
static_assert(kMaxRootCertificatesPerFabric <= 254, "Per spec, kMaxRootCertificatesPerFabric must be at most 254");

// Limit is set per-fabric
static constexpr uint16_t kMaxCertificatesPerEndpoint = UINT16_MAX;

/// @brief struct used to identify a certificate
struct CertificateId
{
    uint16_t mCertificateId = kUndefinedCertificateId;

    CertificateId() = default;
    CertificateId(uint16_t id) : mCertificateId(id) {}

    void Clear() { mCertificateId = kUndefinedCertificateId; }

    bool IsValid() { return (mCertificateId != kUndefinedCertificateId); }

    uint16_t & Value() { return mCertificateId; }
    const uint16_t & Value() const { return mCertificateId; }

    bool operator==(const CertificateId & other) const { return (mCertificateId == other.mCertificateId); }
};

class RootCertificateTable : public app::Storage::FabricTableImpl<CertificateId, CertificateTable::RootCertStruct>
{
public:
    using Super = app::Storage::FabricTableImpl<CertificateId, CertificateTable::RootCertStruct>;

    RootCertificateTable() : Super(kMaxRootCertificatesPerFabric, kMaxCertificatesPerEndpoint) {}
    ~RootCertificateTable() { Finish(); };
};

class ClientCertificateTable : public app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertWithKey>
{
public:
    using Super = app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertWithKey>;

    ClientCertificateTable() : Super(kMaxClientCertificatesPerFabric, kMaxCertificatesPerEndpoint) {}
    ~ClientCertificateTable() { Finish(); };
};

class CertificateTableImpl : public CertificateTable
{
public:
    CertificateTableImpl() {}
    ~CertificateTableImpl() { Finish(); };

    bool IsInitialized() { return (mStorage != nullptr); }

    CHIP_ERROR Init(PersistentStorageDelegate & storage) override;
    void Finish() override;

    CHIP_ERROR SetEndpoint(EndpointId endpoint);

    // Data
    CHIP_ERROR UpsertRootCertificateEntry(FabricIndex fabric_index, Optional<TLSCAID> & id, RootBuffer & buffer,
                                          const ByteSpan & certificate) override;
    CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID id, BufferedRootCert & entry) override;
    CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID id) override;
    CHIP_ERROR IterateRootCertificates(FabricIndex fabric, BufferedRootCert & store, IterateRootCertFnType iterateFn) override;
    CHIP_ERROR RemoveRootCertificate(FabricIndex fabric, TLSCAID id) override;
    CHIP_ERROR GetRootCertificateCount(FabricIndex fabric, uint8_t & outCount) override;

    CHIP_ERROR PrepareClientCertificate(FabricIndex fabric, const ByteSpan & nonce, ClientBuffer & buffer, Optional<TLSCCDID> & id,
                                        MutableByteSpan & csr, MutableByteSpan & nonceSignature) override;
    CHIP_ERROR UpdateClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, ClientBuffer & buffer,
                                            const ClientCertStruct & entry) override;
    CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, BufferedClientCert & entry) override;
    CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id) override;
    CHIP_ERROR IterateClientCertificates(FabricIndex fabric, BufferedClientCert & store,
                                         IterateClientCertFnType iterateFn) override;
    CHIP_ERROR RemoveClientCertificate(FabricIndex fabric, TLSCCDID id) override;
    CHIP_ERROR GetClientCertificateCount(FabricIndex fabric, uint8_t & outCount) override;

    CHIP_ERROR RemoveFabric(FabricIndex fabric) override;

private:
    CHIP_ERROR FindRootCertificateEntry(TLSCAID id, FabricIndex out_fabric);
    CHIP_ERROR FindClientCertificateEntry(TLSCCDID id, FabricIndex out_fabric);

    EndpointId mEndpointId = kInvalidEndpointId;
    RootCertificateTable mRootCertificates;
    ClientCertificateTable mClientCertificates;
    PersistentStorageDelegate * mStorage = nullptr;
};

} // namespace Tls
} // namespace Clusters
} // namespace app
} // namespace chip
