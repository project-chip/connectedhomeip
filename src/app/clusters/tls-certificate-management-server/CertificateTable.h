/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <clusters/TlsCertificateManagement/Structs.h>
#include <lib/support/PersistentData.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Tls {

using TLSCAID  = uint16_t;
using TLSCCDID = uint16_t;

/// @brief CertificateTable is meant as an interface between the TLS clusters and certificates.
class CertificateTable
{
public:
    using RootCertStruct   = TlsCertificateManagement::Structs::TLSCertStruct::DecodableType;
    using ClientCertStruct = TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::DecodableType;

    /// @brief a root cert along with an associated buffer for the cert payload
    struct BufferedRootCert
    {
        RootCertStruct cert;
        PersistentStore<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES> buffer;
    };

    /// @brief a client cert along with an associated buffer for the cert payload
    struct BufferedClientCert
    {
        ClientCertStruct cert;
        PersistentStore<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES> buffer;
    };

    CertificateTable(){};

    virtual ~CertificateTable(){};

    // Not copyable
    CertificateTable(const CertificateTable &) = delete;

    CertificateTable & operator=(const CertificateTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate & storage) = 0;
    virtual void Finish()                                        = 0;

    // Data
    virtual CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id, BufferedRootCert & entry)      = 0;
    virtual CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID certificate_id)                                = 0;
    virtual CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id, BufferedClientCert & entry) = 0;
    virtual CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID certificate_id)                             = 0;
};

} // namespace Tls
} // namespace Clusters
} // namespace app
} // namespace chip
