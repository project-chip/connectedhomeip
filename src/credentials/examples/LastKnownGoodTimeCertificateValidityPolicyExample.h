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
