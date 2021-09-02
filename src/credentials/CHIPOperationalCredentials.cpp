/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      working with CHIP Operational Credentials.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <credentials/CHIPOperationalCredentials.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Credentials {

static constexpr size_t kOperationalCertificatesMax = 3;

using namespace chip::Crypto;

CHIP_ERROR OperationalCredentialSet::Init(uint8_t maxCertsArraySize)
{
    VerifyOrReturnError(mOpCreds == nullptr, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(maxCertsArraySize > 0, CHIP_ERROR_INVALID_ARGUMENT);
    mOpCreds = reinterpret_cast<ChipCertificateSet *>(chip::Platform::MemoryAlloc(sizeof(ChipCertificateSet) * maxCertsArraySize));
    VerifyOrReturnError(mOpCreds != nullptr, CHIP_ERROR_NO_MEMORY);

    mOpCredCount                = 0;
    mMaxCerts                   = maxCertsArraySize;
    mMemoryAllocInternal        = true;
    mChipDeviceCredentialsCount = 0;
    mDeviceOpCredKeypairCount   = 0;

    CleanupMaps();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::Init(ChipCertificateSet * certSetsArray, uint8_t certSetsArraySize)
{
    VerifyOrReturnError(mOpCreds == nullptr, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(certSetsArray != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(certSetsArraySize > 0, CHIP_ERROR_INVALID_ARGUMENT);

    mOpCredCount                = certSetsArraySize;
    mOpCreds                    = certSetsArray;
    mMaxCerts                   = certSetsArraySize;
    mMemoryAllocInternal        = false;
    mChipDeviceCredentialsCount = 0;
    mDeviceOpCredKeypairCount   = 0;

    CleanupMaps();

    return CHIP_NO_ERROR;
}

void OperationalCredentialSet::Release()
{
    if (mMemoryAllocInternal)
    {
        if (mOpCreds != nullptr)
        {
            Clear();
            chip::Platform::MemoryFree(mOpCreds);
            mOpCreds = nullptr;
        }
        mMemoryAllocInternal = false;
    }
    else
    {
        mOpCreds = nullptr;
    }

    for (size_t i = 0; i < kOperationalCredentialsMax; ++i)
    {
        if (mChipDeviceCredentials[i].nodeCredential.mCredential != nullptr)
        {
            chip::Platform::MemoryFree(mChipDeviceCredentials[i].nodeCredential.mCredential);
            mChipDeviceCredentials[i].nodeCredential.mCredential = nullptr;
            mChipDeviceCredentials[i].nodeCredential.mLen        = 0;
            mChipDeviceCredentials[i].trustedRootId              = CertificateKeyId();
        }
        mDeviceOpCredKeypair[i].trustedRootId = CertificateKeyId();
    }

    mChipDeviceCredentialsCount = 0;
    mDeviceOpCredKeypairCount   = 0;
}

void OperationalCredentialSet::Clear()
{
    for (int i = 0; i < mOpCredCount; i++)
    {
        mOpCreds[i].~ChipCertificateSet();
    }

    mOpCredCount = 0;
}

void OperationalCredentialSet::CleanupMaps()
{
    for (size_t i = 0; i < kOperationalCredentialsMax; ++i)
    {
        mChipDeviceCredentials[i].trustedRootId              = CertificateKeyId();
        mChipDeviceCredentials[i].nodeCredential.mCredential = nullptr;
        mChipDeviceCredentials[i].nodeCredential.mLen        = 0;

        mDeviceOpCredKeypair[i].trustedRootId = CertificateKeyId();
    }
}

ChipCertificateSet * OperationalCredentialSet::FindCertSet(const CertificateKeyId & trustedRootId) const
{
    for (uint8_t i = 0; i < mOpCredCount; i++)
    {
        ChipCertificateSet * certSet = &mOpCreds[i];

        for (uint8_t j = 0; j < certSet->GetCertCount(); j++)
        {
            const ChipCertificateData * cert = &certSet->GetCertSet()[j];
            if (cert->mCertFlags.Has(CertFlags::kIsTrustAnchor) && cert->mAuthKeyId.data_equal(trustedRootId))
            {
                return certSet;
            }
        }
    }

    return nullptr;
}

bool OperationalCredentialSet::IsCertSetInTheOpCredSet(const ChipCertificateSet * cert) const
{
    for (uint8_t i = 0; i < mOpCredCount; i++)
    {
        if (cert == &mOpCreds[i])
        {
            return true;
        }
    }

    return false;
}

bool OperationalCredentialSet::IsTrustedRootIn(const CertificateKeyId & trustedRoot) const
{
    for (uint16_t i = 0; i < mOpCredCount; ++i)
    {
        if (GetTrustedRootId(i).data_equal(trustedRoot))
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR OperationalCredentialSet::ValidateCert(const CertificateKeyId & trustedRootId, const ChipCertificateData * cert,
                                                  ValidationContext & context)
{
    ChipCertificateSet * chipCertificateSet;

    chipCertificateSet = FindCertSet(trustedRootId);
    VerifyOrReturnError(chipCertificateSet != nullptr, CHIP_ERROR_CERT_NOT_FOUND);
    VerifyOrReturnError(chipCertificateSet->IsCertInTheSet(cert), CHIP_ERROR_INVALID_ARGUMENT);

    return chipCertificateSet->ValidateCert(cert, context);
}

CHIP_ERROR OperationalCredentialSet::FindValidCert(const CertificateKeyId & trustedRootId, const ChipDN & subjectDN,
                                                   const CertificateKeyId & subjectKeyId, ValidationContext & context,
                                                   const ChipCertificateData ** certData)
{
    ChipCertificateSet * chipCertificateSet;

    chipCertificateSet = FindCertSet(trustedRootId);
    VerifyOrReturnError(chipCertificateSet != nullptr, CHIP_ERROR_CERT_NOT_FOUND);

    return chipCertificateSet->FindValidCert(subjectDN, subjectKeyId, context, certData);
}

CHIP_ERROR OperationalCredentialSet::SignMsg(const CertificateKeyId & trustedRootId, const uint8_t * msg, const size_t msg_length,
                                             P256ECDSASignature & out_signature)
{
    return GetNodeKeypairAt(trustedRootId)->ECDSA_sign_msg(msg, msg_length, out_signature);
}

CertificateKeyId OperationalCredentialSet::GetTrustedRootId(uint16_t certSetIndex) const
{
    VerifyOrReturnError(certSetIndex <= mOpCredCount, CertificateKeyId());

    const ChipCertificateData * chipCertificateData = mOpCreds[certSetIndex].GetCertSet();
    uint8_t numberCertificates                      = mOpCreds[certSetIndex].GetCertCount();

    for (uint8_t i = 0; i < numberCertificates; ++i)
    {
        if (chipCertificateData[i].mCertFlags.Has(CertFlags::kIsTrustAnchor))
        {
            return chipCertificateData[i].mAuthKeyId;
        }
    }
    return CertificateKeyId();
}

CHIP_ERROR OperationalCredentialSet::SetDevOpCred(const CertificateKeyId & trustedRootId, const uint8_t * chipDeviceCredentials,
                                                  uint16_t chipDeviceCredentialsLen)
{
    NodeCredential newCredential;

    VerifyOrReturnError(mChipDeviceCredentialsCount < kOperationalCredentialsMax, CHIP_ERROR_NO_MEMORY);

    newCredential.mCredential = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(chipDeviceCredentialsLen));
    VerifyOrReturnError(newCredential.mCredential != nullptr, CHIP_ERROR_NO_MEMORY);

    memcpy(newCredential.mCredential, chipDeviceCredentials, chipDeviceCredentialsLen);
    newCredential.mLen = chipDeviceCredentialsLen;

    mChipDeviceCredentials[mChipDeviceCredentialsCount].trustedRootId  = trustedRootId;
    mChipDeviceCredentials[mChipDeviceCredentialsCount].nodeCredential = newCredential;

    ++mChipDeviceCredentialsCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::SetDevOpCredKeypair(const CertificateKeyId & trustedRootId, P256Keypair * newKeypair)
{
    P256SerializedKeypair serializedKeypair;

    VerifyOrReturnError(mDeviceOpCredKeypairCount < kOperationalCredentialsMax, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(newKeypair->Serialize(serializedKeypair));
    ReturnErrorOnFailure(mDeviceOpCredKeypair[mDeviceOpCredKeypairCount].keypair.Deserialize(serializedKeypair));

    mDeviceOpCredKeypair[mDeviceOpCredKeypairCount].trustedRootId = trustedRootId;

    ++mDeviceOpCredKeypairCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::ToSerializable(const CertificateKeyId & trustedRootId,
                                                    OperationalCredentialSerializable & serializable)
{
    const NodeCredential * nodeCredential = GetNodeCredentialAt(trustedRootId);
    P256Keypair * keypair                 = GetNodeKeypairAt(trustedRootId);
    P256SerializedKeypair serializedKeypair;
    ChipCertificateSet * certificateSet  = FindCertSet(trustedRootId);
    const ChipCertificateData * dataSet  = nullptr;
    uint8_t * ptrSerializableCerts[]     = { serializable.mRootCertificate, serializable.mCACertificate };
    uint16_t * ptrSerializableCertsLen[] = { &serializable.mRootCertificateLen, &serializable.mCACertificateLen };

    VerifyOrReturnError(certificateSet != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(keypair->Serialize(serializedKeypair));
    VerifyOrReturnError(serializedKeypair.Length() <= sizeof(serializable.mNodeKeypair), CHIP_ERROR_INVALID_ARGUMENT);

    dataSet = certificateSet->GetCertSet();
    VerifyOrReturnError(dataSet != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mNodeCredentialLen = nodeCredential->mLen;

    memcpy(serializable.mNodeCredential, nodeCredential->mCredential, nodeCredential->mLen);
    memcpy(serializable.mNodeKeypair, serializedKeypair, serializedKeypair.Length());
    serializable.mNodeKeypairLen = static_cast<uint16_t>(serializedKeypair.Length());

    for (uint8_t i = 0; i < certificateSet->GetCertCount(); ++i)
    {
        VerifyOrReturnError(CanCastTo<uint16_t>(dataSet[i].mCertificate.size()), CHIP_ERROR_INTERNAL);
        memcpy(ptrSerializableCerts[i], dataSet[i].mCertificate.data(), dataSet[i].mCertificate.size());
        *ptrSerializableCertsLen[i] = static_cast<uint16_t>(dataSet[i].mCertificate.size());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::FromSerializable(const OperationalCredentialSerializable & serializable)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    P256Keypair keypair;
    P256SerializedKeypair serializedKeypair;
    ChipCertificateSet certificateSet;
    CertificateKeyId trustedRootId;

    SuccessOrExit(err = certificateSet.Init(kOperationalCertificatesMax));

    err = certificateSet.LoadCert(ByteSpan(serializable.mRootCertificate, serializable.mRootCertificateLen),
                                  BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    SuccessOrExit(err);

    trustedRootId = certificateSet.GetLastCert()->mAuthKeyId;

    if (serializable.mCACertificateLen != 0)
    {
        err = certificateSet.LoadCert(ByteSpan(serializable.mCACertificate, serializable.mCACertificateLen),
                                      BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);
    }

    LoadCertSet(&certificateSet);

    memcpy(serializedKeypair, serializable.mNodeKeypair, serializable.mNodeKeypairLen);
    SuccessOrExit(err = serializedKeypair.SetLength(serializable.mNodeKeypairLen));

    SuccessOrExit(err = keypair.Deserialize(serializedKeypair));

    SuccessOrExit(err = SetDevOpCredKeypair(trustedRootId, &keypair));

    SuccessOrExit(err = SetDevOpCred(trustedRootId, serializable.mNodeCredential, serializable.mNodeCredentialLen));

exit:
    certificateSet.Release();

    return err;
}

const NodeCredential * OperationalCredentialSet::GetNodeCredentialAt(const CertificateKeyId & trustedRootId) const
{
    for (size_t i = 0; i < kOperationalCredentialsMax && mChipDeviceCredentials[i].nodeCredential.mCredential != nullptr; ++i)
    {
        if (trustedRootId.data_equal(mChipDeviceCredentials[i].trustedRootId))
        {
            return &mChipDeviceCredentials[i].nodeCredential;
        }
    }

    return nullptr;
}

P256Keypair * OperationalCredentialSet::GetNodeKeypairAt(const CertificateKeyId & trustedRootId)
{
    for (size_t i = 0; i < kOperationalCredentialsMax && !mDeviceOpCredKeypair[i].trustedRootId.empty(); ++i)
    {
        if (trustedRootId.data_equal(mDeviceOpCredKeypair[i].trustedRootId))
        {
            return &mDeviceOpCredKeypair[i].keypair;
        }
    }

    return nullptr;
}

const ChipCertificateData * OperationalCredentialSet::GetRootCertificate(const CertificateKeyId & trustedRootId) const
{
    for (size_t certChainIdx = 0; certChainIdx < mOpCredCount; certChainIdx++)
    {
        ChipCertificateSet * certSet = &mOpCreds[certChainIdx];

        for (size_t ipkIdx = 0; ipkIdx < certSet->GetCertCount(); ipkIdx++)
        {
            const ChipCertificateData * cert = &certSet->GetCertSet()[ipkIdx];
            if (cert->mCertFlags.Has(CertFlags::kIsTrustAnchor) && cert->mAuthKeyId.data_equal(trustedRootId))
            {
                return cert;
            }
        }
    }

    return nullptr;
}

} // namespace Credentials
} // namespace chip
