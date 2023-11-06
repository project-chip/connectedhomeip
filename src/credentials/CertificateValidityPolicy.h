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

#include <credentials/CHIPCert.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Credentials {

enum class CertificateValidityResult
{
    kValid                         = 0, // current time is known and is within the validity period bounded by [notBefore, notAfter]
    kNotYetValid                   = 1, // current time is known and falls before the validity period bounded by notBefore
    kExpired                       = 2, // current time is known and falls after the validity period bounded by notAfter
    kNotExpiredAtLastKnownGoodTime = 3, // Last Known Good Time is known and notAfter occurs at or after this
    kExpiredAtLastKnownGoodTime    = 4, // Last Known Good Time is known and notAfter occurs before this
    kTimeUnknown                   = 5, // No time source is available
};

/// Callback to request application acceptance or rejection of the path
/// segment based upon the CertificateValidityResult.
class CertificateValidityPolicy
{
public:
    virtual ~CertificateValidityPolicy() {}

    /**
     *  If a policy is provided to CHIPCert, this method is invoked to
     *  determine what action an application determines is appropriate given
     *  CHIPCert's evaluation of certificate validity based upon the best
     *  available time source.  If no policy is provided, CHIPCert enforces a
     *  default policy.
     *
     * @param cert CHIP Certificate from a peer certificate chain to be evaluated based upon application-enacted expiration policies
     * @param depth the depth of the certificate in the chain, where the leaf is at depth 0
     * @return CHIP_NO_ERROR if CHIPCert should accept the certificate; an appropriate CHIP_ERROR if it should be rejected
     */
    virtual CHIP_ERROR ApplyCertificateValidityPolicy(const ChipCertificateData * cert, uint8_t depth,
                                                      CertificateValidityResult result) = 0;

    /**
     * Default policy that will be used if no other policy is defined.  This is
     * exposed to allow other policies to explicitly delegate to it as needed.
     */
    static CHIP_ERROR ApplyDefaultPolicy(const ChipCertificateData * cert, uint8_t depth, CertificateValidityResult result);
};

class IgnoreCertificateValidityPeriodPolicy : public CertificateValidityPolicy
{
public:
    IgnoreCertificateValidityPeriodPolicy() {}

    /**
     * This certificate validity policy does not validate NotBefore or
     * NotAfter to accommodate platforms that may have wall clock time, but
     * where it is unreliable.
     *
     * Last Known Good Time is also not considered in this policy.
     *
     * @param cert CHIP Certificate for which we are evaluating validity
     * @param depth the depth of the certificate in the chain, where the leaf is at depth 0
     * @return CHIP_NO_ERROR if CHIPCert should accept the certificate; an appropriate CHIP_ERROR if it should be rejected
     */
    CHIP_ERROR ApplyCertificateValidityPolicy(const Credentials::ChipCertificateData * cert, uint8_t depth,
                                              Credentials::CertificateValidityResult result) override
    {
        switch (result)
        {
        case Credentials::CertificateValidityResult::kValid:
        case Credentials::CertificateValidityResult::kNotYetValid:
        case Credentials::CertificateValidityResult::kExpired:
        case Credentials::CertificateValidityResult::kNotExpiredAtLastKnownGoodTime:
        case Credentials::CertificateValidityResult::kExpiredAtLastKnownGoodTime:
        case Credentials::CertificateValidityResult::kTimeUnknown:
            return CHIP_NO_ERROR;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
};

} // namespace Credentials
} // namespace chip
