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
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <protocols/secure_channel/RendezvousParameters.h>

namespace chip {
namespace Controller {

class DeviceCommissioner;

class AutoCommissioner : public CommissioningDelegate
{
public:
    AutoCommissioner() {}
    virtual ~AutoCommissioner();
    CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params) override;
    void SetOperationalCredentialsDelegate(OperationalCredentialsDelegate * operationalCredentialsDelegate) override;

    virtual CHIP_ERROR StartCommissioning(DeviceCommissioner * commissioner, CommissioneeDeviceProxy * proxy) override;

    virtual CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report) override;

private:
    CommissioningStage GetNextCommissioningStage(CommissioningStage currentStage, CHIP_ERROR & lastErr);
    void ReleaseDAC();
    void ReleasePAI();

    CHIP_ERROR SetDAC(const ByteSpan & dac);
    CHIP_ERROR SetPAI(const ByteSpan & pai);

    ByteSpan GetDAC() const { return ByteSpan(mDAC, mDACLen); }
    ByteSpan GetPAI() const { return ByteSpan(mPAI, mPAILen); }

    CHIP_ERROR NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac, AesCcm128KeySpan ipk, NodeId adminSubject);
    Optional<System::Clock::Timeout> GetCommandTimeout(CommissioningStage stage);
    EndpointId GetEndpoint(const CommissioningStage & stage);

    DeviceCommissioner * mCommissioner                               = nullptr;
    CommissioneeDeviceProxy * mCommissioneeDeviceProxy               = nullptr;
    OperationalDeviceProxy * mOperationalDeviceProxy                 = nullptr;
    OperationalCredentialsDelegate * mOperationalCredentialsDelegate = nullptr;
    CommissioningParameters mParams                                  = CommissioningParameters();
    // Memory space for the commisisoning parameters that come in as ByteSpans - the caller is not guaranteed to retain this memory
    uint8_t mSsid[CommissioningParameters::kMaxSsidLen];
    uint8_t mCredentials[CommissioningParameters::kMaxCredentialsLen];
    uint8_t mThreadOperationalDataset[CommissioningParameters::kMaxThreadDatasetLen];

    bool mNeedsNetworkSetup = false;
    ReadCommissioningInfo mDeviceCommissioningInfo;

    // TODO: Why were the nonces statically allocated, but the certs dynamically allocated?
    uint8_t * mDAC   = nullptr;
    uint16_t mDACLen = 0;
    uint8_t * mPAI   = nullptr;
    uint16_t mPAILen = 0;
    uint8_t mAttestationNonce[kAttestationNonceLength];
    uint8_t mCSRNonce[kCSRNonceLength];
    uint8_t mNOCertBuffer[Credentials::kMaxCHIPCertLength];
    uint8_t mICACertBuffer[Credentials::kMaxCHIPCertLength];
};
} // namespace Controller
} // namespace chip
