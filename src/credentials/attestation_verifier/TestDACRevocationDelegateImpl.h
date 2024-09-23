/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/Span.h>

#include <string>

namespace chip {
namespace Credentials {

class TestDACRevocationDelegateImpl : public DeviceAttestationRevocationDelegate
{
public:
    TestDACRevocationDelegateImpl()  = default;
    ~TestDACRevocationDelegateImpl() = default;

    /**
     * @brief Verify whether or not the given DAC chain is revoked.
     *
     * @param[in] info All of the information required to check for revoked DAC chain.
     * @param[in] onCompletion Callback handler to provide Attestation Information Verification result to the caller of
     *                         CheckForRevokedDACChain().
     */
    void CheckForRevokedDACChain(
        const DeviceAttestationVerifier::AttestationInfo & info,
        Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> * onCompletion) override;

    // Set the path to the device attestation revocation set JSON file.
    // revocation set can be generated using credentials/generate-revocation-set.py script
    // This API returns CHIP_ERROR_INVALID_ARGUMENT if the path is null.
    CHIP_ERROR SetDeviceAttestationRevocationSetPath(std::string_view path);

    // Clear the path to the device attestation revocation set JSON file.
    // This can be used to skip the revocation check
    void ClearDeviceAttestationRevocationSetPath();

private:
    CHIP_ERROR GetAKIDHexStr(const ByteSpan & certDer, MutableCharSpan & outAKIDHexStr);
    CHIP_ERROR GetSerialNumberHexStr(const ByteSpan & certDer, MutableCharSpan & outSerialNumberHexStr);
    CHIP_ERROR GetIssuerNameBase64Str(const ByteSpan & certDer, MutableCharSpan & outIssuerNameBase64String);
    bool IsEntryInRevocationSet(const CharSpan & akidHexStr, const CharSpan & issuerNameBase64Str,
                                const CharSpan & serialNumberHexStr);
    bool IsCertificateRevoked(const ByteSpan & certDer);

    std::string mDeviceAttestationRevocationSetPath;
};

} // namespace Credentials
} // namespace chip
