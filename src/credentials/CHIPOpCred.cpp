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

#include <credentials/CHIPOpCred.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Credentials {

using namespace chip::Crypto;

ChipOperationalCredentialSet::ChipOperationalCredentialSet()
{
    mOpCreds              = nullptr;
    mOpCredCount          = 0;
    mMaxCerts             = 0;
    mDeviceCredentialsDER = nullptr;
    mDeviceCredentialsLen = 0;
}

ChipOperationalCredentialSet::~ChipOperationalCredentialSet() {}

CHIP_ERROR ChipOperationalCredentialSet::Init(ChipCertificateSet * certSetsArray, uint8_t certSetsArraySize,
                                              P256Keypair * deviceOpCredKeypair, const uint8_t * deviceCredentialsDER,
                                              uint16_t deviceCredentialsLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(certSetsArray != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(certSetsArraySize > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    mOpCredCount          = certSetsArraySize;
    mOpCreds              = certSetsArray;
    mMaxCerts             = certSetsArraySize;
    mDeviceCredentialsDER = deviceCredentialsDER;
    mDeviceCredentialsLen = deviceCredentialsLen;
    mDeviceOpCredKeypair  = deviceOpCredKeypair;

exit:
    return err;
}

ChipCertificateSet * ChipOperationalCredentialSet::FindCertSet(const CertificateKeyId & trustedRootId) const
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

bool ChipOperationalCredentialSet::IsCertSetInTheOpCredSet(const ChipCertificateSet * cert) const
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

bool ChipOperationalCredentialSet::IsTrustedRootIn(const CertificateKeyId & trustedRoot) const
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

CHIP_ERROR ChipOperationalCredentialSet::ValidateCert(const CertificateKeyId & trustedRootId, const ChipCertificateData * cert,
                                                      ValidationContext & context)
{
    CHIP_ERROR err;
    ChipCertificateSet * chipCertificateSet;

    chipCertificateSet = FindCertSet(trustedRootId);
    VerifyOrExit(chipCertificateSet != nullptr, err = CHIP_ERROR_CERT_NOT_FOUND);
    VerifyOrExit(chipCertificateSet->IsCertInTheSet(cert), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = chipCertificateSet->ValidateCert(cert, context);

exit:
    return err;
}

CHIP_ERROR ChipOperationalCredentialSet::FindValidCert(const CertificateKeyId & trustedRootId, const ChipDN & subjectDN,
                                                       const CertificateKeyId & subjectKeyId, ValidationContext & context,
                                                       ChipCertificateData *& cert)
{
    CHIP_ERROR err;
    ChipCertificateSet * chipCertificateSet;

    chipCertificateSet = FindCertSet(trustedRootId);
    VerifyOrExit(chipCertificateSet != nullptr, err = CHIP_ERROR_CERT_NOT_FOUND);

    err = chipCertificateSet->FindValidCert(subjectDN, subjectKeyId, context, cert);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipOperationalCredentialSet::SignMsg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature)
{
    return mDeviceOpCredKeypair->ECDSA_sign_msg(msg, msg_length, out_signature);
}

const CertificateKeyId * ChipOperationalCredentialSet::GetTrustedRootId(uint16_t certSetIndex) const
{
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

} // namespace Credentials
} // namespace chip
