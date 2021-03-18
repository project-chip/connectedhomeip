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
#include <support/CHIPMem.h>
#include <support/ReturnMacros.h>

namespace chip {
namespace Credentials {

using namespace chip::Crypto;

CHIP_ERROR OperationalCredentialSet::Init(uint8_t maxCertsArraySize)
{
    VerifyOrReturnError(maxCertsArraySize > 0, CHIP_ERROR_INVALID_ARGUMENT);
    mOpCreds = reinterpret_cast<ChipCertificateSet *>(chip::Platform::MemoryAlloc(sizeof(ChipCertificateSet) * maxCertsArraySize));
    VerifyOrReturnError(mOpCreds != nullptr, CHIP_ERROR_NO_MEMORY);

    mOpCredCount         = 0;
    mMaxCerts            = maxCertsArraySize;
    mMemoryAllocInternal = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::Init(ChipCertificateSet * certSetsArray, uint8_t certSetsArraySize)
{
    VerifyOrReturnError(certSetsArray != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(certSetsArraySize > 0, CHIP_ERROR_INVALID_ARGUMENT);

    mOpCredCount         = certSetsArraySize;
    mOpCreds             = certSetsArray;
    mMaxCerts            = certSetsArraySize;
    mMemoryAllocInternal = false;

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

    {
        std::map<CertificateKeyId, NodeCredential, CertificateKeyIdCompare>::iterator it;

        for (it = mChipDeviceCredentials.begin(); it != mChipDeviceCredentials.end(); ++it)
        {
            if (it->second.mCredential != nullptr)
            {
                chip::Platform::MemoryFree(it->second.mCredential);
                it->second.mCredential = nullptr;
            }
        }
    }

    {
        std::map<CertificateKeyId, P256Keypair *, CertificateKeyIdCompare>::iterator it;

        for (it = mDeviceOpCredKeypair.begin(); it != mDeviceOpCredKeypair.end(); ++it)
        {
            if (it->second != nullptr)
            {
                chip::Platform::Delete(it->second);
                it->second = nullptr;
            }
        }
    }
}

void OperationalCredentialSet::Clear()
{
    for (int i = 0; i < mOpCredCount; i++)
    {
        mOpCreds[i].~ChipCertificateSet();
    }

    mOpCredCount = 0;
}

ChipCertificateSet * OperationalCredentialSet::FindCertSet(const CertificateKeyId & trustedRootId) const
{
    for (uint8_t i = 0; i < mOpCredCount; i++)
    {
        ChipCertificateSet * certSet = &mOpCreds[i];

        for (uint8_t j = 0; j < certSet->GetCertCount(); j++)
        {
            const ChipCertificateData * cert = &certSet->GetCertSet()[j];
            if (cert->mCertFlags.Has(CertFlags::kIsTrustAnchor) && cert->mAuthKeyId.IsEqual(trustedRootId))
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
        const CertificateKeyId * trustedRootId = GetTrustedRootId(i);
        if (trustedRootId->IsEqual(trustedRoot))
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
                                                   ChipCertificateData *& cert)
{
    ChipCertificateSet * chipCertificateSet;

    chipCertificateSet = FindCertSet(trustedRootId);
    VerifyOrReturnError(chipCertificateSet != nullptr, CHIP_ERROR_CERT_NOT_FOUND);

    return chipCertificateSet->FindValidCert(subjectDN, subjectKeyId, context, cert);
}

CHIP_ERROR OperationalCredentialSet::SignMsg(const CertificateKeyId & trustedRootId, const uint8_t * msg, const size_t msg_length,
                                             P256ECDSASignature & out_signature)
{
    return mDeviceOpCredKeypair.at(trustedRootId)->ECDSA_sign_msg(msg, msg_length, out_signature);
}

const CertificateKeyId * OperationalCredentialSet::GetTrustedRootId(uint16_t certSetIndex) const
{
    VerifyOrReturnError(certSetIndex <= mOpCredCount, nullptr);

    const ChipCertificateData * chipCertificateData = mOpCreds[certSetIndex].GetCertSet();
    uint8_t numberCertificates                      = mOpCreds[certSetIndex].GetCertCount();

    for (uint8_t i = 0; i < numberCertificates; ++i)
    {
        if (chipCertificateData[i].mCertFlags.Has(CertFlags::kIsTrustAnchor))
        {
            return &chipCertificateData[i].mAuthKeyId;
        }
    }
    return nullptr;
}

CHIP_ERROR OperationalCredentialSet::SetDevOpCred(const CertificateKeyId & trustedRootId, const uint8_t * chipDeviceCredentials,
                                                  uint16_t chipDeviceCredentialsLen)
{
    NodeCredential newCredential;

    newCredential.mCredential = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(chipDeviceCredentialsLen));
    VerifyOrReturnError(newCredential.mCredential != nullptr, CHIP_ERROR_NO_MEMORY);

    memcpy(newCredential.mCredential, chipDeviceCredentials, chipDeviceCredentialsLen);
    newCredential.mLen = chipDeviceCredentialsLen;

    mChipDeviceCredentials.insert(std::make_pair(trustedRootId, newCredential));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialSet::SetDevOpCredKeypair(const CertificateKeyId & trustedRootId, P256Keypair * newKeypair)
{
    P256SerializedKeypair serializedKeypair;
    P256Keypair * keypair = chip::Platform::New<P256Keypair>();

    VerifyOrReturnError(keypair != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(newKeypair->Serialize(serializedKeypair));
    ReturnErrorOnFailure(keypair->Deserialize(serializedKeypair));

    mDeviceOpCredKeypair.insert(std::make_pair(trustedRootId, keypair));

    return CHIP_NO_ERROR;
}

} // namespace Credentials
} // namespace chip
