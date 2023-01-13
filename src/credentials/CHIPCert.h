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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/PeerId.h>
#include <lib/core/TLV.h>
#include <lib/support/BitFlags.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

static constexpr uint32_t kKeyIdentifierLength                 = static_cast<uint32_t>(Crypto::kSubjectKeyIdentifierLength);
static constexpr uint32_t kChip32bitAttrUTF8Length             = 8;
static constexpr uint32_t kChip64bitAttrUTF8Length             = 16;
static constexpr uint16_t kX509NoWellDefinedExpirationDateYear = 9999;

// As per specifications (6.3.5. Node Operational Credentials Certificates)
static constexpr uint32_t kMaxCHIPCertLength = 400;
static constexpr uint32_t kMaxDERCertLength  = 600;

// The decode buffer is used to reconstruct TBS section of X.509 certificate, which doesn't include signature.
static constexpr uint32_t kMaxCHIPCertDecodeBufLength = kMaxDERCertLength - Crypto::kMax_ECDSA_Signature_Length_Der;

/** Data Element Tags for the CHIP Certificate
 */
enum
{
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
    kTag_Extensions              = 10, /**< [ list ] Certificate extensions. */
    kTag_ECDSASignature          = 11, /**< [ byte string ] The ECDSA signature for the certificate. */

    // ---- Context-specific Tags for certificate extensions ----
    kTag_BasicConstraints       = 1, /**< [ structure ] Identifies whether the subject of the certificate is a CA. */
    kTag_KeyUsage               = 2, /**< [ unsigned int ] Bits identifying key usage, per RFC5280. */
    kTag_ExtendedKeyUsage       = 3, /**< [ array ] Enumerated values giving the purposes for which the public key can be used. */
    kTag_SubjectKeyIdentifier   = 4, /**< [ byte string ] Identifier of the certificate's public key. */
    kTag_AuthorityKeyIdentifier = 5, /**< [ byte string ] Identifier of the public key used to sign the certificate. */
    kTag_FutureExtension        = 6, /**< [ byte string ] Arbitrary extension. DER encoded SEQUENCE as in X.509 form. */

    // ---- Context-specific Tags for BasicConstraints Structure ----
    kTag_BasicConstraints_IsCA = 1,              /**< [ boolean ] True if the certificate can be used to verify certificate
                                                    signatures. */
    kTag_BasicConstraints_PathLenConstraint = 2, /**< [ unsigned int ] Maximum number of subordinate intermediate certificates. */
};

/** Identifies the purpose or application of certificate
 *
 * A certificate type is a label that describes a certificate's purpose or application.
 * Certificate types are not carried as attributes of the corresponding certificates, but
 * rather are derived from the certificate's structure and/or the context in which it is used.
 * The certificate type enumeration includes a set of pre-defined values describing commonly
 * used certificate applications.
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
    kCertType_Root            = 0x01, /**< A CHIP Root certificate. */
    kCertType_ICA             = 0x02, /**< A CHIP Intermediate CA certificate. */
    kCertType_Node            = 0x03, /**< A CHIP node certificate. */
    kCertType_FirmwareSigning = 0x04, /**< A CHIP firmware signing certificate. Note that CHIP doesn't
                                           specify how firmware images are signed and implementation of
                                           firmware image signing is manufacturer-specific. The CHIP
                                           certificate format supports encoding of firmware signing
                                           certificates if chosen by the manufacturer to use them. */
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
    kExtPresent_KeyUsage         = 0x0002, /**< Key usage extension is present in the certificate. */
    kExtPresent_ExtendedKeyUsage = 0x0004, /**< Extended key usage extension is present in the certificate. */
    kExtPresent_SubjectKeyId     = 0x0008, /**< Subject key identifier extension is present in the certificate. */
    kExtPresent_AuthKeyId        = 0x0010, /**< Authority key identifier extension is present in the certificate. */
    kExtPresent_FutureIsCritical = 0x0020, /**< Future extension marked as critical is present in the certificate. */
    kPathLenConstraintPresent    = 0x0040, /**< Path length constraint is present in the certificate. */
    kIsCA                        = 0x0080, /**< Indicates that certificate is a CA certificate. */
    kIsTrustAnchor               = 0x0100, /**< Indicates that certificate is a trust anchor. */
    kTBSHashPresent              = 0x0200, /**< Indicates that TBS hash of the certificate was generated and stored. */
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

enum
{
    kNullCertTime = 0
};

/**
 *  @struct ChipRDN
 *
 *  @brief
 *    A data structure representing a Relative Distinguished Name (RDN) in a CHIP certificate.
 */
struct ChipRDN
{
    CharSpan mString;                                         /**< Attribute value when encoded as a string. */
    uint64_t mChipVal;                                        /**< CHIP specific DN attribute value. */
    chip::ASN1::OID mAttrOID = chip::ASN1::kOID_NotSpecified; /**< DN attribute CHIP OID. */
    bool mAttrIsPrintableString;                              /**< Specifies if attribute is a printable string type. */

    bool IsEqual(const ChipRDN & other) const;
    bool IsEmpty() const { return mAttrOID == chip::ASN1::kOID_NotSpecified; }
    void Clear()
    {
        mAttrOID               = chip::ASN1::kOID_NotSpecified;
        mAttrIsPrintableString = false;
        mChipVal               = 0;
        mString                = CharSpan{};
    }
};

/**
 *  @brief
 *    A data structure representing a Distinguished Name (DN) in a CHIP certificate.
 */
class ChipDN
{
public:
    ChipDN();
    ~ChipDN();

    void Clear();

    /**
     * @brief Add CHIP-specific attribute to the DN.
     *
     * @param oid     CHIP-specific OID for DN attribute.
     * @param val     CHIP-specific DN attribute value.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR AddAttribute(chip::ASN1::OID oid, uint64_t val);

    /**
     * @brief Add CASE Authenticated Tags (CATs) attributes to the DN.
     *
     * @param cats    Array of CAT values.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR AddCATs(const chip::CATValues & cats);

    /**
     * @brief Add string attribute to the DN.
     *
     * @param oid     String OID for DN attribute.
     * @param val     A CharSpan object containing a pointer and length of the DN string attribute
     *                buffer. The value in the buffer should remain valid while the object is in use.
     * @param isPrintableString  Specifies if attribute ASN1 type is a printable string.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR AddAttribute(chip::ASN1::OID oid, CharSpan val, bool isPrintableString);

    inline CHIP_ERROR AddAttribute_CommonName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_CommonName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_Surname(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_Surname, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_SerialNumber(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_SerialNumber, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_CountryName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_CountryName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_LocalityName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_LocalityName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_StateOrProvinceName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_StateOrProvinceName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_OrganizationName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_OrganizationName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_OrganizationalUnitName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_OrganizationalUnitName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_Title(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_Title, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_Name(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_Name, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_GivenName(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_GivenName, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_Initials(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_Initials, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_GenerationQualifier(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_GenerationQualifier, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_DNQualifier(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_DNQualifier, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_Pseudonym(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_Pseudonym, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_DomainComponent(CharSpan val, bool isPrintableString)
    {
        return AddAttribute(ASN1::kOID_AttributeType_DomainComponent, val, isPrintableString);
    }
    inline CHIP_ERROR AddAttribute_MatterNodeId(uint64_t val) { return AddAttribute(ASN1::kOID_AttributeType_MatterNodeId, val); }
    inline CHIP_ERROR AddAttribute_MatterFirmwareSigningId(uint64_t val)
    {
        return AddAttribute(ASN1::kOID_AttributeType_MatterFirmwareSigningId, val);
    }
    inline CHIP_ERROR AddAttribute_MatterICACId(uint64_t val) { return AddAttribute(ASN1::kOID_AttributeType_MatterICACId, val); }
    inline CHIP_ERROR AddAttribute_MatterRCACId(uint64_t val) { return AddAttribute(ASN1::kOID_AttributeType_MatterRCACId, val); }
    inline CHIP_ERROR AddAttribute_MatterFabricId(uint64_t val)
    {
        return AddAttribute(ASN1::kOID_AttributeType_MatterFabricId, val);
    }
    inline CHIP_ERROR AddAttribute_MatterCASEAuthTag(CASEAuthTag val)
    {
        return AddAttribute(ASN1::kOID_AttributeType_MatterCASEAuthTag, val);
    }

    /**
     * @brief Determine type of a CHIP certificate.
     *        This method performs an assessment of a certificate's type based on the structure
     *        of its subject DN.
     *
     * @param certType  A reference to the certificate type value.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR GetCertType(uint8_t & certType) const;

    /**
     * @brief Retrieve the ID of a CHIP certificate.
     *
     * @param certId  A reference to the certificate ID value.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR GetCertChipId(uint64_t & certId) const;

    /**
     * @brief Retrieve the Fabric ID of a CHIP certificate.
     **/
    CHIP_ERROR GetCertFabricId(uint64_t & fabricId) const;

    /**
     * @brief Encode ChipDN attributes in TLV form.
     **/
    CHIP_ERROR EncodeToTLV(chip::TLV::TLVWriter & writer, TLV::Tag tag) const;

    /**
     * @brief Decode ChipDN attributes from TLV encoded format.
     *
     * @param reader  A TLVReader positioned at the ChipDN TLV list.
     **/
    CHIP_ERROR DecodeFromTLV(chip::TLV::TLVReader & reader);

    /**
     * @brief Encode ChipDN attributes in ASN1 form.
     **/
    CHIP_ERROR EncodeToASN1(ASN1::ASN1Writer & writer) const;

    /**
     * @brief Decode ChipDN attributes from ASN1 encoded format.
     *
     * @param reader  A ASN1Reader positioned at the ChipDN ASN1 list.
     **/
    CHIP_ERROR DecodeFromASN1(ASN1::ASN1Reader & reader);

    bool IsEqual(const ChipDN & other) const;

    /**
     * @brief Determine if DN is empty (doesn't have DN attributes).
     *
     * @return true if DN is empty, false otherwise.
     **/
    bool IsEmpty() const { return RDNCount() == 0; }

    static_assert((CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES) >= 5, "Spec requires at least 5 RDN to be supported per Matter TLV cert");
    ChipRDN rdn[CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES];

    uint8_t RDNCount() const;
};

/**
 *  @brief  A data structure for holding a certificate key identifier, without the ownership of it.
 */
using CertificateKeyId = FixedByteSpan<kKeyIdentifierLength>;

/**
 *  @brief  A data structure for holding a P256 ECDSA signature, without the ownership of it.
 */
using P256ECDSASignatureSpan = FixedByteSpan<Crypto::kP256_ECDSA_Signature_Length_Raw>;

/**
 *  @brief  A data structure for holding a P256 Public Key, without the ownership of it.
 */
using P256PublicKeySpan = FixedByteSpan<Crypto::kP256_PublicKey_Length>;

/**
 *  @brief  A data structure for holding a P256 Integer, without the ownership of it.
 */
using P256IntegerSpan = FixedByteSpan<Crypto::kP256_FE_Length>;

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
    bool IsEqual(const ChipCertificateData & other) const;

    ByteSpan mCertificate;                      /**< Original raw buffer data. */
    ChipDN mSubjectDN;                          /**< Certificate Subject DN. */
    ChipDN mIssuerDN;                           /**< Certificate Issuer DN. */
    CertificateKeyId mSubjectKeyId;             /**< Certificate Subject public key identifier. */
    CertificateKeyId mAuthKeyId;                /**< Certificate Authority public key identifier. */
    uint32_t mNotBeforeTime;                    /**< Certificate validity: Not Before field. */
    uint32_t mNotAfterTime;                     /**< Certificate validity: Not After field. */
    P256PublicKeySpan mPublicKey;               /**< Certificate public key. */
    uint16_t mPubKeyCurveOID;                   /**< Public key Elliptic Curve CHIP OID. */
    uint16_t mPubKeyAlgoOID;                    /**< Public key algorithm CHIP OID. */
    uint16_t mSigAlgoOID;                       /**< Certificate signature algorithm CHIP OID. */
    BitFlags<CertFlags> mCertFlags;             /**< Certificate data flags. */
    BitFlags<KeyUsageFlags> mKeyUsageFlags;     /**< Certificate key usage extensions flags. */
    BitFlags<KeyPurposeFlags> mKeyPurposeFlags; /**< Certificate extended key usage extensions flags. */
    uint8_t mPathLenConstraint;                 /**< Basic constraint: path length. */
    P256ECDSASignatureSpan mSignature;          /**< Certificate signature. */

    uint8_t mTBSHash[Crypto::kSHA256_Hash_Length]; /**< Certificate TBS hash. */
};

/**
 * @brief Decode CHIP certificate.
 *        It is required that the CHIP certificate in the chipCert buffer stays valid while
 *        the certData is used.
 *
 * @param chipCert     Buffer containing CHIP certificate.
 * @param certData     Structure containing data extracted from the CHIP certificate.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR DecodeChipCert(const ByteSpan chipCert, ChipCertificateData & certData);

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
 * @param x509Cert        CHIP X.509 DER encoded certificate.
 * @param chipCert        Buffer to store converted certificate in CHIP format.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ConvertX509CertToChipCert(const ByteSpan x509Cert, MutableByteSpan & chipCert);

/**
 * @brief Convert CHIP certificate to the standard X.509 DER encoded certificate.
 *
 * @param chipCert        CHIP certificate in CHIP TLV encoding.
 * @param x509Cert        Buffer to store converted certificate in X.509 DER format.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR ConvertChipCertToX509Cert(const ByteSpan chipCert, MutableByteSpan & x509Cert);

/**
 * Validate CHIP Root CA Certificate (RCAC) in ByteSpan TLV-encoded form.
 * This function performs RCAC parsing, checks SubjectDN validity, verifies that SubjectDN
 * and IssuerDN are equal, verifies that SKID and AKID are equal, validates certificate signature.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR ValidateChipRCAC(const ByteSpan & rcac);

struct FutureExtension
{
    ByteSpan OID;
    ByteSpan Extension;
};

struct X509CertRequestParams
{
    int64_t SerialNumber;
    uint32_t ValidityStart;
    uint32_t ValidityEnd;
    ChipDN SubjectDN;
    ChipDN IssuerDN;
    Optional<FutureExtension> FutureExt;
};

/**
 * @brief Generate a new X.509 DER encoded Root CA certificate
 *
 * @param requestParams   Certificate request parameters.
 * @param issuerKeypair   The certificate signing key
 * @param x509Cert        Buffer to store signed certificate in X.509 DER format.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR NewRootX509Cert(const X509CertRequestParams & requestParams, Crypto::P256Keypair & issuerKeypair,
                           MutableByteSpan & x509Cert);

/**
 * @brief Generate a new X.509 DER encoded Intermediate CA certificate
 *
 * @param requestParams   Certificate request parameters.
 * @param subjectPubkey   The public key of subject
 * @param issuerKeypair   The certificate signing key
 * @param x509Cert        Buffer to store signed certificate in X.509 DER format.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR NewICAX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                          Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert);

/**
 * @brief Generate a new X.509 DER encoded Node operational certificate
 *
 * @param requestParams   Certificate request parameters.
 * @param subjectPubkey   The public key of subject
 * @param issuerKeypair   The certificate signing key
 * @param x509Cert        Buffer to store signed certificate in X.509 DER format.
 *
 * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
 **/
CHIP_ERROR NewNodeOperationalX509Cert(const X509CertRequestParams & requestParams, const Crypto::P256PublicKey & subjectPubkey,
                                      Crypto::P256Keypair & issuerKeypair, MutableByteSpan & x509Cert);

/**
 * @brief
 *   Convert a certificate date/time (in the form of an ASN.1 universal time structure) into a CHIP Epoch time.
 *
 * @note
 *   This function makes no attempt to verify the correct range of the input time other than year.
 *   Therefore callers must make sure the supplied values are valid prior to invocation.
 *
 * @param asn1Time   The calendar date/time to be converted.
 * @param epochTime  A reference to an integer that will receive CHIP Epoch time.
 *
 * @retval  #CHIP_NO_ERROR                      If the input time was successfully converted.
 * @retval  #ASN1_ERROR_UNSUPPORTED_ENCODING    If the input time contained a year value that could not
 *                                              be represented in a CHIP epoch UTC time value.
 **/
CHIP_ERROR ASN1ToChipEpochTime(const chip::ASN1::ASN1UniversalTime & asn1Time, uint32_t & epochTime);

/**
 * @brief
 *   Convert a CHIP epoch UTC time into an ASN.1 universal time structure.
 *
 * @param epochTime  A CHIP epoch UTC time to be converted.
 * @param asn1Time   A reference to an ASN1UniversalTime structure to receive the date/time.
 *
 * @retval  #CHIP_NO_ERROR                      If the input time was successfully converted.
 */
CHIP_ERROR ChipEpochToASN1Time(uint32_t epochTime, chip::ASN1::ASN1UniversalTime & asn1Time);

/**
 *  @return  True if the OID represents a CHIP-defined 64-bit distinguished named attribute.
 **/
inline bool IsChip64bitDNAttr(chip::ASN1::OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_MatterNodeId || oid == chip::ASN1::kOID_AttributeType_MatterFirmwareSigningId ||
            oid == chip::ASN1::kOID_AttributeType_MatterICACId || oid == chip::ASN1::kOID_AttributeType_MatterRCACId ||
            oid == chip::ASN1::kOID_AttributeType_MatterFabricId);
}

/**
 *  @return  True if the OID represents a CHIP-defined 32-bit distinguished named attribute.
 **/
inline bool IsChip32bitDNAttr(chip::ASN1::OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_MatterCASEAuthTag);
}

/**
 *  @return  True if the OID represents a CHIP-defined distinguished named attribute.
 **/
inline bool IsChipDNAttr(chip::ASN1::OID oid)
{
    return (IsChip64bitDNAttr(oid) || IsChip32bitDNAttr(oid));
}

/**
 * @brief Convert an ASN.1 DER encoded integer to a raw big-endian integer.
 *
 * @param derInt     P256 integer in ASN.1 DER encoded form.
 * @param rawInt     Buffer to store converted raw integer.
 * @param rawIntLen  The length of the converted raw integer.
 *
 * @retval  #CHIP_NO_ERROR  If the integer value was successfully converted.
 */
CHIP_ERROR ConvertIntegerDERToRaw(ByteSpan derInt, uint8_t * rawInt, const uint16_t rawIntLen);

/**
 * @brief Convert a raw CHIP signature to an ASN.1 DER encoded signature structure.
 *
 * @param[in]     rawSig  P256 ECDSA signature in raw form.
 * @param[in,out] derSig  Output buffer to receive the converted ASN.1 DER encoded signature.
 *                        `derSig` must be at least `kMax_ECDSA_Signature_Length_Der` bytes long.
 *                        The `derSig` size will be set to the actual DER encoded signature length on success.
 *
 * @retval  #CHIP_NO_ERROR  If the signature value was successfully converted.
 */
CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, MutableByteSpan & derSig);

/**
 * @brief Convert a raw CHIP ECDSA signature to an ASN.1 DER encoded signature structure.
 *
 * @param rawSig        P256 ECDSA signature in raw form.
 * @param writer        A reference to the ASN1Writer to store ASN.1 DER encoded signature.
 *
 * @retval  #CHIP_NO_ERROR  If the signature value was successfully converted.
 */
CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, ASN1::ASN1Writer & writer);

/**
 * @brief Convert an ASN.1 DER encoded ECDSA signature to a raw CHIP signature.
 *
 * @param reader        A reference to the ASN1Reader positioned at the beginning of the
 *                      DER encoded ECDSA signature.
 * @param writer        A reference to the TLVWriter to store TLV encoded ECDSA signature element.
 * @param tag           Tag to use for TLV encoded signature.
 *
 * @retval  #CHIP_NO_ERROR  If the signature value was successfully converted.
 */
CHIP_ERROR ConvertECDSASignatureDERToRaw(ASN1::ASN1Reader & reader, chip::TLV::TLVWriter & writer, uint64_t tag);

/**
 * Extract the Fabric ID from an operational certificate that has already been
 * parsed.
 *
 * This function can be used to extract Fabric ID from an ICA certificate.
 * These certificates may not contain a NodeID, so ExtractNodeIdFabricIdFromOpCert()
 * cannot be used for such certificates.
 *
 * @return CHIP_ERROR_NOT_FOUND if the passed-in cert does not have RDN
 * corresponding to FabricID.
 */
CHIP_ERROR ExtractFabricIdFromCert(const ChipCertificateData & cert, FabricId * fabricId);

/**
 * Extract Node ID and Fabric ID from an operational certificate that has already been
 * parsed.
 *
 * @return CHIP_ERROR_NOT_FOUND if the passed-in cert does not have at
 * least one NodeId RDN and one FabricId RDN in the Subject DN.  No other
 * validation (e.g. checkign that there is exactly one RDN of each type) is
 * performed.
 */
CHIP_ERROR ExtractNodeIdFabricIdFromOpCert(const ChipCertificateData & opcert, NodeId * nodeId, FabricId * fabricId);

/**
 * Extract Node ID, Fabric ID and Compressed Fabric ID from an operational
 * certificate and its associated root certificate.
 *
 * @return CHIP_ERROR on failure or CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(ByteSpan rcac, ByteSpan noc, CompressedFabricId & compressedFabricId,
                                                              FabricId & fabricId, NodeId & nodeId);

/**
 * Extract Node ID and Compressed Fabric ID from an operational certificate
 * and its associated root certificate.
 *
 * @return CHIP_ERROR on failure or CHIP_NO_ERROR otherwise.
 */
CHIP_ERROR ExtractNodeIdCompressedFabricIdFromOpCerts(ByteSpan rcac, ByteSpan noc, CompressedFabricId & compressedFabricId,
                                                      NodeId & nodeId);

/**
 * Extract CASE Authenticated Tags from an operational certificate in ByteSpan TLV-encoded form.
 *
 * All values in the 'cats' struct will be set either to a valid CAT value or zero (undefined) value.
 *
 * @return CHIP_ERROR_INVALID_ARGUMENT if the passed-in cert is not NOC.
 * @return CHIP_ERROR_BUFFER_TOO_SMALL if there are too many CATs in the NOC
 */
CHIP_ERROR ExtractCATsFromOpCert(const ByteSpan & opcert, CATValues & cats);

/**
 * Extract CASE Authenticated Tags from an operational certificate that has already been
 * parsed.
 *
 * All values in the 'cats' struct will be set either to a valid CAT value or to the kUndefinedCAT value.
 *
 * @return CHIP_ERROR_INVALID_ARGUMENT if the passed-in cert is not NOC.
 * @return CHIP_ERROR_BUFFER_TOO_SMALL if the passed-in CATs array is too small.
 */
CHIP_ERROR ExtractCATsFromOpCert(const ChipCertificateData & opcert, CATValues & cats);

/**
 * Extract Fabric ID from an operational certificate in ByteSpan TLV-encoded
 * form.  This does not perform any sort of validation on the certificate
 * structure other than parsing it.
 *
 * Can return any error that can be returned from parsing the cert or from the
 * ChipCertificateData* version of ExtractNodeIdFabricIdFromOpCert.
 */
CHIP_ERROR ExtractFabricIdFromCert(const ByteSpan & opcert, FabricId * fabricId);

/**
 * Extract Node ID and Fabric ID from an operational certificate in ByteSpan TLV-encoded
 * form.  This does not perform any sort of validation on the certificate
 * structure other than parsing it.
 *
 * Can return any error that can be returned from parsing the cert or from the
 * ChipCertificateData* version of ExtractNodeIdFabricIdFromOpCert.
 */
CHIP_ERROR ExtractNodeIdFabricIdFromOpCert(const ByteSpan & opcert, NodeId * nodeId, FabricId * fabricId);

/**
 * Extract Public Key from a chip certificate in ByteSpan TLV-encoded form.
 * This does not perform any sort of validation on the certificate structure
 * other than parsing it.
 *
 * Can return any error that can be returned from parsing the cert.
 */
CHIP_ERROR ExtractPublicKeyFromChipCert(const ByteSpan & chipCert, P256PublicKeySpan & publicKey);

/**
 * Extract Not Before Time from a chip certificate in ByteSpan TLV-encoded form.
 * Output format is seconds referenced from the CHIP epoch.
 *
 * This does not perform any sort of validation on the certificate structure
 * other than parsing it.
 *
 * @param chipCert CHIP certificate in TLV-encoded form
 * @param notBeforeChipEpochTime (out) certificate NotBefore time as seconds from the CHIP epoch
 * @return CHIP_NO_ERROR if certificate parsing was successful, else an appropriate CHIP_ERROR
 */
CHIP_ERROR ExtractNotBeforeFromChipCert(const ByteSpan & chipCert, chip::System::Clock::Seconds32 & notBeforeChipEpochTime);

/**
 * Extract Subject Key Identifier from a chip certificate in ByteSpan TLV-encoded form.
 * This does not perform any sort of validation on the certificate structure
 * other than parsing it.
 *
 * Can return any error that can be returned from parsing the cert.
 */
CHIP_ERROR ExtractSKIDFromChipCert(const ByteSpan & chipCert, CertificateKeyId & skid);

/**
 * Extract Subject Distinguished Name (DN) from a chip certificate in ByteSpan TLV-encoded form.
 * It is required that the certificate in the chipCert buffer stays valid while the `dn` output is used.
 *
 * Can return any error that can be returned from parsing the cert.
 */
CHIP_ERROR ExtractSubjectDNFromChipCert(const ByteSpan & chipCert, ChipDN & dn);

/**
 * Extract Subject Distinguished Name (DN) from a chip certificate in ByteSpan X509 DER-encoded form.
 * It is required that the certificate in the chipCert buffer stays valid while the `dn` output is used.
 *
 * Can return any error that can be returned from converting and parsing the cert.
 */
CHIP_ERROR ExtractSubjectDNFromX509Cert(const ByteSpan & x509Cert, ChipDN & dn);

} // namespace Credentials
} // namespace chip
