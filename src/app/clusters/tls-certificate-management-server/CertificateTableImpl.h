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

    bool operator==(const CertificateId & other) const { return (mCertificateId == other.mCertificateId); }
};

class RootCertificateTable : public app::Storage::FabricTableImpl<CertificateId, CertificateTable::RootCertStruct>
{
public:
    using Super = app::Storage::FabricTableImpl<CertificateId, CertificateTable::RootCertStruct>;

    RootCertificateTable() : Super(kMaxRootCertificatesPerFabric, kMaxCertificatesPerEndpoint) {}
    ~RootCertificateTable() { Finish(); };
};

class ClientCertificateTable : public app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertStruct>
{
public:
    using Super = app::Storage::FabricTableImpl<CertificateId, CertificateTable::ClientCertStruct>;

    ClientCertificateTable() : Super(kMaxClientCertificatesPerFabric, kMaxCertificatesPerEndpoint) {}
    ~ClientCertificateTable() { Finish(); };
};

class CertificateTableImpl : public CertificateTable
{
public:
    CertificateTableImpl() {}
    ~CertificateTableImpl() { Finish(); };

    CHIP_ERROR Init(PersistentStorageDelegate & storage) override;
    void Finish() override;

    void SetEndpoint(EndpointId endpoint);

    // Data
    CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id, BufferedRootCert & entry) override;
    CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id) override;
    CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id, BufferedClientCert & entry) override;
    CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id) override;

private:
    RootCertificateTable mRootCertificates;
    ClientCertificateTable mClientCertificates;
};

} // namespace Tls
} // namespace Clusters
} // namespace app
} // namespace chip
