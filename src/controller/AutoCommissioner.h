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
#pragma once

#include <app/AttributePathParams.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/ScopedBuffer.h>
#include <protocols/secure_channel/RendezvousParameters.h>

namespace chip {

namespace Testing {

    class AutoCommissionerTestAccess;

} // namespace Testing

namespace Controller {

    class DeviceCommissioner;

    class AutoCommissioner : public CommissioningDelegate {

        friend class chip::Testing::AutoCommissionerTestAccess;

    public:
        AutoCommissioner();
        ~AutoCommissioner() override;
        CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params) override;
        const CommissioningParameters & GetCommissioningParameters() const override;
        void SetOperationalCredentialsDelegate(OperationalCredentialsDelegate * operationalCredentialsDelegate) override;

        CHIP_ERROR StartCommissioning(DeviceCommissioner * commissioner, CommissioneeDeviceProxy * proxy) override;
        void StopCommissioning() { mStopCommissioning = true; };

        CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report) override;

        ByteSpan GetAttestationElements() const { return ByteSpan(mAttestationElements, mAttestationElementsLen); }
        ByteSpan GetAttestationSignature() const { return ByteSpan(mAttestationSignature, mAttestationSignatureLen); }
        ByteSpan GetAttestationNonce() const { return ByteSpan(mAttestationNonce); }

    protected:
        virtual void CleanupCommissioning();
        CommissioningStage GetNextCommissioningStage(CommissioningStage currentStage, CHIP_ERROR & lastErr);
        DeviceCommissioner * GetCommissioner() { return mCommissioner; }
        CHIP_ERROR PerformStep(CommissioningStage nextStage);
        CommissioneeDeviceProxy * GetCommissioneeDeviceProxy() { return mCommissioneeDeviceProxy; }
        /**
         * The device argument to GetCommandTimeout is the device whose session will
         * be used for sending the relevant command.
         */
        Optional<System::Clock::Timeout> GetCommandTimeout(DeviceProxy * device, CommissioningStage stage) const;
        CommissioningParameters mParams = CommissioningParameters();

    private:
        DeviceProxy * GetDeviceProxyForStep(CommissioningStage nextStage);

        // Adjust the failsafe timer if CommissioningDelegate GetCASEFailsafeTimerSeconds is set
        void SetCASEFailsafeTimerIfNeeded();

        const ByteSpan GetDAC() { return mDAC.Span(); }
        const ByteSpan GetPAI() { return mPAI.Span(); }

        CHIP_ERROR NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac, Crypto::IdentityProtectionKeySpan ipk,
            NodeId adminSubject);
        EndpointId GetEndpoint(const CommissioningStage & stage) const;
        CommissioningStage GetNextCommissioningStageInternal(CommissioningStage currentStage, CHIP_ERROR & lastErr);

        CHIP_ERROR VerifyICDRegistrationInfo(const CommissioningParameters & params);

        // Helper function to determine whether next stage should be kWiFiNetworkSetup,
        // kThreadNetworkSetup, kRequestWiFiCredentials, kRequestThreadCredentials, or
        // kCleanup, depending on whether network information has been provided that matches
        // the thread/wifi endpoint of the target.
        CommissioningStage GetNextCommissioningStageNetworkSetup(CommissioningStage currentStage, CHIP_ERROR & lastErr);

        // Helper function to determine if a scan attempt should be made given the
        // scan attempt commissioning params and the corresponding network endpoint of
        // the target.
        bool IsScanNeeded()
        {
            return ((mParams.GetAttemptWiFiNetworkScan().ValueOr(false) && mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId) || (mParams.GetAttemptThreadNetworkScan().ValueOr(false) && mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId));
        };

        // Helper function to determine whether secondary network interface is supported.
        // Only true if the target has endpoints for both Wi-Fi and Thread, we can
        // still talk to it after the first attempt to put it on the network, and
        // either we have credentials for both network types or we have credentials
        // for either network type (in which case we will prompt for the credentials).
        bool IsSecondaryNetworkSupported() const
        {
            return ((mParams.GetSupportsConcurrentConnection().ValueOr(false) && mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId && mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId) && mParams.GetWiFiCredentials().HasValue() == mParams.GetThreadOperationalDataset().HasValue());
        }

        bool IsSomeNetworkSupported() const
        {
            return mDeviceCommissioningInfo.network.wifi.endpoint != kInvalidEndpointId || mDeviceCommissioningInfo.network.thread.endpoint != kInvalidEndpointId;
        }

        // TryingPrimaryNetwork() and TryingSecondaryNetwork() will only be true if
        // we decided that a secondary network is supported by our combination of
        // commissioner and commissionee.
        enum class NetworkAttemptType : uint8_t {
            // We will only try one network type.
            kSingle,
            // We will try two network types and we are trying the primary right now.
            kPrimary,
            // We tried the primary and if failed and we are trying the secondary
            // now.
            kSecondary,
        };

        void TryPrimaryNetwork() { mTryingNetworkType = NetworkAttemptType::kPrimary; }
        bool TryingPrimaryNetwork() const { return mTryingNetworkType == NetworkAttemptType::kPrimary; }
        void TrySecondaryNetwork() { mTryingNetworkType = NetworkAttemptType::kSecondary; }
        bool TryingSecondaryNetwork() const { return mTryingNetworkType == NetworkAttemptType::kSecondary; }
        void ResetNetworkAttemptType() { mTryingNetworkType = NetworkAttemptType::kSingle; }

        NetworkAttemptType mTryingNetworkType = NetworkAttemptType::kSingle;

        bool mStopCommissioning = false;

        DeviceCommissioner * mCommissioner = nullptr;
        CommissioneeDeviceProxy * mCommissioneeDeviceProxy = nullptr;
        OperationalCredentialsDelegate * mOperationalCredentialsDelegate = nullptr;
        OperationalDeviceProxy mOperationalDeviceProxy;

        // BEGIN memory space for commissioning parameters that are Spans or similar pointers:
        // The caller is not guaranteed to retain the memory these parameters point to.
        uint8_t mSsid[CommissioningParameters::kMaxSsidLen];
        uint8_t mCredentials[CommissioningParameters::kMaxCredentialsLen];
        uint8_t mThreadOperationalDataset[CommissioningParameters::kMaxThreadDatasetLen];
        char mCountryCode[CommissioningParameters::kMaxCountryCodeLen];

        // Time zone is statically allocated because it is max 2 and not trivially destructible
        static constexpr size_t kMaxSupportedTimeZones = 2;
        app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type mTimeZoneBuf[kMaxSupportedTimeZones];
        static constexpr size_t kMaxTimeZoneNameLen = 64;
        char mTimeZoneNames[kMaxTimeZoneNameLen][kMaxSupportedTimeZones];

        // DSTOffsetStructs are similarly not trivially destructible. They don't have a defined size, but we're
        // going to do static allocation of the buffers anyway until we replace chip::Optional with std::optional.
        static constexpr size_t kMaxSupportedDstStructs = 10;
        app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type mDstOffsetsBuf[kMaxSupportedDstStructs];

        static constexpr size_t kMaxDefaultNtpSize = 128;

        char mDefaultNtp[kMaxDefaultNtpSize];

        uint8_t mICDSymmetricKey[Crypto::kAES_CCM128_Key_Length];
        Platform::ScopedMemoryBufferWithSize<app::AttributePathParams> mExtraReadPaths;

        // END memory space for commisisoning parameters

        bool mNeedsNetworkSetup = false;
        ReadCommissioningInfo mDeviceCommissioningInfo;
        bool mNeedsDST = false;

        bool mNeedIcdRegistration = false;
        // TODO: Why were the nonces statically allocated, but the certs dynamically allocated?
        Platform::ScopedMemoryBufferWithSize<uint8_t> mDAC;
        Platform::ScopedMemoryBufferWithSize<uint8_t> mPAI;

        uint8_t mAttestationNonce[kAttestationNonceLength];
        uint8_t mCSRNonce[kCSRNonceLength];
        uint8_t mNOCertBuffer[Credentials::kMaxCHIPCertLength];
        uint8_t mICACertBuffer[Credentials::kMaxCHIPCertLength];

        uint16_t mAttestationElementsLen = 0;
        uint8_t mAttestationElements[Credentials::kMaxRspLen];
        uint16_t mAttestationSignatureLen = 0;
        uint8_t mAttestationSignature[Crypto::kMax_ECDSA_Signature_Length];
    };
} // namespace Controller
} // namespace chip
