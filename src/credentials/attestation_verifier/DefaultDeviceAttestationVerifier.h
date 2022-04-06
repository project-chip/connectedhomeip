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

#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

namespace chip {
namespace Credentials {

/**
 * @brief Get implementation of a PAA root store containing a basic set of static PAA roots
 *        sufficient for *testing* only.
 *
 * WARNING: The PAA list known to this PAA root store is a reduced subset that will likely
 *          cause users of it to fail attestation procedure in some cases. This is provided
 *          to support tests and examples, not to be used by real commissioners, as it
 *          contains several test roots which are not trustworthy for certified product usage.
 *
 * @returns a singleton AttestationTrustStore that contains some well-known PAA test root certs.
 */
const AttestationTrustStore * GetTestAttestationTrustStore();

/**
 * @brief Get implementation of a sample DAC verifier to validate device
 *        attestation procedure.
 *
 * @param[in] paaRootStore Pointer to the AttestationTrustStore instance to be used by implementation
 *                         of default DeviceAttestationVerifier. Caller must ensure storage is
 *                         always available while the DeviceAttestationVerifier could be used.
 *
 * @returns a singleton DeviceAttestationVerifier that satisfies basic device attestation procedure requirements.
 */
DeviceAttestationVerifier * GetDefaultDACVerifier(const AttestationTrustStore * paaRootStore);

} // namespace Credentials
} // namespace chip
