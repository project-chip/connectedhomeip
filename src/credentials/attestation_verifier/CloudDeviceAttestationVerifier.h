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

class CloudDACVerifier : public DefaultDACVerifier
{
public:
    CloudDACVerifier() {}

    void VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion) override;

protected:
    AttestationVerificationResult CheckPAA(const DeviceAttestationVerifier::AttestationInfo & info,
                                           DeviceInfoForAttestation & deviceInfo, Platform::ScopedMemoryBuffer<uint8_t> & paaCert,
                                           MutableByteSpan & paaDerBuffer, Crypto::AttestationCertVidPid & paaVidPid,
                                           Crypto::AttestationCertVidPid & paiVidPid);

    AttestationVerificationResult CheckCertTimes(const DeviceAttestationVerifier::AttestationInfo & info,
                                                 MutableByteSpan & paaDerBuffer);

    AttestationVerificationResult CheckCertChain(const DeviceAttestationVerifier::AttestationInfo & info,
                                                 MutableByteSpan & paaDerBuffer);

    AttestationVerificationResult CheckCertDeclaration(const DeviceAttestationVerifier::AttestationInfo & info,
                                                       MutableByteSpan & paaDerBuffer, Crypto::AttestationCertVidPid & dacVidPid,
                                                       Crypto::AttestationCertVidPid & paiVidPid,
                                                       Crypto::AttestationCertVidPid & paaVidPid,
                                                       DeviceInfoForAttestation & deviceInfo);
};

} // namespace Credentials
} // namespace chip
