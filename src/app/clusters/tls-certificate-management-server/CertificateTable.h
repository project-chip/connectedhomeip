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
namespace chip {
namespace app {
namespace Clusters {

/// @brief CertificateTable is meant as an interface between the TLS clusters and certificates.
class CertificateTable
{
public:
    CertificateTable(){};

    virtual ~CertificateTable(){};

    // Not copyable
    CertificateTable(const CertificateTable &) = delete;

    CertificateTable & operator=(const CertificateTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate * storage) = 0;
    virtual void Finish()                                        = 0;

    // Data
    virtual CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, uint16_t certificate_id, TLSCertStruct & entry) = 0;
    virtual CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, uint16_t certificate_id)                        = 0;
    virtual CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, uint16_t certificate_id,
                                                 TLSClientCertificateDetailStruct & entry)                               = 0;
    virtual CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, uint16_t certificate_id)                      = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip
