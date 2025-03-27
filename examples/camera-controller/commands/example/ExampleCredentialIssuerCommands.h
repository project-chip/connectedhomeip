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

#include <commands/common/CredentialIssuerCommands.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

class ExampleCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    CHIP_ERROR InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage) override
    {
        return mOpCredsIssuer.Initialize(storage);
    }
    CHIP_ERROR SetupDeviceAttestation(chip::Controller::SetupParams & setupParams,
                                      const chip::Credentials::AttestationTrustStore * trustStore) override
    {
        chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

        mDacVerifier                          = chip::Credentials::GetDefaultDACVerifier(trustStore);
        setupParams.deviceAttestationVerifier = mDacVerifier;
        mDacVerifier->EnableCdTestKeySupport(mAllowTestCdSigningKey);

        return CHIP_NO_ERROR;
    }
    chip::Controller::OperationalCredentialsDelegate * GetCredentialIssuer() override { return &mOpCredsIssuer; }
    void SetCredentialIssuerCATValues(chip::CATValues cats) override { mOpCredsIssuer.SetCATValuesForNextNOCRequest(cats); }
    CHIP_ERROR GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
                                          chip::Crypto::P256Keypair & keypair, chip::MutableByteSpan & rcac,
                                          chip::MutableByteSpan & icac, chip::MutableByteSpan & noc) override
    {
        return mOpCredsIssuer.GenerateNOCChainAfterValidation(nodeId, fabricId, cats, keypair.Pubkey(), rcac, icac, noc);
    }

    CHIP_ERROR AddAdditionalCDVerifyingCerts(const std::vector<std::vector<uint8_t>> & additionalCdCerts) override
    {
        VerifyOrReturnError(mDacVerifier != nullptr, CHIP_ERROR_INCORRECT_STATE);

        for (const auto & cert : additionalCdCerts)
        {
            auto cdTrustStore = mDacVerifier->GetCertificationDeclarationTrustStore();
            VerifyOrReturnError(cdTrustStore != nullptr, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(cdTrustStore->AddTrustedKey(chip::ByteSpan(cert.data(), cert.size())));
        }

        return CHIP_NO_ERROR;
    }

    void SetCredentialIssuerOption(CredentialIssuerOptions option, bool isEnabled) override
    {
        switch (option)
        {
        case CredentialIssuerOptions::kMaximizeCertificateSizes:
            mUsesMaxSizedCerts = isEnabled;
            mOpCredsIssuer.SetMaximallyLargeCertsUsed(mUsesMaxSizedCerts);
            break;
        case CredentialIssuerOptions::kAllowTestCdSigningKey:
            mAllowTestCdSigningKey = isEnabled;
            if (mDacVerifier != nullptr)
            {
                mDacVerifier->EnableCdTestKeySupport(isEnabled);
            }
            break;
        default:
            break;
        }
    }

    bool GetCredentialIssuerOption(CredentialIssuerOptions option) override
    {
        switch (option)
        {
        case CredentialIssuerOptions::kMaximizeCertificateSizes:
            return mUsesMaxSizedCerts;
        case CredentialIssuerOptions::kAllowTestCdSigningKey:
            return mAllowTestCdSigningKey;
        default:
            return false;
        }
    }

protected:
    bool mUsesMaxSizedCerts = false;
    // Starts true for legacy purposes
    bool mAllowTestCdSigningKey = true;

private:
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
    chip::Credentials::DeviceAttestationVerifier * mDacVerifier;
};
