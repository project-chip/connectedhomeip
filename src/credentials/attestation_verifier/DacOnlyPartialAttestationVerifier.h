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

#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>

namespace chip {
namespace Credentials {

/**
 * @brief
 *   This class is based upon the DefaultDACVerifier but has all checks removed which require
 * local availability of trust anchors that are not available from the commissionee, such as the
 * PAA root certificates and the CSA keys used to sign the Certification Declaration (CD).
 *
 *   This class should only be used in conjunction with an OperationalCredentialsDelegate
 * which performs the removed checks. For example, an OperationalCredentialsDelegate implementation
 * might send the DAC chain and signed CD to custom code which obtains these keys from the DCL.
 *
 * Specifically, the following list of checks have been removed:
 * (1) Make sure the PAA is valid and approved by CSA.
 * (2) vid-scoped PAA check: if the PAA is vid scoped, then its vid must match the DAC vid.
 * (3) cert chain check: verify PAI is signed by PAA, and DAC is signed by PAI.
 * (4) PAA subject key id extraction: the PAA subject key must match the PAA key referenced in the PAI.
 * (5) CD signature check: make sure a valid CSA CD key is used to sign the CD.
 *
 * Any other checks performed by the DefaultDACVerifier should be performed here too. Changes
 * made to DefaultDACVerifier::VerifyAttestationInformation should be made to
 * PartialDACVerifier::VerifyAttestationInformation.
 */
class PartialDACVerifier : public DefaultDACVerifier
{
public:
    PartialDACVerifier() {}

    /**
     * @brief
     * The implementation should track DefaultDACVerifier::VerifyAttestationInformation but with the checks
     * disabled that are outlined at the top of DacOnlyPartialAttestationVerifier.h.
     */
    void VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion) override;

protected:
};

} // namespace Credentials
} // namespace chip
