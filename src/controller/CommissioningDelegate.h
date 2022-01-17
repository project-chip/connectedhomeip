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
#include <app/OperationalDeviceProxy.h>
#include <controller/CommissioneeDeviceProxy.h>

namespace chip {
namespace Controller {

enum CommissioningStage : uint8_t
{
    kError,
    kSecurePairing,
    kArmFailsafe,
    // kConfigTime,  // NOT YET IMPLEMENTED
    // kConfigTimeZone,  // NOT YET IMPLEMENTED
    // kConfigDST,  // NOT YET IMPLEMENTED
    kConfigRegulatory,
    kDeviceAttestation,
    kCheckCertificates,
    kConfigACL,
    kWiFiNetworkSetup,
    kThreadNetworkSetup,
    kWiFiNetworkEnable,
    kThreadNetworkEnable,
    kFindOperational,
    kSendComplete,
    kCleanup,
};

struct WiFiCredentials
{
    ByteSpan ssid;
    // TODO(cecille): We should add a PII bytespan concept.
    ByteSpan credentials;
    WiFiCredentials(ByteSpan newSsid, ByteSpan newCreds) : ssid(newSsid), credentials(newCreds) {}
};
class CommissioningParameters
{
public:
    static constexpr size_t kMaxThreadDatasetLen = 254;
    static constexpr size_t kMaxSsidLen          = 32;
    static constexpr size_t kMaxCredentialsLen   = 64;
    bool HasCSRNonce() const { return mCSRNonce.HasValue(); }
    bool HasAttestationNonce() const { return mAttestationNonce.HasValue(); }
    bool HasWiFiCredentials() const { return mWiFiCreds.HasValue(); }
    bool HasThreadOperationalDataset() const { return mThreadOperationalDataset.HasValue(); }
    const Optional<ByteSpan> GetCSRNonce() const { return mCSRNonce; }
    const Optional<ByteSpan> GetAttestationNonce() const { return mAttestationNonce; }
    const Optional<WiFiCredentials> GetWiFiCredentials() const { return mWiFiCreds; }
    const Optional<ByteSpan> GetThreadOperationalDataset() const { return mThreadOperationalDataset; }

    // The lifetime of the buffer csrNonce is pointing to, should exceed the lifetime of CommissioningParameters object.
    CommissioningParameters & SetCSRNonce(ByteSpan csrNonce)
    {
        mCSRNonce.SetValue(csrNonce);
        return *this;
    }

    // The lifetime of the buffer attestationNonce is pointing to, should exceed the lifetime of CommissioningParameters object.
    CommissioningParameters & SetAttestationNonce(ByteSpan attestationNonce)
    {
        mAttestationNonce.SetValue(attestationNonce);
        return *this;
    }
    CommissioningParameters & SetWiFiCredentials(WiFiCredentials wifiCreds)
    {
        mWiFiCreds.SetValue(wifiCreds);
        return *this;
    }

    CommissioningParameters & SetThreadOperationalDataset(ByteSpan threadOperationalDataset)
    {

        mThreadOperationalDataset.SetValue(threadOperationalDataset);
        return *this;
    }
    void SetCompletionStatus(CHIP_ERROR err) { completionStatus = err; }
    CHIP_ERROR GetCompletionStatus() { return completionStatus; }

private:
    Optional<ByteSpan> mCSRNonce;         ///< CSR Nonce passed by the commissioner
    Optional<ByteSpan> mAttestationNonce; ///< Attestation Nonce passed by the commissioner
    Optional<WiFiCredentials> mWiFiCreds;
    Optional<ByteSpan> mThreadOperationalDataset;
    CHIP_ERROR completionStatus = CHIP_NO_ERROR;
};

class CommissioningDelegate
{
public:
    virtual ~CommissioningDelegate(){};
    struct CommissioningReport
    {
        CommissioningStage stageCompleted;
        // TODO: Add other things the delegate needs to know.
        union
        {
            struct
            {
                OperationalDeviceProxy * operationalProxy;
            } OperationalNodeFoundData;
        };
    };
    virtual void CommissioningStepFinished(CHIP_ERROR err, CommissioningReport report) = 0;
};

} // namespace Controller
} // namespace chip
