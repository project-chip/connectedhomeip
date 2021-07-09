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
    TLVType type;
    uint64_t tag;

    reader.Init(chipCert, chipCertLen);

    err = reader.Next();
    SuccessOrExit(err);

    type = reader.GetType();
    tag  = reader.GetTag();

    VerifyOrExit((type == kTLVType_Structure || type == kTLVType_Array) && (tag == AnonymousTag),
                 err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    err = LoadCert(reader, decodeFlags, ByteSpan(chipCert, chipCertLen));

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCert(TLVReader & reader, BitFlags<CertDecodeFlags> decodeFlags, ByteSpan chipCert)
{
    ASN1Writer writer; // ASN1Writer is used to encode TBS portion of the certificate for the purpose of signature
                       // validation, which should be performed on the TBS data encoded in ASN.1 DER form.
    ChipCertificateData cert;
    cert.Clear();

    // Must be positioned on the structure element representing the certificate.
    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_INVALID_ARGUMENT);

    cert.mCertificate = chipCert;

    {
        TLVType containerType;

        // Enter the certificate structure...
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        // Initialize an ASN1Writer and convert the TBS (to-be-signed) portion of the certificate to ASN.1 DER
        // encoding.  At the same time, parse various components within the certificate and set the corresponding
        // fields in the CertificateData object.
        writer.Init(mDecodeBuf, mDecodeBufSize);
        ReturnErrorOnFailure(DecodeConvertTBSCert(reader, writer, cert));

        // Verify the cert has both the Subject Key Id and Authority Key Id extensions present.
        // Only certs with both these extensions are supported for the purposes of certificate validation.
        VerifyOrReturnError(cert.mCertFlags.HasAll(CertFlags::kExtPresent_SubjectKeyId, CertFlags::kExtPresent_AuthKeyId),
                            CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        // Verify the cert was signed with ECDSA-SHA256. This is the only signature algorithm currently supported.
        VerifyOrReturnError(cert.mSigAlgoOID == kOID_SigAlgo_ECDSAWithSHA256, CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);

        // If requested, generate the hash of the TBS portion of the certificate...
        if (decodeFlags.Has(CertDecodeFlags::kGenerateTBSHash))
        {
            // Generate a SHA hash of the encoded TBS certificate.
            chip::Crypto::Hash_SHA256(mDecodeBuf, writer.GetLengthWritten(), cert.mTBSHash);

            cert.mCertFlags.Set(CertFlags::kTBSHashPresent);
        }

        // Decode the certificate's signature...
        ReturnErrorOnFailure(DecodeECDSASignature(reader, cert));

        // Verify no more elements in the certificate.
        ReturnErrorOnFailure(reader.VerifyEndOfContainer());

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
    }

    // If requested by the caller, mark the certificate as trusted.
    if (decodeFlags.Has(CertDecodeFlags::kIsTrustAnchor))
    {
        cert.mCertFlags.Set(CertFlags::kIsTrustAnchor);
    }

    // Check if this cert matches any currently loaded certificates
    for (uint32_t i = 0; i < mCertCount; i++)
    {
        if (cert.IsEqual(mCerts[i]))
        {
            // This cert is already loaded. Let's skip adding this cert.
            return CHIP_NO_ERROR;
        }
    }

    // Verify we have room for the new certificate.
    VerifyOrReturnError(mCertCount < mMaxCerts, CHIP_ERROR_NO_MEMORY);

    new (&mCerts[mCertCount]) ChipCertificateData(cert);
    mCertCount++;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen, BitFlags<CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    TLVReader reader;
    TLVType type;
    uint64_t tag;

    reader.Init(chipCerts, chipCertsLen);

    err = reader.Next();
    SuccessOrExit(err);

    type = reader.GetType();
    tag  = reader.GetTag();

    VerifyOrExit((type == kTLVType_Structure || type == kTLVType_Array) && (tag == AnonymousTag),
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
        if (cert.mSubjectKeyId.data_equal(subjectKeyId))
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
    P256PublicKey caPublicKey;
    P256ECDSASignature signature;
    uint16_t derSigLen;

    ReturnErrorOnFailure(
        ConvertECDSASignatureRawToDER(cert->mSignature, signature, static_cast<uint16_t>(signature.Capacity()), derSigLen));

    ReturnErrorOnFailure(signature.SetLength(derSigLen));

    memcpy(caPublicKey, caCert->mPublicKey.data(), caCert->mPublicKey.size());

    ReturnErrorOnFailure(caPublicKey.ECDSA_validate_hash_signature(cert->mTBSHash, chip::Crypto::kSHA256_Hash_Length, signature));

    return CHIP_NO_ERROR;
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
        // TODO - enable check for certificate validity dates
        // VerifyOrExit(context.mEffectiveTime >= cert->mNotBeforeTime, err = CHIP_ERROR_CERT_NOT_VALID_YET);
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
    if (cert->mIssuerDN.IsEqual(cert->mSubjectDN) && cert->mAuthKeyId.data_equal(cert->mSubjectKeyId))
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
    if (subjectDN.IsEmpty() && subjectKeyId.empty())
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
        if (!subjectKeyId.empty() && !candidateCert->mSubjectKeyId.data_equal(subjectKeyId))
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
    mSubjectKeyId      = CertificateKeyId();
    mAuthKeyId         = CertificateKeyId();
    mNotBeforeTime     = 0;
    mNotAfterTime      = 0;
    mPublicKey         = P256PublicKeySpan();
    mPubKeyCurveOID    = 0;
    mPubKeyAlgoOID     = 0;
    mSigAlgoOID        = 0;
    mPathLenConstraint = 0;
    mCertFlags.ClearAll();
    mKeyUsageFlags.ClearAll();
    mKeyPurposeFlags.ClearAll();
    mSignature = P256ECDSASignatureSpan();

    memset(mTBSHash, 0, sizeof(mTBSHash));
}

bool ChipCertificateData::IsEqual(const ChipCertificateData & other) const
{
    // TODO - Add an operator== on BitFlags class.
    return mSubjectDN.IsEqual(other.mSubjectDN) && mIssuerDN.IsEqual(other.mIssuerDN) &&
        mSubjectKeyId.data_equal(other.mSubjectKeyId) && mAuthKeyId.data_equal(other.mAuthKeyId) &&
        (mNotBeforeTime == other.mNotBeforeTime) && (mNotAfterTime == other.mNotAfterTime) &&
        mPublicKey.data_equal(other.mPublicKey) && (mPubKeyCurveOID == other.mPubKeyCurveOID) &&
        (mPubKeyAlgoOID == other.mPubKeyAlgoOID) && (mSigAlgoOID == other.mSigAlgoOID) &&
        (mCertFlags.Raw() == other.mCertFlags.Raw()) && (mKeyUsageFlags.Raw() == other.mKeyUsageFlags.Raw()) &&
        (mKeyPurposeFlags.Raw() == other.mKeyPurposeFlags.Raw()) && (mPathLenConstraint == other.mPathLenConstraint) &&
        mSignature.data_equal(other.mSignature) && (memcmp(mTBSHash, other.mTBSHash, sizeof(mTBSHash)) == 0);
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
        return mChipVal == other.mChipVal;
    }
    else
    {
        return mString.data_equal(other.mString);
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
    uint8_t rdnCount = RDNCount();

    VerifyOrReturnError(rdnCount < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(IsChipDNAttr(oid), CHIP_ERROR_INVALID_ARGUMENT);

    if (IsChip32bitDNAttr(oid))
    {
        VerifyOrReturnError(val <= UINT32_MAX, CHIP_ERROR_INVALID_ARGUMENT);
    }

    rdn[rdnCount].mAttrOID = oid;
    rdn[rdnCount].mChipVal = val;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::AddAttribute(chip::ASN1::OID oid, ByteSpan val)
{
    uint8_t rdnCount = RDNCount();

    VerifyOrReturnError(rdnCount < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(!IsChipDNAttr(oid), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(oid != kOID_NotSpecified, CHIP_ERROR_INVALID_ARGUMENT);

    rdn[rdnCount].mAttrOID = oid;
    rdn[rdnCount].mString  = val;

    return CHIP_NO_ERROR;
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

CHIP_ERROR ChipDN::GetCertChipId(uint64_t & chipId) const
{
    uint8_t rdnCount = RDNCount();

    chipId = 0;

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        switch (rdn[i].mAttrOID)
        {
        case kOID_AttributeType_ChipRootId:
        case kOID_AttributeType_ChipICAId:
        case kOID_AttributeType_ChipNodeId:
        case kOID_AttributeType_ChipFirmwareSigningId:
            VerifyOrReturnError(chipId == 0, CHIP_ERROR_WRONG_CERT_TYPE);

            chipId = rdn[i].mChipVal;
            break;
        default:
            break;
        }
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR ConvertIntegerDERToRaw(ByteSpan derInt, uint8_t * rawInt, const uint16_t rawIntLen)
{
    VerifyOrReturnError(!derInt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(rawInt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const uint8_t * derIntData = derInt.data();
    size_t derIntLen           = derInt.size();

    /* one leading zero is allowed for positive integer in ASN1 DER format */
    if (*derIntData == 0)
    {
        derIntData++;
        derIntLen--;
    }

    VerifyOrReturnError(derIntLen <= rawIntLen, CHIP_ERROR_INVALID_ARGUMENT);

    if (derIntLen > 0)
    {
        VerifyOrReturnError(*derIntData != 0, CHIP_ERROR_INVALID_ARGUMENT);
    }

    memset(rawInt, 0, (rawIntLen - derIntLen));
    memcpy(rawInt + (rawIntLen - derIntLen), derIntData, derIntLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertIntegerRawToDER(P256IntegerSpan rawInt, uint8_t * derInt, const uint16_t derIntBufSize, uint16_t & derIntLen)
{
    static_assert(rawInt.size() <= UINT16_MAX - 1, "P256 raw integer doesn't fit in a uint16_t");

    VerifyOrReturnError(!rawInt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(derInt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const uint8_t * rawIntData = rawInt.data();
    size_t rawIntLen           = rawInt.size();

    while (*rawIntData == 0)
    {
        rawIntData++;
        rawIntLen--;
    }

    if (*rawIntData & 0x80) /* Need Leading Zero */
    {
        VerifyOrReturnError(derIntBufSize >= rawIntLen + 1, CHIP_ERROR_BUFFER_TOO_SMALL);

        *derInt++ = 0;
        derIntLen = static_cast<uint16_t>(rawIntLen + 1);
    }
    else
    {
        VerifyOrReturnError(derIntBufSize >= rawIntLen, CHIP_ERROR_BUFFER_TOO_SMALL);

        derIntLen = static_cast<uint16_t>(rawIntLen);
    }

    memcpy(derInt, rawIntData, rawIntLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, uint8_t * derSig, const uint16_t derSigBufSize,
                                         uint16_t & derSigLen)
{
    ASN1Writer writer;

    VerifyOrReturnError(derSig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    writer.Init(derSig, derSigBufSize);

    ReturnErrorOnFailure(ConvertECDSASignatureRawToDER(rawSig, writer));

    derSigLen = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t derInt[kP256_FE_Length + 1];
    uint16_t derIntLen;

    VerifyOrReturnError(!rawSig.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    // Ecdsa-Sig-Value ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // r INTEGER
        ReturnErrorOnFailure(ConvertIntegerRawToDER(P256IntegerSpan(rawSig.data()), derInt, sizeof(derInt), derIntLen));
        ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, derInt, derIntLen));

        // s INTEGER
        ReturnErrorOnFailure(
            ConvertIntegerRawToDER(P256IntegerSpan(rawSig.data() + kP256_FE_Length), derInt, sizeof(derInt), derIntLen));
        ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, derInt, derIntLen));
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

} // namespace Credentials
} // namespace chip
