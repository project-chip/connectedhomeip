/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRAttestationTrustStoreBridge.h"
#import "NSDataSpanConversion.h"

CHIP_ERROR MTRAttestationTrustStoreBridge::GetProductAttestationAuthorityCert(
    const chip::ByteSpan & skid, chip::MutableByteSpan & outPaaDerBuffer) const
{
    VerifyOrReturnError(skid.size() == chip::Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

    size_t paaIdx;
    chip::ByteSpan candidate;

    for (paaIdx = 0; paaIdx < mPaaCerts.count; ++paaIdx) {
        uint8_t skidBuf[chip::Crypto::kSubjectKeyIdentifierLength] = { 0 };
        candidate = AsByteSpan(mPaaCerts[paaIdx]);
        chip::MutableByteSpan candidateSkidSpan { skidBuf };
        VerifyOrReturnError(
            CHIP_NO_ERROR == chip::Crypto::ExtractSKIDFromX509Cert(candidate, candidateSkidSpan), CHIP_ERROR_INTERNAL);

        if (skid.data_equal(candidateSkidSpan)) {
            // Found a match
            return CopySpanToMutableSpan(candidate, outPaaDerBuffer);
        }
    }
    return CHIP_ERROR_CA_CERT_NOT_FOUND;
}
