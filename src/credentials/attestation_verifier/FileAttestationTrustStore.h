/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

#include <array>
#include <vector>

namespace chip {
namespace Credentials {

class FileAttestationTrustStore : public AttestationTrustStore
{
public:
    FileAttestationTrustStore(const char * paaTrustStorePath = nullptr, const char * cdTrustStorePath = nullptr);
    ~FileAttestationTrustStore();

    CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & outPaaDerBuffer) const override;
    CHIP_ERROR GetCertificationDeclarationSigningKey(const ByteSpan & skid, Crypto::P256PublicKey & pubKey) const override;

    bool IsInitialized() const { return mIsInitialized; }
    size_t paaCount() const { return mPAADerCerts.size(); };
    size_t cdCount() const { return mCDDerCerts.size(); };

protected:
    std::vector<std::array<uint8_t, kMaxDERCertLength>> mPAADerCerts;
    std::vector<std::array<uint8_t, kMaxDERCertLength>> mCDDerCerts;

private:
    void LoadTrustStore(const char * trustStorePath, std::vector<std::array<uint8_t, kMaxDERCertLength>> & certs);

    bool mIsInitialized = false;

    void Cleanup();
};

} // namespace Credentials
} // namespace chip
