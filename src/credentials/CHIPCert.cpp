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
 *      This file implements objects for modeling and working with
 *      CHIP certificates.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stddef.h>

#include <asn1/ASN1.h>
#include <asn1/ASN1Macros.h>
#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <protocols/Protocols.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/TimeUtils.h>

namespace chip {
namespace Credentials {

using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Protocols;
using namespace chip::Crypto;

extern CHIP_ERROR DecodeConvertTBSCert(TLVReader & reader, ASN1Writer & writer, ChipCertificateData & certData);

ChipCertificateSet::ChipCertificateSet()
{
    Certs                = nullptr;
    CertCount            = 0;
    MaxCerts             = 0;
    mDecodeBuf           = nullptr;
    mDecodeBufSize       = 0;
    mMemoryAllocInternal = false;
}

CHIP_ERROR ChipCertificateSet::Init(uint8_t maxCertsArraySize, uint16_t decodeBufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(maxCertsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(decodeBufSize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    Certs = reinterpret_cast<ChipCertificateData *>(chip::Platform::MemoryAlloc(sizeof(ChipCertificateData) * maxCertsArraySize));
    VerifyOrExit(Certs != nullptr, err = CHIP_ERROR_NO_MEMORY);

    CertCount            = 0;
    MaxCerts             = maxCertsArraySize;
    mDecodeBuf           = nullptr;
    mDecodeBufSize       = decodeBufSize;
    mMemoryAllocInternal = true;

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::Init(ChipCertificateData * certsArray, uint8_t certsArraySize, uint8_t * decodeBuf,
                                    uint16_t decodeBufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(certsArray != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(certsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(decodeBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(decodeBufSize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    CertCount            = 0;
    Certs                = certsArray;
    MaxCerts             = certsArraySize;
    mDecodeBuf           = decodeBuf;
    mDecodeBufSize       = decodeBufSize;
    mMemoryAllocInternal = false;

exit:
    return err;
}

void ChipCertificateSet::Release()
{
    if (mMemoryAllocInternal)
    {
        if (Certs != nullptr)
        {
            chip::Platform::MemoryFree(Certs);
            Certs = nullptr;
        }
        if (mDecodeBuf != nullptr)
        {
            chip::Platform::MemoryFree(mDecodeBuf);
            mDecodeBuf = nullptr;
        }
    }
}

void ChipCertificateSet::Clear()
{
    memset(Certs, 0, sizeof(ChipCertificateData) * MaxCerts);
    CertCount = 0;
}

CHIP_ERROR ChipCertificateSet::LoadCert(const uint8_t * chipCert, uint32_t chipCertLen, uint16_t decodeFlags,
                                        ChipCertificateData *& cert)
{
    CHIP_ERROR err;
    TLVReader reader;

    reader.Init(chipCert, chipCertLen);
    reader.ImplicitProfileId = kProtocol_OpCredentials;

    err = reader.Next(kTLVType_Structure, ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificate));
    SuccessOrExit(err);

    err = LoadCert(reader, decodeFlags, cert);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCert(TLVReader & reader, uint16_t decodeFlags, ChipCertificateData *& cert)
{
    CHIP_ERROR err;
    ASN1Writer writer;
    uint8_t * decodeBuf = nullptr;

    cert = nullptr;

    // Must be positioned on the structure element representing the certificate.
    VerifyOrExit(reader.GetType() == kTLVType_Structure, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Verify we have room for the new certificate.
    VerifyOrExit(CertCount < MaxCerts, err = CHIP_ERROR_NO_MEMORY);

    if (mMemoryAllocInternal)
    {
        decodeBuf = reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(mDecodeBufSize));
        VerifyOrExit(decodeBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);
    }
    else
    {
        decodeBuf = mDecodeBuf;
    }

    cert = &Certs[CertCount];
    memset(cert, 0, sizeof(*cert));

    // Record the starting point of the certificate's elements.
    cert->EncodedCert = reader.GetReadPoint();

    {
        TLVType containerType;

        // Enter the certificate structure...
        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        // Initialize an ASN1Writer and convert the TBS (to-be-signed) portion of the certificate to ASN.1 DER
        // encoding.  At the same time, parse various components within the certificate and set the corresponding
        // fields in the CertificateData object.
        writer.Init(decodeBuf, mDecodeBufSize);
        err = DecodeConvertTBSCert(reader, writer, *cert);
        SuccessOrExit(err);

        // Verify the cert has both the Subject Key Id and Authority Key Id extensions present.
        // Only certs with both these extensions are supported for the purposes of certificate validation.
        {
            const uint16_t expectedFlags = kCertFlag_ExtPresent_SubjectKeyId | kCertFlag_ExtPresent_AuthKeyId;
            VerifyOrExit((cert->CertFlags & expectedFlags) == expectedFlags, err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
        }

        // Verify the cert was signed with ECDSA-SHA256. This is the only signature algorithms currently supported.
        VerifyOrExit(cert->SigAlgoOID == kOID_SigAlgo_ECDSAWithSHA256, err = CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);

        // If requested, generate the hash of the TBS portion of the certificate...
        if ((decodeFlags & kDecodeFlag_GenerateTBSHash) != 0)
        {
            // Finish writing the ASN.1 DER encoding of the TBS certificate.
            err = writer.Finalize();
            SuccessOrExit(err);

            // Generate a SHA hash of the encoded TBS certificate.
            chip::Crypto::Hash_SHA256(decodeBuf, writer.GetLengthWritten(), cert->TBSHash);

            cert->CertFlags |= kCertFlag_TBSHashPresent;
        }

        // Extract the certificate's signature...
        {
            TLVType containerType2;

            // Verify the tag and type
            VerifyOrExit(reader.GetType() == kTLVType_Structure, err = CHIP_ERROR_WRONG_TLV_TYPE);
            VerifyOrExit(reader.GetTag() == ContextTag(kTag_ECDSASignature), err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

            err = reader.EnterContainer(containerType2);
            SuccessOrExit(err);

            // Extract the signature r value
            err = reader.Next(kTLVType_ByteString, ContextTag(kTag_ECDSASignature_r));
            SuccessOrExit(err);
            err = reader.GetDataPtr(reinterpret_cast<const uint8_t *&>(cert->Signature.R));
            SuccessOrExit(err);
            cert->Signature.RLen = reader.GetLength();

            // Extract the signature s value
            err = reader.Next(kTLVType_ByteString, ContextTag(kTag_ECDSASignature_s));
            SuccessOrExit(err);
            err = reader.GetDataPtr(reinterpret_cast<const uint8_t *&>(cert->Signature.S));
            SuccessOrExit(err);
            cert->Signature.SLen = reader.GetLength();

            err = reader.ExitContainer(containerType2);
            SuccessOrExit(err);
        }

        err = reader.ExitContainer(containerType);
        SuccessOrExit(err);
    }

    // Record the overall size of the certificate.
    cert->EncodedCertLen = reader.GetReadPoint() - cert->EncodedCert;

    CertCount++;

    // If requested by the caller, mark the certificate as trusted.
    if (decodeFlags & kDecodeFlag_IsTrusted)
    {
        cert->CertFlags |= kCertFlag_IsTrusted;
    }

    // Assign a default type for the certificate based on its subject and attributes.
    err = DetermineCertType(*cert);
    SuccessOrExit(err);

exit:
    if (mMemoryAllocInternal)
    {
        if (decodeBuf != nullptr)
        {
            chip::Platform::MemoryFree(decodeBuf);
            decodeBuf = nullptr;
        }
    }

    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen, uint16_t decodeFlags)
{
    CHIP_ERROR err;
    TLVReader reader;
    TLVType type;
    uint64_t tag;

    reader.Init(chipCerts, chipCertsLen);
    reader.ImplicitProfileId = kProtocol_OpCredentials;

    err = reader.Next();
    SuccessOrExit(err);

    type = reader.GetType();
    tag  = reader.GetTag();

    VerifyOrExit((type == kTLVType_Structure && tag == ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificate)) ||
                     (type == kTLVType_Array && tag == ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificateList)),
                 err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    err = LoadCerts(reader, decodeFlags);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(TLVReader & reader, uint16_t decodeFlags)
{
    CHIP_ERROR err;
    ChipCertificateData * cert;

    // If positioned on a structure, we assume that structure is a single certificate.
    if (reader.GetType() == kTLVType_Structure)
    {
        err = LoadCert(reader, decodeFlags, cert);
        SuccessOrExit(err);
    }

    // Other we expect to be position on an Array or Path that contains a sequence of
    // zero or more certificates...
    else
    {
        TLVType containerType;

        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            err = LoadCert(reader, decodeFlags, cert);
            SuccessOrExit(err);
        }
        if (err != CHIP_END_OF_TLV)
            ExitNow();

        err = reader.ExitContainer(containerType);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::AddTrustedKey(uint64_t caId, OID curveOID, const uint8_t * pubKey, uint16_t pubKeyLen,
                                             const uint8_t * pubKeyId, uint16_t pubKeyIdLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipCertificateData * cert;

    // Verify we have room for the new certificate.
    VerifyOrExit(CertCount < MaxCerts, err = CHIP_ERROR_NO_MEMORY);

    cert = &Certs[CertCount];
    memset(cert, 0, sizeof(*cert));
    cert->SubjectDN.AttrOID          = kOID_AttributeType_ChipCAId;
    cert->SubjectDN.AttrValue.ChipId = caId;
    cert->IssuerDN                   = cert->SubjectDN;
    cert->PubKeyCurveOID             = curveOID;
    cert->PublicKey                  = pubKey;
    cert->PublicKeyLen               = pubKeyLen;
    cert->SubjectKeyId.Id            = pubKeyId;
    cert->SubjectKeyId.Len           = pubKeyIdLen;
    cert->AuthKeyId                  = cert->SubjectKeyId;
    cert->KeyUsageFlags              = kKeyUsageFlag_KeyCertSign;
    cert->CertType                   = kCertType_CA;
    cert->CertFlags = kCertFlag_AuthKeyIdPresent | kCertFlag_ExtPresent_AuthKeyId | kCertFlag_ExtPresent_BasicConstraints |
        kCertFlag_ExtPresent_SubjectKeyId | kCertFlag_ExtPresent_KeyUsage | kCertFlag_IsCA | kCertFlag_IsTrusted;

    CertCount++;

exit:
    return err;
}

const ChipCertificateData * ChipCertificateSet::FindCert(const CertificateKeyId & subjectKeyId) const
{
    for (uint8_t i = 0; i < CertCount; i++)
    {
        ChipCertificateData & cert = Certs[i];
        if (cert.SubjectKeyId.IsEqual(subjectKeyId))
            return &cert;
    }
    return nullptr;
}

CHIP_ERROR ChipCertificateSet::ValidateCert(ChipCertificateData & cert, ValidationContext & context)
{
    CHIP_ERROR err;

    VerifyOrExit(&cert >= Certs && &cert < &Certs[CertCount], err = CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_CONFIG_DEBUG_CERT_VALIDATION
    if (context.CertValidationResults != nullptr)
    {
        VerifyOrExit(context.CertValidationResultsLen >= CertCount, err = CHIP_ERROR_INVALID_ARGUMENT);

        for (uint8_t i = 0; i < context.CertValidationResultsLen; i++)
            context.CertValidationResults[i] = CHIP_CERT_NOT_USED;
    }
#endif

    context.TrustAnchor = nullptr;

    err = ValidateCert(cert, context, context.ValidateFlags, 0);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId,
                                             ValidationContext & context, ChipCertificateData *& cert)
{
    CHIP_ERROR err;

#if CHIP_CONFIG_DEBUG_CERT_VALIDATION

    if (context.CertValidationResults != nullptr)
    {
        VerifyOrExit(context.CertValidationResultsLen >= CertCount, err = CHIP_ERROR_INVALID_ARGUMENT);

        for (uint8_t i = 0; i < context.CertValidationResultsLen; i++)
            context.CertValidationResults[i] = CHIP_CERT_NOT_USED;
    }

#endif

    context.TrustAnchor = nullptr;

    err = FindValidCert(subjectDN, subjectKeyId, context, context.ValidateFlags, 0, cert);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::VerifySignature(ChipCertificateData & cert, ChipCertificateData & caCert)
{
    CHIP_ERROR err;
    P256PublicKey caPublicKey;
    P256ECDSASignature signature;
    uint8_t tmpBuf[100];
    ASN1Writer writer;

    writer.Init(tmpBuf, sizeof(tmpBuf));

    // Ecdsa-Sig-Value ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // r INTEGER
        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, cert.Signature.R, cert.Signature.RLen);
        SuccessOrExit(err);

        // s INTEGER
        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, cert.Signature.S, cert.Signature.SLen);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

    err = writer.Finalize();
    SuccessOrExit(err);

    memcpy(signature, tmpBuf, writer.GetLengthWritten());
    err = signature.SetLength(writer.GetLengthWritten());
    SuccessOrExit(err);

    memcpy(caPublicKey, caCert.PublicKey, caCert.PublicKeyLen);

    err = caPublicKey.ECDSA_validate_hash_signature(cert.TBSHash, chip::Crypto::kSHA256_Hash_Length, signature);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::ValidateCert(ChipCertificateData & cert, ValidationContext & context, uint16_t validateFlags,
                                            uint8_t depth)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    ChipCertificateData * caCert = nullptr;
    enum
    {
        kLastSecondOfDay = kSecondsPerDay - 1
    };

    // If the depth is greater than 0 then the certificate is required to be a CA certificate...
    if (depth > 0)
    {
        // Verify the isCA flag is present.
        VerifyOrExit((cert.CertFlags & kCertFlag_IsCA) != 0, err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify the key usage extension is present and contains the 'keyCertSign' flag.
        VerifyOrExit((cert.CertFlags & kCertFlag_ExtPresent_KeyUsage) != 0 && (cert.KeyUsageFlags & kKeyUsageFlag_KeyCertSign) != 0,
                     err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify that the certificate type is set to "CA".
        VerifyOrExit(cert.CertType == kCertType_CA, err = CHIP_ERROR_WRONG_CERT_TYPE);

        // If a path length constraint was included, verify the cert depth vs. the specified constraint.
        //
        // From the RFC, the path length constraint "gives the maximum number of non-self-issued
        // intermediate certificates that may follow this certificate in a valid certification path.
        // (Note: The last certificate in the certification path is not an intermediate certificate,
        // and is not included in this limit...)"
        //
        if ((cert.CertFlags & kCertFlag_PathLenConstPresent) != 0)
            VerifyOrExit((depth - 1) <= cert.PathLenConstraint, err = CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED);
    }

    // Otherwise verify the desired certificate usages/purposes/type given in the validation context...
    else
    {
        // If a set of desired key usages has been specified, verify that the key usage extension exists
        // in the certificate and that the corresponding usages are supported.
        if (context.RequiredKeyUsages != 0)
            VerifyOrExit((cert.CertFlags & kCertFlag_ExtPresent_KeyUsage) != 0 &&
                             (cert.KeyUsageFlags & context.RequiredKeyUsages) == context.RequiredKeyUsages,
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // If a set of desired key purposes has been specified, verify that the extended key usage extension
        // exists in the certificate and that the corresponding purposes are supported.
        if (context.RequiredKeyPurposes != 0)
            VerifyOrExit((cert.CertFlags & kCertFlag_ExtPresent_ExtendedKeyUsage) != 0 &&
                             (cert.KeyPurposeFlags & context.RequiredKeyPurposes) == context.RequiredKeyPurposes,
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // If a required certificate type has been specified, verify it against the current certificate's type.
        if (context.RequiredCertType != kCertType_NotSpecified)
            VerifyOrExit(cert.CertType == context.RequiredCertType, err = CHIP_ERROR_WRONG_CERT_TYPE);
    }

    // Verify the validity time of the certificate, if requested.
    if (cert.NotBeforeDate != 0 && (validateFlags & kValidateFlag_IgnoreNotBefore) == 0)
        VerifyOrExit(context.EffectiveTime >= PackedCertDateToTime(cert.NotBeforeDate), err = CHIP_ERROR_CERT_NOT_VALID_YET);
    if (cert.NotAfterDate != 0 && (validateFlags & kValidateFlag_IgnoreNotAfter) == 0)
        VerifyOrExit(context.EffectiveTime <= PackedCertDateToTime(cert.NotAfterDate) + kLastSecondOfDay,
                     err = CHIP_ERROR_CERT_EXPIRED);

    // If the certificate itself is trusted, then it is implicitly valid.  Record this certificate as the trust
    // anchor and return success.
    if ((cert.CertFlags & kCertFlag_IsTrusted) != 0)
    {
        context.TrustAnchor = &cert;
        ExitNow(err = CHIP_NO_ERROR);
    }

    // Otherwise we must validate the certificate by looking for a chain of valid certificates up to a trusted
    // certificate known as the 'trust anchor'.

    // Fail validation if the certificate is self-signed. Since we don't trust this certificate (see the check above) and
    // it has no path we can follow to a trust anchor, it can't be considered valid.
    if (cert.IssuerDN.IsEqual(cert.SubjectDN) && cert.AuthKeyId.IsEqual(cert.SubjectKeyId))
        ExitNow(err = CHIP_ERROR_CERT_NOT_TRUSTED);

    // Verify that the certificate depth is less than the total number of certificates. It is technically possible to create
    // a circular chain of certificates.  Limiting the maximum depth of the certificate path prevents infinite
    // recursion in such a case.
    VerifyOrExit(depth < CertCount, err = CHIP_ERROR_CERT_PATH_TOO_LONG);

    // Verify that a hash of the 'to-be-signed' portion of the certificate has been computed. We will need this to
    // verify the cert's signature below.
    VerifyOrExit((cert.CertFlags & kCertFlag_TBSHashPresent) != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Search for a valid CA certificate that matches the Issuer DN and Authority Key Id of the current certificate.
    // Fail if no acceptable certificate is found.
    err = FindValidCert(cert.IssuerDN, cert.AuthKeyId, context, validateFlags, depth + 1, caCert);
    if (err != CHIP_NO_ERROR)
        ExitNow(err = CHIP_ERROR_CA_CERT_NOT_FOUND);

    // Verify signature of the current certificate against public key of the CA certificate. If signature verification
    // succeeds, the current certificate is valid.
    err = VerifySignature(cert, *caCert);
    SuccessOrExit(err);

exit:

#if CHIP_CONFIG_DEBUG_CERT_VALIDATION
    if (context.CertValidationResults != nullptr)
        context.CertValidationResults[&cert - Certs] = err;
#endif

    return err;
}

CHIP_ERROR ChipCertificateSet::FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId,
                                             ValidationContext & context, uint16_t validateFlags, uint8_t depth,
                                             ChipCertificateData *& cert)
{
    CHIP_ERROR err;

    // Default error if we don't find any matching cert.
    err = (depth > 0) ? CHIP_ERROR_CA_CERT_NOT_FOUND : CHIP_ERROR_CERT_NOT_FOUND;

    // Fail immediately if neither of the input criteria are specified.
    if (subjectDN.IsEmpty() && subjectKeyId.IsEmpty())
        ExitNow();

    // For each cert in the set...
    for (uint8_t i = 0; i < CertCount; i++)
    {
        ChipCertificateData & candidateCert = Certs[i];

        // Skip the certificate if its subject DN and key id do not match the input criteria.
        if (!subjectDN.IsEmpty() && !candidateCert.SubjectDN.IsEqual(subjectDN))
            continue;
        if (!subjectKeyId.IsEmpty() && !candidateCert.SubjectKeyId.IsEqual(subjectKeyId))
            continue;

        // Attempt to validate the cert.  If the cert is valid, return it to the caller. Otherwise,
        // save the returned error and continue searching.  If there are no other matching certs this
        // will be the error returned to the caller.
        err = ValidateCert(candidateCert, context, validateFlags, depth);
        if (err == CHIP_NO_ERROR)
        {
            cert = &candidateCert;
            ExitNow();
        }
    }

    cert = nullptr;

exit:
    return err;
}

void ValidationContext::Reset()
{
    EffectiveTime     = 0;
    TrustAnchor       = nullptr;
    SigningCert       = nullptr;
    RequiredKeyUsages = kKeyUsage_NotSpecified;
    ValidateFlags     = 0;
#if CHIP_CONFIG_DEBUG_CERT_VALIDATION
    CertValidationResults    = CHIP_NO_ERROR;
    CertValidationResultsLen = 0;
#endif
    RequiredKeyPurposes = kKeyPurpose_NotSpecified;
    RequiredCertType    = kCertType_NotSpecified;
}

CHIP_ERROR DetermineCertType(ChipCertificateData & cert)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the BasicConstraints isCA flag is true...
    if ((cert.CertFlags & kCertFlag_IsCA) != 0)
    {
        // Verify the key usage extension is present and contains the 'keyCertSign' flag.
        VerifyOrExit((cert.CertFlags & kCertFlag_ExtPresent_KeyUsage) != 0 && (cert.KeyUsageFlags & kKeyUsageFlag_KeyCertSign) != 0,
                     err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Set the certificate type to CA.
        cert.CertType = kCertType_CA;
    }

    // If the certificate subject contains a ChipNodeId attribute set the certificate type to Node.
    else if (cert.SubjectDN.AttrOID == kOID_AttributeType_ChipNodeId)
    {
        cert.CertType = kCertType_Node;
    }

    // If the certificate subject contains a ChipSoftwarePublisherId attribute set the certificate type to FirmwareSigning.
    else if (cert.SubjectDN.AttrOID == kOID_AttributeType_ChipSoftwarePublisherId)
    {
        cert.CertType = kCertType_FirmwareSigning;
    }

    else
    {
        err = CHIP_ERROR_WRONG_CERT_TYPE;
    }

exit:
    return err;
}

bool ChipDN::IsEqual(const ChipDN & other) const
{
    if (AttrOID == kOID_Unknown || AttrOID == kOID_NotSpecified || AttrOID != other.AttrOID)
        return false;

    if (IsChipIdX509Attr(AttrOID))
        return AttrValue.ChipId == other.AttrValue.ChipId;
    else
        return (AttrValue.String.Len == other.AttrValue.String.Len &&
                memcmp(AttrValue.String.Value, other.AttrValue.String.Value, AttrValue.String.Len) == 0);
}

bool CertificateKeyId::IsEqual(const CertificateKeyId & other) const
{
    return Id != nullptr && other.Id != nullptr && Len == other.Len && memcmp(Id, other.Id, Len) == 0;
}

DLL_EXPORT CHIP_ERROR PackCertTime(const ASN1UniversalTime & time, uint32_t & packedTime)
{
    enum
    {
        kCertTimeBaseYear = 2020,
        kCertTimeMaxYear  = kCertTimeBaseYear +
            UINT32_MAX / (kMonthsPerYear * kMaxDaysPerMonth * kHoursPerDay * kMinutesPerHour * kSecondsPerMinute),
        kX509NoWellDefinedExpirationDateYear = 9999
    };

    // The packed time in a CHIP certificate cannot represent dates prior to 2020/01/01.
    if (time.Year < kCertTimeBaseYear)
        return ASN1_ERROR_UNSUPPORTED_ENCODING;

    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no well-defined expiration date'.
    // We represent that as a packed time value of 0, which for simplicity's sake is assigned to any
    // date in the associated year.
    if (time.Year == kX509NoWellDefinedExpirationDateYear)
    {
        packedTime = kNullCertTime;
        return CHIP_NO_ERROR;
    }

    // Technically packed certificate time values could grow beyond 32bits. However we restrict it here
    // to dates that fit within 32bits to reduce code size and eliminate the need for 64bit math.
    if (time.Year > kCertTimeMaxYear)
        return ASN1_ERROR_UNSUPPORTED_ENCODING;

    packedTime = time.Year - kCertTimeBaseYear;
    packedTime = packedTime * kMonthsPerYear + time.Month - 1;
    packedTime = packedTime * kMaxDaysPerMonth + time.Day - 1;
    packedTime = packedTime * kHoursPerDay + time.Hour;
    packedTime = packedTime * kMinutesPerHour + time.Minute;
    packedTime = packedTime * kSecondsPerMinute + time.Second;

    return CHIP_NO_ERROR;
}

DLL_EXPORT CHIP_ERROR UnpackCertTime(uint32_t packedTime, ASN1UniversalTime & time)
{
    enum
    {
        kCertTimeBaseYear                    = 2020,
        kX509NoWellDefinedExpirationDateYear = 9999,
    };

    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no well-defined expiration date'.
    // We represent that as a packed time value of 0.
    if (packedTime == kNullCertTime)
    {
        time.Year   = kX509NoWellDefinedExpirationDateYear;
        time.Month  = kMonthsPerYear;
        time.Day    = kMaxDaysPerMonth;
        time.Hour   = kHoursPerDay - 1;
        time.Minute = kMinutesPerHour - 1;
        time.Second = kSecondsPerMinute - 1;
    }

    else
    {
        time.Second = packedTime % kSecondsPerMinute;
        packedTime /= kSecondsPerMinute;

        time.Minute = packedTime % kMinutesPerHour;
        packedTime /= kMinutesPerHour;

        time.Hour = packedTime % kHoursPerDay;
        packedTime /= kHoursPerDay;

        time.Day = (packedTime % kMaxDaysPerMonth) + 1;
        packedTime /= kMaxDaysPerMonth;

        time.Month = (packedTime % kMonthsPerYear) + 1;
        packedTime /= kMonthsPerYear;

        time.Year = packedTime + kCertTimeBaseYear;
    }

    return CHIP_NO_ERROR;
}

DLL_EXPORT uint16_t PackedCertTimeToDate(uint32_t packedTime)
{
    return static_cast<uint16_t>(packedTime / kSecondsPerDay);
}

DLL_EXPORT uint32_t PackedCertDateToTime(uint16_t packedDate)
{
    return static_cast<uint32_t>(packedDate * kSecondsPerDay);
}

DLL_EXPORT uint32_t SecondsSinceEpochToPackedCertTime(uint32_t secondsSinceEpoch)
{
    chip::ASN1::ASN1UniversalTime asn1Time;
    uint32_t packedTime;

    // Convert seconds-since-epoch to calendar date and time and store in an ASN1UniversalTime structure.
    SecondsSinceEpochToCalendarTime(secondsSinceEpoch, asn1Time.Year, asn1Time.Month, asn1Time.Day, asn1Time.Hour, asn1Time.Minute,
                                    asn1Time.Second);

    // Convert the calendar date/time to a packed certificate date/time.
    PackCertTime(asn1Time, packedTime);

    return packedTime;
}

} // namespace Credentials
} // namespace chip
