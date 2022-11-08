/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AttestationTrustStoreBridge.h"
#include <lib/support/CodeUtils.h>

AttestationTrustStoreBridge::~AttestationTrustStoreBridge()
{
    if (!mPaaCerts.empty())
    {
        for (auto paaCert : mPaaCerts)
        {
            paaCert.clear();
            paaCert.shrink_to_fit();
        }
        mPaaCerts.clear();
        mPaaCerts.shrink_to_fit();
    }
}

CHIP_ERROR AttestationTrustStoreBridge::GetProductAttestationAuthorityCert(const chip::ByteSpan & skid,
                                                                           chip::MutableByteSpan & outPaaDerBuffer) const
{
    VerifyOrReturnError(skid.size() == chip::Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

    for (auto paaCert : mPaaCerts)
    {
        chip::ByteSpan candidate                                   = chip::ByteSpan(paaCert.data(), paaCert.size());
        uint8_t skidBuf[chip::Crypto::kSubjectKeyIdentifierLength] = { 0 };
        chip::MutableByteSpan candidateSkidSpan{ skidBuf };
        VerifyOrReturnError(CHIP_NO_ERROR == chip::Crypto::ExtractSKIDFromX509Cert(candidate, candidateSkidSpan),
                            CHIP_ERROR_INTERNAL);
        if (skid.data_equal(candidateSkidSpan))
        {
            // Found a match
            return CopySpanToMutableSpan(candidate, outPaaDerBuffer);
        }
    }
    return CHIP_ERROR_CA_CERT_NOT_FOUND;
}
