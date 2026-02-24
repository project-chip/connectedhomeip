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

#include <app/storage/TableEntry.h>
#include <clusters/TlsCertificateManagement/Structs.h>
#include <crypto/CHIPCryptoPAL.h>
#include <functional>
#include <lib/support/CommonIterator.h>
#include <lib/support/PersistentData.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Tls {

using TLSCAID                              = uint16_t;
using TLSCCDID                             = uint16_t;
static constexpr uint16_t kMaxRootCertId   = 65534;
static constexpr uint16_t kMaxClientCertId = 65534;

/// @brief CertificateTable is meant as an interface between the TLS clusters and certificates.
class CertificateTable
{
public:
    using RootCertStruct   = TlsCertificateManagement::Structs::TLSCertStruct::DecodableType;
    using ClientCertStruct = TlsCertificateManagement::Structs::TLSClientCertificateDetailStruct::DecodableType;

    struct ClientCertWithKey
    {
        ClientCertStruct detail;
        // TODO(gmarcosb): We probably want to support using a handle here when the key is stored elsewhere,
        // see for example Symmetric128BitsKeyHandle
        Crypto::P256SerializedKeypair key;
    };

    using IterateRootCertFnType   = std::function<CHIP_ERROR(CommonIterator<RootCertStruct> & iterator)>;
    using IterateClientCertFnType = std::function<CHIP_ERROR(CommonIterator<ClientCertWithKey> & iterator)>;
    using RootBuffer              = PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES>;
    using ClientBuffer            = PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES>;

    /// @brief a root cert along with an associated buffer for the cert payload. RootCertStruct has a ByteSpan,
    /// and this wrapper ensures that the underlying buffer for the ByteSpan has a long-enough lifetime.
    struct BufferedRootCert
    {
        BufferedRootCert(RootBuffer & buffer) : mBuffer(buffer) {}

        RootCertStruct mCert;
        inline const RootCertStruct & GetCert() const { return mCert; }
        inline RootCertStruct & GetCert() { return mCert; }

    private:
        friend class CertificateTable;
        RootBuffer & mBuffer;
    };

    /// @brief a client cert along with an associated buffer for the cert payload.  ClientCertStruct contains various
    /// lists and bytespans, and this wrapper ensures that the underlying buffers for those data structures
    /// have long-enough lifetimes.
    struct BufferedClientCert
    {
        BufferedClientCert(ClientBuffer & buffer) : mBuffer(buffer) {}

        ClientCertWithKey mCertWithKey;
        inline const ClientCertStruct & GetCert() const { return mCertWithKey.detail; }
        inline ClientCertStruct & GetCert() { return mCertWithKey.detail; }

    private:
        friend class CertificateTable;
        ClientBuffer & mBuffer;
    };

    CertificateTable(){};

    virtual ~CertificateTable(){};

    // Not copyable
    CertificateTable(const CertificateTable &) = delete;

    CertificateTable & operator=(const CertificateTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate & storage) = 0;
    virtual void Finish()                                        = 0;

    // Data
    /**
     * @brief If id has a value, updates the existing certificate; otherwise, inserts a new certificate.
     *
     * @param buffer[out] a temporary buffer for temporary serialization, if necessary
     * @param id[in/out] if a value is present, updates the certificate - otherwise, inserts the certificate & returns the ID.
     * @param entry[in] the value to set
     */
    virtual CHIP_ERROR UpsertRootCertificateEntry(FabricIndex fabric_index, Optional<TLSCAID> & id, RootBuffer & buffer,
                                                  const ByteSpan & certificate) = 0;

    /**
     * @brief Loads the specified (fabric_index, id) root cert into entry; if the implementation
     * requires a buffer to load the entry, it is provided via entry.mBuffer
     *
     * @param entry[out] the resulting loaded entry, where entry.mCert will contain the loaded certificate
     */
    virtual CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID id, BufferedRootCert & entry) = 0;
    virtual CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID id)                           = 0;

    /**
     * @brief Executes iterateFn with an iterator for root certificates. The iterator passed to iterateFn has a guaranteed lifetime
     * of the method call.
     *
     * @param[in] fabric The fabric the certificate is associated with
     * @param[out] store A buffer to load the entry into as needed
     * @param[in] iterateFn lambda to execute for the iterator.  If this function returns an error result,
     * iteration stops and the error is returned from this method.
     */
    virtual CHIP_ERROR IterateRootCertificates(FabricIndex fabric, BufferedRootCert & store, IterateRootCertFnType iterateFn) = 0;
    virtual CHIP_ERROR RemoveRootCertificate(FabricIndex fabric, TLSCAID id)                                                  = 0;
    virtual CHIP_ERROR GetRootCertificateCount(FabricIndex fabric, uint8_t & outCount)                                        = 0;

    /**
      * @brief Creates a key pair and assigns a certificate ID for a client certificate, but does not commit. To commit the
      certificate, use UpdateClientCertificateEntry
      *  with the id matching the output of this call.

      * @param nonce[in] the nonce to be used for creating nonceSignature
      * @param buffer[in] a temporary buffer for temporary serialization, if necessary
      * @param id[in/out] the pre-existing generated ID for the client certificate if present (in), or a generated ID as an output
      if not
      * @param csr[out] a DER-encoded certificate signing request using the newly-created key pair
      * @param nonceSignature[out] a nonce signature
      */
    virtual CHIP_ERROR PrepareClientCertificate(FabricIndex fabric, const ByteSpan & nonce, ClientBuffer & buffer,
                                                Optional<TLSCCDID> & id, MutableByteSpan & csr,
                                                MutableByteSpan & nonceSignature) = 0;

    /**
     * @brief Updates the existing client certificate. If the certificate was created via PrepareClientCertificate but not yet
     * committed, commits the respective key pair along with entry.
     * @param buffer[out] a temporary buffer for temporary serialization, if necessary
     */
    virtual CHIP_ERROR UpdateClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, ClientBuffer & buffer,
                                                    const ClientCertStruct & entry) = 0;

    /**
     * @brief Loads the specified (fabric_index, id) client cert into entry; if the implementation
     * requires a buffer to load the entry, it is provided via entry.mBuffer
     *
     * @param entry[out] the resulting loaded entry, where entry.mCert will contain the loaded certificate
     */
    virtual CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, BufferedClientCert & entry) = 0;
    virtual CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id)                             = 0;

    /**
     * @brief Executes iterateFn with an iterator for client certificates. The iterator passed to iterateFn has a guaranteed
     * lifetime of the method call.
     *
     * @param[in] fabric The fabric the certificate is associated with
     * @param[out] store A buffer to load the entry into as needed
     * @param[in] iterateFn lambda to execute for the iterator.  If this function returns an error result,
     * iteration stops, and the error result is returned from this method.
     */
    virtual CHIP_ERROR IterateClientCertificates(FabricIndex fabric, BufferedClientCert & store,
                                                 IterateClientCertFnType iterateFn)      = 0;
    virtual CHIP_ERROR RemoveClientCertificate(FabricIndex fabric, TLSCCDID id)          = 0;
    virtual CHIP_ERROR GetClientCertificateCount(FabricIndex fabric, uint8_t & outCount) = 0;

    /**
     * @brief Removes all data and certificates associated with the specified fabric.
     *
     * @param[in] fabric The fabric to remove.
     */
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric) = 0;

protected:
    static inline PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES> & GetBuffer(BufferedRootCert & bufferedCert)
    {
        return bufferedCert.mBuffer;
    }
    static inline PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES> & GetBuffer(BufferedClientCert & bufferedCert)
    {
        return bufferedCert.mBuffer;
    }
};

/** @brief
 *  Defines methods for implementing application-specific logic for checking if a certificate
 *  has no blocking dependencies and can be removed.
 */
class CertificateDependencyChecker
{
public:
    CertificateDependencyChecker() = default;

    virtual ~CertificateDependencyChecker() = default;

    /**
     * @brief Checks whether the root certificate with the given (matterEndpoint, fabric, id) has no dependencies
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the root certificate to remove.
     * @return CHIP_NO_ERROR if the certificate can be removed.
     */
    virtual CHIP_ERROR RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) = 0;

    /**
     * @brief Checks whether the client certificate with the given (matterEndpoint, fabric, id) has no dependencies
     *
     * @param[in] matterEndpoint The matter endpoint to query against
     * @param[in] fabric The fabric the certificate is associated with
     * @param[in] id The id of the client certificate to remove.
     * @return CHIP_NO_ERROR if the certificate can be removed.
     */
    virtual CHIP_ERROR ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id) = 0;
};

} // namespace Tls
} // namespace Clusters

} // namespace app
} // namespace chip
