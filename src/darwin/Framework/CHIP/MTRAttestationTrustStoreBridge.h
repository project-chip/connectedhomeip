/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRError_Internal.h"
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

NS_ASSUME_NONNULL_BEGIN

class MTRAttestationTrustStoreBridge : public chip::Credentials::AttestationTrustStore {
public:
    MTRAttestationTrustStoreBridge(NSArray<NSData *> * paaCerts)
        : mPaaCerts(paaCerts)
    {
    }
    ~MTRAttestationTrustStoreBridge() {};

    CHIP_ERROR GetProductAttestationAuthorityCert(
        const chip::ByteSpan & skid, chip::MutableByteSpan & outPaaDerBuffer) const override;

private:
    NSArray<NSData *> * mPaaCerts;
};

NS_ASSUME_NONNULL_END
