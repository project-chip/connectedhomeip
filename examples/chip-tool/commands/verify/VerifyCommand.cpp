/*
 *   Copyright (c) 2026 Project CHIP Authors
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

#include "VerifyCommand.h"
#include <commands/common/DeviceScanner.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/dnssd/Types.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadDiscoveryCode.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/PASESession.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#include "../dcl/DCLClient.h"
#include "../dcl/DisplayTermsAndConditions.h"

#include <inttypes.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace ::chip;
using namespace ::chip::Controller;
using namespace ::chip::Crypto;

CHIP_ERROR VerifyCommand::Run()
{
    CHIP_ERROR err = CHIPCommand::Run();
    PrintDeviceInformation();
    return err;
}

CHIP_ERROR VerifyCommand::RunCommand()
{
    CurrentCommissioner().RegisterPairingDelegate(this);
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(this);
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = CurrentCommissioner().EstablishPASEConnection(mNodeId, mOnboardingPayload, DiscoveryType::kAll);

    return err;
}

chip::Optional<uint16_t> VerifyCommand::FailSafeExpiryTimeoutSecs() const
{
    return Optional<uint16_t>();
}

void VerifyCommand::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
                                                 chip::DeviceProxy * device,
                                                 const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                                 chip::Credentials::AttestationVerificationResult attestationResult)
{

    auto dac      = info.dacDerBuffer();
    auto pai      = info.paiDerBuffer();
    auto cdBuffer = info.cdBuffer();

    VerifyOrDie(SetBuffer(mDacCertBuf, dac) == CHIP_NO_ERROR);
    VerifyOrDie(SetBuffer(mPaiCertBuf, pai) == CHIP_NO_ERROR);

    if (cdBuffer.HasValue())
    {
        VerifyOrDie(SetBuffer(mCdBuf, cdBuffer.Value()) == CHIP_NO_ERROR);
    }

    mVendorId          = info.BasicInformationVendorId();
    mProductId         = info.BasicInformationProductId();
    mAttestationResult = attestationResult;

    uint8_t paiAkidBuf[chip::Crypto::kAuthorityKeyIdentifierLength];
    chip::MutableByteSpan paiAkid(paiAkidBuf);
    if (chip::Crypto::ExtractAKIDFromX509Cert(pai, paiAkid) == CHIP_NO_ERROR)
    {
        auto * verifier        = CurrentCommissioner().GetDeviceAttestationVerifier();
        auto * defaultVerifier = static_cast<chip::Credentials::DefaultDACVerifier *>(verifier);
        const chip::Credentials::AttestationTrustStore * trustStore     = defaultVerifier->GetAttestationTrustStore();
        const chip::Credentials::AttestationTrustStore * testTrustStore = chip::Credentials::GetTestAttestationTrustStore();

        uint8_t paaScratchBuf[chip::Credentials::kMaxDERCertLength];
        chip::MutableByteSpan paaDerBuffer(paaScratchBuf);

        CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
        if (trustStore != nullptr)
        {
            err = trustStore->GetProductAttestationAuthorityCert(paiAkid, paaDerBuffer);
        }
        if (err == CHIP_ERROR_NOT_IMPLEMENTED)
        {
            // Use test trust store as fallback.
            err = testTrustStore->GetProductAttestationAuthorityCert(paiAkid, paaDerBuffer);
        }
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrDie(SetBuffer(mPaaCertBuf, chip::ByteSpan(paaDerBuffer.data(), paaDerBuffer.size())) == CHIP_NO_ERROR);
        }
    }

    CHIP_ERROR err = CurrentCommissioner().StopPairing(mNodeId);
    VerifyOrDie(err == CHIP_NO_ERROR);
}

bool VerifyCommand::ShouldWaitAfterDeviceAttestation()
{
    return true; // Required to force OnDeviceAttestationCompleted to run
}

void VerifyCommand::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) {}
void VerifyCommand::OnPairingComplete(CHIP_ERROR error)
{
    if (error != CHIP_NO_ERROR)
    {
        ChipLogProgress(chipTool, "PASE failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    ChipLogProgress(chipTool, "PASE complete, starting commissioning for certificate fetch");

    CommissioningParameters params;
    params.SetDeviceAttestationDelegate(this);
    CHIP_ERROR err = CurrentCommissioner().Commission(mNodeId, params);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Failed to start commissioning: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
void VerifyCommand::OnPairingDeleted(CHIP_ERROR error) {}
void VerifyCommand::OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) {}
void VerifyCommand::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error)
{
    SetCommandExitStatus(CHIP_NO_ERROR); // Stop the command
}
void VerifyCommand::OnICDRegistrationComplete(chip::ScopedNodeId deviceId, uint32_t icdCounter) {}
void VerifyCommand::OnICDStayActiveComplete(chip::ScopedNodeId deviceId, uint32_t promisedActiveDuration) {}
void VerifyCommand::OnCommissioningStageStart(chip::PeerId peerId, chip::Controller::CommissioningStage stageStarting) {}
CHIP_ERROR VerifyCommand::WiFiCredentialsNeeded(chip::EndpointId endpoint)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR VerifyCommand::ThreadCredentialsNeeded(chip::EndpointId endpoint)
{
    return CHIP_NO_ERROR;
}

void VerifyCommand::OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData & nodeData) {}

void VerifyCommand::PrintDeviceInformation()
{

    printf("VendorId: 0x%04X\n", mVendorId);
    printf("ProductId: 0x%04X\n", mProductId);

    printf("DAC:\n");
    PrintCert("CERTIFICATE", DacDerBuffer().Value());
    printf("PAI:\n");
    PrintCert("CERTIFICATE", PaiDerBuffer().Value());

    if (CdBuffer().HasValue())
    {
        auto cdBuffer = CdBuffer().Value();
        printf("CD:\n");
        for (size_t i = 0; i < cdBuffer.size(); i++)
        {
            printf("%02x", cdBuffer[i]);
        }
        printf("\n");
    }
    else
    {
        printf("No CD\n");
    }

    if (PaaDerBuffer().HasValue())
    {
        printf("PAA:\n");
        PrintCert("CERTIFICATE", PaaDerBuffer().Value());
    }
    else
    {
        printf("No PAA\n");
    }

    printf("Attestation: %s\n", GetAttestationResultDescription(mAttestationResult));
}

void VerifyCommand::PrintCert(const char * name, chip::ByteSpan buffer)
{
    PemEncoder encoder(name, buffer);
    const char * line = encoder.NextLine();
    while (line != nullptr)
    {
        printf("%s\n", line);
        line = encoder.NextLine();
    }
}
