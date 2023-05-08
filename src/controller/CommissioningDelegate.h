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
#include <app/OperationalSessionSetup.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/attestation_verifier/DeviceAttestationDelegate.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/Variant.h>
#include <system/SystemClock.h>

namespace chip {
namespace Controller {

class DeviceCommissioner;

enum CommissioningStage : uint8_t
{
    kError,
    kSecurePairing,              ///< Establish a PASE session with the device
    kReadCommissioningInfo,      ///< Query General Commissioning Attributes and Network Features
    kArmFailsafe,                ///< Send ArmFailSafe (0x30:0) command to the device
    kConfigRegulatory,           ///< Send SetRegulatoryConfig (0x30:2) command to the device
    kSendPAICertificateRequest,  ///< Send PAI CertificateChainRequest (0x3E:2) command to the device
    kSendDACCertificateRequest,  ///< Send DAC CertificateChainRequest (0x3E:2) command to the device
    kSendAttestationRequest,     ///< Send AttestationRequest (0x3E:0) command to the device
    kAttestationVerification,    ///< Verify AttestationResponse (0x3E:1) validity
    kSendOpCertSigningRequest,   ///< Send CSRRequest (0x3E:4) command to the device
    kValidateCSR,                ///< Verify CSRResponse (0x3E:5) validity
    kGenerateNOCChain,           ///< TLV encode Node Operational Credentials (NOC) chain certs
    kSendTrustedRootCert,        ///< Send AddTrustedRootCertificate (0x3E:11) command to the device
    kSendNOC,                    ///< Send AddNOC (0x3E:6) command to the device
    kWiFiNetworkSetup,           ///< Send AddOrUpdateWiFiNetwork (0x31:2) command to the device
    kThreadNetworkSetup,         ///< Send AddOrUpdateThreadNetwork (0x31:3) command to the device
    kFailsafeBeforeWiFiEnable,   ///< Extend the fail-safe before doing kWiFiNetworkEnable
    kFailsafeBeforeThreadEnable, ///< Extend the fail-safe before doing kThreadNetworkEnable
    kWiFiNetworkEnable,          ///< Send ConnectNetwork (0x31:6) command to the device for the WiFi network
    kThreadNetworkEnable,        ///< Send ConnectNetwork (0x31:6) command to the device for the Thread network
    kFindOperational,            ///< Perform operational discovery and establish a CASE session with the device
    kSendComplete,               ///< Send CommissioningComplete (0x30:4) command to the device
    kCleanup,                    ///< Call delegates with status, free memory, clear timers and state
    /// Send ScanNetworks (0x31:0) command to the device.
    /// ScanNetworks can happen anytime after kArmFailsafe.
    /// However, the cirque tests fail if it is earlier in the list
    kScanNetworks,
    /// Waiting for the higher layer to provide network credentials before continuing the workflow.
    /// Call CHIPDeviceController::NetworkCredentialsReady() when CommissioningParameters is populated with
    /// network credentials to use in kWiFiNetworkSetup or kThreadNetworkSetup steps.
    kNeedsNetworkCreds,
};

const char * StageToString(CommissioningStage stage);

struct WiFiCredentials
{
    ByteSpan ssid;
    ByteSpan credentials;
    WiFiCredentials(ByteSpan newSsid, ByteSpan newCreds) : ssid(newSsid), credentials(newCreds) {}
};

struct NOCChainGenerationParameters
{
    ByteSpan nocsrElements;
    ByteSpan signature;
};

struct CompletionStatus
{
    CompletionStatus() : err(CHIP_NO_ERROR), failedStage(NullOptional), attestationResult(NullOptional) {}
    CHIP_ERROR err;
    Optional<CommissioningStage> failedStage;
    Optional<Credentials::AttestationVerificationResult> attestationResult;
    Optional<app::Clusters::GeneralCommissioning::CommissioningError> commissioningError;
    Optional<app::Clusters::NetworkCommissioning::NetworkCommissioningStatus> networkCommissioningStatus;
};

constexpr uint16_t kDefaultFailsafeTimeout = 60;

// Per spec, all commands that are sent with the failsafe armed need at least
// a 30s timeout.
constexpr System::Clock::Timeout kMinimumCommissioningStepTimeout = System::Clock::Seconds16(30);

class CommissioningParameters
{
public:
    static constexpr size_t kMaxThreadDatasetLen = 254;
    static constexpr size_t kMaxSsidLen          = 32;
    static constexpr size_t kMaxCredentialsLen   = 64;
    static constexpr size_t kMaxCountryCodeLen   = 2;

    // Value to use when setting the commissioning failsafe timer on the node being commissioned.
    // If the failsafe timer value is passed in as part of the commissioning parameters, that value will be used. If not supplied,
    // the AutoCommissioner will set this to the recommended value read from the node. If that is not set, it will fall back to the
    // default kDefaultFailsafeTimeout.
    // This value should be set before running PerformCommissioningStep for the kArmFailsafe step.
    const Optional<uint16_t> GetFailsafeTimerSeconds() const { return mFailsafeTimerSeconds; }

    // Value to use when re-setting the commissioning failsafe timer immediately prior to operational discovery.
    // If a CASE failsafe timer value is passed in as part of the commissioning parameters, then the failsafe timer
    // will be reset using this value before operational discovery begins. If not supplied, then the AutoCommissioner
    // will not automatically reset the failsafe timer before operational discovery begins. It can be useful for the
    // commissioner to set the CASE failsafe timer to a small value (ex. 30s) when the regular failsafe timer is set
    // to a larger value to accommodate user interaction during setup (network credential selection, user consent
    // after device attestation).
    const Optional<uint16_t> GetCASEFailsafeTimerSeconds() const { return mCASEFailsafeTimerSeconds; }

    // The location (indoor/outdoor) of the node being commissioned.
    // The node regulartory location (indoor/outdoor) should be set by the commissioner explicitly as it may be different than the
    // location of the commissioner. This location will be set on the node if the node supports configurable regulatory location
    // (from GetLocationCapability - see below). If the regulatory location is not supplied, this will fall back to the location in
    // GetDefaultRegulatoryLocation and then to Outdoor (most restrictive).
    // This value should be set before calling PerformCommissioningStep for the kConfigRegulatory step.
    const Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> GetDeviceRegulatoryLocation() const
    {
        return mDeviceRegulatoryLocation;
    }

    // The country code to be used for the node, if set.
    Optional<CharSpan> GetCountryCode() const { return mCountryCode; }

    // Nonce sent to the node to use during the CSR request.
    // When using the AutoCommissioner, this value will be ignored in favour of the value supplied by the
    // OperationalCredentialsDelegate ObtainCsrNonce function. If the credential delegate is not supplied, the value supplied here
    // will be used. If neither is supplied random value will be used as a fallback.
    // This value must be set before calling PerformCommissioningStep for the kSendOpCertSigningRequest step.
    const Optional<ByteSpan> GetCSRNonce() const { return mCSRNonce; }

    // Nonce value sent to the node to use during the attestation request.
    // When using the AutoCommissioner, this value will fall back to random if not supplied.
    // If a non-random value is to be used, the value must be set before calling PerformCommissioningStep for the
    // kSendAttestationRequest step.
    const Optional<ByteSpan> GetAttestationNonce() const { return mAttestationNonce; }

    // WiFi SSID and credentials to use when adding/updating and enabling WiFi on the node.
    // This value must be set before calling PerformCommissioningStep for the kWiFiNetworkSetup or kWiFiNetworkEnable steps.
    const Optional<WiFiCredentials> GetWiFiCredentials() const { return mWiFiCreds; }

    // Thread operational dataset to use when adding/updating and enabling the thread network on the node.
    // This value must be set before calling PerformCommissioningStep for the kThreadNetworkSetup or kThreadNetworkEnable steps.
    const Optional<ByteSpan> GetThreadOperationalDataset() const { return mThreadOperationalDataset; }

    // The NOCSR parameters (elements and signature) returned from the node. In the AutoCommissioner, this is set using the data
    // returned from the kSendOpCertSigningRequest stage.
    // This value must be set before calling PerformCommissioningStep for the kGenerateNOCChain step.
    const Optional<NOCChainGenerationParameters> GetNOCChainGenerationParameters() const { return mNOCChainGenerationParameters; }

    // The root certificate for the operational certificate chain. In the auto commissioner, this is set by by the kGenerateNOCChain
    // stage through the OperationalCredentialsDelegate.
    // This value must be set before calling PerformCommissioningStep for the kSendTrustedRootCert step.
    const Optional<ByteSpan> GetRootCert() const { return mRootCert; }

    // The node operational certificate for the node being commissioned. In the AutoCommissioner, this is set by by the
    // kGenerateNOCChain stage through the OperationalCredentialsDelegate.
    // This value must be set before calling PerformCommissioningStep for the kSendNOC step.
    // This value must also be set before calling PerformCommissioningStep for the kSendTrustedRootCert step, as it is used to set
    // the node id in the DeviceProxy.
    const Optional<ByteSpan> GetNoc() const { return mNoc; }

    // The intermediate certificate for the node being commissioned. In the AutoCommissioner, this is set by by the
    // kGenerateNOCChain stage through the OperationalCredentialsDelegate.
    // This value should be set before calling PerformCommissioningStep for the kSendNOC step.
    const Optional<ByteSpan> GetIcac() const { return mIcac; }

    // Epoch key for the identity protection key for the node being commissioned. In the AutoCommissioner, this is set by by the
    // kGenerateNOCChain stage through the OperationalCredentialsDelegate.
    // This value must be set before calling PerformCommissioningStep for the kSendNOC step.
    const Optional<IdentityProtectionKeySpan> GetIpk() const
    {
        return mIpk.HasValue() ? Optional<IdentityProtectionKeySpan>(mIpk.Value().Span()) : Optional<IdentityProtectionKeySpan>();
    }

    // Admin subject id used for the case access control entry created if the AddNOC command succeeds. In the AutoCommissioner, this
    // is set by by the kGenerateNOCChain stage through the OperationalCredentialsDelegate.
    // This must be set before calling PerformCommissioningStep for the kSendNOC step.
    const Optional<NodeId> GetAdminSubject() const { return mAdminSubject; }

    // Attestation elements from the node. These are obtained from node in response to the AttestationRequest command. In the
    // AutoCommissioner, this is automatically set from the report from the kSendAttestationRequest stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerification step.
    const Optional<ByteSpan> GetAttestationElements() const { return mAttestationElements; }

    // Attestation signature from the node. This is obtained from node in response to the AttestationRequest command. In the
    // AutoCommissioner, this is automatically set from the report from the kSendAttestationRequest stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerification step.
    const Optional<ByteSpan> GetAttestationSignature() const { return mAttestationSignature; }

    // Product attestation intermediate certificate from the node. This is obtained from the node in response to the
    // CertificateChainRequest command for the PAI. In the AutoCommissioner, this is automatically set from the report from the
    // kSendPAICertificateRequest stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerificationstep.
    const Optional<ByteSpan> GetPAI() const { return mPAI; }

    // Device attestation certificate from the node. This is obtained from the node in response to the CertificateChainRequest
    // command for the DAC. In the AutoCommissioner, this is automatically set from the report from the kSendDACCertificateRequest
    // stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerification step.
    const Optional<ByteSpan> GetDAC() const { return mDAC; }

    // Node ID when a matching fabric is found in the Node Operational Credentials cluster.
    // In the AutoCommissioner, this is set from kReadCommissioningInfo stage.
    const Optional<NodeId> GetRemoteNodeId() const { return mRemoteNodeId; }

    // Node vendor ID from the basic information cluster. In the AutoCommissioner, this is automatically set from report from the
    // kReadCommissioningInfo stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerification step.
    const Optional<VendorId> GetRemoteVendorId() const { return mRemoteVendorId; }

    // Node product ID from the basic information cluster. In the AutoCommissioner, this is automatically set from report from the
    // kReadCommissioningInfo stage.
    // This must be set before calling PerformCommissioningStep for the kAttestationVerification step.
    const Optional<uint16_t> GetRemoteProductId() const { return mRemoteProductId; }

    // Default regulatory location set by the node, as read from the GeneralCommissioning cluster. In the AutoCommissioner, this is
    // automatically set from report from the kReadCommissioningInfo stage.
    // This should be set before calling PerformCommissioningStep for the kConfigRegulatory step.
    const Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> GetDefaultRegulatoryLocation() const
    {
        return mDefaultRegulatoryLocation;
    }

    // Location capabilities of the node, as read from the GeneralCommissioning cluster. In the AutoCommissioner, this is
    // automatically set from report from the kReadCommissioningInfo stage.
    // This should be set before calling PerformCommissioningStep for the kConfigRegulatory step.
    const Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> GetLocationCapability() const
    {
        return mLocationCapability;
    }

    // Status to send when calling CommissioningComplete on the PairingDelegate during the kCleanup step. The AutoCommissioner uses
    // this to pass through any error messages received during commissioning.
    const CompletionStatus & GetCompletionStatus() const { return completionStatus; }

    CommissioningParameters & SetFailsafeTimerSeconds(uint16_t seconds)
    {
        mFailsafeTimerSeconds.SetValue(seconds);
        return *this;
    }

    CommissioningParameters & SetCASEFailsafeTimerSeconds(uint16_t seconds)
    {
        mCASEFailsafeTimerSeconds.SetValue(seconds);
        return *this;
    }

    CommissioningParameters & SetDeviceRegulatoryLocation(app::Clusters::GeneralCommissioning::RegulatoryLocationType location)
    {
        mDeviceRegulatoryLocation.SetValue(location);
        return *this;
    }

    // The lifetime of the buffer countryCode is pointing to should exceed the
    // lifetime of CommissioningParameters object.
    CommissioningParameters & SetCountryCode(CharSpan countryCode)
    {
        mCountryCode.SetValue(countryCode);
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

    // If a ThreadOperationalDataset is provided, then the ThreadNetworkScan will not be attempted
    CommissioningParameters & SetThreadOperationalDataset(ByteSpan threadOperationalDataset)
    {

        mThreadOperationalDataset.SetValue(threadOperationalDataset);
        mAttemptThreadNetworkScan = MakeOptional(static_cast<bool>(false));
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
    CommissioningParameters & SetIpk(const IdentityProtectionKeySpan ipk)
    {
        mIpk.SetValue(IdentityProtectionKey(ipk));
        return *this;
    }
    CommissioningParameters & SetAdminSubject(const NodeId adminSubject)
    {
        mAdminSubject.SetValue(adminSubject);
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
    CommissioningParameters & SetRemoteNodeId(NodeId id)
    {
        mRemoteNodeId = MakeOptional(id);
        return *this;
    }
    CommissioningParameters & SetRemoteVendorId(VendorId id)
    {
        mRemoteVendorId = MakeOptional(id);
        return *this;
    }
    CommissioningParameters & SetRemoteProductId(uint16_t id)
    {
        mRemoteProductId = MakeOptional(id);
        return *this;
    }
    CommissioningParameters & SetDefaultRegulatoryLocation(app::Clusters::GeneralCommissioning::RegulatoryLocationType location)
    {
        mDefaultRegulatoryLocation = MakeOptional(location);
        return *this;
    }
    CommissioningParameters & SetLocationCapability(app::Clusters::GeneralCommissioning::RegulatoryLocationType capability)
    {
        mLocationCapability = MakeOptional(capability);
        return *this;
    }
    void SetCompletionStatus(const CompletionStatus & status) { completionStatus = status; }

    CommissioningParameters & SetDeviceAttestationDelegate(Credentials::DeviceAttestationDelegate * deviceAttestationDelegate)
    {
        mDeviceAttestationDelegate = deviceAttestationDelegate;
        return *this;
    }

    Credentials::DeviceAttestationDelegate * GetDeviceAttestationDelegate() const { return mDeviceAttestationDelegate; }

    // If an SSID is provided, and AttemptWiFiNetworkScan is true,
    // then a directed scan will be performed using the SSID provided in the WiFiCredentials object
    Optional<bool> GetAttemptWiFiNetworkScan() const { return mAttemptWiFiNetworkScan; }
    CommissioningParameters & SetAttemptWiFiNetworkScan(bool attemptWiFiNetworkScan)
    {
        mAttemptWiFiNetworkScan = MakeOptional(attemptWiFiNetworkScan);
        return *this;
    }

    // If a ThreadOperationalDataset is provided, then the ThreadNetworkScan will not be attempted
    Optional<bool> GetAttemptThreadNetworkScan() const { return mAttemptThreadNetworkScan; }
    CommissioningParameters & SetAttemptThreadNetworkScan(bool attemptThreadNetworkScan)
    {
        if (!mThreadOperationalDataset.HasValue())
        {
            mAttemptThreadNetworkScan = MakeOptional(attemptThreadNetworkScan);
        }
        return *this;
    }

    // Only perform the PASE steps of commissioning.
    // Commissioning will be completed by another admin on the network.
    Optional<bool> GetSkipCommissioningComplete() const { return mSkipCommissioningComplete; }
    CommissioningParameters & SetSkipCommissioningComplete(bool skipCommissioningComplete)
    {
        mSkipCommissioningComplete = MakeOptional(skipCommissioningComplete);
        return *this;
    }

    // Check for matching fabric on target device by reading fabric list and looking for a
    // fabricId and RootCert match. If a match is detected, then use GetNodeId() to
    // access the nodeId for the device on the matching fabric.
    bool GetCheckForMatchingFabric() const { return mCheckForMatchingFabric; }
    CommissioningParameters & SetCheckForMatchingFabric(bool checkForMatchingFabric)
    {
        mCheckForMatchingFabric = checkForMatchingFabric;
        return *this;
    }

    // Clear all members that depend on some sort of external buffer.  Can be
    // used to make sure that we are not holding any dangling pointers.
    void ClearExternalBufferDependentValues()
    {
        mCSRNonce.ClearValue();
        mAttestationNonce.ClearValue();
        mWiFiCreds.ClearValue();
        mCountryCode.ClearValue();
        mThreadOperationalDataset.ClearValue();
        mNOCChainGenerationParameters.ClearValue();
        mRootCert.ClearValue();
        mNoc.ClearValue();
        mIcac.ClearValue();
        mIpk.ClearValue();
        mAttestationElements.ClearValue();
        mAttestationSignature.ClearValue();
        mPAI.ClearValue();
        mDAC.ClearValue();
    }

private:
    // Items that can be set by the commissioner
    Optional<uint16_t> mFailsafeTimerSeconds;
    Optional<uint16_t> mCASEFailsafeTimerSeconds;
    Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> mDeviceRegulatoryLocation;
    Optional<ByteSpan> mCSRNonce;
    Optional<ByteSpan> mAttestationNonce;
    Optional<WiFiCredentials> mWiFiCreds;
    Optional<CharSpan> mCountryCode;
    Optional<ByteSpan> mThreadOperationalDataset;
    Optional<NOCChainGenerationParameters> mNOCChainGenerationParameters;
    Optional<ByteSpan> mRootCert;
    Optional<ByteSpan> mNoc;
    Optional<ByteSpan> mIcac;
    Optional<IdentityProtectionKey> mIpk;
    Optional<NodeId> mAdminSubject;
    // Items that come from the device in commissioning steps
    Optional<ByteSpan> mAttestationElements;
    Optional<ByteSpan> mAttestationSignature;
    Optional<ByteSpan> mPAI;
    Optional<ByteSpan> mDAC;
    Optional<NodeId> mRemoteNodeId;
    Optional<VendorId> mRemoteVendorId;
    Optional<uint16_t> mRemoteProductId;
    Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> mDefaultRegulatoryLocation;
    Optional<app::Clusters::GeneralCommissioning::RegulatoryLocationType> mLocationCapability;
    CompletionStatus completionStatus;
    Credentials::DeviceAttestationDelegate * mDeviceAttestationDelegate =
        nullptr; // Delegate to handle device attestation failures during commissioning
    Optional<bool> mAttemptWiFiNetworkScan;
    Optional<bool> mAttemptThreadNetworkScan; // This automatically gets set to false when a ThreadOperationalDataset is set
    Optional<bool> mSkipCommissioningComplete;
    bool mCheckForMatchingFabric = false;
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

struct CSRResponse
{
    CSRResponse(ByteSpan elements, ByteSpan newSignature) : nocsrElements(elements), signature(newSignature) {}
    ByteSpan nocsrElements;
    ByteSpan signature;
};

struct NocChain
{
    NocChain(ByteSpan newNoc, ByteSpan newIcac, ByteSpan newRcac, IdentityProtectionKeySpan newIpk, NodeId newAdminSubject) :
        noc(newNoc), icac(newIcac), rcac(newRcac), ipk(newIpk), adminSubject(newAdminSubject)
    {}
    ByteSpan noc;
    ByteSpan icac;
    ByteSpan rcac;
    IdentityProtectionKeySpan ipk;
    NodeId adminSubject;
};

struct OperationalNodeFoundData
{
    OperationalNodeFoundData(OperationalDeviceProxy proxy) : operationalProxy(proxy) {}
    OperationalDeviceProxy operationalProxy;
};

struct NetworkClusterInfo
{
    EndpointId endpoint = kInvalidEndpointId;
    app::Clusters::NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::TypeInfo::DecodableType minConnectionTime;
};
struct NetworkClusters
{
    NetworkClusterInfo wifi;
    NetworkClusterInfo thread;
    NetworkClusterInfo eth;
};
struct BasicClusterInfo
{
    VendorId vendorId  = VendorId::Common;
    uint16_t productId = 0;
};
struct GeneralCommissioningInfo
{
    uint64_t breadcrumb          = 0;
    uint16_t recommendedFailsafe = 0;
    app::Clusters::GeneralCommissioning::RegulatoryLocationType currentRegulatoryLocation =
        app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoorOutdoor;
    app::Clusters::GeneralCommissioning::RegulatoryLocationType locationCapability =
        app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoorOutdoor;
    ;
};

struct ReadCommissioningInfo
{
    NetworkClusters network;
    BasicClusterInfo basic;
    GeneralCommissioningInfo general;
    NodeId nodeId = kUndefinedNodeId;
};

struct AttestationErrorInfo
{
    AttestationErrorInfo(Credentials::AttestationVerificationResult result) : attestationResult(result) {}
    Credentials::AttestationVerificationResult attestationResult;
};

struct CommissioningErrorInfo
{
    CommissioningErrorInfo(app::Clusters::GeneralCommissioning::CommissioningError result) : commissioningError(result) {}
    app::Clusters::GeneralCommissioning::CommissioningError commissioningError;
};

struct NetworkCommissioningStatusInfo
{
    NetworkCommissioningStatusInfo(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus result) :
        networkCommissioningStatus(result)
    {}
    app::Clusters::NetworkCommissioning::NetworkCommissioningStatus networkCommissioningStatus;
};

class CommissioningDelegate
{
public:
    virtual ~CommissioningDelegate(){};
    /* CommissioningReport is returned after each commissioning step is completed. The reports for each step are:
     * kReadCommissioningInfo - ReadCommissioningInfo
     * kArmFailsafe: CommissioningErrorInfo if there is an error
     * kConfigRegulatory: CommissioningErrorInfo if there is an error
     * kSendPAICertificateRequest: RequestedCertificate
     * kSendDACCertificateRequest: RequestedCertificate
     * kSendAttestationRequest: AttestationResponse
     * kAttestationVerification: AttestationErrorInfo if there is an error
     * kSendOpCertSigningRequest: CSRResponse
     * kGenerateNOCChain: NocChain
     * kSendTrustedRootCert: None
     * kSendNOC: none
     * kWiFiNetworkSetup: NetworkCommissioningStatusInfo if there is an error
     * kThreadNetworkSetup: NetworkCommissioningStatusInfo if there is an error
     * kWiFiNetworkEnable: NetworkCommissioningStatusInfo if there is an error
     * kThreadNetworkEnable: NetworkCommissioningStatusInfo if there is an error
     * kFindOperational: OperationalNodeFoundData
     * kSendComplete: CommissioningErrorInfo if there is an error
     * kCleanup: none
     */
    struct CommissioningReport
        : Variant<RequestedCertificate, AttestationResponse, CSRResponse, NocChain, OperationalNodeFoundData, ReadCommissioningInfo,
                  AttestationErrorInfo, CommissioningErrorInfo, NetworkCommissioningStatusInfo>
    {
        CommissioningReport() : stageCompleted(CommissioningStage::kError) {}
        CommissioningStage stageCompleted;
    };
    virtual CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params)                           = 0;
    virtual const CommissioningParameters & GetCommissioningParameters() const                                      = 0;
    virtual void SetOperationalCredentialsDelegate(OperationalCredentialsDelegate * operationalCredentialsDelegate) = 0;
    virtual CHIP_ERROR StartCommissioning(DeviceCommissioner * commissioner, CommissioneeDeviceProxy * proxy)       = 0;
    virtual CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err, CommissioningReport report)                        = 0;
};

} // namespace Controller
} // namespace chip
