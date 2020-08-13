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
 *      working with CHIP security certificates.
 *
 */

#ifndef CHIPCERT_H_
#define CHIPCERT_H_

#include <string.h>

#include <core/CHIPTLV.h>
#include <support/ASN1.h>
#include <support/DLLUtil.h>
#include <support/crypto/EllipticCurve.h>
#include <support/crypto/HashAlgos.h>

namespace chip {
namespace Profiles {
namespace Security {

using chip::ASN1::OID;
using chip::Crypto::EncodedECDSASignature;
using chip::Crypto::EncodedECPrivateKey;
using chip::Crypto::EncodedECPublicKey;
using chip::TLV::TLVReader;
using chip::TLV::TLVWriter;

/** X.509 Certificate Key Purpose Flags
 */
enum
{
    kKeyPurposeFlag_ServerAuth      = 0x01,
    kKeyPurposeFlag_ClientAuth      = 0x02,
    kKeyPurposeFlag_CodeSigning     = 0x04,
    kKeyPurposeFlag_EmailProtection = 0x08,
    kKeyPurposeFlag_TimeStamping    = 0x10,
    kKeyPurposeFlag_OCSPSigning     = 0x20,

    kKeyPurposeFlag_Max = 0xFF,
};

/** X.509 Certificate Key Usage Flags
 */
enum
{
    kKeyUsageFlag_DigitalSignature = 0x0001,
    kKeyUsageFlag_NonRepudiation   = 0x0002,
    kKeyUsageFlag_KeyEncipherment  = 0x0004,
    kKeyUsageFlag_DataEncipherment = 0x0008,
    kKeyUsageFlag_KeyAgreement     = 0x0010,
    kKeyUsageFlag_KeyCertSign      = 0x0020,
    kKeyUsageFlag_CRLSign          = 0x0040,
    kKeyUsageFlag_EncipherOnly     = 0x0080,
    kKeyUsageFlag_DecipherOnly     = 0x0100,

    kKeyUsageFlag_Max = 0xFFFF,
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

    kCertFlag_Max = 0xFFFF,
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

// ChipDN -- Represents a Distinguished Name in a CHIP certificate.
class ChipDN
{
public:
    union
    {
        uint64_t ChipId;
        struct
        {
            const uint8_t * Value;
            uint32_t Len;
        } String;
    } AttrValue;
    OID AttrOID;

    bool IsEqual(const ChipDN & other) const;
    bool IsEmpty(void) const { return AttrOID == chip::ASN1::kOID_NotSpecified; }
    void Clear(void) { AttrOID = chip::ASN1::kOID_NotSpecified; }
};

// CertificateKeyId -- Represents a certificate key identifier.
class CertificateKeyId
{
public:
    const uint8_t * Id;
    uint8_t Len;

    bool IsEqual(const CertificateKeyId & other) const;
    bool IsEmpty(void) const { return Id == NULL; }
    void Clear(void) { Id = NULL; }
};

// ChipCertificateData -- In-memory representation of data extracted
//   from a CHIP certificate.
class ChipCertificateData
{
public:
    enum
    {
        kMaxTBSHashLen = chip::Platform::Security::SHA256::kHashLength
    };

    ChipDN SubjectDN;
    ChipDN IssuerDN;
    CertificateKeyId SubjectKeyId;
    CertificateKeyId AuthKeyId;
    union
    {
        EncodedECPublicKey EC;
    } PublicKey;
    union
    {
        EncodedECDSASignature EC;
    } Signature;
    uint32_t PubKeyCurveId;
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
    uint8_t TBSHash[kMaxTBSHashLen];
};

// ValidationContext -- Context information used during certification validation.
class ValidationContext
{
public:
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

// ChipCertificateSet -- Collection of CHIP certificate data providing methods for
//   certificate validation and signature verification.
class DLL_EXPORT ChipCertificateSet
{
public:
    typedef void * (*AllocFunct)(size_t size);
    typedef void (*FreeFunct)(void * p);

    ChipCertificateSet(void);

    ChipCertificateData * Certs; // [READ-ONLY] Pointer to array of certificate data
    uint8_t CertCount;           // [READ-ONLY] Number of certificates in Certs array
    uint8_t MaxCerts;            // [READ-ONLY] Length of Certs array

    CHIP_ERROR Init(uint8_t maxCerts, uint16_t decodeBufSize);
    CHIP_ERROR Init(uint8_t maxCerts, uint16_t decodeBufSize, AllocFunct allocFunct, FreeFunct freeFunct);
    CHIP_ERROR Init(ChipCertificateData * certBuf, uint8_t certBufSize, uint8_t * decodeBuf, uint16_t decodeBufSize);
    void Release(void);
    void Clear(void);

    CHIP_ERROR LoadCert(const uint8_t * weaveCert, uint32_t weaveCertLen, uint16_t decodeFlags, ChipCertificateData *& cert);
    CHIP_ERROR LoadCert(TLVReader & reader, uint16_t decodeFlags, ChipCertificateData *& cert);
    CHIP_ERROR LoadCerts(const uint8_t * encodedCerts, uint32_t encodedCertsLen, uint16_t decodeFlags);
    CHIP_ERROR LoadCerts(TLVReader & reader, uint16_t decodeFlags);
    CHIP_ERROR AddTrustedKey(uint64_t caId, uint32_t curveId, const EncodedECPublicKey & pubKey, const uint8_t * pubKeyId,
                             uint16_t pubKeyIdLen);
    CHIP_ERROR SaveCerts(TLVWriter & writer, ChipCertificateData * firstCert, bool includeTrusted);
    ChipCertificateData * FindCert(const CertificateKeyId & subjectKeyId) const;
    ChipCertificateData * LastCert(void) const { return (CertCount > 0) ? &Certs[CertCount - 1] : NULL; }

    CHIP_ERROR ValidateCert(ChipCertificateData & cert, ValidationContext & context);
    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             ChipCertificateData *& cert);

    CHIP_ERROR GenerateECDSASignature(const uint8_t * msgHash, uint8_t msgHashLen, ChipCertificateData & cert,
                                      const EncodedECPrivateKey & privKey, EncodedECDSASignature & encodedSig);
    CHIP_ERROR VerifyECDSASignature(const uint8_t * msgHash, uint8_t msgHashLen, const EncodedECDSASignature & encodedSig,
                                    ChipCertificateData & cert);

protected:
    AllocFunct mAllocFunct;
    FreeFunct mFreeFunct;
    uint8_t * mDecodeBuf;
    uint16_t mDecodeBufSize;

    CHIP_ERROR FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId, ValidationContext & context,
                             uint16_t validateFlags, uint8_t depth, ChipCertificateData *& cert);
    CHIP_ERROR ValidateCert(ChipCertificateData & cert, ValidationContext & context, uint16_t validateFlags, uint8_t depth);
};

extern CHIP_ERROR DecodeChipCert(const uint8_t * weaveCert, uint32_t weaveCertLen, ChipCertificateData & certData);
extern CHIP_ERROR DecodeChipCert(TLVReader & reader, ChipCertificateData & certData);

/**
 * Generate an ECDSA signature using local CHIP node's private key.
 *
 * When invoked, implementations must compute a signature on the given hash value using the node's
 * private key.
 *
 * @param[in] hash              A buffer containing the hash of the certificate to be signed.
 * @param[in] hashLen           The length in bytes of the hash.
 * @param[in] ecdsaSig          A reference to the ecdsa signature object, where result of
 *                              this function to be stored.
 *
 * @retval #CHIP_NO_ERROR      If the operation succeeded.
 */
typedef CHIP_ERROR (*GenerateECDSASignatureFunct)(const uint8_t * hash, uint8_t hashLen, EncodedECDSASignature & ecdsaSig);

extern CHIP_ERROR GenerateOperationalDeviceCert(uint64_t deviceId, EncodedECPublicKey & devicePubKey, uint8_t * cert,
                                                uint16_t certBufSize, uint16_t & certLen,
                                                GenerateECDSASignatureFunct genCertSignature);

extern CHIP_ERROR DecodeChipDN(TLVReader & reader, ChipDN & dn);

extern CHIP_ERROR ConvertX509CertToChipCert(const uint8_t * x509Cert, uint32_t x509CertLen, uint8_t * weaveCertBuf,
                                            uint32_t weaveCertBufSize, uint32_t & weaveCertLen);
extern CHIP_ERROR ConvertChipCertToX509Cert(const uint8_t * weaveCert, uint32_t weaveCertLen, uint8_t * x509CertBuf,
                                            uint32_t x509CertBufSize, uint32_t & x509CertLen);

extern CHIP_ERROR DetermineCertType(ChipCertificateData & cert);

extern CHIP_ERROR PackCertTime(const chip::ASN1::ASN1UniversalTime & time, uint32_t & packedTime);
extern CHIP_ERROR UnpackCertTime(uint32_t packedTime, chip::ASN1::ASN1UniversalTime & time);
extern uint16_t PackedCertTimeToDate(uint32_t packedTime);
extern uint32_t PackedCertDateToTime(uint16_t packedDate);
extern uint32_t SecondsSinceEpochToPackedCertTime(uint32_t secondsSinceEpoch);

inline void ValidationContext::Reset()
{
    memset((void *) this, 0, sizeof(*this));
}

// True if the OID represents a CHIP-defined X.509 distinguished named attribute.
inline bool IsChipX509Attr(OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_ChipDeviceId || oid == chip::ASN1::kOID_AttributeType_ChipServiceEndpointId ||
            oid == chip::ASN1::kOID_AttributeType_ChipCAId || oid == chip::ASN1::kOID_AttributeType_ChipSoftwarePublisherId);
}

// True if the OID represents a CHIP-defined X.509 distinguished named attribute
// that contains a 64-bit CHIP id.
inline bool IsChipIdX509Attr(OID oid)
{
    return (oid == chip::ASN1::kOID_AttributeType_ChipDeviceId || oid == chip::ASN1::kOID_AttributeType_ChipServiceEndpointId ||
            oid == chip::ASN1::kOID_AttributeType_ChipCAId || oid == chip::ASN1::kOID_AttributeType_ChipSoftwarePublisherId);
}

} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPCERT_H_ */
