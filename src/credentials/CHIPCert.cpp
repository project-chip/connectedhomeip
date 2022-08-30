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
 *      This file implements objects for modeling and working with
 *      CHIP certificates.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stddef.h>

#include <credentials/CHIPCert.h>
#include <credentials/CHIPCertificateSet.h>
#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TimeUtils.h>
#include <protocols/Protocols.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

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
    mMemoryAllocInternal = false;
}

ChipCertificateSet::~ChipCertificateSet()
{
    Release();
}

CHIP_ERROR ChipCertificateSet::Init(uint8_t maxCertsArraySize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(maxCertsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    mCerts = reinterpret_cast<ChipCertificateData *>(chip::Platform::MemoryAlloc(sizeof(ChipCertificateData) * maxCertsArraySize));
    VerifyOrExit(mCerts != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mMaxCerts            = maxCertsArraySize;
    mMemoryAllocInternal = true;

    Clear();

exit:
    if (err != CHIP_NO_ERROR)
    {
        Release();
    }

    return err;
}

CHIP_ERROR ChipCertificateSet::Init(ChipCertificateData * certsArray, uint8_t certsArraySize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(certsArray != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(certsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    mCerts               = certsArray;
    mMaxCerts            = certsArraySize;
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

CHIP_ERROR ChipCertificateSet::LoadCert(const ByteSpan chipCert, BitFlags<CertDecodeFlags> decodeFlags)
{
    TLVReader reader;

    reader.Init(chipCert);

    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag()));

    return LoadCert(reader, decodeFlags, chipCert);
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

        // If requested to generate the TBSHash.
        if (decodeFlags.Has(CertDecodeFlags::kGenerateTBSHash))
        {
            chip::Platform::ScopedMemoryBuffer<uint8_t> asn1TBSBuf;
            ReturnErrorCodeIf(!asn1TBSBuf.Alloc(kMaxCHIPCertDecodeBufLength), CHIP_ERROR_NO_MEMORY);

            // Initialize an ASN1Writer and convert the TBS (to-be-signed) portion of the certificate to ASN.1 DER
            // encoding. At the same time, parse various components within the certificate and set the corresponding
            // fields in the CertificateData object.
            writer.Init(asn1TBSBuf.Get(), kMaxCHIPCertDecodeBufLength);
            ReturnErrorOnFailure(DecodeConvertTBSCert(reader, writer, cert));

            // Generate a SHA hash of the encoded TBS certificate.
            chip::Crypto::Hash_SHA256(asn1TBSBuf.Get(), writer.GetLengthWritten(), cert.mTBSHash);

            cert.mCertFlags.Set(CertFlags::kTBSHashPresent);
        }
        else
        {
            // Initialize an ASN1Writer as a NullWriter.
            writer.InitNullWriter();
            ReturnErrorOnFailure(DecodeConvertTBSCert(reader, writer, cert));
        }

        // Verify the cert has both the Subject Key Id and Authority Key Id extensions present.
        // Only certs with both these extensions are supported for the purposes of certificate validation.
        VerifyOrReturnError(cert.mCertFlags.HasAll(CertFlags::kExtPresent_SubjectKeyId, CertFlags::kExtPresent_AuthKeyId),
                            CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        // Verify the cert was signed with ECDSA-SHA256. This is the only signature algorithm currently supported.
        VerifyOrReturnError(cert.mSigAlgoOID == kOID_SigAlgo_ECDSAWithSHA256, CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);

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

CHIP_ERROR ChipCertificateSet::ReleaseLastCert()
{
    ChipCertificateData * lastCert = (mCertCount > 0) ? &mCerts[mCertCount - 1] : nullptr;
    VerifyOrReturnError(lastCert != nullptr, CHIP_ERROR_INTERNAL);

    lastCert->~ChipCertificateData();
    --mCertCount;

    return CHIP_NO_ERROR;
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
    VerifyOrReturnError(IsCertInTheSet(cert), CHIP_ERROR_INVALID_ARGUMENT);

    context.mTrustAnchor = nullptr;

    return ValidateCert(cert, context, 0);
}

CHIP_ERROR ChipCertificateSet::FindValidCert(const ChipDN & subjectDN, const CertificateKeyId & subjectKeyId,
                                             ValidationContext & context, const ChipCertificateData ** certData)
{
    context.mTrustAnchor = nullptr;

    return FindValidCert(subjectDN, subjectKeyId, context, 0, certData);
}

CHIP_ERROR ChipCertificateSet::VerifySignature(const ChipCertificateData * cert, const ChipCertificateData * caCert)
{
#ifdef ENABLE_HSM_ECDSA_VERIFY
    P256PublicKeyHSM caPublicKey;
#else
    P256PublicKey caPublicKey;
#endif
    P256ECDSASignature signature;

    VerifyOrReturnError((cert != nullptr) && (caCert != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(signature.SetLength(cert->mSignature.size()));
    memcpy(signature, cert->mSignature.data(), cert->mSignature.size());

    memcpy(caPublicKey, caCert->mPublicKey.data(), caCert->mPublicKey.size());

    ReturnErrorOnFailure(caPublicKey.ECDSA_validate_hash_signature(cert->mTBSHash, chip::Crypto::kSHA256_Hash_Length, signature));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipCertificateSet::ValidateCert(const ChipCertificateData * cert, ValidationContext & context, uint8_t depth)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    const ChipCertificateData * caCert = nullptr;
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

    // Verify NotBefore and NotAfter validity of the certificates.
    //
    // See also ASN1ToChipEpochTime().
    //
    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no
    // well-defined expiration date'.  In CHIP TLV-encoded certificates, this
    // special value is represented as a CHIP Epoch time value of 0 sec
    // (2000-01-01 00:00:00 UTC).
    CertificateValidityResult validityResult;
    if (context.mEffectiveTime.Is<CurrentChipEpochTime>())
    {
        if (context.mEffectiveTime.Get<CurrentChipEpochTime>().count() < cert->mNotBeforeTime)
        {
            ChipLogDetail(SecureChannel, "Certificate's mNotBeforeTime (%" PRIu32 ") is after current time (%" PRIu32 ")",
                          cert->mNotBeforeTime, context.mEffectiveTime.Get<CurrentChipEpochTime>().count());
            validityResult = CertificateValidityResult::kNotYetValid;
        }
        else if (cert->mNotAfterTime != kNullCertTime &&
                 context.mEffectiveTime.Get<CurrentChipEpochTime>().count() > cert->mNotAfterTime)
        {
            ChipLogDetail(SecureChannel, "Certificate's mNotAfterTime (%" PRIu32 ") is before current time (%" PRIu32 ")",
                          cert->mNotAfterTime, context.mEffectiveTime.Get<CurrentChipEpochTime>().count());
            validityResult = CertificateValidityResult::kExpired;
        }
        else
        {
            validityResult = CertificateValidityResult::kValid;
        }
    }
    else if (context.mEffectiveTime.Is<LastKnownGoodChipEpochTime>())
    {
        // Last Known Good Time may not be moved forward except at the time of
        // commissioning or firmware update, so we can't use it to validate
        // NotBefore.  However, so long as firmware build times are properly
        // recorded and certificates loaded during commissioning are in fact
        // valid at the time of commissioning, observing a NotAfter that falls
        // before Last Known Good Time is a reliable indicator that the
        // certificate in question is expired.  Check for this.
        if (cert->mNotAfterTime != 0 && context.mEffectiveTime.Get<LastKnownGoodChipEpochTime>().count() > cert->mNotAfterTime)
        {
            ChipLogDetail(SecureChannel, "Certificate's mNotAfterTime (%" PRIu32 ") is before last known good time (%" PRIu32 ")",
                          cert->mNotAfterTime, context.mEffectiveTime.Get<LastKnownGoodChipEpochTime>().count());
            validityResult = CertificateValidityResult::kExpiredAtLastKnownGoodTime;
        }
        else
        {
            validityResult = CertificateValidityResult::kNotExpiredAtLastKnownGoodTime;
        }
    }
    else
    {
        validityResult = CertificateValidityResult::kTimeUnknown;
    }

    if (context.mValidityPolicy != nullptr)
    {
        SuccessOrExit(err = context.mValidityPolicy->ApplyCertificateValidityPolicy(cert, depth, validityResult));
    }
    else
    {
        switch (validityResult)
        {
        case CertificateValidityResult::kValid:
        case CertificateValidityResult::kNotExpiredAtLastKnownGoodTime:
        // By default, we do not enforce certificate validity based upon a Last
        // Known Good Time source.  However, implementations may always inject a
        // policy that does enforce based upon this.
        case CertificateValidityResult::kExpiredAtLastKnownGoodTime:
        case CertificateValidityResult::kTimeUnknown:
            break;
        case CertificateValidityResult::kNotYetValid:
            ExitNow(err = CHIP_ERROR_CERT_NOT_VALID_YET);
            break;
        case CertificateValidityResult::kExpired:
            ExitNow(err = CHIP_ERROR_CERT_EXPIRED);
            break;
        default:
            ExitNow(err = CHIP_ERROR_INTERNAL);
            break;
        }
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
    err = FindValidCert(cert->mIssuerDN, cert->mAuthKeyId, context, static_cast<uint8_t>(depth + 1), &caCert);
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
                                             ValidationContext & context, uint8_t depth, const ChipCertificateData ** certData)
{
    CHIP_ERROR err;

    *certData = nullptr;

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
        err = ValidateCert(candidateCert, context, depth);
        if (err == CHIP_NO_ERROR)
        {
            *certData = candidateCert;
            ExitNow();
        }
    }

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
    mEffectiveTime  = EffectiveTime{};
    mTrustAnchor    = nullptr;
    mValidityPolicy = nullptr;
    mRequiredKeyUsages.ClearAll();
    mRequiredKeyPurposes.ClearAll();
    mRequiredCertType = kCertType_NotSpecified;
}

bool ChipRDN::IsEqual(const ChipRDN & other) const
{
    if (mAttrOID == kOID_Unknown || mAttrOID == kOID_NotSpecified || mAttrOID != other.mAttrOID ||
        mAttrIsPrintableString != other.mAttrIsPrintableString)
    {
        return false;
    }

    if (IsChipDNAttr(mAttrOID))
    {
        return mChipVal == other.mChipVal;
    }

    return mString.data_equal(other.mString);
}

ChipDN::ChipDN()
{
    Clear();
}

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
        VerifyOrReturnError(CanCastTo<uint32_t>(val), CHIP_ERROR_INVALID_ARGUMENT);
    }

    rdn[rdnCount].mAttrOID               = oid;
    rdn[rdnCount].mChipVal               = val;
    rdn[rdnCount].mAttrIsPrintableString = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::AddCATs(const chip::CATValues & cats)
{
    VerifyOrReturnError(cats.AreValid(), CHIP_ERROR_INVALID_ARGUMENT);

    for (auto & cat : cats.values)
    {
        if (cat != kUndefinedCAT)
        {
            ReturnErrorOnFailure(AddAttribute_MatterCASEAuthTag(cat));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::AddAttribute(chip::ASN1::OID oid, CharSpan val, bool isPrintableString)
{
    uint8_t rdnCount = RDNCount();

    VerifyOrReturnError(rdnCount < CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(!IsChipDNAttr(oid), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(oid != kOID_NotSpecified, CHIP_ERROR_INVALID_ARGUMENT);

    rdn[rdnCount].mAttrOID               = oid;
    rdn[rdnCount].mString                = val;
    rdn[rdnCount].mAttrIsPrintableString = isPrintableString;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::GetCertType(uint8_t & certType) const
{
    uint8_t lCertType    = kCertType_NotSpecified;
    bool fabricIdPresent = false;
    bool catsPresent     = false;
    uint8_t rdnCount     = RDNCount();

    certType = kCertType_NotSpecified;

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        if (rdn[i].mAttrOID == kOID_AttributeType_MatterRCACId)
        {
            VerifyOrReturnError(lCertType == kCertType_NotSpecified, CHIP_ERROR_WRONG_CERT_DN);

            lCertType = kCertType_Root;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_MatterICACId)
        {
            VerifyOrReturnError(lCertType == kCertType_NotSpecified, CHIP_ERROR_WRONG_CERT_DN);

            lCertType = kCertType_ICA;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_MatterNodeId)
        {
            VerifyOrReturnError(lCertType == kCertType_NotSpecified, CHIP_ERROR_WRONG_CERT_DN);
            VerifyOrReturnError(IsOperationalNodeId(rdn[i].mChipVal), CHIP_ERROR_WRONG_NODE_ID);
            lCertType = kCertType_Node;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_MatterFirmwareSigningId)
        {
            VerifyOrReturnError(lCertType == kCertType_NotSpecified, CHIP_ERROR_WRONG_CERT_DN);

            lCertType = kCertType_FirmwareSigning;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_MatterFabricId)
        {
            // Only one fabricId attribute is allowed per DN.
            VerifyOrReturnError(!fabricIdPresent, CHIP_ERROR_WRONG_CERT_DN);
            VerifyOrReturnError(IsValidFabricId(rdn[i].mChipVal), CHIP_ERROR_WRONG_CERT_DN);
            fabricIdPresent = true;
        }
        else if (rdn[i].mAttrOID == kOID_AttributeType_MatterCASEAuthTag)
        {
            VerifyOrReturnError(CanCastTo<CASEAuthTag>(rdn[i].mChipVal), CHIP_ERROR_WRONG_CERT_DN);
            VerifyOrReturnError(IsValidCASEAuthTag(static_cast<CASEAuthTag>(rdn[i].mChipVal)), CHIP_ERROR_WRONG_CERT_DN);
            catsPresent = true;
        }
    }

    if (lCertType == kCertType_Node)
    {
        VerifyOrReturnError(fabricIdPresent, CHIP_ERROR_WRONG_CERT_DN);
    }
    else
    {
        VerifyOrReturnError(!catsPresent, CHIP_ERROR_WRONG_CERT_DN);
    }

    certType = lCertType;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::GetCertChipId(uint64_t & chipId) const
{
    uint8_t rdnCount = RDNCount();

    chipId = 0;

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        switch (rdn[i].mAttrOID)
        {
        case kOID_AttributeType_MatterRCACId:
        case kOID_AttributeType_MatterICACId:
        case kOID_AttributeType_MatterNodeId:
        case kOID_AttributeType_MatterFirmwareSigningId:
            VerifyOrReturnError(chipId == 0, CHIP_ERROR_WRONG_CERT_DN);

            chipId = rdn[i].mChipVal;
            break;
        default:
            break;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::GetCertFabricId(uint64_t & fabricId) const
{
    uint8_t rdnCount = RDNCount();

    fabricId = kUndefinedFabricId;

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        switch (rdn[i].mAttrOID)
        {
        case kOID_AttributeType_MatterFabricId:
            // Ensure only one FabricID RDN present, since start value is kUndefinedFabricId, which is reserved and never seen.
            VerifyOrReturnError(fabricId == kUndefinedFabricId, CHIP_ERROR_WRONG_CERT_DN);
            VerifyOrReturnError(IsValidFabricId(rdn[i].mChipVal), CHIP_ERROR_WRONG_CERT_DN);
            fabricId = rdn[i].mChipVal;
            break;
        default:
            break;
        }
    }

    VerifyOrReturnError(IsValidFabricId(fabricId), CHIP_ERROR_WRONG_CERT_DN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::EncodeToTLV(TLVWriter & writer, Tag tag) const
{
    TLVType outerContainer;
    uint8_t rdnCount = RDNCount();

    ReturnErrorOnFailure(writer.StartContainer(tag, kTLVType_List, outerContainer));

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        // Derive the TLV tag number from the enum value assigned to the attribute type OID. For attributes that can be
        // either UTF8String or PrintableString, use the high bit in the tag number to distinguish the two.
        uint8_t tlvTagNum = GetOIDEnum(rdn[i].mAttrOID);
        if (rdn[i].mAttrIsPrintableString)
        {
            tlvTagNum |= 0x80;
        }

        if (IsChipDNAttr(rdn[i].mAttrOID))
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(tlvTagNum), rdn[i].mChipVal));
        }
        else
        {
            ReturnErrorOnFailure(writer.PutString(ContextTag(tlvTagNum), rdn[i].mString));
        }
    }

    return writer.EndContainer(outerContainer);
}

CHIP_ERROR ChipDN::DecodeFromTLV(TLVReader & reader)
{
    CHIP_ERROR err;
    TLVType outerContainer;

    static constexpr uint32_t kOID_AttributeIsPrintableString_Flag = 0x00000080;
    static constexpr uint32_t kOID_AttributeType_Mask              = 0x0000007F;

    VerifyOrReturnError(reader.GetType() == kTLVType_List, CHIP_ERROR_WRONG_TLV_TYPE);

    // Enter the List TLV element that represents the DN in TLV format.
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer));

    // Read the RDN attributes in the List.
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // Get the TLV tag, make sure it is a context tag and extract the context tag number.
        Tag tlvTag = reader.GetTag();
        VerifyOrReturnError(IsContextTag(tlvTag), CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tlvTagNum = TagNumFromTag(tlvTag);

        // Derive the OID of the corresponding ASN.1 attribute from the TLV tag number.
        // The numeric value of the OID is encoded in the bottom 7 bits of the TLV tag number.
        // This eliminates the need for a translation table/switch statement but has the
        // effect of tying the two encodings together.
        //
        // NOTE: In the event that the computed OID value is not one that we recognize
        // (specifically, is not in the table of OIDs defined in ASN1OID.h) then the
        // macro call below that encodes the attribute's object id (ASN1_ENCODE_OBJECT_ID)
        // will fail for lack of the OID's encoded representation.  Given this there's no
        // need to test the validity of the OID here.
        //
        OID attrOID = GetOID(kOIDCategory_AttributeType, static_cast<uint8_t>(tlvTagNum & kOID_AttributeType_Mask));

        bool attrIsPrintableString = (tlvTagNum & kOID_AttributeIsPrintableString_Flag) == kOID_AttributeIsPrintableString_Flag;

        // For 64-bit CHIP-defined DN attributes.
        if (IsChip64bitDNAttr(attrOID))
        {
            uint64_t chipAttr;
            VerifyOrReturnError(attrIsPrintableString == false, CHIP_ERROR_INVALID_TLV_TAG);
            ReturnErrorOnFailure(reader.Get(chipAttr));
            if (attrOID == chip::ASN1::kOID_AttributeType_MatterNodeId)
            {
                VerifyOrReturnError(IsOperationalNodeId(attrOID), CHIP_ERROR_WRONG_NODE_ID);
            }
            else if (attrOID == chip::ASN1::kOID_AttributeType_MatterFabricId)
            {
                VerifyOrReturnError(IsValidFabricId(attrOID), CHIP_ERROR_INVALID_ARGUMENT);
            }
            ReturnErrorOnFailure(AddAttribute(attrOID, chipAttr));
        }
        // For 32-bit CHIP-defined DN attributes.
        else if (IsChip32bitDNAttr(attrOID))
        {
            uint32_t chipAttr;
            VerifyOrReturnError(attrIsPrintableString == false, CHIP_ERROR_INVALID_TLV_TAG);
            ReturnErrorOnFailure(reader.Get(chipAttr));
            if (attrOID == chip::ASN1::kOID_AttributeType_MatterCASEAuthTag)
            {
                VerifyOrReturnError(IsValidCASEAuthTag(chipAttr), CHIP_ERROR_INVALID_ARGUMENT);
            }
            ReturnErrorOnFailure(AddAttribute(attrOID, chipAttr));
        }
        // Otherwise the attribute is one of the supported X.509 attributes
        else
        {
            CharSpan asn1Attr;
            ReturnErrorOnFailure(reader.Get(asn1Attr));
            ReturnErrorOnFailure(AddAttribute(attrOID, asn1Attr, attrIsPrintableString));
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerContainer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDN::EncodeToASN1(ASN1Writer & writer) const
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t rdnCount = RDNCount();

    ASN1_START_SEQUENCE
    {
        for (uint8_t i = 0; i < rdnCount; i++)
        {
            ASN1_START_SET
            {
                char chipAttrStr[kChip64bitAttrUTF8Length];
                CharSpan asn1Attr;
                uint8_t asn1Tag;
                chip::ASN1::OID attrOID = rdn[i].mAttrOID;

                if (IsChip64bitDNAttr(attrOID))
                {
                    ReturnErrorOnFailure(
                        Encoding::Uint64ToHex(rdn[i].mChipVal, chipAttrStr, sizeof(chipAttrStr), Encoding::HexFlags::kUppercase));
                    asn1Attr = CharSpan(chipAttrStr, kChip64bitAttrUTF8Length);
                    asn1Tag  = kASN1UniversalTag_UTF8String;
                }
                else if (IsChip32bitDNAttr(attrOID))
                {
                    ReturnErrorOnFailure(Encoding::Uint32ToHex(static_cast<uint32_t>(rdn[i].mChipVal), chipAttrStr,
                                                               sizeof(chipAttrStr), Encoding::HexFlags::kUppercase));
                    asn1Attr = CharSpan(chipAttrStr, kChip32bitAttrUTF8Length);
                    asn1Tag  = kASN1UniversalTag_UTF8String;
                }
                else
                {
                    asn1Attr = rdn[i].mString;

                    // Determine the appropriate ASN.1 tag for the DN attribute.
                    // - DomainComponent is always an IA5String.
                    // - For all other ASN.1 defined attributes, bit 0x80 in the TLV tag value conveys whether the attribute
                    //   is a UTF8String or a PrintableString (in some cases the certificate generator has a choice).
                    if (attrOID == kOID_AttributeType_DomainComponent)
                    {
                        asn1Tag = kASN1UniversalTag_IA5String;
                    }
                    else
                    {
                        asn1Tag = rdn[i].mAttrIsPrintableString ? kASN1UniversalTag_PrintableString : kASN1UniversalTag_UTF8String;
                    }
                }

                // AttributeTypeAndValue ::= SEQUENCE
                ASN1_START_SEQUENCE
                {
                    // type AttributeType
                    // AttributeType ::= OBJECT IDENTIFIER
                    ASN1_ENCODE_OBJECT_ID(attrOID);

                    VerifyOrReturnError(CanCastTo<uint16_t>(asn1Attr.size()), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

                    // value AttributeValue
                    // AttributeValue ::= ANY -- DEFINED BY AttributeType
                    ReturnErrorOnFailure(writer.PutString(asn1Tag, asn1Attr.data(), static_cast<uint16_t>(asn1Attr.size())));
                }
                ASN1_END_SEQUENCE;
            }
            ASN1_END_SET;
        }
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR ChipDN::DecodeFromASN1(ASN1Reader & reader)
{
    CHIP_ERROR err;

    // RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
    ASN1_PARSE_ENTER_SEQUENCE
    {
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            // RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
            ASN1_ENTER_SET
            {
                // AttributeTypeAndValue ::= SEQUENCE
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    // type AttributeType
                    // AttributeType ::= OBJECT IDENTIFIER
                    OID attrOID;
                    ASN1_PARSE_OBJECT_ID(attrOID);
                    VerifyOrReturnError(GetOIDCategory(attrOID) == kOIDCategory_AttributeType, ASN1_ERROR_INVALID_ENCODING);

                    // AttributeValue ::= ANY -- DEFINED BY AttributeType
                    ASN1_PARSE_ANY;

                    uint8_t attrTag = reader.GetTag();

                    // Can only support UTF8String, PrintableString and IA5String.
                    VerifyOrReturnError(reader.GetClass() == kASN1TagClass_Universal &&
                                            (attrTag == kASN1UniversalTag_PrintableString ||
                                             attrTag == kASN1UniversalTag_UTF8String || attrTag == kASN1UniversalTag_IA5String),
                                        ASN1_ERROR_UNSUPPORTED_ENCODING);

                    // CHIP attributes must be UTF8Strings.
                    if (IsChipDNAttr(attrOID))
                    {
                        VerifyOrReturnError(attrTag == kASN1UniversalTag_UTF8String, ASN1_ERROR_INVALID_ENCODING);
                    }

                    // If 64-bit CHIP attribute.
                    if (IsChip64bitDNAttr(attrOID))
                    {
                        uint64_t chipAttr;
                        VerifyOrReturnError(Encoding::UppercaseHexToUint64(reinterpret_cast<const char *>(reader.GetValue()),
                                                                           static_cast<size_t>(reader.GetValueLen()),
                                                                           chipAttr) == sizeof(uint64_t),
                                            ASN1_ERROR_INVALID_ENCODING);

                        if (attrOID == chip::ASN1::kOID_AttributeType_MatterNodeId)
                        {
                            VerifyOrReturnError(IsOperationalNodeId(chipAttr), CHIP_ERROR_WRONG_NODE_ID);
                        }
                        else if (attrOID == chip::ASN1::kOID_AttributeType_MatterFabricId)
                        {
                            VerifyOrReturnError(IsValidFabricId(chipAttr), CHIP_ERROR_WRONG_CERT_DN);
                        }

                        ReturnErrorOnFailure(AddAttribute(attrOID, chipAttr));
                    }
                    // If 32-bit CHIP attribute.
                    else if (IsChip32bitDNAttr(attrOID))
                    {
                        CASEAuthTag chipAttr;
                        VerifyOrReturnError(Encoding::UppercaseHexToUint32(reinterpret_cast<const char *>(reader.GetValue()),
                                                                           reader.GetValueLen(), chipAttr) == sizeof(CASEAuthTag),
                                            ASN1_ERROR_INVALID_ENCODING);

                        VerifyOrReturnError(IsValidCASEAuthTag(chipAttr), CHIP_ERROR_WRONG_CERT_DN);

                        ReturnErrorOnFailure(AddAttribute(attrOID, chipAttr));
                    }
                    // Otherwise, it is a string.
                    else
                    {
                        ReturnErrorOnFailure(AddAttribute(attrOID,
                                                          CharSpan(Uint8::to_const_char(reader.GetValue()), reader.GetValueLen()),
                                                          attrTag == kASN1UniversalTag_PrintableString));
                    }
                }
                ASN1_EXIT_SEQUENCE;

                // Only one AttributeTypeAndValue allowed per RDN.
                err = reader.Next();
                ReturnErrorCodeIf(err == CHIP_NO_ERROR, ASN1_ERROR_UNSUPPORTED_ENCODING);
                VerifyOrReturnError(err == ASN1_END, err);
            }
            ASN1_EXIT_SET;
        }
    }
    ASN1_EXIT_SEQUENCE;

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

DLL_EXPORT CHIP_ERROR ASN1ToChipEpochTime(const chip::ASN1::ASN1UniversalTime & asn1Time, uint32_t & epochTime)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // X.509/RFC5280 defines the special time 99991231235959Z to mean 'no well-defined expiration date'.
    // In CHIP certificate it is represented as a CHIP Epoch time value of 0 sec (2000-01-01 00:00:00 UTC).
    //
    // While it is not conventional to use this special value for NotBefore, for simplicity we convert all
    // time values of 99991231235959Z to CHIP epoch zero seconds.  ChipEpochToASN1Time performs the inverse
    // translation for conversions in the other direction.
    //
    // If in a conversion from X509 to CHIP TLV format the input X509 certificate encloses 99991231235959Z
    // for NotBefore, this will be converted to the CHIP Epoch time value of 0 and consuming code will
    // handle this transparently, as logic considering a NotBefore time at the CHIP epoch will evaluate all
    // possible unsigned offsets from the CHIP epoch as valid, which is equivalent to ignoring NotBefore.
    //
    // If in a conversion from X509 to CHIP TLV format the input X509 certificate encloses a NotBefore time
    // at the CHIP epoch itself, 2000-01-01 00:00:00, a resultant conversion to CHIP TLV certificate format
    // will appear to have an invalid TBS signature when the symmetric ChipEpochToASN1Time produces
    // 99991231235959Z for NotBefore during signature validation.
    //
    // Thus such certificates, when passing through this code, will not appear valid.  This should be
    // immediately evident at commissioning time.
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
    // In CHIP certificate it is represented as a CHIP Epoch time value of 0 secs (2000-01-01 00:00:00 UTC).
    //
    // For simplicity and symmetry with ASN1ToChipEpochTime, this method makes this conversion for all
    // times, which in consuming code can create a conversion from CHIP epoch 0 seconds to 99991231235959Z
    // for NotBefore, which is not conventional.
    //
    // If an original X509 certificate encloses a NotBefore time that this the CHIP Epoch itself, 2000-01-01
    // 00:00:00, the resultant X509 certificate in a conversion back from CHIP TLV format using this time
    // conversion method will instead enclose the NotBefore time 99991231235959Z, which will invalidiate the
    // TBS signature.  Thus, certificates with this specific attribute are not usable with this code.
    // Attempted installation of such certficates will fail during commissioning.
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

CHIP_ERROR ValidateChipRCAC(const ByteSpan & rcac)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;
    ValidationContext validContext;
    uint8_t certType;

    // Note that this function doesn't check RCAC NotBefore / NotAfter time validity.
    // It is assumed that RCAC should be valid at the time of installation by definition.

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(rcac, CertDecodeFlags::kGenerateTBSHash));

    ReturnErrorOnFailure(certData.mSubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_Root, CHIP_ERROR_WRONG_CERT_TYPE);

    VerifyOrReturnError(certData.mSubjectDN.IsEqual(certData.mIssuerDN), CHIP_ERROR_WRONG_CERT_TYPE);

    VerifyOrReturnError(certData.mSubjectKeyId.data_equal(certData.mAuthKeyId), CHIP_ERROR_WRONG_CERT_TYPE);

    VerifyOrReturnError(certData.mCertFlags.Has(CertFlags::kIsCA), CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
    if (certData.mCertFlags.Has(CertFlags::kPathLenConstraintPresent))
    {
        VerifyOrReturnError(certData.mPathLenConstraint <= 1, CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
    }

    VerifyOrReturnError(certData.mKeyUsageFlags.Has(KeyUsageFlags::kKeyCertSign), CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

    return ChipCertificateSet::VerifySignature(&certData, &certData);
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

CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, MutableByteSpan & derSig)
{
    ASN1Writer writer;

    writer.Init(derSig);

    ReturnErrorOnFailure(ConvertECDSASignatureRawToDER(rawSig, writer));

    derSig.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan rawSig, ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t derInt[kP256_FE_Length + kEmitDerIntegerWithoutTagOverhead];

    VerifyOrReturnError(!rawSig.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    // Ecdsa-Sig-Value ::= SEQUENCE
    ASN1_START_SEQUENCE
    {
        // r INTEGER
        {
            MutableByteSpan derIntSpan(derInt, sizeof(derInt));
            ReturnErrorOnFailure(ConvertIntegerRawToDerWithoutTag(P256IntegerSpan(rawSig.data()), derIntSpan));
            ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, derIntSpan.data(),
                                                 static_cast<uint16_t>(derIntSpan.size())));
        }

        // s INTEGER
        {
            MutableByteSpan derIntSpan(derInt, sizeof(derInt));
            ReturnErrorOnFailure(ConvertIntegerRawToDerWithoutTag(P256IntegerSpan(rawSig.data() + kP256_FE_Length), derIntSpan));
            ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, derIntSpan.data(),
                                                 static_cast<uint16_t>(derIntSpan.size())));
        }
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR ExtractNodeIdFabricIdFromOpCert(const ChipCertificateData & opcert, NodeId * outNodeId, FabricId * outFabricId)
{
    // Since we assume the cert is pre-validated, we are going to assume that
    // its subject in fact has both a node id and a fabric id.
    ReturnErrorCodeIf(outNodeId == nullptr || outFabricId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    NodeId nodeId      = 0;
    FabricId fabricId  = kUndefinedFabricId;
    bool foundNodeId   = false;
    bool foundFabricId = false;

    const ChipDN & subjectDN = opcert.mSubjectDN;
    for (uint8_t i = 0; i < subjectDN.RDNCount(); ++i)
    {
        const auto & rdn = subjectDN.rdn[i];
        if (rdn.mAttrOID == ASN1::kOID_AttributeType_MatterNodeId)
        {
            nodeId      = rdn.mChipVal;
            foundNodeId = true;
        }
        else if (rdn.mAttrOID == ASN1::kOID_AttributeType_MatterFabricId)
        {
            fabricId      = rdn.mChipVal;
            foundFabricId = true;
        }
    }
    if (!foundNodeId || !foundFabricId)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    *outNodeId   = nodeId;
    *outFabricId = fabricId;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(ByteSpan rcac, ByteSpan noc, CompressedFabricId & compressedFabricId,
                                                              FabricId & fabricId, NodeId & nodeId)
{
    Crypto::P256PublicKey rootPubKey;
    Credentials::P256PublicKeySpan rootPubKeySpan;
    ReturnErrorOnFailure(ExtractPublicKeyFromChipCert(rcac, rootPubKeySpan));
    rootPubKey = Crypto::P256PublicKey(rootPubKeySpan);
    ReturnErrorOnFailure(Credentials::ExtractNodeIdFabricIdFromOpCert(noc, &nodeId, &fabricId));
    ReturnErrorOnFailure(GenerateCompressedFabricId(rootPubKey, fabricId, compressedFabricId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractNodeIdCompressedFabricIdFromOpCerts(ByteSpan rcac, ByteSpan noc, CompressedFabricId & compressedFabricId,
                                                      NodeId & nodeId)
{
    FabricId fabricId;
    ReturnErrorOnFailure(ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(rcac, noc, compressedFabricId, fabricId, nodeId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractFabricIdFromCert(const ChipCertificateData & cert, FabricId * fabricId)
{
    const ChipDN & subjectDN = cert.mSubjectDN;
    for (uint8_t i = 0; i < subjectDN.RDNCount(); ++i)
    {
        const auto & rdn = subjectDN.rdn[i];
        if (rdn.mAttrOID == ASN1::kOID_AttributeType_MatterFabricId)
        {
            *fabricId = rdn.mChipVal;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExtractCATsFromOpCert(const ByteSpan & opcert, CATValues & cats)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(opcert, BitFlags<CertDecodeFlags>()));

    return ExtractCATsFromOpCert(certData, cats);
}

CHIP_ERROR ExtractCATsFromOpCert(const ChipCertificateData & opcert, CATValues & cats)
{
    uint8_t catCount = 0;
    uint8_t certType;

    ReturnErrorOnFailure(opcert.mSubjectDN.GetCertType(certType));
    VerifyOrReturnError(certType == kCertType_Node, CHIP_ERROR_INVALID_ARGUMENT);

    const ChipDN & subjectDN = opcert.mSubjectDN;
    for (uint8_t i = 0; i < subjectDN.RDNCount(); ++i)
    {
        const auto & rdn = subjectDN.rdn[i];
        if (rdn.mAttrOID == ASN1::kOID_AttributeType_MatterCASEAuthTag)
        {
            // This error should never happen in practice because valid NOC cannot have more
            // than kMaxSubjectCATAttributeCount CATs in its subject. The check that it is
            // valid NOC was done above.
            ReturnErrorCodeIf(catCount == cats.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
            VerifyOrReturnError(CanCastTo<CASEAuthTag>(rdn.mChipVal), CHIP_ERROR_INVALID_ARGUMENT);
            cats.values[catCount++] = static_cast<CASEAuthTag>(rdn.mChipVal);
        }
    }
    for (size_t i = catCount; i < cats.size(); ++i)
    {
        cats.values[i] = kUndefinedCAT;
    }

    // Make sure the set contained valid data, otherwise it's an invalid cert
    VerifyOrReturnError(cats.AreValid(), CHIP_ERROR_WRONG_CERT_DN);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractFabricIdFromCert(const ByteSpan & opcert, FabricId * fabricId)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;
    ReturnErrorOnFailure(certSet.Init(&certData, 1));
    ReturnErrorOnFailure(certSet.LoadCert(opcert, BitFlags<CertDecodeFlags>()));
    return ExtractFabricIdFromCert(certData, fabricId);
}

CHIP_ERROR ExtractNodeIdFabricIdFromOpCert(const ByteSpan & opcert, NodeId * nodeId, FabricId * fabricId)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(opcert, BitFlags<CertDecodeFlags>()));

    return ExtractNodeIdFabricIdFromOpCert(certData, nodeId, fabricId);
}

CHIP_ERROR ExtractPublicKeyFromChipCert(const ByteSpan & chipCert, P256PublicKeySpan & publicKey)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(chipCert, BitFlags<CertDecodeFlags>()));

    publicKey = certData.mPublicKey;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractNotBeforeFromChipCert(const ByteSpan & chipCert, chip::System::Clock::Seconds32 & notBeforeChipEpochTime)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(chipCert, BitFlags<CertDecodeFlags>()));

    notBeforeChipEpochTime = chip::System::Clock::Seconds32(certData.mNotBeforeTime);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractSKIDFromChipCert(const ByteSpan & chipCert, CertificateKeyId & skid)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(chipCert, BitFlags<CertDecodeFlags>()));

    skid = certData.mSubjectKeyId;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractSubjectDNFromChipCert(const ByteSpan & chipCert, ChipDN & dn)
{
    ChipCertificateSet certSet;
    ChipCertificateData certData;

    ReturnErrorOnFailure(certSet.Init(&certData, 1));

    ReturnErrorOnFailure(certSet.LoadCert(chipCert, BitFlags<CertDecodeFlags>()));

    dn = certData.mSubjectDN;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractSubjectDNFromX509Cert(const ByteSpan & x509Cert, ChipDN & dn)
{
    CHIP_ERROR err;
    ASN1Reader reader;

    VerifyOrReturnError(CanCastTo<uint32_t>(x509Cert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    reader.Init(x509Cert);

    // Certificate ::= SEQUENCE
    ASN1_PARSE_ENTER_SEQUENCE
    {
        // tbsCertificate TBSCertificate,
        // TBSCertificate ::= SEQUENCE
        ASN1_PARSE_ENTER_SEQUENCE
        {
            // Skip version [0] EXPLICIT Version DEFAULT v1
            ASN1_PARSE_ELEMENT(kASN1TagClass_ContextSpecific, 0);

            // Skip serialNumber CertificateSerialNumber
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);

            // Skip signature AlgorithmIdentifier
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Sequence);

            // Skip issuer Name
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Sequence);

            // Skip validity Validity,
            ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Sequence);

            // Decode subject Name,
            ReturnErrorOnFailure(dn.DecodeFromASN1(reader));
        }
        ASN1_SKIP_AND_EXIT_SEQUENCE;
    }
    ASN1_SKIP_AND_EXIT_SEQUENCE;

exit:
    return err;
}

} // namespace Credentials
} // namespace chip
