/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app/util/basic-types.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <vector>

namespace chip {
namespace Controller {
struct SetupParams;
class OperationalCredentialsDelegate;
} // namespace Controller
} // namespace chip

class CredentialIssuerCommands
{
public:
    virtual ~CredentialIssuerCommands() {}

    /**
     * @brief
     *   This function is used to initialize the Credentials Issuer, if needed.
     *
     * @param[in] storage A reference to the storage, where the Credentials Issuer can optionally use to access the keypair in
     *                    storage.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage) = 0;

    /**
     * @brief
     *   This function is used to setup Device Attestation Singletons and intialize Setup/Commissioning Parameters with a custom
     *   Device Attestation Verifier object.
     *
     * @param[in] setupParams A reference to the Setup/Commissioning Parameters, to be initialized with custom Device Attestation
     *                        Verifier.
     * @param[in] trustStore  A pointer to the PAA trust store to use to find valid PAA roots.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR SetupDeviceAttestation(chip::Controller::SetupParams & setupParams,
                                              const chip::Credentials::AttestationTrustStore * trustStore) = 0;

    /**
     * @brief Add a list of additional non-default CD verifying keys (by certificate)
     *
     * Must be called AFTER SetupDeviceAttestation.
     *
     * @param additionalCdCerts - vector of X.509 DER verifying cert bodies
     * @return CHIP_NO_ERROR on succes, another CHIP_ERROR on internal failures.
     */
    virtual CHIP_ERROR AddAdditionalCDVerifyingCerts(const std::vector<std::vector<uint8_t>> & additionalCdCerts) = 0;

    virtual chip::Controller::OperationalCredentialsDelegate * GetCredentialIssuer() = 0;

    virtual void SetCredentialIssuerCATValues(chip::CATValues cats) = 0;

    /**
     * @brief
     *   This function is used to Generate NOC Chain for the Controller/Commissioner. Parameters follow the example implementation,
     *   so some parameters may not translate to the real remote Credentials Issuer policy.
     *
     * @param[in] nodeId   The desired NodeId for the generated NOC Chain - May be optional/unused in some implementations.
     * @param[in] fabricId The desired FabricId for the generated NOC Chain - May be optional/unused in some implementations.
     * @param[in] cats     The desired CATs for the generated NOC Chain - May be optional/unused in some implementations.
     * @param[in] keypair  The desired Keypair for the generated NOC Chain - May be optional/unused in some implementations.
     * @param[in,out] rcac  Buffer to hold the Root Certificate of the generated NOC Chain.
     * @param[in,out] icac  Buffer to hold the Intermediate Certificate of the generated NOC Chain.
     * @param[in,out] noc   Buffer to hold the Leaf Certificate of the generated NOC Chain.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
                                                  chip::Crypto::P256Keypair & keypair, chip::MutableByteSpan & rcac,
                                                  chip::MutableByteSpan & icac, chip::MutableByteSpan & noc) = 0;

    // All options must start false
    enum CredentialIssuerOptions : uint8_t
    {
        kMaximizeCertificateSizes = 0, // If set, certificate chains will be maximized for testing via padding
        kAllowTestCdSigningKey    = 1, // If set, allow development/test SDK CD verifying key to be used
    };

    virtual void SetCredentialIssuerOption(CredentialIssuerOptions option, bool isEnabled)
    {
        // Do nothing
        (void) option;
        (void) isEnabled;
    }

    virtual bool GetCredentialIssuerOption(CredentialIssuerOptions option)
    {
        // All options always start false
        return false;
    }
};
