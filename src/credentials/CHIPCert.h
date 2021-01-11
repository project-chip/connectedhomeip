/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <string.h>

#include <asn1/ASN1.h>
#include <core/CHIPConfig.h>
#include <core/CHIPTLV.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/BitFlags.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Credentials {

/** Data Element Tags for the CHIP Certificate
 */
enum
{
    // ---- Top-level Protocol-Specific Tags ----
    kTag_ChipCertificate      = 1, /**< [ structure ] A CHIP certificate. */
    kTag_ChipCertificateArray = 2, /**< [ array ] An array of CHIP certificates. */

    // ---- Context-specific Tags for ChipCertificate Structure ----
    kTag_SerialNumber            = 1,  /**< [ byte string ] Certificate serial number, in BER integer encoding. */
    kTag_SignatureAlgorithm      = 2,  /**< [ unsigned int ] Enumerated value identifying the certificate signature algorithm. */
    kTag_Issuer                  = 3,  /**< [ list ] The issuer distinguished name of the certificate. */
    kTag_NotBefore               = 4,  /**< [ unsigned int ] Certificate validity period start (certificate date format). */
    kTag_NotAfter                = 5,  /**< [ unsigned int ] Certificate validity period end (certificate date format). */
    kTag_Subject                 = 6,  /**< [ list ] The subject distinguished name of the certificate. */
    kTag_PublicKeyAlgorithm      = 7,  /**< [ unsigned int ] Identifies the algorithm with which the public key can be used. */
    kTag_EllipticCurveIdentifier = 8,  /**< [ unsigned int ] For EC certs, identifies the elliptic curve used. */
    kTag_EllipticCurvePublicKey  = 9,  /**< [ byte string ] The elliptic curve public key, in X9.62 encoded format. */
    kTag_ECDSASignature          = 10, /**< [ structure ] The ECDSA signature for the certificate. */
    // ---- Tags identifying certificate extensions (tag numbers 128 - 255) ----
    kCertificateExtensionTagsStart = 128,
    kTag_BasicConstraints          = 128, /**< [ structure ] Identifies whether the subject of the certificate is a CA. */
    kTag_KeyUsage                  = 129, /**< [ structure ] Purpose of the key contained in the certificate. */
    kTag_ExtendedKeyUsage          = 130, /**< [ structure ] Purposes for which the certified public key may be used. */
    kTag_SubjectKeyIdentifier      = 131, /**< [ structure ] Information about the certificate's public key. */
    kTag_AuthorityKeyIdentifier    = 132, /**< [ structure ] Information about the public key used to sign the certificate. */
    kCertificateExtensionTagsEnd   = 255,

    // ---- Context-specific Tags for ECDSASignature Structure ----
    kTag_ECDSASignature_r = 1, /**< [ byte string ] ECDSA r value, in ASN.1 integer encoding. */
    kTag_ECDSASignature_s = 2, /**< [ byte string ] ECDSA s value, in ASN.1 integer encoding. */

    // ---- Context-specific Tags for AuthorityKeyIdentifier Structure ----
    kTag_AuthorityKeyIdentifier_Critical = 1,      /**< [ boolean ] True if the AuthorityKeyIdentifier extension is critical.
                                                      Otherwise absent. */
    kTag_AuthorityKeyIdentifier_KeyIdentifier = 2, /**< [ byte string ] Unique identifier of the issuer's public key,
                                                      per RFC5280. */

    // ---- Context-specific Tags for SubjectKeyIdentifier Structure ----
    kTag_SubjectKeyIdentifier_Critical = 1,      /**< [ boolean ] True if the SubjectKeyIdentifier extension is critical.
                                                    Otherwise absent. */
    kTag_SubjectKeyIdentifier_KeyIdentifier = 2, /**< [ byte string ] Unique identifier for certificate's public key,
                                                    per RFC5280. */

    // ---- Context-specific Tags for KeyUsage Structure ----
    kTag_KeyUsage_Critical = 1, /**< [ boolean ] True if the KeyUsage extension is critical. Otherwise absent. */
    kTag_KeyUsage_KeyUsage = 2, /**< [ unsigned int ] Integer containing key usage bits, per to RFC5280. */

    // ---- Context-specific Tags for BasicConstraints Structure ----
    kTag_BasicConstraints_Critical = 1,          /**< [ boolean ] True if the BasicConstraints extension is critical.
                                                    Otherwise absent. */
    kTag_BasicConstraints_IsCA = 2,              /**< [ boolean ] True if the certificate can be used to verify certificate
                                                    signatures. */
    kTag_BasicConstraints_PathLenConstraint = 3, /**< [ unsigned int ] Maximum number of subordinate intermediate certificates. */

    // ---- Context-specific Tags for ExtendedKeyUsage Structure ----
    kTag_ExtendedKeyUsage_Critical    = 1, /**< [ boolean ] True if the ExtendedKeyUsage extension is critical. Otherwise absent. */
    kTag_ExtendedKeyUsage_KeyPurposes = 2, /**< [ array ] Array of enumerated values giving the purposes for which the public key
                                              can be used. */
};

/** Identifies the purpose or application of certificate
 *
 * A certificate type is a label that describes a certificate's purpose or application.
 * Certificate types are not carried as attributes of the corresponding certificates, but
 * rather are derived from the certificate's structure and/or the context in which it is used.
 * The certificate type enumeration includes a set of pre-defined values describing commonly
 * used certificate applications.  Developers can also extend the certificate type value
 * range with application-specific types that described custom certificates or certificates
 * with unique security properties.
 *
 * Certificate types are primarily used in the implementation of access control policies,
 * where access to application features is influenced by the type of certificate presented
 * by a requester.
 *
 * @note Cert type is an API data type only; it should never be sent over-the-wire.
 */
enum
{
    kCertType_NotSpecified    = 0x00, /**< The certificate's type has not been specified. */
    kCertType_CA              = 0x01, /**< A CHIP CA certificate. */
    kCertType_Node            = 0x02, /**< A CHIP node certificate. */
    kCertType_FirmwareSigning = 0x03, /**< A CHIP firmware signing certificate. */
    kCertType_AppDefinedBase  = 0x7F, /**< Application-specific certificate types should have values >= this value. */
};

/** X.509 Certificate Key Purpose Flags
 *
 *    The flags order must match the enumeration order of corresponding kOID_KeyPurpose values.
 */
enum class KeyPurposeFlags : uint8_t
{
    kServerAuth      = 0x01, /**< Extended key usage is server authentication. */
    kClientAuth      = 0x02, /**< Extended key usage is client authentication. */
    kCodeSigning     = 0x04, /**< Extended key usage is code signing. */
    kEmailProtection = 0x08, /**< Extended key usage is email protection. */
    kTimeStamping    = 0x10, /**< Extended key usage is time stamping. */
    kOCSPSigning     = 0x20, /**< Extended key usage is OCSP signing. */
};

/** X.509 Certificate Key Usage Flags
 */
enum class KeyUsageFlags : uint16_t
{
    kDigitalSignature = 0x0001, /**< Key usage is digital signature. */
    kNonRepudiation   = 0x0002, /**< Key usage is non-repudiation. */
    kKeyEncipherment  = 0x0004, /**< Key usage is key encipherment. */
    kDataEncipherment = 0x0008, /**< Key usage is data encipherment. */
    kKeyAgreement     = 0x0010, /**< Key usage is key agreement. */
    kKeyCertSign      = 0x0020, /**< Key usage is key cert signing. */
    kCRLSign          = 0x0040, /**< Key usage is CRL signing. */
    kEncipherOnly     = 0x0080, /**< Key usage is encipher only. */
    kDecipherOnly     = 0x0100, /**< Key usage is decipher only. */
};

/** CHIP Certificate Flags
 *
 * Contains information about a certificate that has been loaded into a ChipCertificateData object.
 */
enum class CertFlags : uint16_t
{
    kExtPresent_BasicConstraints = 0x0001, /**< Basic constraints extension is present in the certificate. */
    kExtPresent_KeyUsage         = 0x0002, /**< Key usage extention is present in the certificate. */
    kExtPresent_ExtendedKeyUsage = 0x0004, /**< Extended key usage extention is present in the certificate. */
    kExtPresent_SubjectKeyId     = 0x0008, /**< Subject key identifier extention is present in the certificate. */
    kExtPresent_AuthKeyId        = 0x0010, /**< Authority key identifier extention is present in the certificate. */
    kPathLenConstraintPresent    = 0x0020, /**< Path length constraint is present in the certificate. */
    kIsCA                        = 0x0040, /**< Indicates that certificate is a CA certificate. */
    kIsTrustAnchor               = 0x0080, /**< Indicates that certificate is a trust anchor. */
    kTBSHashPresent              = 0x0100, /**< Indicates that TBS hash of the certificate was generated and stored. */
};

/** CHIP Certificate Decode Flags
 *
 * Contains information specifying how a certificate should be decoded.
 */
enum class CertDecodeFlags : uint8_t
{
    kGenerateTBSHash = 0x01, /**< Indicates that to-be-signed (TBS) hash of the certificate should be calculated when certificate is
                                loaded. The TBS hash is then used to validate certificate signature. Normally, all certificates
                                (except trust anchor) in the certificate validation chain require TBS hash. */
    kIsTrustAnchor = 0x02,   /**< Indicates that the corresponding certificate is trust anchor. */
};

/** CHIP Certificate Validate Flags
 *
 * Contains information specifying how a certificate should be validated.
 */
enum class CertValidateFlags : uint8_t
{
    kIgnoreNotBefore = 0x01, /**< Indicate that a Not Before field should be ignored when doing certificate validation. This
                                flag applies to all certificates in the validation chain. */
    kIgnoreNotAfter = 0x02,  /**< Indicate that a Not After field should be ignored when doing certificate validation. This
                                flag applies to all certificates in the validation chain. */
};

enum
{
    kNullCertTime = 0
};

/**
 *  @struct ChipDN
 *
 *  @brief
 *    A data structure representing a Distinguished Name (DN) in a CHIP certificate.
 */
struct ChipDN
{
    union
    {
        uint64_t mChipId; /**< CHIP specific DN attribute value. */
        struct
        {
            const uint8_t * mValue; /**< Pointer to the DN attribute value. */
            uint32_t mLen;          /**< DN attribute length. */
        } mString;                  /**< DN attribute structure when encoded as a string. */
    } mAttrValue;                   /**< DN attribute value union: string or unsigned integer. */
    chip::ASN1::OID mAttrOID;       /**< DN attribute CHIP OID. */

    bool IsEqual(const ChipDN & other) const;
    bool IsEmpty() const { return mAttrOID == chip::ASN1::kOID_NotSpecified; }
    void Clear() { mAttrOID = chip::ASN1::kOID_NotSpecified; }
};

/**
 *  @struct CertificateKeyId
 *
 *  @brief
 *    A data structure representing a certificate key identifier.
 */
struct CertificateKeyId
{
    const uint8_t * mId; /**< Pointer to the key identifier. Encoded as Octet String and represented as the ASN.1 DER Integer (X.690
                            standard). */
    uint8_t mLen;        /**< Key identifier length. */

    bool IsEqual(const CertificateKeyId & other) const;
    bool IsEmpty() const { return mId == nullptr; }
    void Clear() { mId = nullptr; }
};

/**
 *  @struct ChipCertificateData
 *
 *  @brief
 *    In-memory representation of data extracted from a CHIP certificate.
 *
 *    Some of the fields in this structure are pointers to the fields in the original
 *    CHIP certificate. That CHIP certificate is stored in a separate buffer and it is
 *    required that data in that buffer remains valid while the corresponding
 *    ChipCertificateData structure is used.
 */
struct ChipCertificateData
{
    ChipCertificateData();
    ~ChipCertificateData();

    void Clear();

    ChipDN mSubjectDN;                                   /**< Certificate Subject DN. */
    ChipDN mIssuerDN;                                    /**< Certificate Issuer DN. */
    CertificateKeyId mSubjectKeyId;                      /**< Certificate Subject public key identifier. */
    CertificateKeyId mAuthKeyId;                         /**< Certificate Authority public key identifier. */
    uint16_t mNotBeforeDate;                             /**< Certificate validity: Not Before field. */
    uint16_t mNotAfterDate;                              /**< Certificate validity: Not After field. */
    const uint8_t * mPublicKey;                          /**< Pointer to the certificate public key. */
    uint8_t mPublicKeyLen;                               /**< Certificate public key length. */
    uint16_t mPubKeyCurveOID;                            /**< Public key Elliptic Curve CHIP OID. */
    uint16_t mPubKeyAlgoOID;                             /**< Public key algorithm CHIP OID. */
    uint16_t mSigAlgoOID;                                /**< Certificate signature algorithm CHIP OID. */
    BitFlags<uint16_t, CertFlags> mCertFlags;            /**< Certificate data flags. */
    BitFlags<uint16_t, KeyUsageFlags> mKeyUsageFlags;    /**< Certificate key usage extensions flags. */
    BitFlags<uint8_t, KeyPurposeFlags> mKeyPurposeFlags; /**< Certificate extended key usage extensions flags. */
    uint8_t mPathLenConstraint;                          /**< Basic constraint: path length. */
    uint8_t mCertType;                                   /**< Certificate type. */
    struct
    {
        const uint8_t * R; /**< Pointer to the R element of the signature, encoded as ASN.1 DER Integer. */
        uint8_t RLen;      /**< Length of R. */
        const uint8_t * S; /**< Pointer to the S element of the signature, encoded as ASN.1 DER Integer. */
        uint8_t SLen;      /**< Length of R. */
    } mSignature;          /**< Certificate signature structure. */
    uint8_t mTBSHash[chip::Crypto::kSHA256_Hash_Length]; /**< Certificate TBS hash. */
};

/**
 *  @struct ValidationContext
 *
 *  @brief
 *    Context information used during certification validation.
 */
struct ValidationContext
{
    uint32_t mEffectiveTime;                                 /**< Current time in the CHIP Packed Certificate Time format. */
    const ChipCertificateData * mTrustAnchor;                /**< Pointer to the Trust Anchor Certificate data structure. */
    const ChipCertificateData * mSigningCert;                /**< Pointer to the Signing Certificate data structure. */
    BitFlags<uint16_t, KeyUsageFlags> mRequiredKeyUsages;    /**< Key usage extensions that should be present in the
                                                                validated certificate. */
    BitFlags<uint8_t, KeyPurposeFlags> mRequiredKeyPurposes; /**< Extended Key usage extensions that should be present
                                                                in the validated certificate. */
    BitFlags<uint8_t, CertValidateFlags> mValidateFlags;     /**< Certificate validation flags, specifying how a certificate
                                                                should be validated. */
    uint8_t mRequiredCertType;                               /**< Required certificate type. */

    void Reset();
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

    /**
     * @brief Initialize ChipCertificateSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated internally using chip::Platform::MemoryAlloc() and freed with chip::Platform::MemoryFree().
     *
     * @param maxCertsArraySize  Maximum number of CHIP certificates to be loaded to the set.
     * @param decodeBufSize      Size of the buffer that should be allocated to perform CHIP certificate decoding.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Init(uint8_t maxCertsArraySize, uint16_t decodeBufSize);

    /**
     * @brief Initialize ChipCertificateSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated externally and methods in this class don't need to deal with memory allocations.
     *
     * @param certsArray      A pointer to the array of the ChipCertificateData structures.
     * @param certsArraySize  Number of ChipCertificateData entries in the array.
     * @param decodeBuf       Buffer to use for temporary storage of intermediate processing results.
     * @param decodeBufSize   Size of decoding buffer.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Init(ChipCertificateData * certsArray, uint8_t certsArraySize, uint8_t * decodeBuf, uint16_t decodeBufSize);

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
     * @param chipCertLen  The length of the certificate buffer.
     * @param decodeFlags  Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(const uint8_t * chipCert, uint32_t chipCertLen, BitFlags<uint8_t, CertDecodeFlags> decodeFlags);

    /**
     * @brief Load CHIP certificate into set.
     *        It is required that the CHIP certificate in the reader's underlying buffer stays valid while
     *        the certificate data in the set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param reader       A TLVReader positioned at the CHIP certificate TLV structure.
     * @param decodeFlags  Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(chip::TLV::TLVReader & reader, BitFlags<uint8_t, CertDecodeFlags> decodeFlags);

    /**
     * @brief Load CHIP certificates into set.
     *        It is required that the CHIP certificates in the chipCerts buffer stays valid while
     *        the certificates data in the set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param chipCerts     Buffer containing array of certificates or a single certificate encoded in CHIP TLV form.
     * @param chipCertsLen  The length of the certificates buffer.
     * @param decodeFlags   Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen, BitFlags<uint8_t, CertDecodeFlags> decodeFlags);

    /**
     * @brief Load CHIP certificates into set.
     *        It is required that the CHIP certificates in the reader's underlying buffer stays valid while
     *        the certificates data in the set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param reader       A TLVReader positioned at the CHIP certificates TLV array
     *                     or TLV structure for a single certificate.
     * @param decodeFlags  Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCerts(chip::TLV::TLVReader & reader, BitFlags<uint8_t, CertDecodeFlags> decodeFlags);

    /**
     * @brief Add trusted anchor key to the certificate set.
     *        It is required that the public key and public key Id in the pubKey and pubKeyId buffers
     *        stay valid while the certificate set is used.
     *        In case of an error the certificate set is left in the same state as prior to this call.
     *
     * @param caId         CA certificate CHIP identifier.
     * @param curveOID     Elliptic curve CHIP OID.
     * @param pubKey       Trusted public key.
     * @param pubKeyLen    The length of the trusted public key.
     * @param pubKeyId     Trusted public key identifier.
     * @param pubKeyIdLen  The length of the trusted public key identifier.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR AddTrustedKey(uint64_t caId, chip::ASN1::OID curveOID, const uint8_t * pubKey, uint8_t pubKeyLen,
                             const uint8_t * pubKeyId, uint8_t pubKeyIdLen);

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
     * @param cert     Pointer to the CHIP certificiate to be validated. The certificate is
     *                 required to be in this set, otherwise this function returns error.
     * @param context  Certificate validation context.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(const ChipCertificateData * cert, ValidationContext & context);

    /**
     * @brief Find and validate CHIP certificate.
     *
     * @param subjectDN     Subject distinguished name to use as certificate search parameter.
     * @param subjectKeyId  Subject key identifier to use as certificate search parameter.
     * @param context       Certificate validation context.
     * @param cert          A pointer to the valid CHIP certificate that matches search criteria.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             ChipCertificateData *& cert);

    /**
     * @brief Verify CHIP certificate signature.
     *
     * @param cert    Pointer to the CHIP certificiate which signature should be validated.
     * @param caCert  Pointer to the CA certificate of the verified certificate.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    static CHIP_ERROR VerifySignature(const ChipCertificateData * cert, const ChipCertificateData * caCert);

private:
    ChipCertificateData * mCerts; /**< Pointer to an array of certificate data. */
    uint8_t mCertCount;           /**< Number of certificates in mCerts array. */
    uint8_t mMaxCerts;            /**< Length of mCerts array. */
    uint8_t * mDecodeBuf;         /**< Certificate decode buffer. */
    uint16_t mDecodeBufSize;      /**< Certificate decode buffer size. */
    bool mMemoryAllocInternal;    /**< Indicates whether temporary memory buffers are allocated internally. */

    /**
     * @brief Find and validate CHIP certificate.
     *
     * @param subjectDN      Subject distinguished name to use as certificate search parameter.
     * @param subjectKeyId   Subject key identifier to use as certificate search parameter.
     * @param context        Certificate validation context.
     * @param validateFlags  Certificate validation flags.
     * @param depth          Depth of the current certificate in the certificate validation chain.
     * @param cert           A pointer to the valid CHIP certificate that matches search criteria.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             BitFlags<uint8_t, CertValidateFlags> validateFlags, uint8_t depth, ChipCertificateData *& cert);

    /**
     * @brief Validate CHIP certificate.
     *
     * @param cert           Pointer to the CHIP certificiate to be validated.
     * @param context        Certificate validation context.
     * @param validateFlags  Certificate validation flags.
     * @param depth          Depth of the current certificate in the certificate validation chain.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(const ChipCertificateData * cert, ValidationContext & context,
                            BitFlags<uint8_t, CertValidateFlags> validateFlags, uint8_t depth);
};

/**
 * @brief Decode CHIP certificate.
 *        It is required that the CHIP certificate in the chipCert buffer stays valid while
 *        the certData is used.
 *
 * @param chipCert     Buffer containing CHIP certificate.
 * @param chipCertLen  The length of the CHIP certificate.
 * @param certData     Structure containing data extracted from the CHIP certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeChipCert(const uint8_t * chipCert, uint32_t chipCertLen, ChipCertificateData & certData);

/**
 * @brief Decode CHIP certificate.
 *        It is required that the CHIP certificate in the reader's underlying buffer stays valid while
 *        the certData is used.
 *
 * @param reader    A TLVReader positioned at the CHIP certificate TLV structure.
 * @param certData  Structure containing data extracted from the CHIP certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeChipCert(chip::TLV::TLVReader & reader, ChipCertificateData & certData);

/**
 * @brief Decode CHIP Distinguished Name (DN).
 *        It is required that the CHIP DN in the reader's underlying buffer stays valid while
 *        the dn structure is used.
 *
 * @param reader  A TLVReader positioned at the CHIP DN TLV structure.
 * @param dn      Distinguished Name structure.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeChipDN(chip::TLV::TLVReader & reader, ChipDN & dn);

/**
 * @brief Convert standard X.509 certificate to CHIP certificate.
 *
 * @param x509Cert        Buffer containing X.509 DER encoded certificate.
 * @param x509CertLen     The length of the X.509 DER encoded certificate.
 * @param chipCertBuf     Buffer to store converted certificate in CHIP format.
 * @param chipCertBufSize The size of the buffer to store converted certificate.
 * @param chipCertLen     The length of the converted certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ConvertX509CertToChipCert(const uint8_t * x509Cert, uint32_t x509CertLen, uint8_t * chipCertBuf,
                                     uint32_t chipCertBufSize, uint32_t & chipCertLen);

/**
 * @brief Convert CHIP certificate to the standard X.509 DER encoded certificate.
 *
 * @param chipCert        Buffer containing CHIP certificate.
 * @param chipCertLen     The length of the CHIP certificate.
 * @param x509CertBuf     Buffer to store converted certificate in X.509 DER format.
 * @param x509CertBufSize The size of the buffer to store converted certificate.
 * @param x509CertLen     The length of the converted certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ConvertChipCertToX509Cert(const uint8_t * chipCert, uint32_t chipCertLen, uint8_t * x509CertBuf,
                                     uint32_t x509CertBufSize, uint32_t & x509CertLen);

/**
 * Determine type of a CHIP certificate.
 *
 * This function performs an assessment of a certificate's type and sets cert.mCertType value
 * based on the structure of its subject DN and the extensions present. Applications are free
 * to override this assessment by setting cert.mCertType to another value, including an
 * application-defined one.
 *
 * In general, applications will only trust a peer's certificate if it chains to a trusted
 * root certificate.  However, the type assigned to a certificate can influence the *nature*
 * of this trust, e.g. to allow or disallow access to certain features.  Because of this,
 * changes to this algorithm can have VERY SIGNIFICANT and POTENTIALLY CATASTROPHIC effects
 * on overall system security, and should not be made without a thorough understanding of
 * the implications.
 **/
CHIP_ERROR DetermineCertType(ChipCertificateData & cert);

/**
 * @brief
 *   Convert a certificate date/time (in the form of an ASN.1 universal time structure) into a packed
 *   certificate date/time.
 *
 * @details
 *   Packed certificate date/times provide a compact representation for the time values within a certificate
 *   (notBefore and notAfter) that does not require full calendar math to interpret.
 *
 *   A packed certificate date/time contains the fields of a calendar date/time--i.e. year, month, day, hour,
 *   minute, second--packed into an unsigned integer. The bit representation is organized such that
 *   ordinal comparisons of packed date/time values correspond to the natural ordering of the corresponding
 *   times.  To reduce their size, packed certificate date/times are limited to representing times that are on
 *   or after 2020/01/01 00:00:00.  When housed within a 32-bit unsigned integer, packed certificate
 *   date/times can represent times up to the year 2153.
 *
 * @note
 *   This function makes no attempt to verify the correct range of the input time other than year.
 *   Therefore callers must make sure the supplied values are valid prior to invocation.
 *
 * @param time        The calendar date/time to be converted.
 * @param packedTime  A reference to an integer that will receive packed date/time.
 *
 * @retval  #CHIP_NO_ERROR                      If the input time was successfully converted.
 * @retval  #ASN1_ERROR_UNSUPPORTED_ENCODING    If the input time contained a year value that could not
 *                                              be represented in a packed certificate time value.
 **/
CHIP_ERROR PackCertTime(const chip::ASN1::ASN1UniversalTime & time, uint32_t & packedTime);

/**
 * @brief
 *   Unpack a packed certificate date/time into an ASN.1 universal time structure.
 *
 * @param packedTime  A packed certificate time to be unpacked.
 * @param time        A reference to an ASN1UniversalTime structure to receive the unpacked date/time.
 *
 * @retval  #CHIP_NO_ERROR                      If the input time was successfully unpacked.
 */
CHIP_ERROR UnpackCertTime(uint32_t packedTime, chip::ASN1::ASN1UniversalTime & time);

/**
 * @brief
 *   Convert a packed certificate date/time to a packed certificate date.
 *
 * @details
 *   A packed certificate date contains the fields of a calendar date--year, month, day--packed into an
 *   unsigned integer.  The bits are organized such that ordinal comparisons of packed date values
 *   correspond to the natural ordering of the corresponding dates.  To reduce their size, packed
 *   certificate dates are limited to representing dates on or after 2020/01/01.  When housed within
 *   a 16-bit unsigned integer, packed certificate dates can represent dates up to the year 2196.
 *
 * @param packedTime  The packed certificate date/time to be converted.
 *
 * @return A corresponding packed certificate date.
 **/
uint16_t PackedCertTimeToDate(uint32_t packedTime);

/**
 * @brief
 *   Convert a packed certificate date to a corresponding packed certificate date/time, where
 *   the time portion of the value is set to 00:00:00.
 *
 * @param packedDate  The packed certificate date to be converted.
 *
 * @return  A corresponding packed certificate date/time.
 **/
uint32_t PackedCertDateToTime(uint16_t packedDate);

/**
 * @brief
 *   Convert the number of seconds since 1970-01-01 00:00:00 UTC to a packed certificate date/time.
 *
 * @param secondsSinceEpoch  Number of seconds since 1970-01-01 00:00:00 UTC.
 *                           Note: this value is compatible with *positive* values
 *                           of the POSIX time_t value, up to the year 2105.
 *
 * @return  A corresponding packed certificate date/time.
 **/
uint32_t SecondsSinceEpochToPackedCertTime(uint32_t secondsSinceEpoch);

/**
 *  @return  True if the OID represents a CHIP-defined X.509 distinguished named attribute.
 **/
inline bool IsChipX509Attr(chip::ASN1::OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_ChipNodeId || oid == chip::ASN1::kOID_AttributeType_ChipCAId ||
            oid == chip::ASN1::kOID_AttributeType_ChipSoftwarePublisherId || oid == chip::ASN1::kOID_AttributeType_ChipFabricId);
}

/**
 *  @return  True if the OID represents a CHIP-defined X.509 distinguished named attribute
 *           that contains a 64-bit CHIP id.
 **/
inline bool IsChipIdX509Attr(chip::ASN1::OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_ChipNodeId || oid == chip::ASN1::kOID_AttributeType_ChipCAId ||
            oid == chip::ASN1::kOID_AttributeType_ChipSoftwarePublisherId);
}

} // namespace Credentials
} // namespace chip
