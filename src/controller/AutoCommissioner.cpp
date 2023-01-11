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

#include <app/InteractionModelTimeout.h>
#include <controller/CHIPDeviceController.h>
#include <credentials/CHIPCert.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Controller {

AutoCommissioner::AutoCommissioner()
{
    SetCommissioningParameters(CommissioningParameters());
}

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
    if (params.GetFailsafeTimerSeconds().HasValue())
    {
        ChipLogProgress(Controller, "Setting failsafe timer from parameters");
        mParams.SetFailsafeTimerSeconds(params.GetFailsafeTimerSeconds().Value());
    }

    if (params.GetAdminSubject().HasValue())
    {
        ChipLogProgress(Controller, "Setting adminSubject from parameters");
        mParams.SetAdminSubject(params.GetAdminSubject().Value());
    }

    if (params.GetThreadOperationalDataset().HasValue())
    {
        ByteSpan dataset = params.GetThreadOperationalDataset().Value();
        if (dataset.size() > CommissioningParameters::kMaxThreadDatasetLen)
        {
            ChipLogError(Controller, "Thread operational data set is too large");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memcpy(mThreadOperationalDataset, dataset.data(), dataset.size());
        ChipLogProgress(Controller, "Setting thread operational dataset from parameters");
        mParams.SetThreadOperationalDataset(ByteSpan(mThreadOperationalDataset, dataset.size()));
    }

    if (params.GetAttemptThreadNetworkScan().HasValue())
    {
        ChipLogProgress(Controller, "Setting attempt thread scan from parameters");
        mParams.SetAttemptThreadNetworkScan(params.GetAttemptThreadNetworkScan().Value());
    }

    if (params.GetWiFiCredentials().HasValue())
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

    if (params.GetAttemptWiFiNetworkScan().HasValue())
    {
        ChipLogProgress(Controller, "Setting attempt wifi scan from parameters");
        mParams.SetAttemptWiFiNetworkScan(params.GetAttemptWiFiNetworkScan().Value());
    }

    if (params.GetCountryCode().HasValue())
    {
        auto code = params.GetCountryCode().Value();
        MutableCharSpan copiedCode(mCountryCode);
        if (CopyCharSpanToMutableCharSpan(code, copiedCode) == CHIP_NO_ERROR)
        {
            mParams.SetCountryCode(copiedCode);
        }
        else
        {
            ChipLogError(Controller, "Country code is too large: %u", static_cast<unsigned>(code.size()));
        }
    }

    // If the AttestationNonce is passed in, using that else using a random one..
    if (params.GetAttestationNonce().HasValue())
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

    if (params.GetCSRNonce().HasValue())
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

    if (params.GetSkipCommissioningComplete().HasValue())
    {
        ChipLogProgress(Controller, "Setting PASE-only commissioning from parameters");
        mParams.SetSkipCommissioningComplete(params.GetSkipCommissioningComplete().Value());
    }

    return CHIP_NO_ERROR;
}

const CommissioningParameters & AutoCommissioner::GetCommissioningParameters() const
{
    return mParams;
}

CommissioningStage AutoCommissioner::GetNextCommissioningStage(CommissioningStage currentStage, CHIP_ERROR & lastErr)
{
    auto nextStage = GetNextCommissioningStageInternal(currentStage, lastErr);
    if (lastErr == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Commissioning stage next step: '%s' -> '%s'", StageToString(currentStage),
                        StageToString(nextStage));
    }
    else
    {
        ChipLogProgress(Controller, "Going from commissioning step '%s' with lastErr = '%s' -> '%s'", StageToString(currentStage),
                        lastErr.AsString(), StageToString(nextStage));
    }
    return nextStage;
}

CommissioningStage AutoCommissioner::GetNextCommissioningStageInternal(CommissioningStage currentStage, CHIP_ERROR & lastErr)
{
    if (mStopCommissioning)
    {
        return CommissioningStage::kCleanup;
    }
    if (lastErr != CHIP_NO_ERROR)
    {
        return CommissioningStage::kCleanup;
    }

    switch (currentStage)
    {
    case CommissioningStage::kSecurePairing:
        return CommissioningStage::kReadCommissioningInfo;
    case CommissioningStage::kReadCommissioningInfo:
        if (mDeviceCommissioningInfo.general.breadcrumb > 0)
        {
            // If the breadcrumb is 0, the failsafe was disarmed.
            // We failed on network setup or later, the node failsafe has not been re-armed and the breadcrumb has not been reset.
            // Per the spec, we restart from after adding the NOC.
            return GetNextCommissioningStage(CommissioningStage::kSendNOC, lastErr);
        }
        return CommissioningStage::kArmFailsafe;
    case CommissioningStage::kArmFailsafe:
        if (mNeedsNetworkSetup)
        {
            // if there is a WiFi or a Thread endpoint, then perform scan
            if ((mParams.GetAttemptWiFiNetworkScan().ValueOr(false) &&
                 mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId) ||
                (mParams.GetAttemptThreadNetworkScan().ValueOr(false) &&
                 mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId))
            {
                return CommissioningStage::kScanNetworks;
            }
            ChipLogProgress(Controller, "No NetworkScan enabled or WiFi/Thread endpoint not specified, skipping ScanNetworks");
        }
        else
        {
            ChipLogProgress(Controller, "Not a BLE connection, skipping ScanNetworks");
        }
        // skip scan step
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kScanNetworks:
        return CommissioningStage::kNeedsNetworkCreds;
    case CommissioningStage::kNeedsNetworkCreds:
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
        return CommissioningStage::kValidateCSR;
    case CommissioningStage::kValidateCSR:
        return CommissioningStage::kGenerateNOCChain;
    case CommissioningStage::kGenerateNOCChain:
        return CommissioningStage::kSendTrustedRootCert;
    case CommissioningStage::kSendTrustedRootCert:
        return CommissioningStage::kSendNOC;
    case CommissioningStage::kSendNOC:
        // TODO(cecille): device attestation casues operational cert provisioinging to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
        if (mNeedsNetworkSetup)
        {
            if (mParams.GetWiFiCredentials().HasValue() && mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId)
            {
                return CommissioningStage::kWiFiNetworkSetup;
            }
            if (mParams.GetThreadOperationalDataset().HasValue() &&
                mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId)
            {
                return CommissioningStage::kThreadNetworkSetup;
            }

            ChipLogError(Controller, "Required network information not provided in commissioning parameters");
            ChipLogError(Controller, "Parameters supplied: wifi (%s) thread (%s)",
                         mParams.GetWiFiCredentials().HasValue() ? "yes" : "no",
                         mParams.GetThreadOperationalDataset().HasValue() ? "yes" : "no");
            ChipLogError(Controller, "Device supports: wifi (%s) thread(%s)",
                         mDeviceCommissioningInfo.network.wifi.endpoint == kInvalidEndpointId ? "no" : "yes",
                         mDeviceCommissioningInfo.network.thread.endpoint == kInvalidEndpointId ? "no" : "yes");
            lastErr = CHIP_ERROR_INVALID_ARGUMENT;
            return CommissioningStage::kCleanup;
        }
        else
        {
            if (mParams.GetSkipCommissioningComplete().ValueOr(false))
            {
                return CommissioningStage::kCleanup;
            }
            return CommissioningStage::kFindOperational;
        }
    case CommissioningStage::kWiFiNetworkSetup:
        if (mParams.GetThreadOperationalDataset().HasValue() &&
            mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId)
        {
            return CommissioningStage::kThreadNetworkSetup;
        }
        else
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
    case CommissioningStage::kThreadNetworkSetup:
        if (mParams.GetWiFiCredentials().HasValue() && mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId)
        {
            return CommissioningStage::kWiFiNetworkEnable;
        }
        else
        {
            return CommissioningStage::kThreadNetworkEnable;
        }

    case CommissioningStage::kWiFiNetworkEnable:
        if (mParams.GetThreadOperationalDataset().HasValue() &&
            mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId)
        {
            return CommissioningStage::kThreadNetworkEnable;
        }
        else if (mParams.GetSkipCommissioningComplete().ValueOr(false))
        {
            return CommissioningStage::kCleanup;
        }
        else
        {
            return CommissioningStage::kFindOperational;
        }
    case CommissioningStage::kThreadNetworkEnable:
        if (mParams.GetSkipCommissioningComplete().ValueOr(false))
        {
            return CommissioningStage::kCleanup;
        }
        return CommissioningStage::kFindOperational;
    case CommissioningStage::kFindOperational:
        return CommissioningStage::kSendComplete;
    case CommissioningStage::kSendComplete:
        return CommissioningStage::kCleanup;

    // Neither of these have a next stage so return kError;
    case CommissioningStage::kCleanup:
    case CommissioningStage::kError:
        return CommissioningStage::kError;
    }
    return CommissioningStage::kError;
}

EndpointId AutoCommissioner::GetEndpoint(const CommissioningStage & stage) const
{
    switch (stage)
    {
    case CommissioningStage::kWiFiNetworkSetup:
    case CommissioningStage::kWiFiNetworkEnable:
        return mDeviceCommissioningInfo.network.wifi.endpoint;
    case CommissioningStage::kThreadNetworkSetup:
    case CommissioningStage::kThreadNetworkEnable:
        return mDeviceCommissioningInfo.network.thread.endpoint;
    default:
        return kRootEndpointId;
    }
}

CHIP_ERROR AutoCommissioner::StartCommissioning(DeviceCommissioner * commissioner, CommissioneeDeviceProxy * proxy)
{
    if (commissioner == nullptr)
    {
        ChipLogError(Controller, "Invalid DeviceCommissioner");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (proxy == nullptr || !proxy->GetSecureSession().HasValue())
    {
        ChipLogError(Controller, "Device proxy secure session error");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mStopCommissioning       = false;
    mCommissioner            = commissioner;
    mCommissioneeDeviceProxy = proxy;
    mNeedsNetworkSetup =
        mCommissioneeDeviceProxy->GetSecureSession().Value()->AsSecureSession()->GetPeerAddress().GetTransportType() ==
        Transport::Type::kBle;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    CommissioningStage nextStage = GetNextCommissioningStage(CommissioningStage::kSecurePairing, err);
    mCommissioner->PerformCommissioningStep(mCommissioneeDeviceProxy, nextStage, mParams, this, GetEndpoint(nextStage),
                                            GetCommandTimeout(mCommissioneeDeviceProxy, nextStage));
    return CHIP_NO_ERROR;
}

Optional<System::Clock::Timeout> AutoCommissioner::GetCommandTimeout(DeviceProxy * device, CommissioningStage stage) const
{
    // Network clusters can indicate the time required to connect, so if we are
    // connecting, use that time as our "how long it takes to process server
    // side" time.  Otherwise pick a time that should be enough for the command
    // processing: 7s for slow steps that can involve crypto, the default IM
    // timeout otherwise.
    // TODO: is this a reasonable estimate for the slow-crypto cases?
    constexpr System::Clock::Timeout kSlowCryptoProcessingTime = System::Clock::Seconds16(7);

    System::Clock::Timeout timeout;
    switch (stage)
    {
    case CommissioningStage::kWiFiNetworkEnable:
        ChipLogProgress(Controller, "Setting wifi connection time min = %u",
                        mDeviceCommissioningInfo.network.wifi.minConnectionTime);
        timeout = System::Clock::Seconds16(mDeviceCommissioningInfo.network.wifi.minConnectionTime);
        break;
    case CommissioningStage::kThreadNetworkEnable:
        timeout = System::Clock::Seconds16(mDeviceCommissioningInfo.network.thread.minConnectionTime);
        break;
    case CommissioningStage::kSendNOC:
    case CommissioningStage::kSendOpCertSigningRequest:
        timeout = kSlowCryptoProcessingTime;
        break;
    default:
        timeout = app::kExpectedIMProcessingTime;
        break;
    }

    // Adjust the timeout for our session transport latency, if we have access
    // to a session.
    auto sessionHandle = device->GetSecureSession();
    if (sessionHandle.HasValue())
    {
        timeout = sessionHandle.Value()->ComputeRoundTripTimeout(timeout);
    }

    // Enforce the spec minimal timeout.  Maybe this enforcement should live in
    // the DeviceCommissioner?
    if (timeout < kMinimumCommissioningStepTimeout)
    {
        timeout = kMinimumCommissioningStepTimeout;
    }

    return MakeOptional(timeout);
}

CHIP_ERROR AutoCommissioner::NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac, AesCcm128KeySpan ipk,
                                               NodeId adminSubject)
{
    // Reuse ICA Cert buffer for temporary store Root Cert.
    MutableByteSpan rootCert = MutableByteSpan(mICACertBuffer);
    ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(rcac, rootCert));
    mParams.SetRootCert(rootCert);

    MutableByteSpan noCert = MutableByteSpan(mNOCertBuffer);
    ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(noc, noCert));
    mParams.SetNoc(noCert);

    CommissioningStage nextStage = CommissioningStage::kSendTrustedRootCert;
    mCommissioner->PerformCommissioningStep(mCommissioneeDeviceProxy, nextStage, mParams, this, 0,
                                            GetCommandTimeout(mCommissioneeDeviceProxy, nextStage));

    // Trusted root cert has been sent, so we can re-use the icac buffer for the icac.
    if (!icac.empty())
    {
        MutableByteSpan icaCert = MutableByteSpan(mICACertBuffer);
        ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(icac, icaCert));
        mParams.SetIcac(icaCert);
    }
    else
    {
        mParams.SetIcac(ByteSpan());
    }

    mParams.SetIpk(ipk);
    mParams.SetAdminSubject(adminSubject);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AutoCommissioner::CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    CompletionStatus completionStatus;
    completionStatus.err = err;

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Successfully finished commissioning step '%s'", StageToString(report.stageCompleted));
    }
    else
    {
        ChipLogProgress(Controller, "Error on commissioning step '%s': '%s'", StageToString(report.stageCompleted), err.AsString());
    }

    if (err != CHIP_NO_ERROR)
    {
        completionStatus.failedStage = MakeOptional(report.stageCompleted);
        ChipLogError(Controller, "Failed to perform commissioning step %d", static_cast<int>(report.stageCompleted));
        if (report.Is<AttestationErrorInfo>())
        {
            completionStatus.attestationResult = MakeOptional(report.Get<AttestationErrorInfo>().attestationResult);
            if ((report.Get<AttestationErrorInfo>().attestationResult ==
                 Credentials::AttestationVerificationResult::kDacProductIdMismatch) ||
                (report.Get<AttestationErrorInfo>().attestationResult ==
                 Credentials::AttestationVerificationResult::kDacVendorIdMismatch))
            {
                ChipLogError(Controller,
                             "Failed device attestation. Device vendor and/or product ID do not match the IDs expected. "
                             "Verify DAC certificate chain and certification declaration to ensure spec rules followed.");
            }
        }
        else if (report.Is<CommissioningErrorInfo>())
        {
            completionStatus.commissioningError = MakeOptional(report.Get<CommissioningErrorInfo>().commissioningError);
        }
        else if (report.Is<NetworkCommissioningStatusInfo>())
        {
            completionStatus.networkCommissioningStatus =
                MakeOptional(report.Get<NetworkCommissioningStatusInfo>().networkCommissioningStatus);
        }
    }
    else
    {
        switch (report.stageCompleted)
        {
        case CommissioningStage::kReadCommissioningInfo:
            mDeviceCommissioningInfo = report.Get<ReadCommissioningInfo>();
            if (!mParams.GetFailsafeTimerSeconds().HasValue() && mDeviceCommissioningInfo.general.recommendedFailsafe > 0)
            {
                mParams.SetFailsafeTimerSeconds(mDeviceCommissioningInfo.general.recommendedFailsafe);
            }
            mParams.SetRemoteVendorId(mDeviceCommissioningInfo.basic.vendorId)
                .SetRemoteProductId(mDeviceCommissioningInfo.basic.productId)
                .SetDefaultRegulatoryLocation(mDeviceCommissioningInfo.general.currentRegulatoryLocation)
                .SetLocationCapability(mDeviceCommissioningInfo.general.locationCapability);
            if (mDeviceCommissioningInfo.nodeId != kUndefinedNodeId)
            {
                mParams.SetRemoteNodeId(mDeviceCommissioningInfo.nodeId);
            }
            break;
        case CommissioningStage::kSendPAICertificateRequest:
            SetPAI(report.Get<RequestedCertificate>().certificate);
            break;
        case CommissioningStage::kSendDACCertificateRequest:
            SetDAC(report.Get<RequestedCertificate>().certificate);
            break;
        case CommissioningStage::kSendAttestationRequest: {
            auto & elements  = report.Get<AttestationResponse>().attestationElements;
            auto & signature = report.Get<AttestationResponse>().signature;
            if (elements.size() > sizeof(mAttestationElements))
            {
                ChipLogError(Controller, "AutoCommissioner attestationElements buffer size %u larger than cache size %u",
                             static_cast<unsigned>(elements.size()), static_cast<unsigned>(sizeof(mAttestationElements)));
                return CHIP_ERROR_MESSAGE_TOO_LONG;
            }
            memcpy(mAttestationElements, elements.data(), elements.size());
            mAttestationElementsLen = static_cast<uint16_t>(elements.size());
            mParams.SetAttestationElements(ByteSpan(mAttestationElements, elements.size()));
            ChipLogDetail(Controller, "AutoCommissioner setting attestationElements buffer size %u/%u",
                          static_cast<unsigned>(elements.size()),
                          static_cast<unsigned>(mParams.GetAttestationElements().Value().size()));

            if (signature.size() > sizeof(mAttestationSignature))
            {
                ChipLogError(Controller,
                             "AutoCommissioner attestationSignature buffer size %u larger than "
                             "cache size %u",
                             static_cast<unsigned>(signature.size()), static_cast<unsigned>(sizeof(mAttestationSignature)));
                return CHIP_ERROR_MESSAGE_TOO_LONG;
            }
            memcpy(mAttestationSignature, signature.data(), signature.size());
            mAttestationSignatureLen = static_cast<uint16_t>(signature.size());
            mParams.SetAttestationSignature(ByteSpan(mAttestationSignature, signature.size()));

            // TODO: Does this need to be done at runtime? Seems like this could be done earlier and we wouldn't need to hold a
            // reference to the operational credential delegate here
            if (mOperationalCredentialsDelegate != nullptr)
            {
                MutableByteSpan nonce(mCSRNonce);
                ReturnErrorOnFailure(mOperationalCredentialsDelegate->ObtainCsrNonce(nonce));
                mParams.SetCSRNonce(ByteSpan(mCSRNonce, sizeof(mCSRNonce)));
            }
            break;
        }
        case CommissioningStage::kSendOpCertSigningRequest: {
            NOCChainGenerationParameters nocParams;
            nocParams.nocsrElements = report.Get<CSRResponse>().nocsrElements;
            nocParams.signature     = report.Get<CSRResponse>().signature;
            mParams.SetNOCChainGenerationParameters(nocParams);
        }
        break;
        case CommissioningStage::kGenerateNOCChain:
            // For NOC chain generation, we re-use the buffers. NOCChainGenerated triggers the next stage before
            // storing the returned certs, so just return here without triggering the next stage.
            return NOCChainGenerated(report.Get<NocChain>().noc, report.Get<NocChain>().icac, report.Get<NocChain>().rcac,
                                     report.Get<NocChain>().ipk, report.Get<NocChain>().adminSubject);
        case CommissioningStage::kFindOperational:
            mOperationalDeviceProxy = report.Get<OperationalNodeFoundData>().operationalProxy;
            break;
        case CommissioningStage::kCleanup:
            ReleasePAI();
            ReleaseDAC();
            mCommissioneeDeviceProxy = nullptr;
            mOperationalDeviceProxy  = OperationalDeviceProxy();
            mDeviceCommissioningInfo = ReadCommissioningInfo();
            return CHIP_NO_ERROR;
        default:
            break;
        }
    }

    CommissioningStage nextStage = GetNextCommissioningStage(report.stageCompleted, err);
    if (nextStage == CommissioningStage::kError)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // If GetNextCommissioningStage indicated a failure, don't lose track of
    // that.  But don't overwrite any existing failures we had hanging
    // around.
    if (completionStatus.err == CHIP_NO_ERROR)
    {
        completionStatus.err = err;
    }
    mParams.SetCompletionStatus(completionStatus);

    return PerformStep(nextStage);
}

DeviceProxy * AutoCommissioner::GetDeviceProxyForStep(CommissioningStage nextStage)
{
    if (nextStage == CommissioningStage::kSendComplete ||
        (nextStage == CommissioningStage::kCleanup && mOperationalDeviceProxy.GetDeviceId() != kUndefinedNodeId))
    {
        return &mOperationalDeviceProxy;
    }
    return mCommissioneeDeviceProxy;
}

CHIP_ERROR AutoCommissioner::PerformStep(CommissioningStage nextStage)
{
    DeviceProxy * proxy = GetDeviceProxyForStep(nextStage);
    if (proxy == nullptr)
    {
        ChipLogError(Controller, "Invalid device for commissioning");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mCommissioner->PerformCommissioningStep(proxy, nextStage, mParams, this, GetEndpoint(nextStage),
                                            GetCommandTimeout(proxy, nextStage));
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
