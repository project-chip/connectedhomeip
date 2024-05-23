/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
