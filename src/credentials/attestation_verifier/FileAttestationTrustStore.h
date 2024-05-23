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

enum class CertificateValidationMode
{
    // Validate that the certificate is a valid PAA certificate.
    kPAA,
    // Validate just that the certificate has a public key we can extract
    // (e.g. it's a CD signing certificate).
    kPublicKeyOnly,
};

/**
 * @brief Load all X.509 DER certificates in a given path.
 *
 * Silently ignores non-X.509 files and X.509 files that fail validation as
 * determined by the provided validation mode.
 *
 * Returns an empty vector if no files are found or unrecoverable errors arise.
 *
 * @param trustStorePath - path from where to search for certificates.
 * @param validationMode - how the certificate files should be validated.
 * @return a vector of certificate DER data
 */
std::vector<std::vector<uint8_t>> LoadAllX509DerCerts(const char * trustStorePath,
                                                      CertificateValidationMode validationMode = CertificateValidationMode::kPAA);

class FileAttestationTrustStore : public AttestationTrustStore
{
public:
    FileAttestationTrustStore(const char * paaTrustStorePath = nullptr);
    ~FileAttestationTrustStore();

    CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & outPaaDerBuffer) const override;

    bool IsInitialized() const { return mIsInitialized; }
    size_t paaCount() const { return mPAADerCerts.size(); };

protected:
    std::vector<std::vector<uint8_t>> mPAADerCerts;

private:
    bool mIsInitialized = false;

    void Cleanup();
};

} // namespace Credentials
} // namespace chip
