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
    ByteSpan dac = info.dacDerBuffer();
    ByteSpan pai = info.paiDerBuffer();

    mDacDerBuffer      = dac;
    mPaiDerBuffer      = pai;
    mVendorId          = info.BasicInformationVendorId();
    mProductId         = info.BasicInformationProductId();
    mAttestationResult = attestationResult;

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

    printf("VendorId: %d\n", mVendorId);
    printf("ProductId: %d\n", mProductId);

    PrintCert("DAC", mDacDerBuffer);
    PrintCert("PAI", mPaiDerBuffer);

    if (mAttestationResult == chip::Credentials::AttestationVerificationResult::kSuccess)
    {
        printf("Valid certs\n");
    }
    else
    {
        printf("Invalid cert: %d\n", mAttestationResult);
    }
}

void VerifyCommand::PrintCert(const char * name, chip::ByteSpan & buffer)
{
    PemEncoder encoder(name, buffer);
    const char * line = encoder.NextLine();
    while (line != nullptr)
    {
        printf("%s\n", line);
        line = encoder.NextLine();
    }
}
