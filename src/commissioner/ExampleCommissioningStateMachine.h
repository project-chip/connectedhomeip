/*
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

#include <commissioner/States.h>
#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>

namespace chip {
namespace Commissioner {
namespace ExampleCommissioningStateMachine {

constexpr System::Clock::Timeout kCommissionableDiscoveryTimeout = System::Clock::Seconds16(30);
class Timer
{
public:
    virtual ~Timer()                                                  = default;
    virtual CHIP_ERROR ScheduleTimeout(System::Clock::Timeout aDelay) = 0;
    virtual CHIP_ERROR CancelTimeout()                                = 0;
};

using SystemState   = chip::Controller::DeviceControllerSystemState;
using Commissionee  = chip::Commissioner::Commissionee;
using OpCredsIssuer = chip::Controller::OperationalCredentialsDelegate;
using OnSuccess     = std::function<void(Commissionee &)>;
using OnFailure     = std::function<void(Commissionee &)>;

// SDK Events
using Success                  = chip::Commissioner::Events::Success;
using Failure                  = chip::Commissioner::Events::Failure;
using Await                    = chip::Commissioner::Events::Await;
using OnboardingPayload        = chip::Commissioner::Events::OnboardingPayload;
using ParsedPayload            = chip::Platform::SharedPtr<chip::SetupPayload>;
using ArmFailSafe              = chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafe::Type;
using AttestationInformation   = chip::Commissioner::Events::AttestationInformation;
using NocsrInformation         = chip::Commissioner::Events::NocsrInformation;
using OperationalCredentials   = chip::Commissioner::Events::OperationalCredentials;
using NetworkFeatureMap        = app::Clusters::NetworkCommissioning::Attributes::FeatureMap::TypeInfo::DecodableType;
using AddOrUpdateWiFiNetwork   = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type;
using AddOrUpdateThreadNetwork = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type;
using NetworkId                = chip::Commissioner::Events::NetworkId;
using OperationalRecord        = chip::Platform::SharedPtr<Dnssd::ResolvedNodeData>;

// App Events
struct Timeout
{
};
struct InitiateNetworkConfiguration
{
};
struct SkipNetworkConfiguration
{
};
struct InitiateOperationalDiscovery
{
};
struct InvokeCommissioningComplete
{
};
struct Shutdown
{
};

// Variant Event definition
using Event = chip::Variant<Success, Failure, Await, Timeout, OnboardingPayload, ParsedPayload, ArmFailSafe, AttestationInformation,
                            NocsrInformation, OperationalCredentials, InitiateNetworkConfiguration, SkipNetworkConfiguration,
                            NetworkFeatureMap, AddOrUpdateWiFiNetwork, AddOrUpdateThreadNetwork, NetworkId,
                            InitiateOperationalDiscovery, OperationalRecord, InvokeCommissioningComplete, Shutdown>;

class Context : public chip::StateMachine::Context<Event>, public Timer
{
};

// SDK States
using ParsingOnboardingPayload           = chip::Commissioner::States::ParsingOnboardingPayload<Context>;
using CommissionableNodeDiscovery        = chip::Commissioner::States::CommissionableNodeDiscovery<Context>;
using AwaitingCommissionableDiscovery    = chip::Commissioner::States::AwaitingCommissionableDiscovery<Context>;
using InitiatingPase                     = chip::Commissioner::States::PasscodeAuthenticatedSessionEstablishment<Context>;
using InvokingArmFailSafe                = chip::Commissioner::States::InvokingArmFailSafe<Context>;
using InvokingAttestationRequest         = chip::Commissioner::States::InvokingAttestationRequest<Context>;
using InvokingDacCertificateChainRequest = chip::Commissioner::States::InvokingDacCertificateChainRequest<Context>;
using InvokingPaiCertificateChainRequest = chip::Commissioner::States::InvokingPaiCertificateChainRequest<Context>;
using CapturingAttestationChallenge      = chip::Commissioner::States::CapturingAttestationChallenge<Context>;
using InvokingOpCSRRequest               = chip::Commissioner::States::InvokingOpCSRRequest<Context>;
using InvokingAddTrustedRootCertificate  = chip::Commissioner::States::InvokingAddTrustedRootCertificate<Context>;
using InvokingAddNOC                     = chip::Commissioner::States::InvokingAddNOC<Context>;
using ReadingNetworkFeatureMap           = chip::Commissioner::States::ReadingNetworkCommissioningClusterFeatureMap<Context>;
using InvokingAddOrUpdateWiFiNetwork     = chip::Commissioner::States::InvokingAddOrUpdateWiFiNetwork<Context>;
using InvokingAddOrUpdateThreadNetwork   = chip::Commissioner::States::InvokingAddOrUpdateThreadNetwork<Context>;
using InvokingConnectNetwork             = chip::Commissioner::States::InvokingConnectNetwork<Context>;
using OperationalDiscovery               = chip::Commissioner::States::OperationalDiscovery<Context>;
using InitiatingCase                     = chip::Commissioner::States::CertificateAuthenticatedSessionEstablishment<Context>;
using InvokingCommissioningComplete      = chip::Commissioner::States::InvokingCommissioningComplete<Context>;

// App-Specific States

struct Idle : chip::Commissioner::States::Base<Context>
{
    Idle(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "Idle") {}
};

struct AbortingCommissionableDiscovery : CommissionableNodeDiscovery
{
    AbortingCommissionableDiscovery(const CommissionableNodeDiscovery & previous) : CommissionableNodeDiscovery(previous)
    {
        this->mName = "AbortingCommissionableDiscovery";
    }
    void Enter()
    {
        this->mDiscoverer.get()->SetDelegate(this);
        this->mDiscoverer.get()->Shutdown();
    }

private:
    void OnDiscovery() {}
    void OnDiscovererShutdown() { this->mCtx.Dispatch(Event::Create<Success>()); }
};

struct FinishingPase : InitiatingPase
{
    FinishingPase(const InitiatingPase & previous) : InitiatingPase(previous) { this->mName = "FinishingPase"; }

    void Enter() { this->mPairing.get()->SetDelegate(this); }
    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
#if CONFIG_NETWORK_LAYER_BLE
        this->mCommissionee.CloseBle();
#endif
        this->mCtx.Dispatch(Event::Create<Failure>());
    }
};

struct PaseComplete : chip::Commissioner::States::Base<Context>
{
    PaseComplete(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "PaseComplete") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<ArmFailSafe>(ArmFailSafe{ 60 })); }
};

struct FailSafeArmed : chip::Commissioner::States::Base<Context>
{
    FailSafeArmed(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "FailSafeArmed") {}
    void Enter()
    {
        uint8_t attestationNonce[chip::kAttestationNonceLength];
        chip::Crypto::DRBG_get_bytes(attestationNonce, sizeof(attestationNonce));
        this->mCtx.Dispatch(Event::Create<AttestationInformation>(chip::ByteSpan(attestationNonce)));
    }
};

struct AttestationVerification : chip::Commissioner::States::Base<Context>
{
    AttestationVerification(Context & ctx, Commissionee & commissionee, AttestationInformation & attestationInformation) :
        Base<Context>(ctx, commissionee, "AttestationVerification"), mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        auto testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        auto verifier         = chip::Credentials::GetDefaultDACVerifier(testingRootStore);
        auto result           = verifier->VerifyAttestationInformation(
            mAttestationInformation.AttestationElements()->Get(), mAttestationInformation.Challenge()->Get(),
            mAttestationInformation.Signature()->Get(), mAttestationInformation.Pai()->Get(), mAttestationInformation.Dac()->Get(),
            mAttestationInformation.Nonce()->Get());
        if (result == chip::Credentials::AttestationVerificationResult::kSuccess)
        {
            this->mCtx.Dispatch(Event::Create<AttestationInformation>(mAttestationInformation));
        }
        else
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device: err %hu. Look at "
                         "AttestationVerificationResult enum to understand the errors",
                         static_cast<uint16_t>(result));
            // Go look at AttestationVerificationResult enum in src/credentials/DeviceAttestationVerifier.h to understand the
            // errors.
            this->mCtx.Dispatch(Event::Create<Failure>());
        }
    }

private:
    AttestationInformation mAttestationInformation;
};

struct AttestationVerified : chip::Commissioner::States::Base<Context>
{
    AttestationVerified(Context & ctx, Commissionee & commissionee, AttestationInformation & attestationInformation) :
        Base<Context>(ctx, commissionee, "AttestationVerified"), mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        uint8_t csrNonce[kOpCSRNonceLength];
        chip::Crypto::DRBG_get_bytes(csrNonce, sizeof(csrNonce));
        this->mCtx.Dispatch(Event::Create<NocsrInformation>(mAttestationInformation, chip::ByteSpan(csrNonce)));
    }

private:
    AttestationInformation mAttestationInformation;
};

struct OpCSRResponseReceived : chip::Commissioner::States::Base<Context>
{
    OpCSRResponseReceived(Context & ctx, Commissionee & commissionee, NocsrInformation & nocsrInformation) :
        Base<Context>(ctx, commissionee, "OpCSRResponseReceived"), mNocsrInformation(nocsrInformation)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<NocsrInformation>(mNocsrInformation)); }

private:
    NocsrInformation mNocsrInformation;
};

struct SigningCertificates : chip::Commissioner::States::Base<Context>
{
    SigningCertificates(Context & ctx, Commissionee & commissionee, NocsrInformation & nocsrInformation, OpCredsIssuer * issuer,
                        FabricIndex fabricIndex, NodeId nodeId) :
        Base<Context>(ctx, commissionee, "SigningCertificates"),
        mNocsrInformation(nocsrInformation), mIssuer(issuer), mFabricIndex(fabricIndex), mNodeId(nodeId)
    {}
    void Enter()
    {
        // Note that OperationalCredentialsDelegate is just used here for
        // convenience.  The exmaple app can use any signer interface it
        // chooses, and from SigningCertificates state context,
        // everything needed for end-to-end attestation, IPK and admin subject
        // assignment is accessible.
        chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> callback(OnDeviceNOCChainGeneration, this);
        FabricInfo * fabric = this->mCommissionee.mSystemState->Fabrics()->FindFabricWithIndex(mFabricIndex);
        if (fabric == nullptr)
        {
            this->mCtx.Dispatch(Event::Create<Failure>());
            return;
        }
        mIssuer->SetFabricIdForNextNOCRequest(fabric->GetFabricId());
        mIssuer->SetNodeIdForNextNOCRequest(mNodeId);
        CHIP_ERROR err =
            mIssuer->GenerateNOCChain(mNocsrInformation.NocsrElements()->Get(), mNocsrInformation.Signature()->Get(),
                                      mNocsrInformation.Dac()->Get(), mNocsrInformation.Pai()->Get(), chip::ByteSpan(), &callback);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(Event::Create<Failure>());
        }
    }
    void Dispatch(Event event) { this->mCtx.Dispatch(event); }

private:
    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR err, const chip::ByteSpan & derNoc,
                                           const chip::ByteSpan & derIcac, const chip::ByteSpan & derRcac)
    {

        ChipLogProgress(Controller, "Received callback from the CA for NOC Chain generation. Status %s", ErrorStr(err));
        SigningCertificates * state = static_cast<SigningCertificates *>(context);
        OperationalCredentials opCreds;
        SuccessOrExit(err);

        {
            chip::MutableByteSpan chipRcac;
            SuccessOrExit(err = opCreds.Rcac()->Allocate(chip::Credentials::kMaxCHIPCertLength));
            chipRcac = opCreds.Rcac()->GetMutable();
            SuccessOrExit(err = chip::Credentials::ConvertX509CertToChipCert(derRcac, chipRcac));
            SuccessOrExit(err = opCreds.Rcac()->Set(chipRcac));
        }
        if (!derIcac.empty())
        {
            chip::MutableByteSpan chipIcac;
            SuccessOrExit(err = opCreds.Icac()->Allocate(chip::Credentials::kMaxCHIPCertLength));
            chipIcac = opCreds.Icac()->GetMutable();
            SuccessOrExit(err = chip::Credentials::ConvertX509CertToChipCert(derIcac, chipIcac));
            SuccessOrExit(err = opCreds.Icac()->Set(chipIcac));
        }
        {
            chip::MutableByteSpan chipNoc;
            SuccessOrExit(err = opCreds.Noc()->Allocate(chip::Credentials::kMaxCHIPCertLength));
            chipNoc = opCreds.Noc()->GetMutable();
            SuccessOrExit(err = chip::Credentials::ConvertX509CertToChipCert(derNoc, chipNoc));
            SuccessOrExit(err = opCreds.Noc()->Set(chipNoc));
        }
    exit:
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in generating device's operational credentials. Error %s", ErrorStr(err));
            state->Dispatch(Event::Create<Failure>());
        }
        else
        {
            state->Dispatch(Event::Create<OperationalCredentials>(opCreds));
        }
    }

    NocsrInformation mNocsrInformation;
    OpCredsIssuer * mIssuer;
    FabricIndex mFabricIndex;
    NodeId mNodeId;
};

struct CertificatesSigned : chip::Commissioner::States::Base<Context>
{
    CertificatesSigned(Context & ctx, Commissionee & commissionee, OperationalCredentials & opCreds) :
        Base<Context>(ctx, commissionee, "CertificatesSigned"), mOpCreds(opCreds)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<OperationalCredentials>(mOpCreds)); }

private:
    OperationalCredentials mOpCreds;
};

struct OpCredsWritten : chip::Commissioner::States::Base<Context>
{
    OpCredsWritten(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "OpCredsWritten") {}
    void Enter()
    {
        if (!this->mCommissionee.mCommissionableNodeAddress.HasValue())
        {
            this->mCtx.Dispatch(Event::Create<Failure>());
        }
        else if (this->mCommissionee.mCommissionableNodeAddress.Value().GetTransportType() == Transport::Type::kTcp ||
                 this->mCommissionee.mCommissionableNodeAddress.Value().GetTransportType() == Transport::Type::kUdp)
        {
            this->mCtx.Dispatch(Event::Create<SkipNetworkConfiguration>());
        }
        else
        {
            this->mCtx.Dispatch(Event::Create<InitiateNetworkConfiguration>());
        }
    };
};

struct NetworkFeatureMapRead : chip::Commissioner::States::Base<Context>
{
    NetworkFeatureMapRead(Context & ctx, Commissionee & commissionee, NetworkFeatureMap featureMap, ByteSpan operationalDataset,
                          ByteSpan ssid, ByteSpan wiFiCredentials) :
        Base<Context>(ctx, commissionee, "NetworkFeatureMapRead"),
        mFeatureMap(featureMap), mOperationalDataset(operationalDataset), mSsid(ssid), mWiFiCredentials(wiFiCredentials)
    {}
    void Enter()
    {
        // TODO(#13170): Until platform driver support is added for the network
        // commissioning cluster, many devices may incorrectly report the
        // feature map attribute.  When this is resolved, we can branch our
        // state machine on cluster cability (WiFi, Thread or Ethernet). But
        // until then, we will infer commissoinee network type from the
        // credentials given to us and only log the feature map.
        ChipLogDetail(Controller, "Network Feature Map = 0x%08" PRIX32, mFeatureMap);
        if (mSsid.size())
        {
            this->mCtx.Dispatch(Event::Create<AddOrUpdateWiFiNetwork>(AddOrUpdateWiFiNetwork{ mSsid, mWiFiCredentials }));
        }
        else if (mOperationalDataset.size())
        {
            this->mCtx.Dispatch(Event::Create<AddOrUpdateThreadNetwork>(AddOrUpdateThreadNetwork{ mOperationalDataset }));
        }
        else
        {
            // We should only arrive in this state if the commissionee was not
            // located on an IP network.  In such cases, we need to configure a
            // WiFi or Thread network.  If we can't, that's a failure.
            this->mCtx.Dispatch(Event::Create<Failure>());
        }
    };

private:
    NetworkFeatureMap mFeatureMap;
    ByteSpan mOperationalDataset;
    ByteSpan mSsid;
    ByteSpan mWiFiCredentials;
};

struct NetworkAdded : chip::Commissioner::States::Base<Context>
{
    NetworkAdded(Context & ctx, Commissionee & commissionee, NetworkId networkId) :
        Base<Context>(ctx, commissionee, "NetworkAdded"), mNetworkId(networkId)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<NetworkId>(mNetworkId)); }

private:
    NetworkId mNetworkId;
};

struct NetworkEnabled : chip::Commissioner::States::Base<Context>

{
    NetworkEnabled(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "NetworkEnabled") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<InitiateOperationalDiscovery>()); }
};

struct CaseComplete : chip::Commissioner::States::Base<Context>
{
    CaseComplete(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "CaseComplete") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<InvokeCommissioningComplete>()); }
};

struct CommissioningComplete : chip::Commissioner::States::Base<Context>
{
    CommissioningComplete(Context & ctx, Commissionee & commissionee, OnSuccess onSuccess) :
        Base<Context>(ctx, commissionee, "CommissioningComplete"), mOnSuccess(onSuccess)
    {}
    void Enter()
    {
        if (mOnSuccess != nullptr)
        {
            mOnSuccess(this->mCommissionee);
        }
    }

private:
    OnSuccess mOnSuccess;
};

struct Failed : chip::Commissioner::States::Base<Context>

{
    Failed(Context & ctx, Commissionee & commissionee, OnFailure onFailure) :
        Base<Context>(ctx, commissionee, "Failed"), mOnFailure(onFailure)
    {}
    void Enter()
    {
        if (mOnFailure != nullptr)
        {
            mOnFailure(this->mCommissionee);
        }
    }

private:
    OnFailure mOnFailure;
};

// Variant State definition
using State = chip::StateMachine::VariantState<
    Idle, ParsingOnboardingPayload, CommissionableNodeDiscovery, AwaitingCommissionableDiscovery, AbortingCommissionableDiscovery,
    InitiatingPase, FinishingPase, PaseComplete, InvokingArmFailSafe, FailSafeArmed, InvokingAttestationRequest,
    InvokingDacCertificateChainRequest, InvokingPaiCertificateChainRequest, CapturingAttestationChallenge, AttestationVerification,
    AttestationVerified, InvokingOpCSRRequest, OpCSRResponseReceived, SigningCertificates, CertificatesSigned,
    InvokingAddTrustedRootCertificate, InvokingAddNOC, OpCredsWritten, ReadingNetworkFeatureMap, NetworkFeatureMapRead,
    InvokingAddOrUpdateWiFiNetwork, InvokingAddOrUpdateThreadNetwork, NetworkAdded, InvokingConnectNetwork, NetworkEnabled,
    OperationalDiscovery, InitiatingCase, CaseComplete, InvokingCommissioningComplete, CommissioningComplete, Failed>;

class StateFactory
{
public:
    StateFactory(Context & ctx, Commissionee & commissionee) : mCtx(ctx), mCommissionee(commissionee) {}
    void Init(OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId, ByteSpan operationalDataset, ByteSpan ssid,
              ByteSpan wiFiCredentials);
    void SetCallbacks(OnSuccess onSuccess, OnFailure onFailure);

    // clang-format off
    auto CreateIdle()
    {
        return State::Create<Idle>(mCtx, mCommissionee);
    }
    auto CreateParsingOnboardingPayload(OnboardingPayload payload)
    {
        return State::Create<ParsingOnboardingPayload>(mCtx, mCommissionee, payload);
    }
    auto CreateCommissionableNodeDiscovery(ParsedPayload payload)
    {
        return State::Create<CommissionableNodeDiscovery>(mCtx, mCommissionee, payload);
    }
    auto CreateAwaitingCommissionableDiscovery(const CommissionableNodeDiscovery & previous)
    {
        return State::Create<AwaitingCommissionableDiscovery>(previous);
    }
    auto CreateAbortingCommissionableDiscovery(const CommissionableNodeDiscovery & previous)
    {
        return State::Create<AbortingCommissionableDiscovery>(previous);
    }
    auto CreateInitiatingPase(const CommissionableNodeDiscovery & previous)
    {
        return State::Create<InitiatingPase>(previous);
    }
    auto CreateFinishingPase(const InitiatingPase & previous)
    {
        return State::Create<FinishingPase>(previous);
    }
    auto CreatePaseComplete()
    {
        return State::Create<PaseComplete>(mCtx, mCommissionee);
    }
    auto CreateInvokingArmFailSafe(ArmFailSafe request)
    {
        return State::Create<InvokingArmFailSafe>(mCtx, mCommissionee, request);
    }
    auto CreateFailSafeArmed()
    {
        return State::Create<FailSafeArmed>(mCtx, mCommissionee);
    }
    auto CreateInvokingAttestationRequest(AttestationInformation attestationInformation)
    {
        return State::Create<InvokingAttestationRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingDacCertificateChainRequest(AttestationInformation attestationInformation)
    {
        return State::Create<InvokingDacCertificateChainRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingPaiCertificateChainRequest(AttestationInformation attestationInformation)
    {
        return State::Create<InvokingPaiCertificateChainRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateCapturingAttestationChallenge(AttestationInformation attestationInformation)
    {
        return State::Create<CapturingAttestationChallenge>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateAttestationVerification(AttestationInformation attestationInformation)
    {
        return State::Create<AttestationVerification>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateAttestationVerified(AttestationInformation attestationInformation)
    {
        return State::Create<AttestationVerified>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingOpCSRRequest(NocsrInformation nocsrInformation)
    {
        return State::Create<InvokingOpCSRRequest>(mCtx, mCommissionee, nocsrInformation);
    }
    auto CreateOpCSRResponseReceived(NocsrInformation nocsrInformation)
    {
        return State::Create<OpCSRResponseReceived>(mCtx, mCommissionee, nocsrInformation);
    }
    auto CreateSigningCertificates(NocsrInformation nocsrInformation)
    {
        return State::Create<SigningCertificates>(mCtx, mCommissionee, nocsrInformation, mIssuer, mFabricIndex, mNodeId);
    }
    auto CreateCertificatesSigned(OperationalCredentials opCreds)
    {
        return State::Create<CertificatesSigned>(mCtx, mCommissionee, opCreds);
    }
    auto CreateInvokingAddTrustedRootCertificate(OperationalCredentials opCreds)
    {
        return State::Create<InvokingAddTrustedRootCertificate>(mCtx, mCommissionee, opCreds);
    }
    auto CreateInvokingAddNOC(OperationalCredentials opCreds)
    {
        return State::Create<InvokingAddNOC>(mCtx, mCommissionee, opCreds);
    }
    auto CreateOpCredsWritten()
    {
        return State::Create<OpCredsWritten>(mCtx, mCommissionee);
    }
    auto CreateReadingNetworkFeatureMap()
    {
        return State::Create<ReadingNetworkFeatureMap>(mCtx, mCommissionee);
    }
    auto CreateNetworkFeatureMapRead(NetworkFeatureMap featureMap)
    {
        return State::Create<NetworkFeatureMapRead>(mCtx, mCommissionee, featureMap, mOperationalDataset, mSsid, mWiFiCredentials);
    }
    auto CreateInvokingAddOrUpdateWiFiNetwork(AddOrUpdateWiFiNetwork request)
    {
        return State::Create<InvokingAddOrUpdateWiFiNetwork>(mCtx, mCommissionee, request);
    }
    auto CreateInvokingAddOrUpdateThreadNetwork(AddOrUpdateThreadNetwork request)
    {
        return State::Create<InvokingAddOrUpdateThreadNetwork>(mCtx, mCommissionee, request);
    }
    auto CreateNetworkAdded(NetworkId networkId)
    {
        return State::Create<NetworkAdded>(mCtx, mCommissionee, networkId);
    }
    auto CreateInvokingConnectNetwork(NetworkId networkId)
    {
        return State::Create<InvokingConnectNetwork>(mCtx, mCommissionee, networkId);
    }
    auto CreateNetworkEnabled()
    {
        return State::Create<NetworkEnabled>(mCtx, mCommissionee);
    }
    auto CreateOperationalDiscovery()
    {
        return State::Create<OperationalDiscovery>(mCtx, mCommissionee);
    }
    auto CreateInitiatingCase(OperationalRecord record)
    {
        return State::Create<InitiatingCase>(mCtx, mCommissionee, record);
    }
    auto CreateCaseComplete()
    {
        return State::Create<CaseComplete>(mCtx, mCommissionee);
    }
    auto CreateInvokingCommissioningComplete()
    {
        return State::Create<InvokingCommissioningComplete>(mCtx, mCommissionee);
    }
    auto CreateCommissioningComplete()
    {
        return State::Create<CommissioningComplete>(mCtx, mCommissionee, mOnSuccess);
    }
    auto CreateFailed()
    {
        return State::Create<Failed>(mCtx, mCommissionee, mOnFailure);
    }
    // clang-format on

private:
    Context & mCtx;
    Commissionee & mCommissionee;
    OpCredsIssuer * mIssuer;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
    NodeId mNodeId           = kUndefinedNodeId;
    ByteSpan mOperationalDataset;
    ByteSpan mSsid;
    ByteSpan mWiFiCredentials;
    OnSuccess mOnSuccess;
    OnFailure mOnFailure;
};

struct Transitions
{
    Transitions(Context & ctx, Commissionee & commissionee) : mFactory(ctx, commissionee), mCtx(ctx) {}
    chip::StateMachine::Optional<State> operator()(const State & state, const Event & event);
    State GetInitState() { return mFactory.CreateIdle(); }
    StateFactory mFactory;
    Context & mCtx;
};

class ExampleCommissioningStateMachine : public chip::StateMachine::StateMachine<Context, State, Event, Transitions>
{

public:
    ExampleCommissioningStateMachine() :
        chip::StateMachine::StateMachine<Context, State, Event, Transitions>(mTransitions), mTransitions(*this, mCommissionee)
    {}
    void Init(SystemState * systemState, OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId);
    void Init(SystemState * systemState, OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId,
              ByteSpan operationalDataset, ByteSpan ssid, ByteSpan wiFiCredentials);
    void Shutdown();
    CHIP_ERROR Commission(const char * onboardingPayload, OnSuccess onSuccess, OnFailure onFailure);
    CHIP_ERROR Commission(chip::SetupPayload & onboardingPayload, OnSuccess onSuccess, OnFailure onFailure);
    CHIP_ERROR Commission(chip::RendezvousInformationFlags flags, uint16_t discriminator, uint32_t setUpPINCode,
                          OnSuccess onSuccess, OnFailure onFailure);
    CHIP_ERROR GrabCommissionee(chip::Controller::DeviceController & controller);
    CHIP_ERROR ScheduleTimeout(System::Clock::Timeout aDelay);
    CHIP_ERROR CancelTimeout();

private:
    CHIP_ERROR Commission(Event event, OnSuccess onSuccess, OnFailure onFailure);
    static void DispatchTimeout(System::Layer * aLayer, void * appState);
    Transitions mTransitions;
    Commissionee mCommissionee;
};

} // namespace ExampleCommissioningStateMachine
} // namespace Commissioner
} // namespace chip
