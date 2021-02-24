/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file defines data types and objects for modeling and
 *      working with CHIP Operational Credentials.
 *
 */

#pragma once

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Credentials {

using namespace Crypto;

/**
 *  @class ChipOperationalCredentialSet
 *
 *  @brief
 *    Collection of CHIPCertificateSet providing methods for
 *    finding and validating Device Credentials against Roof of Trust chains.
 */
class DLL_EXPORT ChipOperationalCredentialSet
{
public:
    ChipOperationalCredentialSet();
    ~ChipOperationalCredentialSet();

    /**
     * @brief Initialize ChipOperationalCredentialSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated externally and methods in this class don't need to deal with memory allocations.
     *
     * @param certSetsArray         A pointer to the array of the ChipCertificateSet structures.
     * @param certSetsArraySize     Number of ChipCertificateSet entries in the array.
     * @param deviceOpCredKeypair   A pointer to the Device Operational Credential's corresponding Keypair.
     * @param deviceCredentialsDER  A pointer to the raw data buffer in DER format for the Device Operational Credentials.
     * @param deviceCredentialsLen  Length in bytes of the Device Operational Credentials.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Init(ChipCertificateSet * certSetsArray, uint8_t certSetsArraySize, P256Keypair * deviceOpCredKeypair,
                    const uint8_t * deviceCredentialsDER, uint16_t deviceCredentialsLen);

    /**
     * @brief Find certificate set in the set.
     *
     * @param trustedRootId  Authority key identifier of the root certificate-set to be found in the operational credential set.
     *
     * @return A pointer to the certificate data On success. Otherwise, NULL if no certificate found.
     **/
    ChipCertificateSet * FindCertSet(const CertificateKeyId & trustedRootId) const;

    /**
     * @return A pointer to the set of certificate-set data entries.
     **/
    const ChipCertificateSet * GetOpCredSet() const { return mOpCreds; }

    /**
     * @return A pointer to the last certificate-set in the set. Returns NULL if certificate set is empty.
     **/
    const ChipCertificateSet * GetLastCert() const { return (mOpCredCount > 0) ? &mOpCreds[mOpCredCount - 1] : nullptr; }

    /**
     * @return Number of certificate-sets loaded into the set.
     **/
    uint8_t GetCertCount() const { return mOpCredCount; }

    /**
     * @brief Retrieve Trusted Root ID / Authority Key ID of a Certificate Set.
     *
     * @param certSetIndex  Index of the Certificate-set to be used to search for a Trusted Root ID.
     *
     * @return A pointer to the Trusted Root ID on success. Otherwise, nullptr if no Trust Anchor is found.
     **/
    const CertificateKeyId * GetTrustedRootId(uint16_t certSetIndex) const;

    /**
     * @brief Check whether certificate set is in the operational credential set.
     *
     * @param cert  Pointer to the ChipCertificateSet structures.
     *
     * @return True if certificate is in the set, false otherwise.
     **/
    bool IsCertSetInTheOpCredSet(const ChipCertificateSet * cert) const;

    /**
     * @brief Check whether Trusted Root ID is in the operational credential set.
     *
     * @param trustedRoot  Reference to the Trusted Root ID.
     *
     * @return True if Trusted Root ID is in the set, false otherwise.
     **/
    bool IsTrustedRootIn(const CertificateKeyId & trustedRoot) const;

    /**
     * @brief Validate CHIP certificate.
     *
     * @param trustedRootId    Reference to the Trusted Root ID for the Certificate Set to be used
     *                          for validation
     * @param cert              Pointer to the CHIP certificiate to be validated. The certificate is
     *                          required to be in this set, otherwise this function returns error.
     * @param context           Certificate validation context.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(const CertificateKeyId & trustedRootId, const ChipCertificateData * cert, ValidationContext & context);

    /**
     * @brief Find and validate CHIP certificate.
     *
     * @param trustedRootId    Reference to the Trusted Root ID for the Certificate Set to be used
     *                          for validation
     * @param subjectDN         Subject distinguished name to use as certificate search parameter.
     * @param subjectKeyId      Subject key identifier to use as certificate search parameter.
     * @param context           Certificate validation context.
     * @param cert              A pointer to the valid CHIP certificate that matches search criteria.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindValidCert(const CertificateKeyId & trustedRootId, const ChipDN & subjectDN,
                             const CertificateKeyId & subjectKeyId, ValidationContext & context, ChipCertificateData *& cert);

    /**
     * @brief A function to sign a msg using ECDSA and the respective device credentials keypair.
     *
     * @param msg           Message that needs to be signed
     * @param msg_length    Length of message
     * @param out_signature Buffer that will hold the output signature. The signature consists of: 2 EC elements (r and s),
     *                      represented as ASN.1 DER integers, plus the ASN.1 sequence Header
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR SignMsg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature);

    /**
     * @return A pointer to device credentials (in x509 format).
     **/
    const uint8_t * GetDevOpCred() const { return mDeviceCredentialsDER; }

    /**
     * @return Length of the loaded device credentials buffer.
     **/
    uint16_t GetDevOpCredLen() const { return mDeviceCredentialsLen; }

private:
    ChipCertificateSet * mOpCreds; /**< Pointer to an array of certificate data. */
    uint8_t mOpCredCount;          /**< Number of certificates in mOpCreds
                                    array. We maintain the invariant that all
                                    the slots at indices less than
                                    mCertCount have been constructed and slots
                                    at indices >= mCertCount have either never
                                    had their constructor called, or have had
                                    their destructor called since then. */
    uint8_t mMaxCerts;             /**< Length of mOpCreds array. */

    P256Keypair * mDeviceOpCredKeypair;
    const uint8_t * mDeviceCredentialsDER;
    uint16_t mDeviceCredentialsLen;
};

} // namespace Credentials
} // namespace chip
