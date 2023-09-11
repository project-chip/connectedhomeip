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

class StrictCertificateValidityPolicyExample : public CertificateValidityPolicy
{
public:
    ~StrictCertificateValidityPolicyExample() {}

    /**
     * @brief
     *
     * This certificate validity policy is strict in that it rejects all
     * certificates if any of wall clock time or last known good time show
     * them to be invalid.  This policy also rejects certificates if time
     * is unknown.
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
            return CHIP_NO_ERROR;
        case CertificateValidityResult::kNotYetValid:
            return CHIP_ERROR_CERT_NOT_VALID_YET;
        case CertificateValidityResult::kExpiredAtLastKnownGoodTime:
        case CertificateValidityResult::kTimeUnknown:
        case CertificateValidityResult::kExpired:
            return CHIP_ERROR_CERT_EXPIRED;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
};

} // namespace Credentials
} // namespace chip
