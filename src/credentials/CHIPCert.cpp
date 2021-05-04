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
extern CHIP_ERROR DecodeECDSASignature(TLVReader & reader, ChipCertificateData & certData);

ChipCertificateSet::ChipCertificateSet()
{
    mCerts               = nullptr;
    mCertCount           = 0;
    mMaxCerts            = 0;
    mDecodeBuf           = nullptr;
    mDecodeBufSize       = 0;
    mMemoryAllocInternal = false;
}

ChipCertificateSet::~ChipCertificateSet()
{
    Release();
}

CHIP_ERROR ChipCertificateSet::Init(uint8_t maxCertsArraySize, uint16_t decodeBufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(maxCertsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    mCerts = reinterpret_cast<ChipCertificateData *>(chip::Platform::MemoryAlloc(sizeof(ChipCertificateData) * maxCertsArraySize));
    VerifyOrExit(mCerts != nullptr, err = CHIP_ERROR_NO_MEMORY);

    VerifyOrExit(decodeBufSize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    mDecodeBuf = reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(decodeBufSize));
    VerifyOrExit(mDecodeBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mMaxCerts            = maxCertsArraySize;
    mDecodeBufSize       = decodeBufSize;
    mMemoryAllocInternal = true;

    Clear();

exit:
    if (err != CHIP_NO_ERROR)
    {
        Release();
    }

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

    mCerts               = certsArray;
    mMaxCerts            = certsArraySize;
    mDecodeBuf           = decodeBuf;
    mDecodeBufSize       = decodeBufSize;
    mMemoryAllocInternal = false;

    Clear();

exit:
    return err;
}

void ChipCertificateSet::Release()
{
    if (mMemoryAllocInternal)
    {
        if (mCerts != nullptr)
        {
            Clear();
            chip::Platform::MemoryFree(mCerts);
            mCerts = nullptr;
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
    for (int i = 0; i < mMaxCerts; i++)
    {
        mCerts[i].Clear();
    }

    mCertCount = 0;
}

CHIP_ERROR ChipCertificateSet::LoadCert(const uint8_t * chipCert, uint32_t chipCertLen, BitFlags<CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    TLVReader reader;

    reader.Init(chipCert, chipCertLen);
    reader.ImplicitProfileId = Protocols::OpCredentials::Id.ToTLVProfileId();

    err = reader.Next(kTLVType_Structure, ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificate));
    SuccessOrExit(err);

    err = LoadCert(reader, decodeFlags);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCert(TLVReader & reader, BitFlags<CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    ASN1Writer writer; // ASN1Writer is used to encode TBS portion of the certificate for the purpose of signature
                       // validation, which should be performed on the TBS data encoded in ASN.1 DER form.
    ChipCertificateData * cert = nullptr;

    // Must be positioned on the structure element representing the certificate.
    VerifyOrExit(reader.GetType() == kTLVType_Structure, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Verify we have room for the new certificate.
    VerifyOrExit(mCertCount < mMaxCerts, err = CHIP_ERROR_NO_MEMORY);

    cert = new (&mCerts[mCertCount]) ChipCertificateData();

    {
        TLVType containerType;

        // Enter the certificate structure...
        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        // Initialize an ASN1Writer and convert the TBS (to-be-signed) portion of the certificate to ASN.1 DER
        // encoding.  At the same time, parse various components within the certificate and set the corresponding
        // fields in the CertificateData object.
        writer.Init(mDecodeBuf, mDecodeBufSize);
        err = DecodeConvertTBSCert(reader, writer, *cert);
        SuccessOrExit(err);

        // Verify the cert has both the Subject Key Id and Authority Key Id extensions present.
        // Only certs with both these extensions are supported for the purposes of certificate validation.
        VerifyOrExit(cert->mCertFlags.HasAll(CertFlags::kExtPresent_SubjectKeyId, CertFlags::kExtPresent_AuthKeyId),
                     err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        // Verify the cert was signed with ECDSA-SHA256. This is the only signature algorithm currently supported.
        VerifyOrExit(cert->mSigAlgoOID == kOID_SigAlgo_ECDSAWithSHA256, err = CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);

        // If requested, generate the hash of the TBS portion of the certificate...
        if (decodeFlags.Has(CertDecodeFlags::kGenerateTBSHash))
        {
            // Finish writing the ASN.1 DER encoding of the TBS certificate.
            err = writer.Finalize();
            SuccessOrExit(err);

            // Generate a SHA hash of the encoded TBS certificate.
            chip::Crypto::Hash_SHA256(mDecodeBuf, writer.GetLengthWritten(), cert->mTBSHash);

            cert->mCertFlags.Set(CertFlags::kTBSHashPresent);
        }

        // Decode the certificate's signature...
        err = DecodeECDSASignature(reader, *cert);
        SuccessOrExit(err);

        // Verify no more elements in the certificate.
        err = reader.VerifyEndOfContainer();
        SuccessOrExit(err);

        err = reader.ExitContainer(containerType);
        SuccessOrExit(err);
    }

    // If requested by the caller, mark the certificate as trusted.
    if (decodeFlags.Has(CertDecodeFlags::kIsTrustAnchor))
    {
        cert->mCertFlags.Set(CertFlags::kIsTrustAnchor);
    }

    mCertCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (cert != nullptr)
        {
            cert->~ChipCertificateData();
        }
    }

    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen, BitFlags<CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    TLVReader reader;
    TLVType type;
    uint64_t tag;

    reader.Init(chipCerts, chipCertsLen);
    reader.ImplicitProfileId = Protocols::OpCredentials::Id.ToTLVProfileId();

    err = reader.Next();
    SuccessOrExit(err);

    type = reader.GetType();
    tag  = reader.GetTag();

    VerifyOrExit(
        (type == kTLVType_Structure && tag == ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificate)) ||
            (type == kTLVType_Array && tag == ProfileTag(Protocols::OpCredentials::Id.ToTLVProfileId(), kTag_ChipCertificateArray)),
        err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    err = LoadCerts(reader, decodeFlags);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(TLVReader & reader, BitFlags<CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    uint8_t initialCertCount = mCertCount;

    // If positioned on a structure, we assume that structure is a single certificate.
    if (reader.GetType() == kTLVType_Structure)
    {
        err = LoadCert(reader, decodeFlags);
        SuccessOrExit(err);
    }

    // Other we expect to be positioned on an Array that contains a sequence of
    // zero or more certificates...
    else
    {
        TLVType containerType;

        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrExit(reader.GetTag() == AnonymousTag, err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

            err = LoadCert(reader, decodeFlags);
            SuccessOrExit(err);
        }
        if (err != CHIP_END_OF_TLV)
        {
            ExitNow();
        }

        err = reader.ExitContainer(containerType);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        for (uint8_t i = initialCertCount; i < mCertCount; i++)
        {
            mCerts[i].~ChipCertificateData();
        }
        mCertCount = initialCertCount;
    }

    return err;
}

const ChipCertificateData * ChipCertificateSet::FindCert(const CertificateKeyId & subjectKeyId) const
{
    for (uint8_t i = 0; i < mCertCount; i++)
    {
        ChipCertificateData & cert = mCerts[i];
        if (cert.mSubjectKeyId.IsEqual(subjectKeyId))
        {
            return &cert;
        }
    }

    return nullptr;
}

bool ChipCertificateSet::IsCertInTheSet(const ChipCertificateData * cert) const
{
    for (uint8_t i = 0; i < mCertCount; i++)
    {
        if (cert == &mCerts[i])
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR ChipCertificateSet::ValidateCert(const ChipCertificateData * cert, ValidationContext & context)
{
    CHIP_ERROR err;

    VerifyOrExit(IsCertInTheSet(cert), err = CHIP_ERROR_INVALID_ARGUMENT);

    context.mTrustAnchor = nullptr;

    err = ValidateCert(cert, context, context.mValidateFlags, 0);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId,
                                             ValidationContext & context, ChipCertificateData *& cert)
{
    CHIP_ERROR err;

    context.mTrustAnchor = nullptr;

    err = FindValidCert(subjectDN, subjectKeyId, context, context.mValidateFlags, 0, cert);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::VerifySignature(const ChipCertificateData * cert, const ChipCertificateData * caCert)
{
    static constexpr size_t kMaxBytesForDeferredLenList = sizeof(uint8_t *) + // size of a single pointer in the deferred list
        4 + // extra memory allocated for the deferred length field (kLengthFieldReserveSize - 1)
        3;  // the deferred length list is alligned to 32bit boundary

    CHIP_ERROR err;
    P256PublicKey caPublicKey;
    P256ECDSASignature signature;
    uint8_t tmpBuf[signature.Capacity() + kMaxBytesForDeferredLenList];
    ASN1Writer writer;

    writer.Init(tmpBuf, static_cast<uint32_t>(sizeof(tmpBuf)));

    // Ecdsa-Sig-Value ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // r INTEGER
        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, cert->mSignature.R, cert->mSignature.RLen);
        SuccessOrExit(err);

        // s INTEGER
        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, cert->mSignature.S, cert->mSignature.SLen);
        SuccessOrExit(err);
    }
    ASN1_END_SEQUENCE;

    err = writer.Finalize();
    SuccessOrExit(err);

    VerifyOrExit(writer.GetLengthWritten() <= signature.Capacity(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(signature, tmpBuf, writer.GetLengthWritten());
    err = signature.SetLength(writer.GetLengthWritten());
    SuccessOrExit(err);

    memcpy(caPublicKey, caCert->mPublicKey, caCert->mPublicKeyLen);

    err = caPublicKey.ECDSA_validate_hash_signature(cert->mTBSHash, chip::Crypto::kSHA256_Hash_Length, signature);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::ValidateCert(const ChipCertificateData * cert, ValidationContext & context,
                                            BitFlags<CertValidateFlags> validateFlags, uint8_t depth)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    ChipCertificateData * caCert = nullptr;
    uint8_t certType;

    err = cert->mSubjectDN.GetCertType(certType);
    SuccessOrExit(err);

    // Certificate with future-extension marked as "critical" is not allowed.
    VerifyOrExit(!cert->mCertFlags.Has(CertFlags::kExtPresent_FutureIsCritical), err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

    // If the depth is greater than 0 then the certificate is required to be a CA certificate...
    if (depth > 0)
    {
        // Verify the isCA flag is present.
        VerifyOrExit(cert->mCertFlags.Has(CertFlags::kIsCA), err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify the key usage extension is present and contains the 'keyCertSign' flag.
        VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_KeyUsage) && cert->mKeyUsageFlags.Has(KeyUsageFlags::kKeyCertSign),
                     err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify that the certificate type is set to Root or ICA.
        VerifyOrExit(certType == kCertType_ICA || certType == kCertType_Root, err = CHIP_ERROR_WRONG_CERT_TYPE);

        // If a path length constraint was included, verify the cert depth vs. the specified constraint.
        //
        // From the RFC, the path length constraint "gives the maximum number of non-self-issued
        // intermediate certificates that may follow this certificate in a valid certification path.
        // (Note: The last certificate in the certification path is not an intermediate certificate,
        // and is not included in this limit...)"
        //
        if (cert->mCertFlags.Has(CertFlags::kPathLenConstraintPresent))
        {
            VerifyOrExit((depth - 1) <= cert->mPathLenConstraint, err = CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED);
        }
    }

    // Otherwise verify the desired certificate usages/purposes/type given in the validation context...
    else
    {
        // If a set of desired key usages has been specified, verify that the key usage extension exists
        // in the certificate and that the corresponding usages are supported.
        if (context.mRequiredKeyUsages.HasAny())
        {
            VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_KeyUsage) &&
                             cert->mKeyUsageFlags.HasAll(context.mRequiredKeyUsages),
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
        }

        // If a set of desired key purposes has been specified, verify that the extended key usage extension
        // exists in the certificate and that the corresponding purposes are supported.
        if (context.mRequiredKeyPurposes.HasAny())
        {
            VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_ExtendedKeyUsage) &&
                             cert->mKeyPurposeFlags.HasAll(context.mRequiredKeyPurposes),
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
        }

        // If a required certificate type has been specified, verify it against the current certificate's type.
        if (context.mRequiredCertType != kCertType_NotSpecified)
        {
            VerifyOrExit(certType == context.mRequiredCertType, err = CHIP_ERROR_WRONG_CERT_TYPE);
        }
    }

    // Verify the validity time of the certificate, if requested.
    if (cert->mNotBeforeTime != 0 && !validateFlags.Has(CertValidateFlags::kIgnoreNotBefore))
    {
        VerifyOrExit(context.mEffectiveTime >= cert->mNotBeforeTime, err = CHIP_ERROR_CERT_NOT_VALID_YET);
    }
    if (cert->mNotAfterTime != 0 && !validateFlags.Has(CertValidateFlags::kIgnoreNotAfter))
    {
        VerifyOrExit(context.mEffectiveTime <= cert->mNotAfterTime, err = CHIP_ERROR_CERT_EXPIRED);
    }

    // If the certificate itself is trusted, then it is implicitly valid.  Record this certificate as the trust
    // anchor and return success.
    if (cert->mCertFlags.Has(CertFlags::kIsTrustAnchor))
    {
        context.mTrustAnchor = cert;
        ExitNow(err = CHIP_NO_ERROR);
    }

    // Otherwise we must validate the certificate by looking for a chain of valid certificates up to a trusted
    // certificate known as the 'trust anchor'.

    // Fail validation if the certificate is self-signed. Since we don't trust this certificate (see the check above) and
    // it has no path we can follow to a trust anchor, it can't be considered valid.
    if (cert->mIssuerDN.IsEqual(cert->mSubjectDN) && cert->mAuthKeyId.IsEqual(cert->mSubjectKeyId))
    {
        ExitNow(err = CHIP_ERROR_CERT_NOT_TRUSTED);
    }

    // Verify that the certificate depth is less than the total number of certificates. It is technically possible to create
    // a circular chain of certificates.  Limiting the maximum depth of the certificate path prevents infinite
    // recursion in such a case.
    VerifyOrExit(depth < mCertCount, err = CHIP_ERROR_CERT_PATH_TOO_LONG);

    // Verify that a hash of the 'to-be-signed' portion of the certificate has been computed. We will need this to
    // verify the cert's signature below.
    VerifyOrExit(cert->mCertFlags.Has(CertFlags::kTBSHashPresent), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Search for a valid CA certificate that matches the Issuer DN and Authority Key Id of the current certificate.
    // Fail if no acceptable certificate is found.
    err = FindValidCert(cert->mIssuerDN, cert->mAuthKeyId, context, validateFlags, static_cast<uint8_t>(depth + 1), caCert);
    if (err != CHIP_NO_ERROR)
    {
        ExitNow(err = CHIP_ERROR_CA_CERT_NOT_FOUND);
    }

    // Verify signature of the current certificate against public key of the CA certificate. If signature verification
    // succeeds, the current certificate is valid.
    err = VerifySignature(cert, caCert);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId,
                                             ValidationContext & context, BitFlags<CertValidateFlags> validateFlags, uint8_t depth,
                                             ChipCertificateData *& cert)
{
    CHIP_ERROR err;

    // Default error if we don't find any matching cert.
    err = (depth > 0) ? CHIP_ERROR_CA_CERT_NOT_FOUND : CHIP_ERROR_CERT_NOT_FOUND;

    // Fail immediately if neither of the input criteria are specified.
    if (subjectDN.IsEmpty() && subjectKeyId.IsEmpty())
    {
        ExitNow();
    }

    // For each cert in the set...
    for (uint8_t i = 0; i < mCertCount; i++)
    {
        ChipCertificateData * candidateCert = &mCerts[i];

        // Skip the certificate if its subject DN and key id do not match the input criteria.
        if (!subjectDN.IsEmpty() && !candidateCert->mSubjectDN.IsEqual(subjectDN))
        {
            continue;
        }
        if (!subjectKeyId.IsEmpty() && !candidateCert->mSubjectKeyId.IsEqual(subjectKeyId))
        {
            continue;
        }

        // Attempt to validate the cert.  If the cert is valid, return it to the caller. Otherwise,
        // save the returned error and continue searching.  If there are no other matching certs this
        // will be the error returned to the caller.
        err = ValidateCert(candidateCert, context, validateFlags, depth);
        if (err == CHIP_NO_ERROR)
        {
            cert = candidateCert;
            ExitNow();
        }
    }

    cert = nullptr;

exit:
    return err;
}

ChipCertificateData::ChipCertificateData() {}

ChipCertificateData::~ChipCertificateData() {}

void ChipCertificateData::Clear()
{
    mSubjectDN.Clear();
    mIssuerDN.Clear();
    mSubjectKeyId.Clear();
    mAuthKeyId.Clear();
    mNotBeforeTime  = 0;
    mNotAfterTime   = 0;
    mPublicKey      = nullptr;
    mPublicKeyLen   = 0;
    mPubKeyCurveOID = 0;
    mPubKeyAlgoOID  = 0;
    mSigAlgoOID     = 0;
    mCertFlags.ClearAll();
    mKeyUsageFlags.ClearAll();
    mKeyPurposeFlags.ClearAll();
    mPathLenConstraint = 0;
    mSignature.R       = nullptr;
    mSignature.RLen    = 0;
    mSignature.S       = nullptr;
    mSignature.SLen    = 0;
    memset(mTBSHash, 0, sizeof(mTBSHash));
}

void ValidationContext::Reset()
{
    mEffectiveTime = 0;
    mTrustAnchor   = nullptr;
    mSigningCert   = nullptr;
    mRequiredKeyUsages.ClearAll();
    mRequiredKeyPurposes.ClearAll();
    mValidateFlags.ClearAll();
    mRequiredCertType = kCertType_NotSpecified;
}

bool ChipRDN::IsEqual(const ChipRDN & other) const
{
    if (mAttrOID == kOID_Unknown || mAttrOID == kOID_NotSpecified || mAttrOID != other.mAttrOID)
    {
        return false;
    }

    if (IsChipDNAttr(mAttrOID))
    {
        return mAttrValue.mChipVal == other.mAttrValue.mChipVal;
    }
    else
    {
        return (mAttrValue.mString.mLen == other.mAttrValue.mString.mLen &&
                memcmp(mAttrValue.mString.mValue, other.mAttrValue.mString.mValue, mAttrValue.mString.mLen) == 0);
    }
}

ChipDN::ChipDN() {}

ChipDN::~ChipDN() {}

void ChipDN::Clear()
{
    for (uint8_t i = 0; i < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES; i++)
    {
        rdn[i].Clear();
    }
}

uint8_t ChipDN::RDNCount() const
{
    uint8_t count;

    for (count = 0; count < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES; count++)
    {
        if (rdn[count].IsEmpty())
        {
            break;
        }
    }

    return count;
}

CHIP_ERROR ChipDN::AddAttribute(chip::ASN1::OID oid, uint64_t val)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t rdnCount = RDNCount();

    VerifyOrExit(rdnCount < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(IsChipDNAttr(oid), err = CHIP_ERROR_INVALID_ARGUMENT);

    if (IsChip32bitDNAttr(oid))
    {
        VerifyOrExit(val <= UINT32_MAX, err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    rdn[rdnCount].mAttrOID            = oid;
    rdn[rdnCount].mAttrValue.mChipVal = val;

exit:
    return err;
}

CHIP_ERROR ChipDN::AddAttribute(chip::ASN1::OID oid, const uint8_t * val, uint32_t valLen)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t rdnCount = RDNCount();

    VerifyOrExit(rdnCount < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(!IsChipDNAttr(oid), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(oid != kOID_NotSpecified, err = CHIP_ERROR_INVALID_ARGUMENT);

    rdn[rdnCount].mAttrOID                  = oid;
    rdn[rdnCount].mAttrValue.mString.mValue = val;
    rdn[rdnCount].mAttrValue.mString.mLen   = valLen;

exit:
    return err;
}

CHIP_ERROR ChipDN::GetCertType(uint8_t & certType) const
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    uint8_t lCertType    = kCertType_NotSpecified;
    bool fabricIdPresent = false;
    uint8_t rdnCount     = RDNCount();

    certType = kCertType_NotSpecified;

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        if (rdn[i].mAttrOID == kOID_AttributeType_ChipRootId)
        {
            VerifyOrExit(lCertType == kCertType_NotSpecified, err = CHIP_ERROR_WRONG_CERT_TYPE);

            lCertType = kCertType_Root;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_ChipICAId)
        {
            VerifyOrExit(lCertType == kCertType_NotSpecified, err = CHIP_ERROR_WRONG_CERT_TYPE);

            lCertType = kCertType_ICA;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_ChipNodeId)
        {
            VerifyOrExit(lCertType == kCertType_NotSpecified, err = CHIP_ERROR_WRONG_CERT_TYPE);

            lCertType = kCertType_Node;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_ChipFirmwareSigningId)
        {
            VerifyOrExit(lCertType == kCertType_NotSpecified, err = CHIP_ERROR_WRONG_CERT_TYPE);

            lCertType = kCertType_FirmwareSigning;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_ChipFabricId)
        {
            // Only one fabricId attribute is allowed per DN.
            VerifyOrExit(!fabricIdPresent, err = CHIP_ERROR_WRONG_CERT_TYPE);

            fabricIdPresent = true;
        }
    }

    if (lCertType == kCertType_Node)
    {
        VerifyOrExit(fabricIdPresent, err = CHIP_ERROR_WRONG_CERT_TYPE);
    }

    certType = lCertType;

exit:
    return err;
}

bool ChipDN::IsEqual(const ChipDN & other) const
{
    bool res         = true;
    uint8_t rdnCount = RDNCount();

    VerifyOrExit(rdnCount > 0, res = false);
    VerifyOrExit(rdnCount == other.RDNCount(), res = false);

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        VerifyOrExit(rdn[i].IsEqual(other.rdn[i]), res = false);
    }

exit:
    return res;
}

bool CertificateKeyId::IsEqual(const CertificateKeyId & other) const
{
    return mId != nullptr && other.mId != nullptr && mLen == other.mLen && memcmp(mId, other.mId, mLen) == 0;
}

DLL_EXPORT CHIP_ERROR ASN1ToChipEpochTime(const chip::ASN1::ASN1UniversalTime & asn1Time, uint32_t & epochTime)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no well-defined expiration date'.
    // In CHIP certificate it is represented as a CHIP Epoch UTC time value of 0 sec (2020-01-01 00:00:00 UTC).
    if ((asn1Time.Year == kX509NoWellDefinedExpirationDateYear) && (asn1Time.Month == kMonthsPerYear) &&
        (asn1Time.Day == kMaxDaysPerMonth) && (asn1Time.Hour == kHoursPerDay - 1) && (asn1Time.Minute == kMinutesPerHour - 1) &&
        (asn1Time.Second == kSecondsPerMinute - 1))
    {
        epochTime = kNullCertTime;
    }
    else
    {
        if (!CalendarToChipEpochTime(asn1Time.Year, asn1Time.Month, asn1Time.Day, asn1Time.Hour, asn1Time.Minute, asn1Time.Second,
                                     epochTime))
        {
            ExitNow(err = ASN1_ERROR_UNSUPPORTED_ENCODING);
        }
    }

exit:
    return err;
}

DLL_EXPORT CHIP_ERROR ChipEpochToASN1Time(uint32_t epochTime, chip::ASN1::ASN1UniversalTime & asn1Time)
{
    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no well-defined expiration date'.
    // In CHIP certificate it is represented as a CHIP Epoch time value of 0 secs (2020-01-01 00:00:00 UTC).
    if (epochTime == kNullCertTime)
    {
        asn1Time.Year   = kX509NoWellDefinedExpirationDateYear;
        asn1Time.Month  = kMonthsPerYear;
        asn1Time.Day    = kMaxDaysPerMonth;
        asn1Time.Hour   = kHoursPerDay - 1;
        asn1Time.Minute = kMinutesPerHour - 1;
        asn1Time.Second = kSecondsPerMinute - 1;
    }
    else
    {
        ChipEpochToCalendarTime(epochTime, asn1Time.Year, asn1Time.Month, asn1Time.Day, asn1Time.Hour, asn1Time.Minute,
                                asn1Time.Second);
    }

    return CHIP_NO_ERROR;
}

} // namespace Credentials
} // namespace chip
