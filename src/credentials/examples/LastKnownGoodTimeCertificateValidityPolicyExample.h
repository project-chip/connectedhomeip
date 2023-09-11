/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <credentials/CertificateValidityPolicy.h>

namespace chip {
namespace Credentials {

class LastKnownGoodTimeCertificateValidityPolicyExample : public CertificateValidityPolicy
{
public:
    ~LastKnownGoodTimeCertificateValidityPolicyExample() {}

    /**
     * @brief
     *
     * This certificate validity policy will validate NotBefore / NotAfter if
     * current time is known and also validates NotAfter if only Last Known
     * Good Time is known.
     *
     * This provides an example for enforcing certificate expiration on nodes
     * where no current time source is available.
     *
     * @param cert CHIP Certificate for which we are evaluating validity
     * @param depth the depth of the certificate in the chain, where the leaf is at depth 0
     * @return CHIP_NO_ERROR if CHIPCert should accept the certificate; an appropriate CHIP_ERROR if it should be rejected
     */
    CHIP_ERROR ApplyCertificateValidityPolicy(const ChipCertificateData * cert, uint8_t depth,
                                              CertificateValidityResult result) override
    {
        switch (result)
        {
        case CertificateValidityResult::kValid:
        case CertificateValidityResult::kNotExpiredAtLastKnownGoodTime:
        case CertificateValidityResult::kTimeUnknown:
            return CHIP_NO_ERROR;
        case CertificateValidityResult::kNotYetValid:
            return CHIP_ERROR_CERT_NOT_VALID_YET;
        case CertificateValidityResult::kExpired:
        case CertificateValidityResult::kExpiredAtLastKnownGoodTime:
            return CHIP_ERROR_CERT_EXPIRED;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
};

} // namespace Credentials
} // namespace chip
