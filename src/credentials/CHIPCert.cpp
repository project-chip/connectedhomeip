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

    mCertCount           = 0;
    mMaxCerts            = maxCertsArraySize;
    mDecodeBufSize       = decodeBufSize;
    mMemoryAllocInternal = true;

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

    mCertCount           = 0;
    mCerts               = certsArray;
    mMaxCerts            = certsArraySize;
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
        if (mCerts != nullptr)
        {
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
    for (int i = 0; i < mCertCount; i++)
    {
        mCerts[i].Clear();
    }

    mCertCount = 0;
}

CHIP_ERROR ChipCertificateSet::LoadCert(const uint8_t * chipCert, uint32_t chipCertLen,
                                        BitFlags<uint8_t, CertDecodeFlags> decodeFlags)
{
    CHIP_ERROR err;
    TLVReader reader;

    reader.Init(chipCert, chipCertLen);
    reader.ImplicitProfileId = kProtocol_OpCredentials;

    err = reader.Next(kTLVType_Structure, ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificate));
    SuccessOrExit(err);

    err = LoadCert(reader, decodeFlags);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCert(TLVReader & reader, BitFlags<uint8_t, CertDecodeFlags> decodeFlags)
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
        VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_SubjectKeyId) &&
                         cert->mCertFlags.Has(CertFlags::kExtPresent_AuthKeyId),
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

    // Assign a default type for the certificate based on its subject and attributes.
    err = DetermineCertType(*cert);
    SuccessOrExit(err);

    mCertCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (cert != nullptr)
        {
            cert->Clear();
        }
    }

    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(const uint8_t * chipCerts, uint32_t chipCertsLen,
                                         BitFlags<uint8_t, CertDecodeFlags> decodeFlags)
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
                     (type == kTLVType_Array && tag == ProfileTag(kProtocol_OpCredentials, kTag_ChipCertificateArray)),
                 err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    err = LoadCerts(reader, decodeFlags);

exit:
    return err;
}

CHIP_ERROR ChipCertificateSet::LoadCerts(TLVReader & reader, BitFlags<uint8_t, CertDecodeFlags> decodeFlags)
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
            mCerts[i].Clear();
        }
        mCertCount = initialCertCount;
    }

    return err;
}

CHIP_ERROR ChipCertificateSet::AddTrustedKey(uint64_t caId, OID curveOID, const uint8_t * pubKey, uint8_t pubKeyLen,
                                             const uint8_t * pubKeyId, uint8_t pubKeyIdLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipCertificateData * cert;

    // Verify we have room for the new certificate.
    VerifyOrExit(mCertCount < mMaxCerts, err = CHIP_ERROR_NO_MEMORY);

    cert = new (&mCerts[mCertCount]) ChipCertificateData();

    cert->mSubjectDN.mAttrOID           = kOID_AttributeType_ChipCAId;
    cert->mSubjectDN.mAttrValue.mChipId = caId;
    cert->mIssuerDN                     = cert->mSubjectDN;
    cert->mPubKeyCurveOID               = curveOID;
    cert->mPublicKey                    = pubKey;
    cert->mPublicKeyLen                 = pubKeyLen;
    cert->mSubjectKeyId.mId             = pubKeyId;
    cert->mSubjectKeyId.mLen            = pubKeyIdLen;
    cert->mAuthKeyId                    = cert->mSubjectKeyId;
    cert->mCertType                     = kCertType_CA;

    cert->mCertFlags.Set(CertFlags::kExtPresent_BasicConstraints);
    cert->mCertFlags.Set(CertFlags::kExtPresent_KeyUsage);
    cert->mCertFlags.Set(CertFlags::kExtPresent_SubjectKeyId);
    cert->mCertFlags.Set(CertFlags::kExtPresent_AuthKeyId);
    cert->mCertFlags.Set(CertFlags::kIsCA);
    cert->mCertFlags.Set(CertFlags::kIsTrustAnchor);

    cert->mKeyUsageFlags.Set(KeyUsageFlags::kKeyCertSign);

    mCertCount++;

exit:
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
                                            BitFlags<uint8_t, CertValidateFlags> validateFlags, uint8_t depth)
{
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    ChipCertificateData * caCert          = nullptr;
    static constexpr int kLastSecondOfDay = kSecondsPerDay - 1;

    // If the depth is greater than 0 then the certificate is required to be a CA certificate...
    if (depth > 0)
    {
        // Verify the isCA flag is present.
        VerifyOrExit(cert->mCertFlags.Has(CertFlags::kIsCA), err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify the key usage extension is present and contains the 'keyCertSign' flag.
        VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_KeyUsage) && cert->mKeyUsageFlags.Has(KeyUsageFlags::kKeyCertSign),
                     err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify that the certificate type is set to "CA".
        VerifyOrExit(cert->mCertType == kCertType_CA, err = CHIP_ERROR_WRONG_CERT_TYPE);

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
        if (context.mRequiredKeyUsages.Raw() != 0)
        {
            VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_KeyUsage) &&
                             cert->mKeyUsageFlags.Has(context.mRequiredKeyUsages.Raw()),
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
        }

        // If a set of desired key purposes has been specified, verify that the extended key usage extension
        // exists in the certificate and that the corresponding purposes are supported.
        if (context.mRequiredKeyPurposes.Raw() != 0)
        {
            VerifyOrExit(cert->mCertFlags.Has(CertFlags::kExtPresent_ExtendedKeyUsage) &&
                             cert->mKeyPurposeFlags.Has(context.mRequiredKeyPurposes.Raw()),
                         err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);
        }

        // If a required certificate type has been specified, verify it against the current certificate's type.
        if (context.mRequiredCertType != kCertType_NotSpecified)
        {
            VerifyOrExit(cert->mCertType == context.mRequiredCertType, err = CHIP_ERROR_WRONG_CERT_TYPE);
        }
    }

    // Verify the validity time of the certificate, if requested.
    if (cert->mNotBeforeDate != 0 && !validateFlags.Has(CertValidateFlags::kIgnoreNotBefore))
    {
        VerifyOrExit(context.mEffectiveTime >= PackedCertDateToTime(cert->mNotBeforeDate), err = CHIP_ERROR_CERT_NOT_VALID_YET);
    }
    if (cert->mNotAfterDate != 0 && !validateFlags.Has(CertValidateFlags::kIgnoreNotAfter))
    {
        VerifyOrExit(context.mEffectiveTime <= PackedCertDateToTime(cert->mNotAfterDate) + kLastSecondOfDay,
                     err = CHIP_ERROR_CERT_EXPIRED);
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
                                             ValidationContext & context, BitFlags<uint8_t, CertValidateFlags> validateFlags,
                                             uint8_t depth, ChipCertificateData *& cert)
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

ChipCertificateData::ChipCertificateData()
{
    Clear();
}

ChipCertificateData::~ChipCertificateData() {}

void ChipCertificateData::Clear()
{
    mSubjectDN.Clear();
    mIssuerDN.Clear();
    mSubjectKeyId.Clear();
    mAuthKeyId.Clear();
    mNotBeforeDate  = 0;
    mNotAfterDate   = 0;
    mPublicKey      = nullptr;
    mPublicKeyLen   = 0;
    mPubKeyCurveOID = 0;
    mPubKeyAlgoOID  = 0;
    mSigAlgoOID     = 0;
    mCertFlags.SetRaw(0);
    mKeyUsageFlags.SetRaw(0);
    mKeyPurposeFlags.SetRaw(0);
    mPathLenConstraint = 0;
    mCertType          = kCertType_NotSpecified;
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
    mRequiredKeyUsages.SetRaw(0);
    mRequiredKeyPurposes.SetRaw(0);
    mValidateFlags.SetRaw(0);
    mRequiredCertType = kCertType_NotSpecified;
}

CHIP_ERROR DetermineCertType(ChipCertificateData & cert)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the certificate subject contains a ChipCAId attribute...
    if (cert.mSubjectDN.mAttrOID == kOID_AttributeType_ChipCAId)
    {
        // Verify the BasicConstraints isCA flag is true.
        VerifyOrExit(cert.mCertFlags.Has(CertFlags::kIsCA), err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Verify the key usage extension is present and contains the 'keyCertSign' flag.
        VerifyOrExit(cert.mCertFlags.Has(CertFlags::kExtPresent_KeyUsage) && cert.mKeyUsageFlags.Has(KeyUsageFlags::kKeyCertSign),
                     err = CHIP_ERROR_CERT_USAGE_NOT_ALLOWED);

        // Set the certificate type to CA.
        cert.mCertType = kCertType_CA;
    }

    // If the certificate subject contains a ChipNodeId attribute set the certificate type to Node.
    else if (cert.mSubjectDN.mAttrOID == kOID_AttributeType_ChipNodeId)
    {
        cert.mCertType = kCertType_Node;
    }

    // If the certificate subject contains a ChipSoftwarePublisherId attribute set the certificate type to FirmwareSigning.
    else if (cert.mSubjectDN.mAttrOID == kOID_AttributeType_ChipSoftwarePublisherId)
    {
        cert.mCertType = kCertType_FirmwareSigning;
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
    if (mAttrOID == kOID_Unknown || mAttrOID == kOID_NotSpecified || mAttrOID != other.mAttrOID)
    {
        return false;
    }

    if (IsChipIdX509Attr(mAttrOID))
    {
        return mAttrValue.mChipId == other.mAttrValue.mChipId;
    }
    else
    {
        return (mAttrValue.mString.mLen == other.mAttrValue.mString.mLen &&
                memcmp(mAttrValue.mString.mValue, other.mAttrValue.mString.mValue, mAttrValue.mString.mLen) == 0);
    }
}

bool CertificateKeyId::IsEqual(const CertificateKeyId & other) const
{
    return mId != nullptr && other.mId != nullptr && mLen == other.mLen && memcmp(mId, other.mId, mLen) == 0;
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
    {
        return ASN1_ERROR_UNSUPPORTED_ENCODING;
    }

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
    {
        return ASN1_ERROR_UNSUPPORTED_ENCODING;
    }

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
        time.Second = static_cast<uint8_t>(packedTime % kSecondsPerMinute);
        packedTime /= kSecondsPerMinute;

        time.Minute = static_cast<uint8_t>(packedTime % kMinutesPerHour);
        packedTime /= kMinutesPerHour;

        time.Hour = static_cast<uint8_t>(packedTime % kHoursPerDay);
        packedTime /= kHoursPerDay;

        time.Day = static_cast<uint8_t>((packedTime % kMaxDaysPerMonth) + 1);
        packedTime /= kMaxDaysPerMonth;

        time.Month = static_cast<uint8_t>((packedTime % kMonthsPerYear) + 1);
        packedTime /= kMonthsPerYear;

        time.Year = static_cast<uint16_t>(packedTime + kCertTimeBaseYear);
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
