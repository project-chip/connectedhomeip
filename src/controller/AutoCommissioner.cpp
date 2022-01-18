/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <controller/AutoCommissioner.h>

#include <controller/CHIPDeviceController.h>
#include <credentials/CHIPCert.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Controller {

AutoCommissioner::~AutoCommissioner()
{
    ReleaseDAC();
    ReleasePAI();
}

void AutoCommissioner::SetOperationalCredentialsDelegate(OperationalCredentialsDelegate * operationalCredentialsDelegate)
{
    mOperationalCredentialsDelegate = operationalCredentialsDelegate;
}

CHIP_ERROR AutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    mParams = params;
    ChipLogProgress(Controller, "Setting Commissioning parameters");
    if (params.HasThreadOperationalDataset())
    {
        ByteSpan dataset = params.GetThreadOperationalDataset().Value();
        if (dataset.size() > CommissioningParameters::kMaxCredentialsLen)
        {
            ChipLogError(Controller, "Thread operational data set is too large");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mThreadOperationalDataset, dataset.data(), dataset.size());
        ChipLogProgress(Controller, "Setting thread operational dataset from parameters");
        mParams.SetThreadOperationalDataset(ByteSpan(mThreadOperationalDataset, dataset.size()));
    }
    if (params.HasWiFiCredentials())
    {
        WiFiCredentials creds = params.GetWiFiCredentials().Value();
        if (creds.ssid.size() > CommissioningParameters::kMaxSsidLen ||
            creds.credentials.size() > CommissioningParameters::kMaxCredentialsLen)
        {
            ChipLogError(Controller, "Wifi credentials are too large");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mSsid, creds.ssid.data(), creds.ssid.size());
        memcpy(mCredentials, creds.credentials.data(), creds.credentials.size());
        ChipLogProgress(Controller, "Setting wifi credentials from parameters");
        mParams.SetWiFiCredentials(
            WiFiCredentials(ByteSpan(mSsid, creds.ssid.size()), ByteSpan(mCredentials, creds.credentials.size())));
    }
    // If the AttestationNonce is passed in, using that else using a random one..
    if (params.HasAttestationNonce())
    {
        ChipLogProgress(Controller, "Setting attestation nonce from parameters");
        VerifyOrReturnError(params.GetAttestationNonce().Value().size() == sizeof(mAttestationNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mAttestationNonce, params.GetAttestationNonce().Value().data(), params.GetAttestationNonce().Value().size());
    }
    else
    {
        ChipLogProgress(Controller, "Setting attestation nonce to random value");
        Crypto::DRBG_get_bytes(mAttestationNonce, sizeof(mAttestationNonce));
    }
    mParams.SetAttestationNonce(ByteSpan(mAttestationNonce, sizeof(mAttestationNonce)));

    if (params.HasCSRNonce())
    {
        ChipLogProgress(Controller, "Setting CSR nonce from parameters");
        VerifyOrReturnError(params.GetCSRNonce().Value().size() == sizeof(mCSRNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mCSRNonce, params.GetCSRNonce().Value().data(), params.GetCSRNonce().Value().size());
    }
    else
    {
        ChipLogProgress(Controller, "Setting CSR nonce to random value");
        Crypto::DRBG_get_bytes(mCSRNonce, sizeof(mCSRNonce));
    }
    mParams.SetCSRNonce(ByteSpan(mCSRNonce, sizeof(mCSRNonce)));

    return CHIP_NO_ERROR;
}

CommissioningStage AutoCommissioner::GetNextCommissioningStage(CommissioningStage currentStage)
{
    switch (currentStage)
    {
    case CommissioningStage::kSecurePairing:
        return CommissioningStage::kArmFailsafe;
    case CommissioningStage::kArmFailsafe:
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kConfigRegulatory:
        return CommissioningStage::kSendPAICertificateRequest;
    case CommissioningStage::kSendPAICertificateRequest:
        return CommissioningStage::kSendDACCertificateRequest;
    case CommissioningStage::kSendDACCertificateRequest:
        return CommissioningStage::kSendAttestationRequest;
    case CommissioningStage::kSendAttestationRequest:
        return CommissioningStage::kAttestationVerification;
    case CommissioningStage::kAttestationVerification:
        return CommissioningStage::kSendOpCertSigningRequest;
    case CommissioningStage::kSendOpCertSigningRequest:
        return CommissioningStage::kGenerateNOCChain;
    case CommissioningStage::kGenerateNOCChain:
        return CommissioningStage::kSendTrustedRootCert;
    case CommissioningStage::kSendTrustedRootCert:
        return CommissioningStage::kSendNOC;
    case CommissioningStage::kSendNOC:
        // TODO(cecille): device attestation casues operational cert provisioinging to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
        if (mParams.HasWiFiCredentials())
        {
            return CommissioningStage::kWiFiNetworkSetup;
        }
        else if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
            return CommissioningStage::kFindOperational;
#else
            return CommissioningStage::kSendComplete;
#endif
        }
    case CommissioningStage::kWiFiNetworkSetup:
        if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
    case CommissioningStage::kThreadNetworkSetup:
        if (mParams.HasWiFiCredentials())
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
        else
        {
            return CommissioningStage::kThreadNetworkEnable;
        }

    case CommissioningStage::kWiFiNetworkEnable:
        if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkEnable;
        }
        else
        {
            return CommissioningStage::kFindOperational;
        }
    case CommissioningStage::kThreadNetworkEnable:
        return CommissioningStage::kFindOperational;
    case CommissioningStage::kFindOperational:
        return CommissioningStage::kSendComplete;
    case CommissioningStage::kSendComplete:
        return CommissioningStage::kCleanup;

    // Currently unimplemented.
    case CommissioningStage::kConfigACL:
        return CommissioningStage::kError;
    // Neither of these have a next stage so return kError;
    case CommissioningStage::kCleanup:
    case CommissioningStage::kError:
        return CommissioningStage::kError;
    }
    return CommissioningStage::kError;
}

void AutoCommissioner::StartCommissioning(CommissioneeDeviceProxy * proxy)
{
    // TODO: check that there is no commissioning in progress currently.
    mCommissioneeDeviceProxy = proxy;
    mCommissioner->PerformCommissioningStep(mCommissioneeDeviceProxy, CommissioningStage::kArmFailsafe, mParams, this);
}

CHIP_ERROR AutoCommissioner::NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac)
{
    {
        // Reuse NOC Cert buffer for temporary store Root Cert.
        MutableByteSpan rootCert = MutableByteSpan(mNOCCertBuffer);
        ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(rcac, rootCert));
        mParams.SetRootCert(rootCert);
        mCommissioner->PerformCommissioningStep(mCommissioneeDeviceProxy, CommissioningStage::kSendTrustedRootCert, mParams, this);
    }

    NOCerts certs;
    if (!icac.empty())
    {
        MutableByteSpan icaCert = MutableByteSpan(mICACertBuffer);
        ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(icac, icaCert));
        certs.icac = icaCert;
    }
    {
        MutableByteSpan nocCert = MutableByteSpan(mNOCCertBuffer);
        ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(noc, nocCert));
        certs.noc = nocCert;
    }
    mParams.SetNOCerts(certs);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AutoCommissioner::CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to perform commissioning step %d", static_cast<int>(report.stageCompleted));
        return err;
    }
    switch (report.stageCompleted)
    {
    case CommissioningStage::kSendPAICertificateRequest:
        SetPAI(report.requestedCertificate.certificate);
        break;
    case CommissioningStage::kSendDACCertificateRequest:
        SetDAC(report.requestedCertificate.certificate);
        break;
    case CommissioningStage::kSendAttestationRequest:
        // These don't need to be deep copied to local memory because they are used in this one step then never again.
        mParams.SetAttestationElements(report.attestationResponse.attestationElements)
            .SetAttestationSignature(report.attestationResponse.signature);
        // TODO: Does this need to be done at runtime? Seems like this could be done earlier and we woouldn't need to hold a
        // reference to the operational credential delegate here
        if (mOperationalCredentialsDelegate != nullptr)
        {
            MutableByteSpan nonce(mCSRNonce);
            ReturnErrorOnFailure(mOperationalCredentialsDelegate->ObtainCsrNonce(nonce));
            mParams.SetCSRNonce(ByteSpan(mCSRNonce, sizeof(mCSRNonce)));
        }
        break;
    case CommissioningStage::kSendOpCertSigningRequest: {
        NOCChainGenerationParameters nocParams;
        nocParams.nocsrElements = report.attestationResponse.attestationElements;
        nocParams.signature     = report.attestationResponse.signature;
        mParams.SetNOCChainGenerationParameters(nocParams);
    }
    break;
    case CommissioningStage::kGenerateNOCChain:
        // For NOC chain generation, we re-use the buffers. NOCChainGenerated triggers the next stage before
        // storing the returned certs, so just return here without triggering the next stage.
        return NOCChainGenerated(report.nocChain.noc, report.nocChain.icac, report.nocChain.rcac);
    case CommissioningStage::kFindOperational:
        mOperationalDeviceProxy = report.OperationalNodeFoundData.operationalProxy;
        break;
    case CommissioningStage::kCleanup:
        ReleasePAI();
        ReleaseDAC();
        mCommissioneeDeviceProxy = nullptr;
        mOperationalDeviceProxy  = nullptr;
        mParams                  = CommissioningParameters();
        return CHIP_NO_ERROR;
    default:
        break;
    }

    CommissioningStage nextStage = GetNextCommissioningStage(report.stageCompleted);

    DeviceProxy * proxy = mCommissioneeDeviceProxy;
    if (nextStage == CommissioningStage::kSendComplete || nextStage == CommissioningStage::kCleanup)
    {
        proxy = mOperationalDeviceProxy;
    }

    if (proxy == nullptr)
    {
        ChipLogError(Controller, "Invalid device for commissioning");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mCommissioner->PerformCommissioningStep(proxy, nextStage, mParams, this);
    return CHIP_NO_ERROR;
}

void AutoCommissioner::ReleaseDAC()
{
    if (mDAC != nullptr)
    {
        Platform::MemoryFree(mDAC);
    }
    mDACLen = 0;
    mDAC    = nullptr;
}

CHIP_ERROR AutoCommissioner::SetDAC(const ByteSpan & dac)
{
    if (dac.size() == 0)
    {
        ReleaseDAC();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(dac.size() <= Credentials::kMaxDERCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mDACLen != 0)
    {
        ReleaseDAC();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(dac.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mDAC == nullptr)
    {
        mDAC = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(dac.size()));
    }
    VerifyOrReturnError(mDAC != nullptr, CHIP_ERROR_NO_MEMORY);
    mDACLen = static_cast<uint16_t>(dac.size());
    memcpy(mDAC, dac.data(), mDACLen);
    mParams.SetDAC(ByteSpan(mDAC, mDACLen));

    return CHIP_NO_ERROR;
}

void AutoCommissioner::ReleasePAI()
{
    if (mPAI != nullptr)
    {
        chip::Platform::MemoryFree(mPAI);
    }
    mPAILen = 0;
    mPAI    = nullptr;
}

CHIP_ERROR AutoCommissioner::SetPAI(const chip::ByteSpan & pai)
{
    if (pai.size() == 0)
    {
        ReleasePAI();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(pai.size() <= Credentials::kMaxDERCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mPAILen != 0)
    {
        ReleasePAI();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(pai.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mPAI == nullptr)
    {
        mPAI = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(pai.size()));
    }
    VerifyOrReturnError(mPAI != nullptr, CHIP_ERROR_NO_MEMORY);
    mPAILen = static_cast<uint16_t>(pai.size());
    memcpy(mPAI, pai.data(), mPAILen);
    mParams.SetPAI(ByteSpan(mPAI, mPAILen));

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
