/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      working with CHIP certificates.
 *
 */

#pragma once

#include <cstdint>
#include <string.h>

#include "CHIPCert.h"
#include "CertificateValidityPolicy.h"
#include <lib/support/Variant.h>

namespace chip {
namespace Credentials {

struct CurrentChipEpochTime : chip::System::Clock::Seconds32
{
    template <typename... Args>
    CurrentChipEpochTime(Args &&... args) : chip::System::Clock::Seconds32(std::forward<Args>(args)...)
    {}
};

struct LastKnownGoodChipEpochTime : chip::System::Clock::Seconds32
{
    template <typename... Args>
    LastKnownGoodChipEpochTime(Args &&... args) : chip::System::Clock::Seconds32(std::forward<Args>(args)...)
    {}
};

using EffectiveTime = Variant<CurrentChipEpochTime, LastKnownGoodChipEpochTime>;

/**
 *  @struct ValidationContext
 *
 *  @brief
 *    Context information used during certification validation.
 */
struct ValidationContext
{
    EffectiveTime mEffectiveTime;                   /**< Current or last known good CHIP Epoch time. */
    const ChipCertificateData * mTrustAnchor;       /**< Pointer to the Trust Anchor Certificate data structure.
                                                       This value is set during certificate validation process
                                                       to indicate to the caller the trust anchor of the
                                                       validated certificate. */
    BitFlags<KeyUsageFlags> mRequiredKeyUsages;     /**< Key usage extensions that should be present in the
                                                       validated certificate. */
    BitFlags<KeyPurposeFlags> mRequiredKeyPurposes; /**< Extended Key usage extensions that should be present
                                                       in the validated certificate. */
    CertType mRequiredCertType;                     /**< Required certificate type. */

    CertificateValidityPolicy * mValidityPolicy =
        nullptr; /**< Optional application policy to apply for certificate validity period evaluation. */

    void Reset();

    template <typename T>
    void SetEffectiveTime(chip::System::Clock::Seconds32 chipTime)
    {
        mEffectiveTime.Set<T>(chipTime);
    }

    template <typename T>
    CHIP_ERROR SetEffectiveTimeFromUnixTime(chip::System::Clock::Seconds32 unixTime)
    {
        uint32_t chipTime;
        VerifyOrReturnError(UnixEpochToChipEpochTime(unixTime.count(), chipTime), CHIP_ERROR_INVALID_TIME);
        SetEffectiveTime<T>(chip::System::Clock::Seconds32(chipTime));
        return CHIP_NO_ERROR;
    }

    template <typename T>
    CHIP_ERROR SetEffectiveTimeFromAsn1Time(const ASN1::ASN1UniversalTime & asn1Time)
    {
        uint32_t chipTime;
        ReturnErrorOnFailure(ASN1ToChipEpochTime(asn1Time, chipTime));
        SetEffectiveTime<T>(chip::System::Clock::Seconds32(chipTime));
        return CHIP_NO_ERROR;
    }
};

/**
 *  @class ChipCertificateSet
 *
 *  @brief
 *    Collection of CHIP certificate data providing methods for
 *    certificate validation and signature verification.
 */
class DLL_EXPORT ChipCertificateSet
{
public:
    ChipCertificateSet();
    ~ChipCertificateSet();

    ChipCertificateSet & operator=(ChipCertificateSet && aOther)
    {
        mCerts               = aOther.mCerts;
        aOther.mCerts        = nullptr;
        mCertCount           = aOther.mCertCount;
        mMaxCerts            = aOther.mMaxCerts;
        mMemoryAllocInternal = aOther.mMemoryAllocInternal;

        return *this;
    }

    /**
     * @brief Initialize ChipCertificateSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated internally using chip::Platform::MemoryAlloc() and freed with chip::Platform::MemoryFree().
     *
     * @param maxCertsArraySize  Maximum number of CHIP certificates to be loaded to the set.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Init(uint8_t maxCertsArraySize);

    /**
     * @brief Initialize ChipCertificateSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated externally and methods in this class don't need to deal with memory allocations.
     *
     * @param certsArray      A pointer to the array of the ChipCertificateData structures.
     * @param certsArraySize  Number of ChipCertificateData entries in the array.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Init(ChipCertificateData * certsArray, uint8_t certsArraySize);

    /**
     * @brief Release resources allocated by this class.
     **/
    void Release();

    /**
     * @brief Clear certificate data loaded into this set.
     **/
    void Clear();

    /**
     * @brief Load CHIP certificate into set.
     *        It is required that the CHIP certificate in the chipCert buffer stays valid while
     *        the certificate data in the set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param chipCert     Buffer containing certificate encoded in CHIP format.
     * @param decodeFlags  Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(const ByteSpan chipCert, BitFlags<CertDecodeFlags> decodeFlags);

    /**
     * @brief Load CHIP certificate into set.
     *        It is required that the CHIP certificate in the reader's underlying buffer stays valid while
     *        the certificate data in the set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param reader       A TLVReader positioned at the CHIP certificate TLV structure.
     * @param decodeFlags  Certificate decoding option flags.
     * @param chipCert     Buffer containing certificate encoded on CHIP format. It is required that this CHIP certificate
     *                     in chipCert ByteSpan stays valid while the certificate data in the set is used.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(chip::TLV::TLVReader & reader, BitFlags<CertDecodeFlags> decodeFlags, ByteSpan chipCert = ByteSpan());

    CHIP_ERROR ReleaseLastCert();

    /**
     * @brief Find certificate in the set.
     *
     * @param subjectKeyId  Subject key identifier of the certificate to be found in the set.
     *
     * @return A pointer to the certificate data On success. Otherwise, NULL if no certificate found.
     **/
    const ChipCertificateData * FindCert(const CertificateKeyId & subjectKeyId) const;

    /**
     * @return A pointer to the set of certificate data entries.
     **/
    const ChipCertificateData * GetCertSet() const { return mCerts; }

    /**
     * @return A pointer to the last certificate data in the set. Returns NULL if certificate set is empty.
     **/
    const ChipCertificateData * GetLastCert() const { return (mCertCount > 0) ? &mCerts[mCertCount - 1] : nullptr; }

    /**
     * @return Number of certificates loaded into the set.
     **/
    uint8_t GetCertCount() const { return mCertCount; }

    /**
     * @brief Check whether certificate is in the set.
     *
     * @param cert  Pointer to the ChipCertificateData structures.
     *
     * @return True if certificate is in the set, false otherwise.
     **/
    bool IsCertInTheSet(const ChipCertificateData * cert) const;

    /**
     * @brief Validate CHIP certificate.
     *
     * @param cert     Pointer to the CHIP certificate to be validated. The certificate is
     *                 required to be in this set, otherwise this function returns error.
     * @param context  Certificate validation context.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(const ChipCertificateData * cert, ValidationContext & context);

    /**
     * @brief Find and validate CHIP certificate.
     *
     * @param[in]  subjectDN     Subject distinguished name to use as certificate search parameter.
     * @param[in]  subjectKeyId  Subject key identifier to use as certificate search parameter.
     * @param[in]  context       Certificate validation context.
     * @param[out] certData      A slot to write a pointer to the CHIP certificate data that matches search criteria.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             const ChipCertificateData ** certData);

    // Deprecated, use the equivalent free function VerifyCertSignature()
    static CHIP_ERROR VerifySignature(const ChipCertificateData * cert, const ChipCertificateData * caCert);

private:
    ChipCertificateData * mCerts; /**< Pointer to an array of certificate data. */
    uint8_t mCertCount;           /**< Number of certificates in mCerts
                                     array. We maintain the invariant that all
                                     the slots at indices less than
                                     mCertCount have been constructed and slots
                                     at indices >= mCertCount have either never
                                     had their constructor called, or have had
                                     their destructor called since then. */
    uint8_t mMaxCerts;            /**< Length of mCerts array. */
    bool mMemoryAllocInternal;    /**< Indicates whether temporary memory buffers are allocated internally. */

    /**
     * @brief Find and validate CHIP certificate.
     *
     * @param[in]  subjectDN      Subject distinguished name to use as certificate search parameter.
     * @param[in]  subjectKeyId   Subject key identifier to use as certificate search parameter.
     * @param[in]  context        Certificate validation context.
     * @param[in]  depth          Depth of the current certificate in the certificate validation chain.
     * @param[out] certData       A slot to write a pointer to the CHIP certificate data that matches search criteria.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             uint8_t depth, const ChipCertificateData ** certData);

    /**
     * @brief Validate CHIP certificate.
     *
     * @param cert           Pointer to the CHIP certificate to be validated.
     * @param context        Certificate validation context.
     * @param depth          Depth of the current certificate in the certificate validation chain.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(const ChipCertificateData * cert, ValidationContext & context, uint8_t depth);
};

} // namespace Credentials
} // namespace chip
