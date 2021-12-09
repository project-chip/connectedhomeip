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
#include <lib/support/SafeInt.h>

namespace chip {
namespace Controller {

AutoCommissioner::~AutoCommissioner()
{
    ReleaseDAC();
    ReleasePAI();
}

CHIP_ERROR AutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    mParams = params;
    if (params.HasThreadOperationalDataset())
    {
        ByteSpan dataset = params.GetThreadOperationalDataset().Value();
        if (dataset.size() > CommissioningParameters::kMaxCredentialsLen)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mThreadOperationalDataset, dataset.data(), dataset.size());
        mParams.SetThreadOperationalDataset(ByteSpan(mThreadOperationalDataset, dataset.size()));
    }
    if (params.HasWifiCredentials())
    {
        WifiCredentials creds = params.GetWifiCredentials().Value();
        if (creds.ssid.size() > CommissioningParameters::kMaxSsidLen ||
            creds.credentials.size() > CommissioningParameters::kMaxCredentialsLen)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mSsid, creds.ssid.data(), creds.ssid.size());
        memcpy(mCredentials, creds.credentials.data(), creds.credentials.size());
        mParams.SetWifiCredentials(
            WifiCredentials(ByteSpan(mSsid, creds.ssid.size()), ByteSpan(mCredentials, creds.credentials.size())));
    }
    // If the AttestationNonce is passed in, using that else using a random one..
    if (params.HasAttestationNonce())
    {
        VerifyOrReturnError(params.GetAttestationNonce().Value().size() == sizeof(mAttestationNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mAttestationNonce, params.GetAttestationNonce().Value().data(), params.GetAttestationNonce().Value().size());
    }
    else
    {
        Crypto::DRBG_get_bytes(mAttestationNonce, sizeof(mAttestationNonce));
    }
    mParams.SetAttestationNonce(ByteSpan(mAttestationNonce, sizeof(mAttestationNonce)));

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
        return CommissioningStage::kCheckCertificates;
    case CommissioningStage::kCheckCertificates:
        // TODO(cecille): device attestation casues operational cert provisioinging to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
        if (mParams.HasWifiCredentials())
        {
            return CommissioningStage::kWifiNetworkSetup;
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
    case CommissioningStage::kWifiNetworkSetup:
        if (mParams.HasThreadOperationalDataset())
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
            return CommissioningStage::kWifiNetworkEnable;
        }
    case CommissioningStage::kThreadNetworkSetup:
        if (mParams.HasWifiCredentials())
        {
            return CommissioningStage::kWifiNetworkEnable;
        }
        else
        {
            return CommissioningStage::kThreadNetworkEnable;
        }

    case CommissioningStage::kWifiNetworkEnable:
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

CHIP_ERROR AutoCommissioner::CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    switch (report.stageCompleted)
    {
    case CommissioningStage::kSendPAICertificateRequest:
        SetPAI(report.requestedCertificate.certificate);
        break;
    case CommissioningStage::kSendDACCertificateRequest:
        SetDAC(report.requestedCertificate.certificate);
        break;
    case CommissioningStage::kSendAttestationRequest:
        ReturnErrorOnFailure(mCommissioner->ValidateAttestationInfo(
            report.attestationResponse.attestationElements, report.attestationResponse.signature,
            mParams.GetAttestationNonce().Value(), GetPAI(), GetDAC(), mCommissioneeDeviceProxy));
        break;

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

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
