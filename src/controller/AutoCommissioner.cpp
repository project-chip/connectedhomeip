/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <cstring>
#include <type_traits>

namespace chip {
namespace Controller {

using namespace chip::app::Clusters;
using namespace chip::Crypto;
using chip::app::DataModel::MakeNullable;
using chip::app::DataModel::NullNullable;

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

CHIP_ERROR AutoCommissioner::VerifyICDRegistrationInfo(const CommissioningParameters & params)
{
    ChipLogProgress(Controller, "Checking ICD registration parameters");
    if (!params.GetICDSymmetricKey().HasValue())
    {
        ChipLogError(Controller, "Missing ICD symmetric key!");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (params.GetICDSymmetricKey().Value().size() != sizeof(mICDSymmetricKey))
    {
        ChipLogError(Controller, "Invalid ICD symmetric key length!");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!params.GetICDCheckInNodeId().HasValue())
    {
        ChipLogError(Controller, "Missing ICD check-in node id!");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!params.GetICDMonitoredSubject().HasValue())
    {
        ChipLogError(Controller, "Missing ICD monitored subject!");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!params.GetICDClientType().HasValue())
    {
        ChipLogError(Controller, "Missing ICD Client Type!");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    // Our logic below assumes that we can modify mParams without affecting params.
    VerifyOrReturnError(&params != &mParams, CHIP_NO_ERROR);

    // Copy the whole struct (scalars and pointers), but clear any members that might point to
    // external buffers. For those members we have to copy the data over into our own buffers below.
    // Note that all of the copy operations use memmove() instead of memcpy(), because the caller
    // may be passing a modified shallow copy of our CommissioningParmeters, i.e. where various spans
    // already point into the buffers we're copying into, and memcpy() with overlapping buffers is UB.
    mParams = params;
    mParams.ClearExternalBufferDependentValues();

    if (params.GetThreadOperationalDataset().HasValue())
    {
        ByteSpan dataset = params.GetThreadOperationalDataset().Value();
        if (dataset.size() > CommissioningParameters::kMaxThreadDatasetLen)
        {
            ChipLogError(Controller, "Thread operational data set is too large");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memmove(mThreadOperationalDataset, dataset.data(), dataset.size());
        ChipLogProgress(Controller, "Setting thread operational dataset from parameters");
        mParams.SetThreadOperationalDataset(ByteSpan(mThreadOperationalDataset, dataset.size()));
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
        memmove(mSsid, creds.ssid.data(), creds.ssid.size());
        memmove(mCredentials, creds.credentials.data(), creds.credentials.size());
        ChipLogProgress(Controller, "Setting wifi credentials from parameters");
        mParams.SetWiFiCredentials(
            WiFiCredentials(ByteSpan(mSsid, creds.ssid.size()), ByteSpan(mCredentials, creds.credentials.size())));
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
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    // If the AttestationNonce is passed in, using that else using a random one..
    if (params.GetAttestationNonce().HasValue())
    {
        ChipLogProgress(Controller, "Setting attestation nonce from parameters");
        VerifyOrReturnError(params.GetAttestationNonce().Value().size() == sizeof(mAttestationNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memmove(mAttestationNonce, params.GetAttestationNonce().Value().data(), params.GetAttestationNonce().Value().size());
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
        memmove(mCSRNonce, params.GetCSRNonce().Value().data(), params.GetCSRNonce().Value().size());
    }
    else
    {
        ChipLogProgress(Controller, "Setting CSR nonce to random value");
        Crypto::DRBG_get_bytes(mCSRNonce, sizeof(mCSRNonce));
    }
    mParams.SetCSRNonce(ByteSpan(mCSRNonce, sizeof(mCSRNonce)));

    if (params.GetDSTOffsets().HasValue())
    {
        ChipLogProgress(Controller, "Setting DST offsets from parameters");
        size_t size = std::min(params.GetDSTOffsets().Value().size(), kMaxSupportedDstStructs);
        for (size_t i = 0; i < size; ++i)
        {
            mDstOffsetsBuf[i] = params.GetDSTOffsets().Value()[i];
        }
        auto list = app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>(mDstOffsetsBuf, size);
        mParams.SetDSTOffsets(list);
    }
    if (params.GetTimeZone().HasValue())
    {
        ChipLogProgress(Controller, "Setting Time Zone from parameters");
        size_t size = std::min(params.GetTimeZone().Value().size(), kMaxSupportedTimeZones);
        for (size_t i = 0; i < size; ++i)
        {
            mTimeZoneBuf[i] = params.GetTimeZone().Value()[i];
            if (params.GetTimeZone().Value()[i].name.HasValue() &&
                params.GetTimeZone().Value()[i].name.Value().size() <= kMaxTimeZoneNameLen)
            {
                auto span = MutableCharSpan(mTimeZoneNames[i], kMaxTimeZoneNameLen);
                // The buffer backing "span" is statically allocated and is of size kMaxSupportedTimeZones, so this should never
                // fail.
                CopyCharSpanToMutableCharSpan(params.GetTimeZone().Value()[i].name.Value(), span);
                mTimeZoneBuf[i].name.SetValue(span);
            }
            else
            {
                mTimeZoneBuf[i].name.ClearValue();
            }
        }
        auto list = app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>(mTimeZoneBuf, size);
        mParams.SetTimeZone(list);
    }
    if (params.GetDefaultNTP().HasValue())
    {
        ChipLogProgress(Controller, "Setting Default NTP from parameters");
        // This parameter is an optional nullable, so we need to go two levels deep here.
        if (!params.GetDefaultNTP().Value().IsNull() && params.GetDefaultNTP().Value().Value().size() <= kMaxDefaultNtpSize)
        {
            // The buffer backing "span" is statically allocated and is of size kMaxDefaultNtpSize.
            auto span = MutableCharSpan(mDefaultNtp, kMaxDefaultNtpSize);
            CopyCharSpanToMutableCharSpan(params.GetDefaultNTP().Value().Value(), span);
            auto default_ntp = MakeNullable(CharSpan(mDefaultNtp, params.GetDefaultNTP().Value().Value().size()));
            mParams.SetDefaultNTP(default_ntp);
        }
    }

    if (params.GetICDRegistrationStrategy() != ICDRegistrationStrategy::kIgnore && params.GetICDSymmetricKey().HasValue())
    {
        ReturnErrorOnFailure(VerifyICDRegistrationInfo(params));

        // The values must be valid now.
        memmove(mICDSymmetricKey, params.GetICDSymmetricKey().Value().data(), params.GetICDSymmetricKey().Value().size());
        mParams.SetICDSymmetricKey(ByteSpan(mICDSymmetricKey));
        mParams.SetICDCheckInNodeId(params.GetICDCheckInNodeId().Value());
        mParams.SetICDMonitoredSubject(params.GetICDMonitoredSubject().Value());
        mParams.SetICDClientType(params.GetICDClientType().Value());
    }

    auto extraReadPaths = params.GetExtraReadPaths();
    if (extraReadPaths.size() > 0)
    {
        using ReadPath = std::remove_pointer_t<decltype(extraReadPaths.data())>;
        static_assert(std::is_trivially_copyable_v<ReadPath>, "can't use memmove / memcpy, not trivially copyable");

        if (mExtraReadPaths.AllocatedSize() == extraReadPaths.size())
        {
            memmove(mExtraReadPaths.Get(), extraReadPaths.data(), extraReadPaths.size() * sizeof(ReadPath));
        }
        else
        {
            // We can't reallocate mExtraReadPaths yet as this would free the old buffer,
            // and the caller might be passing a sub-span of the old paths.
            decltype(mExtraReadPaths) oldReadPaths(std::move(mExtraReadPaths));
            VerifyOrReturnError(mExtraReadPaths.Alloc(extraReadPaths.size()), CHIP_ERROR_NO_MEMORY);
            memcpy(mExtraReadPaths.Get(), extraReadPaths.data(), extraReadPaths.size() * sizeof(ReadPath));
        }

        mParams.SetExtraReadPaths(mExtraReadPaths.Span());
    }
    else
    {
        mExtraReadPaths.Free();
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

CommissioningStage AutoCommissioner::GetNextCommissioningStageNetworkSetup(CommissioningStage currentStage, CHIP_ERROR & lastErr)
{
    if (IsSecondaryNetworkSupported())
    {
        if (TryingSecondaryNetwork())
        {
            // Try secondary network interface.
            return mDeviceCommissioningInfo.network.wifi.endpoint == kRootEndpointId ? CommissioningStage::kThreadNetworkSetup
                                                                                     : CommissioningStage::kWiFiNetworkSetup;
        }
        // Try primary network interface
        return mDeviceCommissioningInfo.network.wifi.endpoint == kRootEndpointId ? CommissioningStage::kWiFiNetworkSetup
                                                                                 : CommissioningStage::kThreadNetworkSetup;
    }

    if (mParams.GetWiFiCredentials().HasValue() && mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId)
    {
        return CommissioningStage::kWiFiNetworkSetup;
    }
    if (mParams.GetThreadOperationalDataset().HasValue() && mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId)
    {
        return CommissioningStage::kThreadNetworkSetup;
    }

    ChipLogError(Controller, "Required network information not provided in commissioning parameters");
    ChipLogError(Controller, "Parameters supplied: wifi (%s) thread (%s)", mParams.GetWiFiCredentials().HasValue() ? "yes" : "no",
                 mParams.GetThreadOperationalDataset().HasValue() ? "yes" : "no");
    ChipLogError(Controller, "Device supports: wifi (%s) thread(%s)",
                 mDeviceCommissioningInfo.network.wifi.endpoint == kInvalidEndpointId ? "no" : "yes",
                 mDeviceCommissioningInfo.network.thread.endpoint == kInvalidEndpointId ? "no" : "yes");
    lastErr = CHIP_ERROR_INVALID_ARGUMENT;
    return CommissioningStage::kCleanup;
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
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kConfigRegulatory:
        return CommissioningStage::kConfigureTCAcknowledgments;
    case CommissioningStage::kConfigureTCAcknowledgments:
        if (mDeviceCommissioningInfo.requiresUTC)
        {
            return CommissioningStage::kConfigureUTCTime;
        }
        else
        {
            // Time cluster is not supported, move right to DA
            return CommissioningStage::kSendPAICertificateRequest;
        }
    case CommissioningStage::kConfigureUTCTime:
        if (mDeviceCommissioningInfo.requiresTimeZone && mParams.GetTimeZone().HasValue())
        {
            return kConfigureTimeZone;
        }
        else
        {
            return GetNextCommissioningStageInternal(CommissioningStage::kConfigureTimeZone, lastErr);
        }
    case CommissioningStage::kConfigureTimeZone:
        if (mNeedsDST && mParams.GetDSTOffsets().HasValue())
        {
            return CommissioningStage::kConfigureDSTOffset;
        }
        else
        {
            return GetNextCommissioningStageInternal(CommissioningStage::kConfigureDSTOffset, lastErr);
        }
    case CommissioningStage::kConfigureDSTOffset:
        if (mDeviceCommissioningInfo.requiresDefaultNTP && mParams.GetDefaultNTP().HasValue())
        {
            return CommissioningStage::kConfigureDefaultNTP;
        }
        else
        {
            return GetNextCommissioningStageInternal(CommissioningStage::kConfigureDefaultNTP, lastErr);
        }
    case CommissioningStage::kConfigureDefaultNTP:
        return CommissioningStage::kSendPAICertificateRequest;
    case CommissioningStage::kSendPAICertificateRequest:
        return CommissioningStage::kSendDACCertificateRequest;
    case CommissioningStage::kSendDACCertificateRequest:
        return CommissioningStage::kSendAttestationRequest;
    case CommissioningStage::kSendAttestationRequest:
        return CommissioningStage::kAttestationVerification;
    case CommissioningStage::kAttestationVerification:
        return CommissioningStage::kAttestationRevocationCheck;
    case CommissioningStage::kAttestationRevocationCheck:
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
        if (mDeviceCommissioningInfo.requiresTrustedTimeSource && mParams.GetTrustedTimeSource().HasValue())
        {
            return CommissioningStage::kConfigureTrustedTimeSource;
        }
        else
        {
            return GetNextCommissioningStageInternal(CommissioningStage::kConfigureTrustedTimeSource, lastErr);
        }
    case CommissioningStage::kConfigureTrustedTimeSource:
        if (mNeedIcdRegistration)
        {
            if (mParams.GetICDCheckInNodeId().HasValue() && mParams.GetICDMonitoredSubject().HasValue() &&
                mParams.GetICDSymmetricKey().HasValue())
            {
                return CommissioningStage::kICDRegistration;
            }
            return CommissioningStage::kICDGetRegistrationInfo;
        }
        return GetNextCommissioningStageInternal(CommissioningStage::kICDRegistration, lastErr);
    case CommissioningStage::kICDGetRegistrationInfo:
        return CommissioningStage::kICDRegistration;
    case CommissioningStage::kICDRegistration:
        // TODO(cecille): device attestation casues operational cert provisioning to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
        if (mNeedsNetworkSetup)
        {
            // if there is a WiFi or a Thread endpoint, then perform scan
            if (IsScanNeeded())
            {
                // Perform Scan (kScanNetworks) and collect credentials (kNeedsNetworkCreds) right before configuring network.
                // This order of steps allows the workflow to return to collect credentials again if network enablement fails.
                return CommissioningStage::kScanNetworks;
            }
            ChipLogProgress(Controller, "No NetworkScan enabled or WiFi/Thread endpoint not specified, skipping ScanNetworks");

            return GetNextCommissioningStageNetworkSetup(currentStage, lastErr);
        }
        else
        {
            SetCASEFailsafeTimerIfNeeded();
            if (mParams.GetSkipCommissioningComplete().ValueOr(false))
            {
                return CommissioningStage::kCleanup;
            }
            return CommissioningStage::kEvictPreviousCaseSessions;
        }
    case CommissioningStage::kScanNetworks:
        return CommissioningStage::kNeedsNetworkCreds;
    case CommissioningStage::kNeedsNetworkCreds:
        return GetNextCommissioningStageNetworkSetup(currentStage, lastErr);
    case CommissioningStage::kWiFiNetworkSetup:
        return CommissioningStage::kFailsafeBeforeWiFiEnable;
    case CommissioningStage::kThreadNetworkSetup:
        return CommissioningStage::kFailsafeBeforeThreadEnable;
    case CommissioningStage::kFailsafeBeforeWiFiEnable:
        return CommissioningStage::kWiFiNetworkEnable;
    case CommissioningStage::kFailsafeBeforeThreadEnable:
        return CommissioningStage::kThreadNetworkEnable;
    case CommissioningStage::kWiFiNetworkEnable:
        if (mParams.GetSkipCommissioningComplete().ValueOr(false))
        {
            SetCASEFailsafeTimerIfNeeded();
            return CommissioningStage::kCleanup;
        }
        else
        {
            SetCASEFailsafeTimerIfNeeded();
            return CommissioningStage::kEvictPreviousCaseSessions;
        }
    case CommissioningStage::kThreadNetworkEnable:
        SetCASEFailsafeTimerIfNeeded();
        if (mParams.GetSkipCommissioningComplete().ValueOr(false))
        {
            return CommissioningStage::kCleanup;
        }
        return CommissioningStage::kEvictPreviousCaseSessions;
    case CommissioningStage::kEvictPreviousCaseSessions:
        return CommissioningStage::kFindOperationalForStayActive;
    case CommissioningStage::kPrimaryOperationalNetworkFailed:
        if (mDeviceCommissioningInfo.network.wifi.endpoint == kRootEndpointId)
        {
            return CommissioningStage::kRemoveWiFiNetworkConfig;
        }
        else
        {
            return CommissioningStage::kRemoveThreadNetworkConfig;
        }
    case CommissioningStage::kRemoveWiFiNetworkConfig:
    case CommissioningStage::kRemoveThreadNetworkConfig:
        return GetNextCommissioningStageNetworkSetup(currentStage, lastErr);
    case CommissioningStage::kFindOperationalForStayActive:
        return CommissioningStage::kICDSendStayActive;
    case CommissioningStage::kICDSendStayActive:
        return CommissioningStage::kFindOperationalForCommissioningComplete;
    case CommissioningStage::kFindOperationalForCommissioningComplete:
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

// No specific actions to take when an error happens since this command can fail and commissioning can still succeed.
static void OnFailsafeFailureForCASE(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "ExtendFailsafe received failure response %s\n", chip::ErrorStr(error));
}

// No specific actions to take upon success.
static void
OnExtendFailsafeSuccessForCASE(void * context,
                               const app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    ChipLogProgress(Controller, "ExtendFailsafe received ArmFailSafe response errorCode=%u", to_underlying(data.errorCode));
}

void AutoCommissioner::SetCASEFailsafeTimerIfNeeded()
{
    // if there is a final fail-safe timer configured then, send it
    if (mParams.GetCASEFailsafeTimerSeconds().HasValue() && mCommissioneeDeviceProxy != nullptr)
    {
        // send the command via the PASE session (mCommissioneeDeviceProxy) since the CASE portion of commissioning
        // might be done by a different service (ex. PASE is done by a phone app and CASE is done by a Hub).
        // Also, we want the CASE failsafe timer to apply for the time it takes the Hub to perform operational discovery,
        // CASE establishment, and receipt of the commissioning complete command.
        // We know that the mCommissioneeDeviceProxy is still valid at this point since it gets cleared during cleanup
        // and SetCASEFailsafeTimerIfNeeded is always called before that stage.
        //
        // A false return from ExtendArmFailSafe is fine; we don't want to make
        // the fail-safe shorter here.
        mCommissioner->ExtendArmFailSafe(mCommissioneeDeviceProxy, mCommissioner->GetCommissioningStage(),
                                         mParams.GetCASEFailsafeTimerSeconds().Value(),
                                         GetCommandTimeout(mCommissioneeDeviceProxy, CommissioningStage::kArmFailsafe),
                                         OnExtendFailsafeSuccessForCASE, OnFailsafeFailureForCASE);
    }
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
    case CommissioningStage::kRemoveWiFiNetworkConfig:
    case CommissioningStage::kRemoveThreadNetworkConfig:
        return kRootEndpointId;
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

    auto transportType =
        mCommissioneeDeviceProxy->GetSecureSession().Value()->AsSecureSession()->GetPeerAddress().GetTransportType();
    mNeedsNetworkSetup = (transportType == Transport::Type::kBle);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    mNeedsNetworkSetup = mNeedsNetworkSetup || (transportType == Transport::Type::kWiFiPAF);
#endif
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

CHIP_ERROR AutoCommissioner::NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac, IdentityProtectionKeySpan ipk,
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
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error on commissioning step '%s': '%s'", StageToString(report.stageCompleted), err.AsString());
        completionStatus.failedStage = MakeOptional(report.stageCompleted);
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

            if (report.stageCompleted == CommissioningStage::kAttestationVerification)
            {
                ChipLogError(Controller, "Failed verifying attestation information. Now checking DAC chain revoked status.");
                // don't error out until we check for DAC chain revocation status
                err = CHIP_NO_ERROR;
            }
        }
        else if (report.Is<CommissioningErrorInfo>())
        {
            completionStatus.commissioningError = MakeOptional(report.Get<CommissioningErrorInfo>().commissioningError);
        }
        else if (report.Is<NetworkCommissioningStatusInfo>())
        {
            // This report type is used when an error happens in either NetworkConfig or ConnectNetwork commands
            completionStatus.networkCommissioningStatus =
                MakeOptional(report.Get<NetworkCommissioningStatusInfo>().networkCommissioningStatus);

            // If we are configured to scan networks, then don't error out.
            // Instead, allow the app to try another network.
            if (IsScanNeeded())
            {
                if (completionStatus.err == CHIP_NO_ERROR)
                {
                    completionStatus.err = err;
                }
                err = CHIP_NO_ERROR;
                // Walk back the completed stage to kScanNetworks.
                // This will allow the app to try another network.
                report.stageCompleted = CommissioningStage::kScanNetworks;
            }
        }

        if (err != CHIP_NO_ERROR && IsSecondaryNetworkSupported() && !TryingSecondaryNetwork() &&
            completionStatus.failedStage.HasValue() && completionStatus.failedStage.Value() >= kWiFiNetworkSetup &&
            completionStatus.failedStage.Value() <= kICDSendStayActive)
        {
            // Primary network failed, disable primary network interface and try secondary network interface.
            TrySecondaryNetwork();
            err                   = CHIP_NO_ERROR;
            report.stageCompleted = CommissioningStage::kPrimaryOperationalNetworkFailed;
        }
    }
    else
    {
        ChipLogProgress(Controller, "Successfully finished commissioning step '%s'", StageToString(report.stageCompleted));
        switch (report.stageCompleted)
        {
        case CommissioningStage::kReadCommissioningInfo: {
            mDeviceCommissioningInfo = report.Get<ReadCommissioningInfo>();

            if (!mParams.GetFailsafeTimerSeconds().HasValue() && mDeviceCommissioningInfo.general.recommendedFailsafe > 0)
            {
                mParams.SetFailsafeTimerSeconds(mDeviceCommissioningInfo.general.recommendedFailsafe);
            }
            mParams.SetRemoteVendorId(mDeviceCommissioningInfo.basic.vendorId)
                .SetRemoteProductId(mDeviceCommissioningInfo.basic.productId)
                .SetDefaultRegulatoryLocation(mDeviceCommissioningInfo.general.currentRegulatoryLocation)
                .SetLocationCapability(mDeviceCommissioningInfo.general.locationCapability);
            // Don't send DST unless the device says it needs it
            mNeedsDST = false;

            mParams.SetSupportsConcurrentConnection(mDeviceCommissioningInfo.supportsConcurrentConnection);

            if (mParams.GetCheckForMatchingFabric())
            {
                NodeId nodeId = mDeviceCommissioningInfo.remoteNodeId;
                if (nodeId != kUndefinedNodeId)
                {
                    mParams.SetRemoteNodeId(nodeId);
                }
            }

            mNeedIcdRegistration = false;
            if (mParams.GetICDRegistrationStrategy() != ICDRegistrationStrategy::kIgnore)
            {
                if (mDeviceCommissioningInfo.icd.isLIT && mDeviceCommissioningInfo.icd.checkInProtocolSupport)
                {
                    mNeedIcdRegistration = true;
                    ChipLogDetail(Controller, "AutoCommissioner: ICD supports the check-in protocol.");
                }
                else if (mParams.GetICDStayActiveDurationMsec().HasValue())
                {
                    ChipLogDetail(Controller, "AutoCommissioner: Clear ICD StayActiveDurationMsec");
                    mParams.ClearICDStayActiveDurationMsec();
                }
            }
            break;
        }
        case CommissioningStage::kConfigureTimeZone:
            mNeedsDST = report.Get<TimeZoneResponseInfo>().requiresDSTOffsets;
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
        case CommissioningStage::kICDGetRegistrationInfo:
            // Noting to do. The ICD registation info is handled elsewhere.
            break;
        case CommissioningStage::kICDRegistration:
            // Noting to do. DevicePairingDelegate will handle this.
            break;
        case CommissioningStage::kFindOperationalForStayActive:
        case CommissioningStage::kFindOperationalForCommissioningComplete:
            mOperationalDeviceProxy = report.Get<OperationalNodeFoundData>().operationalProxy;
            break;
        case CommissioningStage::kCleanup:
            if (IsSecondaryNetworkSupported() && TryingSecondaryNetwork())
            {
                ResetTryingSecondaryNetwork();
            }
            ReleasePAI();
            ReleaseDAC();
            mCommissioneeDeviceProxy = nullptr;
            mOperationalDeviceProxy  = OperationalDeviceProxy();
            mDeviceCommissioningInfo = ReadCommissioningInfo();
            mNeedsDST                = false;
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
    if (nextStage == CommissioningStage::kSendComplete || nextStage == CommissioningStage::kICDSendStayActive ||
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
    // Perform any last minute parameter adjustments before calling the commissioner object
    switch (nextStage)
    {
    case CommissioningStage::kConfigureTimeZone:
        if (mParams.GetTimeZone().Value().size() > mDeviceCommissioningInfo.maxTimeZoneSize)
        {
            mParams.SetTimeZone(app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>(
                mParams.GetTimeZone().Value().SubSpan(0, mDeviceCommissioningInfo.maxTimeZoneSize)));
        }
        break;
    case CommissioningStage::kConfigureDSTOffset:
        if (mParams.GetDSTOffsets().Value().size() > mDeviceCommissioningInfo.maxDSTSize)
        {
            mParams.SetDSTOffsets(app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>(
                mParams.GetDSTOffsets().Value().SubSpan(0, mDeviceCommissioningInfo.maxDSTSize)));
        }
        break;
    default:
        break;
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
