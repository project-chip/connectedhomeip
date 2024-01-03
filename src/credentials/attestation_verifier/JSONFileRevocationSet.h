/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <json/json.h>

namespace chip {
namespace Credentials {

/**
 * @brief Load a Revocation Set from a given path.
 *
 * Returns an empty vector if no files are found or unrecoverable errors arise.
 *
 * @param revocationSetPath - path from where to search for a revocation set.
 * @return a vector of certificate DER data
 */
void LoadJSONRevocationSet(const char * revocationSetPath, Json::Value & revocationSet);

class JSONFileRevocationSet : public RevocationSet
{
public:
    JSONFileRevocationSet(const char * revocationSetPath = nullptr);
    ~JSONFileRevocationSet();

    AttestationVerificationResult IsCertificateRevoked(bool isPaa, Crypto::AttestationCertVidPid vidPidUnderTest, ByteSpan issuer,
                                                       ByteSpan authorityKeyId, ByteSpan serialNumber) const override;

    bool IsInitialized() const { return mIsInitialized; }
    size_t revocationSetCount() const { return mRevocationSet.size(); };

protected:
    Json::Value mRevocationSet;

private:
    bool mIsInitialized = false;

    void Cleanup();
};

} // namespace Credentials
} // namespace chip
