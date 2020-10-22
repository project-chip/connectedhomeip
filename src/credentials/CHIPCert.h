/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <support/DLLUtil.h>

namespace chip {
namespace Credentials {

/** Data Element Tags for the CHIP Certificate
 */
enum
{
    // ---- Top-level Protocol-Specific Tags ----
    kTag_ChipCertificate          = 1, /**< [ structure ] A CHIP certificate. */
    kTag_EllipticCurvePrivateKey  = 2, /**< [ structure ] An elliptic curve private key. */
    kTag_ChipCertificateList      = 3, /**< [ array ] An array of CHIP certificates. */
    kTag_ChipSignature            = 4, /**< [ structure ] A CHIP signature object. */
    kTag_ChipCertificateReference = 5, /**< [ structure ] A CHIP certificate reference object. */

    // ---- Context-specific Tags for ChipCertificate Structure ----
    kTag_SerialNumber            = 1,  /**< [ byte string ] Certificate serial number, in BER integer encoding. */
    kTag_SignatureAlgorithm      = 2,  /**< [ unsigned int ] Enumerated value identifying the certificate signature algorithm. */
    kTag_Issuer                  = 3,  /**< [ path ] The issuer distinguished name of the certificate. */
    kTag_NotBefore               = 4,  /**< [ unsigned int ] Certificate validity period start (certificate date format). */
    kTag_NotAfter                = 5,  /**< [ unsigned int ] Certificate validity period end (certificate date format). */
    kTag_Subject                 = 6,  /**< [ path ] The subject distinguished name of the certificate. */
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

    // ---- Context-specific Tags for EllipticCurvePrivateKey Structure ----
    kTag_EllipticCurvePrivateKey_CurveIdentifier = 1, /**< [ unsigned int ] ChipCurveId identifying the elliptic curve.  */
    kTag_EllipticCurvePrivateKey_PrivateKey      = 2, /**< [ byte string ] Private key encoded using the I2OSP algorithm
                                                         defined in RFC3447. */
    kTag_EllipticCurvePrivateKey_PublicKey = 3, /**< [ byte string ] The elliptic curve public key, in X9.62 encoded format. */

    // ---- Context-specific Tags for ChipSignature Structure ----
    kTag_ChipSignature_ECDSASignatureData    = 1, /**< [ structure ] ECDSA signature data for the signed message. */
    kTag_ChipSignature_SigningCertificateRef = 2, /**< [ structure ] A CHIP certificate reference structure identifying the
                                                     certificate used to generate the signature. If absent, the signature was
                                                     generated by the first certificate in the RelatedCertificates list. */
    kTag_ChipSignature_RelatedCertificates = 3,   /**< [ array ] Array of certificates needed to validate the signature.
                                                     May be omitted if validators are expected to have the necessary certificates
                                                     for validation. At least one of SigningCertificateRef or RelatedCertificates
                                                     must be present. */
    kTag_ChipSignature_SignatureAlgorithm = 4,    /**< [ unsigned int ] Enumerated value identifying the signature algorithm. */

    // ---- Context-specific Tags for CHIP Certificate Reference Structure ----
    kTag_ChipCertificateRef_Subject     = 1, /**< [ path ] The subject DN of the referenced certificate. */
    kTag_ChipCertificateRef_PublicKeyId = 2, /**< [ byte string ] Unique identifier for referenced certificate's public key,
                                                per RFC5280. */

    // ---- Context-specific Tags for CHIP representation of X.509 Distinguished Name Attributes ----
    //
    // The value used here must match *exactly* the OID enum values assigned to the corresponding object ids in the gen_asn1oid.py
    // script.
    //
    // WARNING! Assign no values higher than 127.
    //
    kTag_DNAttrType_CommonName              = 1,  /**< [ UTF8 string ] */
    kTag_DNAttrType_Surname                 = 2,  /**< [ UTF8 string ] */
    kTag_DNAttrType_SerialNumber            = 3,  /**< [ UTF8 string ] */
    kTag_DNAttrType_CountryName             = 4,  /**< [ UTF8 string ] */
    kTag_DNAttrType_LocalityName            = 5,  /**< [ UTF8 string ] */
    kTag_DNAttrType_StateOrProvinceName     = 6,  /**< [ UTF8 string ] */
    kTag_DNAttrType_OrganizationName        = 7,  /**< [ UTF8 string ] */
    kTag_DNAttrType_OrganizationalUnitName  = 8,  /**< [ UTF8 string ] */
    kTag_DNAttrType_Title                   = 9,  /**< [ UTF8 string ] */
    kTag_DNAttrType_Name                    = 10, /**< [ UTF8 string ] */
    kTag_DNAttrType_GivenName               = 11, /**< [ UTF8 string ] */
    kTag_DNAttrType_Initials                = 12, /**< [ UTF8 string ] */
    kTag_DNAttrType_GenerationQualifier     = 13, /**< [ UTF8 string ] */
    kTag_DNAttrType_DNQualifier             = 14, /**< [ UTF8 string ] */
    kTag_DNAttrType_Pseudonym               = 15, /**< [ UTF8 string ] */
    kTag_DNAttrType_DomainComponent         = 16, /**< [ UTF8 string ] */
    kTag_DNAttrType_ChipDeviceId            = 17, /**< [ unsigned int ] */
    kTag_DNAttrType_ChipServiceEndpointId   = 18, /**< [ unsigned int ] */
    kTag_DNAttrType_ChipCAId                = 19, /**< [ unsigned int ] */
    kTag_DNAttrType_ChipSoftwarePublisherId = 20, /**< [ unsigned int ] */
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
enum
{
    kKeyPurpose_NotSpecified        = 0x00,
    kKeyPurposeFlag_ServerAuth      = 0x01,
    kKeyPurposeFlag_ClientAuth      = 0x02,
    kKeyPurposeFlag_CodeSigning     = 0x04,
    kKeyPurposeFlag_EmailProtection = 0x08,
    kKeyPurposeFlag_TimeStamping    = 0x10,
    kKeyPurposeFlag_OCSPSigning     = 0x20,
};

/** X.509 Certificate Key Usage Flags
 */
enum
{
    kKeyUsage_NotSpecified         = 0x0000,
    kKeyUsageFlag_DigitalSignature = 0x0001,
    kKeyUsageFlag_NonRepudiation   = 0x0002,
    kKeyUsageFlag_KeyEncipherment  = 0x0004,
    kKeyUsageFlag_DataEncipherment = 0x0008,
    kKeyUsageFlag_KeyAgreement     = 0x0010,
    kKeyUsageFlag_KeyCertSign      = 0x0020,
    kKeyUsageFlag_CRLSign          = 0x0040,
    kKeyUsageFlag_EncipherOnly     = 0x0080,
    kKeyUsageFlag_DecipherOnly     = 0x0100,
};

/** CHIP Certificate Flags
 *
 * Contains information about a certificate that has been loaded into a ChipCertSet object.
 */
enum
{
    kCertFlag_ExtPresent_AuthKeyId        = 0x0001,
    kCertFlag_ExtPresent_SubjectKeyId     = 0x0002,
    kCertFlag_ExtPresent_KeyUsage         = 0x0004,
    kCertFlag_ExtPresent_BasicConstraints = 0x0008,
    kCertFlag_ExtPresent_ExtendedKeyUsage = 0x0010,
    kCertFlag_AuthKeyIdPresent            = 0x0020,
    kCertFlag_PathLenConstPresent         = 0x0040,
    kCertFlag_IsCA                        = 0x0080,
    kCertFlag_IsTrusted                   = 0x0100,
    kCertFlag_TBSHashPresent              = 0x0200,
    kCertFlag_UnsupportedSubjectDN        = 0x0400,
    kCertFlag_UnsupportedIssuerDN         = 0x0800,
};

/** CHIP Certificate Decode Flags
 *
 * Contains information specifying how a certificate should be decoded.
 */
enum
{
    kDecodeFlag_GenerateTBSHash = 0x0001,
    kDecodeFlag_IsTrusted       = 0x0002
};

/** CHIP Certificate Validate Flags
 *
 * Contains information specifying how a certificate should be validated.
 */
enum
{
    kValidateFlag_IgnoreNotBefore = 0x0001,
    kValidateFlag_IgnoreNotAfter  = 0x0002,
    kValidateFlag_RequireSHA256   = 0x0004,
};

enum
{
    kNullCertTime = 0
};

/**
 *  @struct ChipDN
 *
 *  @brief
 *    A data structure represents a Distinguished Name in a CHIP certificate.
 */
struct ChipDN
{
    union
    {
        uint64_t ChipId;
        struct
        {
            const uint8_t * Value;
            uint32_t Len;
        } String;
    } AttrValue;
    chip::ASN1::OID AttrOID;

    bool IsEqual(const ChipDN & other) const;
    bool IsEmpty() const { return AttrOID == chip::ASN1::kOID_NotSpecified; }
    void Clear() { AttrOID = chip::ASN1::kOID_NotSpecified; }
};

/**
 *  @struct CertificateKeyId
 *
 *  @brief
 *    A data structure represents a certificate key identifier.
 */
struct CertificateKeyId
{
    const uint8_t * Id;
    uint8_t Len;

    bool IsEqual(const CertificateKeyId & other) const;
    bool IsEmpty() const { return Id == nullptr; }
    void Clear() { Id = nullptr; }
};

/**
 *  @struct ChipCertificateData
 *
 *  @brief
 *    In-memory representation of data extracted from a CHIP certificate.
 */
struct ChipCertificateData
{
    ChipDN SubjectDN;
    ChipDN IssuerDN;
    CertificateKeyId SubjectKeyId;
    CertificateKeyId AuthKeyId;
    const uint8_t * PublicKey;
    uint16_t PublicKeyLen;
    struct
    {
        const uint8_t * R;
        uint16_t RLen;
        const uint8_t * S;
        uint16_t SLen;
    } Signature;
    uint16_t PubKeyCurveOID;
    const uint8_t * EncodedCert;
    uint16_t EncodedCertLen;
    uint16_t CertFlags;
    uint16_t KeyUsageFlags;
    uint16_t PubKeyAlgoOID;
    uint16_t SigAlgoOID;
    uint8_t CertType;
    uint8_t KeyPurposeFlags;
    uint16_t NotBeforeDate;
    uint16_t NotAfterDate;
    uint8_t PathLenConstraint;
    uint8_t TBSHash[chip::Crypto::kSHA256_Hash_Length];
};

/**
 *  @struct ValidationContext
 *
 *  @brief
 *    Context information used during certification validation.
 */
struct ValidationContext
{
    uint32_t EffectiveTime;
    ChipCertificateData * TrustAnchor;
    ChipCertificateData * SigningCert;
    uint16_t RequiredKeyUsages;
    uint16_t ValidateFlags;
#if CHIP_CONFIG_DEBUG_CERT_VALIDATION
    CHIP_ERROR * CertValidationResults;
    uint8_t CertValidationResultsLen;
#endif
    uint8_t RequiredKeyPurposes;
    uint8_t RequiredCertType;

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

    ChipCertificateData * Certs; // [READ-ONLY] Pointer to array of certificate data
    uint8_t CertCount;           // [READ-ONLY] Number of certificates in Certs array
    uint8_t MaxCerts;            // [READ-ONLY] Length of Certs array

    /**
     * @brief Initialize ChipCertificateSet.
     *        This initialization method is used when all memory structures needed for operation are
     *        allocated internally using chip::Platform::MemoryAlloc() and freed with chip::Platform::MemoryFree().
     *
     * @param maxCertsArraySize  Muximum number of CHIP certificates to be loaded to the set.
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
     *
     * @param chipCert     Buffer containing certificate encoded in CHIP format.
     * @param chipCertLen  The length of the certificate buffer.
     * @param decodeFlags  Certificate decoding option flags.
     * @param cert         Structure containing data extracted from the CHIP certificate.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(const uint8_t * chipCert, uint32_t chipCertLen, uint16_t decodeFlags, ChipCertificateData *& cert);

    /**
     * @brief Load CHIP certificate into set.
     *
     * @param reader       A TLVReader positioned at the CHIP certificate TLV structure.
     * @param decodeFlags  Certificate decoding option flags.
     * @param cert         Structure containing data extracted from the CHIP certificate.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCert(chip::TLV::TLVReader & reader, uint16_t decodeFlags, ChipCertificateData *& cert);

    /**
     * @brief Load CHIP certificate into set.
     *
     * @param chipCerts     Buffer containing array of certificate encoded in CHIP TLV form.
     * @param chipCertsLen  The length of the certificates buffer.
     * @param decodeFlags   Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen, uint16_t decodeFlags);

    /**
     * @brief Load CHIP certificates into set.
     *
     * @param reader       A TLVReader positioned at the CHIP certificates TLV array
     *                     or TLV structure for a single certificate.
     * @param decodeFlags  Certificate decoding option flags.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR LoadCerts(chip::TLV::TLVReader & reader, uint16_t decodeFlags);

    /**
     * @brief Add trusted anchor key to the certificate set.
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
    CHIP_ERROR AddTrustedKey(uint64_t caId, chip::ASN1::OID curveOID, const uint8_t * pubKey, uint16_t pubKeyLen,
                             const uint8_t * pubKeyId, uint16_t pubKeyIdLen);

    /**
     * @brief Find certificate in the set.
     *
     * @param subjectKeyId  Subject key identifier of the certificate to be found in the set.
     *
     * @return A pointer to the certificate data On success. Otherwise, NULL if no certificate found.
     **/
    const ChipCertificateData * FindCert(const CertificateKeyId & subjectKeyId) const;

    /**
     * @return A pointer to the last certificate data in the set. Returns NULL if certificate set is empty.
     **/
    const ChipCertificateData * LastCert() const { return (CertCount > 0) ? &Certs[CertCount - 1] : nullptr; }

    /**
     * @brief Validate CHIP certificate.
     *
     * @param cert     CHIP certificiate to be validated.
     * @param context  Certificate validation context.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(ChipCertificateData & cert, ValidationContext & context);

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
     * @param cert    CHIP certificiate which signature should be validated.
     * @param caCert  CA certificate of the verified certificate.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR VerifySignature(ChipCertificateData & cert, ChipCertificateData & caCert);

private:
    uint8_t * mDecodeBuf;
    uint16_t mDecodeBufSize;
    bool mMemoryAllocInternal;

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
                             uint16_t validateFlags, uint8_t depth, ChipCertificateData *& cert);

    /**
     * @brief Validate CHIP certificate.
     *
     * @param cert           CHIP certificiate to be validated.
     * @param context        Certificate validation context.
     * @param validateFlags  Certificate validation flags.
     * @param depth          Depth of the current certificate in the certificate validation chain.
     *
     * @return Returns a CHIP_ERROR on validation or other error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ValidateCert(ChipCertificateData & cert, ValidationContext & context, uint16_t validateFlags, uint8_t depth);
};

/**
 * @brief Decode CHIP certificate.
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
 *
 * @param reader    A TLVReader positioned at the CHIP certificate TLV structure.
 * @param certData  Structure containing data extracted from the CHIP certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeChipCert(chip::TLV::TLVReader & reader, ChipCertificateData & certData);

/**
 * @brief Decode CHIP Distinguished Name (DN).
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
 * This function performs an assessment of a certificate's type based on the structure of
 * its subject DN and the extensions present. Applications are free to override this
 * assessment by setting cert.CertType to another value, including an application-defined
 * one.
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
 * @return A corresponding packet certificate date.
 **/
uint16_t PackedCertTimeToDate(uint32_t packedTime);

/**
 * @brief
 *   Convert a packed certificate date to a corresponding packed certificate date/time, where
 *   the time portion of the value is set to 00:00:00.
 *
 * @param packedDate  The packed certificate date to be converted.
 *
 * @return  A corresponding packet certificate date/time.
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
 * @return  A corresponding packet certificate date/time.
 **/
uint32_t SecondsSinceEpochToPackedCertTime(uint32_t secondsSinceEpoch);

/**
 *  @return  True if the OID represents a CHIP-defined X.509 distinguished named attribute.
 **/
inline bool IsChipX509Attr(chip::ASN1::OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_ChipNodeId || oid == chip::ASN1::kOID_AttributeType_ChipCAId ||
            oid == chip::ASN1::kOID_AttributeType_ChipSoftwarePublisherId);
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
