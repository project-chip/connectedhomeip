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
#include <lib/support/Variant.h>

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
    kSendPAICertificateRequest,
    kSendDACCertificateRequest,
    kSendAttestationRequest,
    kAttestationVerification,
    kSendOpCertSigningRequest,
    kGenerateNOCChain,
    kSendTrustedRootCert,
    kSendNOC,
    kWiFiNetworkSetup,
    kThreadNetworkSetup,
    kWiFiNetworkEnable,
    kThreadNetworkEnable,
    kFindOperational,
    kSendComplete,
    kCleanup,
    kConfigACL,
};

struct WiFiCredentials
{
    ByteSpan ssid;
    // TODO(cecille): We should add a PII bytespan concept.
    ByteSpan credentials;
    WiFiCredentials(ByteSpan newSsid, ByteSpan newCreds) : ssid(newSsid), credentials(newCreds) {}
};

struct NOCChainGenerationParameters
{
    ByteSpan nocsrElements;
    ByteSpan signature;
};
struct NOCerts
{
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
    bool HasNOCChainGenerationaParameters() const { return mNOCChainGenerationParameters.HasValue(); }
    bool HasRootCert() const { return mRootCert.HasValue(); }
    bool HasNoc() const { return mNoc.HasValue(); }
    bool HasIcac() const { return mIcac.HasValue(); }
    bool HasAttestationElements() const { return mAttestationElements.HasValue(); }
    bool HasAttestationSignature() const { return mAttestationSignature.HasValue(); }
    bool HasPAI() const { return mPAI.HasValue(); }
    bool HasDAC() const { return mDAC.HasValue(); }
    uint16_t GetFailsafeTimerSeconds() const { return mFailsafeTimerSeconds; }
    const Optional<ByteSpan> GetCSRNonce() const { return mCSRNonce; }
    const Optional<ByteSpan> GetAttestationNonce() const { return mAttestationNonce; }
    const Optional<WiFiCredentials> GetWiFiCredentials() const { return mWiFiCreds; }
    const Optional<ByteSpan> GetThreadOperationalDataset() const { return mThreadOperationalDataset; }
    const Optional<NOCChainGenerationParameters> GetNOCChainGenerationParameters() const { return mNOCChainGenerationParameters; }
    const Optional<ByteSpan> GetRootCert() const { return mRootCert; }
    const Optional<ByteSpan> GetNoc() const { return mNoc; }
    const Optional<ByteSpan> GetIcac() const { return mIcac; }
    const Optional<ByteSpan> GetAttestationElements() const { return mAttestationElements; }
    const Optional<ByteSpan> GetAttestationSignature() const { return mAttestationSignature; }
    const Optional<ByteSpan> GetPAI() const { return mPAI; }
    const Optional<ByteSpan> GetDAC() const { return mDAC; }
    CHIP_ERROR GetCompletionStatus() { return completionStatus; }

    CommissioningParameters & SetFailsafeTimerSeconds(uint16_t seconds)
    {
        mFailsafeTimerSeconds = seconds;
        return *this;
    }

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
    // This parameter should be set with the information returned from kSendOpCertSigningRequest. It must be set before calling
    // kGenerateNOCChain.
    CommissioningParameters & SetNOCChainGenerationParameters(const NOCChainGenerationParameters & params)
    {
        mNOCChainGenerationParameters.SetValue(params);
        return *this;
    }
    // Root certs can be generated from the kGenerateNOCChain step. This must be set before calling kSendTrustedRootCert.
    CommissioningParameters & SetRootCert(const ByteSpan & rcac)
    {
        mRootCert.SetValue(rcac);
        return *this;
    }
    // NOC and intermediate cert can be generated from the kGenerateNOCChain step. NOC must be set before calling
    // kSendTrustedRootCert. ICAC and NOC must be set before calling kSendNOC
    CommissioningParameters & SetNoc(const ByteSpan & noc)
    {
        mNoc.SetValue(noc);
        return *this;
    }
    CommissioningParameters & SetIcac(const ByteSpan & icac)
    {
        mIcac.SetValue(icac);
        return *this;
    }
    CommissioningParameters & SetAttestationElements(const ByteSpan & attestationElements)
    {
        mAttestationElements = MakeOptional(attestationElements);
        return *this;
    }
    CommissioningParameters & SetAttestationSignature(const ByteSpan & attestationSignature)
    {
        mAttestationSignature = MakeOptional(attestationSignature);
        return *this;
    }
    CommissioningParameters & SetPAI(const ByteSpan & pai)
    {
        mPAI = MakeOptional(pai);
        return *this;
    }
    CommissioningParameters & SetDAC(const ByteSpan & dac)
    {
        mDAC = MakeOptional(dac);
        return *this;
    }
    void SetCompletionStatus(CHIP_ERROR err) { completionStatus = err; }

private:
    uint16_t mFailsafeTimerSeconds = 60;
    Optional<ByteSpan> mCSRNonce;         ///< CSR Nonce passed by the commissioner
    Optional<ByteSpan> mAttestationNonce; ///< Attestation Nonce passed by the commissioner
    Optional<WiFiCredentials> mWiFiCreds;
    Optional<ByteSpan> mThreadOperationalDataset;
    Optional<NOCChainGenerationParameters> mNOCChainGenerationParameters;
    Optional<ByteSpan> mRootCert;
    Optional<ByteSpan> mNoc;
    Optional<ByteSpan> mIcac;
    Optional<ByteSpan> mAttestationElements;
    Optional<ByteSpan> mAttestationSignature;
    Optional<ByteSpan> mPAI;
    Optional<ByteSpan> mDAC;
    CHIP_ERROR completionStatus = CHIP_NO_ERROR;
};

struct RequestedCertificate
{
    RequestedCertificate(ByteSpan newCertificate) : certificate(newCertificate) {}
    ByteSpan certificate;
};

struct AttestationResponse
{
    AttestationResponse(ByteSpan newAttestationElements, ByteSpan newSignature) :
        attestationElements(newAttestationElements), signature(newSignature)
    {}
    ByteSpan attestationElements;
    ByteSpan signature;
};

struct NocChain
{
    NocChain(ByteSpan newNoc, ByteSpan newIcac, ByteSpan newRcac) : noc(newNoc), icac(newIcac), rcac(newRcac) {}
    ByteSpan noc;
    ByteSpan icac;
    ByteSpan rcac;
};

struct OperationalNodeFoundData
{
    OperationalNodeFoundData(OperationalDeviceProxy * proxy) : operationalProxy(proxy) {}
    OperationalDeviceProxy * operationalProxy;
};
class CommissioningDelegate
{
public:
    virtual ~CommissioningDelegate(){};

    struct CommissioningReport : Variant<RequestedCertificate, AttestationResponse, NocChain, OperationalNodeFoundData>
    {
        CommissioningReport(CommissioningStage stage) : stageCompleted(stage) {}
        CommissioningStage stageCompleted;
        // TODO: Add other things the delegate needs to know.
    };
    virtual CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err, CommissioningReport report) = 0;
};

} // namespace Controller
} // namespace chip
