/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/JniReferences.h>
#include <lib/support/Span.h>

class AttestationTrustStoreBridge : public chip::Credentials::AttestationTrustStore
{
public:
    AttestationTrustStoreBridge(jobject attestationTrustStoreDelegate) :
        mAttestationTrustStoreDelegate(attestationTrustStoreDelegate)
    {}
    ~AttestationTrustStoreBridge();

    CHIP_ERROR GetProductAttestationAuthorityCert(const chip::ByteSpan & skid,
                                                  chip::MutableByteSpan & outPaaDerBuffer) const override;

protected:
    jobject mAttestationTrustStoreDelegate = nullptr;

    CHIP_ERROR GetPaaCertFromJava(const chip::ByteSpan & skid, chip::MutableByteSpan & outPaaDerBuffer) const;
};
