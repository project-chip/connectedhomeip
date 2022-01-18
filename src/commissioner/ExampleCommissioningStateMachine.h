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

using SystemState   = Controller::DeviceControllerSystemState;
using OpCredsIssuer = Controller::OperationalCredentialsDelegate;
using OnSuccess     = std::function<void(Commissionee &)>;
using OnFailure     = std::function<void(Commissionee &)>;

namespace SdkEvents = Events;

namespace AppEvents {

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

} // namespace AppEvents

// Variant Event definition
using Event = chip::Variant<SdkEvents::Success, SdkEvents::Failure, SdkEvents::Await, SdkEvents::RawOnboardingPayload,
                            SdkEvents::OnboardingPayload, SdkEvents::ArmFailSafe, SdkEvents::AttestationInformation,
                            SdkEvents::NocsrInformation, SdkEvents::OperationalCredentials, SdkEvents::NetworkFeatureMap,
                            SdkEvents::AddOrUpdateWiFiNetwork, SdkEvents::AddOrUpdateThreadNetwork, SdkEvents::NetworkId,
                            SdkEvents::OperationalRecord,

                            AppEvents::Timeout, AppEvents::InitiateNetworkConfiguration, AppEvents::SkipNetworkConfiguration,
                            AppEvents::InitiateOperationalDiscovery, AppEvents::InvokeCommissioningComplete, AppEvents::Shutdown>;

class Context : public chip::StateMachine::Context<Event>, public Timer
{
};

namespace SdkStates {

using Base                               = States::Base<Context>;
using ParsingOnboardingPayload           = States::ParsingOnboardingPayload<Context>;
using CommissionableNodeDiscovery        = States::CommissionableNodeDiscovery<Context>;
using AwaitingCommissionableDiscovery    = States::AwaitingCommissionableDiscovery<Context>;
using InitiatingPase                     = States::PasscodeAuthenticatedSessionEstablishment<Context>;
using InvokingArmFailSafe                = States::InvokingArmFailSafe<Context>;
using InvokingAttestationRequest         = States::InvokingAttestationRequest<Context>;
using InvokingDacCertificateChainRequest = States::InvokingDacCertificateChainRequest<Context>;
using InvokingPaiCertificateChainRequest = States::InvokingPaiCertificateChainRequest<Context>;
using CapturingAttestationChallenge      = States::CapturingAttestationChallenge<Context>;
using InvokingOpCSRRequest               = States::InvokingOpCSRRequest<Context>;
using InvokingAddTrustedRootCertificate  = States::InvokingAddTrustedRootCertificate<Context>;
using InvokingAddNOC                     = States::InvokingAddNOC<Context>;
using ReadingNetworkFeatureMap           = States::ReadingNetworkCommissioningClusterFeatureMap<Context>;
using InvokingAddOrUpdateWiFiNetwork     = States::InvokingAddOrUpdateWiFiNetwork<Context>;
using InvokingAddOrUpdateThreadNetwork   = States::InvokingAddOrUpdateThreadNetwork<Context>;
using InvokingConnectNetwork             = States::InvokingConnectNetwork<Context>;
using OperationalDiscovery               = States::OperationalDiscovery<Context>;
using InitiatingCase                     = States::CertificateAuthenticatedSessionEstablishment<Context>;
using InvokingCommissioningComplete      = States::InvokingCommissioningComplete<Context>;

} // namespace SdkStates

namespace AppStates {

struct Idle : SdkStates::Base
{
    Idle(Context & ctx, Commissionee & commissionee) : Base(ctx, commissionee, "Idle") {}
};

struct AbortingCommissionableDiscovery : SdkStates::CommissionableNodeDiscovery
{
    AbortingCommissionableDiscovery(const SdkStates::CommissionableNodeDiscovery & previous) : CommissionableNodeDiscovery(previous)
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
    void OnDiscovererShutdown() { this->mCtx.Dispatch(Event::Create<SdkEvents::Success>()); }
};

struct FinishingPase : SdkStates::InitiatingPase
{
    FinishingPase(const SdkStates::InitiatingPase & previous) : SdkStates::InitiatingPase(previous)
    {
        this->mName = "FinishingPase";
    }

    void Enter() { this->mPairing.get()->SetDelegate(this); }
    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
#if CONFIG_NETWORK_LAYER_BLE
        this->mCommissionee.CloseBle();
#endif
        this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>());
    }
};

struct PaseComplete : SdkStates::Base
{
    PaseComplete(Context & ctx, Commissionee & commissionee) : Base(ctx, commissionee, "PaseComplete") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<SdkEvents::ArmFailSafe>(SdkEvents::ArmFailSafe{ 60 })); }
};

struct FailSafeArmed : SdkStates::Base
{
    FailSafeArmed(Context & ctx, Commissionee & commissionee) : Base(ctx, commissionee, "FailSafeArmed") {}
    void Enter()
    {
        uint8_t attestationNonce[chip::kAttestationNonceLength];
        chip::Crypto::DRBG_get_bytes(attestationNonce, sizeof(attestationNonce));
        this->mCtx.Dispatch(Event::Create<SdkEvents::AttestationInformation>(chip::ByteSpan(attestationNonce)));
    }
};

struct AttestationVerification : SdkStates::Base
{
    AttestationVerification(Context & ctx, Commissionee & commissionee,
                            SdkEvents::AttestationInformation & attestationInformation) :
        Base(ctx, commissionee, "AttestationVerification"),
        mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        auto testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        auto verifier         = chip::Credentials::GetDefaultDACVerifier(testingRootStore);
        Callback::Callback<Credentials::OnAttestationInformationVerification> callback(OnVerification, this);
        verifier->VerifyAttestationInformation(
            mAttestationInformation.AttestationElements()->Get(), mAttestationInformation.Challenge()->Get(),
            mAttestationInformation.Signature()->Get(), mAttestationInformation.Pai()->Get(), mAttestationInformation.Dac()->Get(),
            mAttestationInformation.Nonce()->Get(), &callback);
    }

    void DispatchSuccess() { this->mCtx.Dispatch(Event::Create<SdkEvents::AttestationInformation>(mAttestationInformation)); }
    void DispatchFailure() { this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>()); }

private:
    static void OnVerification(void * context, Credentials::AttestationVerificationResult result)
    {
        auto state = static_cast<AttestationVerification *>(context);
        if (result == chip::Credentials::AttestationVerificationResult::kSuccess)
        {
            state->DispatchSuccess();
        }
        else
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device: err %hu. Look at "
                         "AttestationVerificationResult enum to understand the errors",
                         static_cast<uint16_t>(result));
            // Go look at AttestationVerificationResult enum in src/credentials/DeviceAttestationVerifier.h to understand the
            // errors.
            state->DispatchFailure();
        }
    }

    SdkEvents::AttestationInformation mAttestationInformation;
};

struct AttestationVerified : SdkStates::Base
{
    AttestationVerified(Context & ctx, Commissionee & commissionee, SdkEvents::AttestationInformation & attestationInformation) :
        Base(ctx, commissionee, "AttestationVerified"), mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        uint8_t csrNonce[kOpCSRNonceLength];
        chip::Crypto::DRBG_get_bytes(csrNonce, sizeof(csrNonce));
        this->mCtx.Dispatch(Event::Create<SdkEvents::NocsrInformation>(mAttestationInformation, chip::ByteSpan(csrNonce)));
    }

private:
    SdkEvents::AttestationInformation mAttestationInformation;
};

struct OpCSRResponseReceived : SdkStates::Base
{
    OpCSRResponseReceived(Context & ctx, Commissionee & commissionee, SdkEvents::NocsrInformation & nocsrInformation) :
        Base(ctx, commissionee, "OpCSRResponseReceived"), mNocsrInformation(nocsrInformation)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<SdkEvents::NocsrInformation>(mNocsrInformation)); }

private:
    SdkEvents::NocsrInformation mNocsrInformation;
};

struct SigningCertificates : SdkStates::Base
{
    SigningCertificates(Context & ctx, Commissionee & commissionee, SdkEvents::NocsrInformation & nocsrInformation,
                        OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId) :
        Base(ctx, commissionee, "SigningCertificates"),
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
            this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>());
            return;
        }
        mIssuer->SetFabricIdForNextNOCRequest(fabric->GetFabricId());
        mIssuer->SetNodeIdForNextNOCRequest(mNodeId);
        CHIP_ERROR err =
            mIssuer->GenerateNOCChain(mNocsrInformation.NocsrElements()->Get(), mNocsrInformation.Signature()->Get(),
                                      mNocsrInformation.Dac()->Get(), mNocsrInformation.Pai()->Get(), chip::ByteSpan(), &callback);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>());
        }
    }
    void Dispatch(Event event) { this->mCtx.Dispatch(event); }

private:
    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR err, const chip::ByteSpan & derNoc,
                                           const chip::ByteSpan & derIcac, const chip::ByteSpan & derRcac)
    {

        ChipLogProgress(Controller, "Received callback from the CA for NOC Chain generation. Status %s", ErrorStr(err));
        SigningCertificates * state = static_cast<SigningCertificates *>(context);
        SdkEvents::OperationalCredentials opCreds;
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
            state->Dispatch(Event::Create<SdkEvents::Failure>());
        }
        else
        {
            state->Dispatch(Event::Create<SdkEvents::OperationalCredentials>(opCreds));
        }
    }

    SdkEvents::NocsrInformation mNocsrInformation;
    OpCredsIssuer * mIssuer;
    FabricIndex mFabricIndex;
    NodeId mNodeId;
};

struct CertificatesSigned : SdkStates::Base
{
    CertificatesSigned(Context & ctx, Commissionee & commissionee, SdkEvents::OperationalCredentials & opCreds) :
        Base(ctx, commissionee, "CertificatesSigned"), mOpCreds(opCreds)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<SdkEvents::OperationalCredentials>(mOpCreds)); }

private:
    SdkEvents::OperationalCredentials mOpCreds;
};

struct OpCredsWritten : SdkStates::Base
{
    OpCredsWritten(Context & ctx, Commissionee & commissionee) : Base(ctx, commissionee, "OpCredsWritten") {}
    void Enter()
    {
        if (!this->mCommissionee.mCommissionableNodeAddress.HasValue())
        {
            this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>());
        }
        else if (this->mCommissionee.mCommissionableNodeAddress.Value().GetTransportType() == Transport::Type::kTcp ||
                 this->mCommissionee.mCommissionableNodeAddress.Value().GetTransportType() == Transport::Type::kUdp)
        {
            this->mCtx.Dispatch(Event::Create<AppEvents::SkipNetworkConfiguration>());
        }
        else
        {
            this->mCtx.Dispatch(Event::Create<AppEvents::InitiateNetworkConfiguration>());
        }
    };
};

struct NetworkFeatureMapRead : SdkStates::Base
{
    NetworkFeatureMapRead(Context & ctx, Commissionee & commissionee, SdkEvents::NetworkFeatureMap featureMap,
                          ByteSpan operationalDataset, ByteSpan ssid, ByteSpan wiFiCredentials) :
        Base(ctx, commissionee, "NetworkFeatureMapRead"),
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
            this->mCtx.Dispatch(
                Event::Create<SdkEvents::AddOrUpdateWiFiNetwork>(SdkEvents::AddOrUpdateWiFiNetwork{ mSsid, mWiFiCredentials }));
        }
        else if (mOperationalDataset.size())
        {
            this->mCtx.Dispatch(
                Event::Create<SdkEvents::AddOrUpdateThreadNetwork>(SdkEvents::AddOrUpdateThreadNetwork{ mOperationalDataset }));
        }
        else
        {
            // We should only arrive in this state if the commissionee was not
            // located on an IP network.  In such cases, we need to configure a
            // WiFi or Thread network.  If we can't, that's a failure.
            this->mCtx.Dispatch(Event::Create<SdkEvents::Failure>());
        }
    };

private:
    SdkEvents::NetworkFeatureMap mFeatureMap;
    ByteSpan mOperationalDataset;
    ByteSpan mSsid;
    ByteSpan mWiFiCredentials;
};

struct NetworkAdded : SdkStates::Base
{
    NetworkAdded(Context & ctx, Commissionee & commissionee, SdkEvents::NetworkId networkId) :
        Base(ctx, commissionee, "NetworkAdded"), mNetworkId(networkId)
    {}
    void Enter() { this->mCtx.Dispatch(Event::Create<SdkEvents::NetworkId>(mNetworkId)); }

private:
    SdkEvents::NetworkId mNetworkId;
};

struct NetworkEnabled : SdkStates::Base

{
    NetworkEnabled(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "NetworkEnabled") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<AppEvents::InitiateOperationalDiscovery>()); }
};

struct CaseComplete : SdkStates::Base
{
    CaseComplete(Context & ctx, Commissionee & commissionee) : Base<Context>(ctx, commissionee, "CaseComplete") {}
    void Enter() { this->mCtx.Dispatch(Event::Create<AppEvents::InvokeCommissioningComplete>()); }
};

struct CommissioningComplete : SdkStates::Base
{
    CommissioningComplete(Context & ctx, Commissionee & commissionee, OnSuccess onSuccess) :
        Base(ctx, commissionee, "CommissioningComplete"), mOnSuccess(onSuccess)
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

struct Failed : SdkStates::Base

{
    Failed(Context & ctx, Commissionee & commissionee, OnFailure onFailure) :
        Base(ctx, commissionee, "Failed"), mOnFailure(onFailure)
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

} // namespace AppStates

// Variant State definition
using State = chip::StateMachine::VariantState<
    SdkStates::ParsingOnboardingPayload, SdkStates::CommissionableNodeDiscovery, SdkStates::AwaitingCommissionableDiscovery,
    SdkStates::InitiatingPase, SdkStates::InvokingArmFailSafe, SdkStates::InvokingAttestationRequest,
    SdkStates::InvokingDacCertificateChainRequest, SdkStates::InvokingPaiCertificateChainRequest,
    SdkStates::CapturingAttestationChallenge, SdkStates::InvokingOpCSRRequest, SdkStates::InvokingAddTrustedRootCertificate,
    SdkStates::InvokingAddNOC, SdkStates::ReadingNetworkFeatureMap, SdkStates::InvokingAddOrUpdateWiFiNetwork,
    SdkStates::InvokingAddOrUpdateThreadNetwork, SdkStates::InvokingConnectNetwork, SdkStates::OperationalDiscovery,
    SdkStates::InitiatingCase, SdkStates::InvokingCommissioningComplete,

    AppStates::Idle, AppStates::AbortingCommissionableDiscovery, AppStates::FinishingPase, AppStates::PaseComplete,
    AppStates::FailSafeArmed, AppStates::AttestationVerification, AppStates::AttestationVerified, AppStates::OpCSRResponseReceived,
    AppStates::SigningCertificates, AppStates::CertificatesSigned, AppStates::OpCredsWritten, AppStates::NetworkFeatureMapRead,
    AppStates::NetworkAdded, AppStates::NetworkEnabled, AppStates::CaseComplete, AppStates::CommissioningComplete,
    AppStates::Failed>;

class StateFactory
{
public:
    StateFactory(Context & ctx, Commissionee & commissionee) : mCtx(ctx), mCommissionee(commissionee) {}
    void Init(OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId, ByteSpan operationalDataset, ByteSpan ssid,
              ByteSpan wiFiCredentials);
    void SetCallbacks(OnSuccess onSuccess, OnFailure onFailure);

    // clang-format off
    auto CreateParsingOnboardingPayload(SdkEvents::RawOnboardingPayload payload)
    {
        return State::Create<SdkStates::ParsingOnboardingPayload>(mCtx, mCommissionee, payload);
    }
    auto CreateCommissionableNodeDiscovery(SdkEvents::OnboardingPayload payload)
    {
        return State::Create<SdkStates::CommissionableNodeDiscovery>(mCtx, mCommissionee, payload);
    }
    auto CreateAwaitingCommissionableDiscovery(const SdkStates::CommissionableNodeDiscovery & previous)
    {
        return State::Create<SdkStates::AwaitingCommissionableDiscovery>(previous);
    }
    auto CreateInitiatingPase(const SdkStates::CommissionableNodeDiscovery & previous)
    {
        return State::Create<SdkStates::InitiatingPase>(previous);
    }
    auto CreateInvokingArmFailSafe(SdkEvents::ArmFailSafe request)
    {
        return State::Create<SdkStates::InvokingArmFailSafe>(mCtx, mCommissionee, request);
    }
    auto CreateInvokingAttestationRequest(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<SdkStates::InvokingAttestationRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingDacCertificateChainRequest(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<SdkStates::InvokingDacCertificateChainRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingPaiCertificateChainRequest(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<SdkStates::InvokingPaiCertificateChainRequest>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateCapturingAttestationChallenge(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<SdkStates::CapturingAttestationChallenge>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateInvokingOpCSRRequest(SdkEvents::NocsrInformation nocsrInformation)
    {
        return State::Create<SdkStates::InvokingOpCSRRequest>(mCtx, mCommissionee, nocsrInformation);
    }
    auto CreateInvokingAddTrustedRootCertificate(SdkEvents::OperationalCredentials opCreds)
    {
        return State::Create<SdkStates::InvokingAddTrustedRootCertificate>(mCtx, mCommissionee, opCreds);
    }
    auto CreateInvokingAddNOC(SdkEvents::OperationalCredentials opCreds)
    {
        return State::Create<SdkStates::InvokingAddNOC>(mCtx, mCommissionee, opCreds);
    }
    auto CreateReadingNetworkFeatureMap()
    {
        return State::Create<SdkStates::ReadingNetworkFeatureMap>(mCtx, mCommissionee);
    }
    auto CreateInvokingAddOrUpdateWiFiNetwork(SdkEvents::AddOrUpdateWiFiNetwork request)
    {
        return State::Create<SdkStates::InvokingAddOrUpdateWiFiNetwork>(mCtx, mCommissionee, request);
    }
    auto CreateInvokingAddOrUpdateThreadNetwork(SdkEvents::AddOrUpdateThreadNetwork request)
    {
        return State::Create<SdkStates::InvokingAddOrUpdateThreadNetwork>(mCtx, mCommissionee, request);
    }
    auto CreateInvokingConnectNetwork(SdkEvents::NetworkId networkId)
    {
        return State::Create<SdkStates::InvokingConnectNetwork>(mCtx, mCommissionee, networkId);
    }
    auto CreateOperationalDiscovery()
    {
        return State::Create<SdkStates::OperationalDiscovery>(mCtx, mCommissionee);
    }
    auto CreateInitiatingCase(SdkEvents::OperationalRecord record)
    {
        return State::Create<SdkStates::InitiatingCase>(mCtx, mCommissionee, record);
    }
    auto CreateInvokingCommissioningComplete()
    {
        return State::Create<SdkStates::InvokingCommissioningComplete>(mCtx, mCommissionee);
    }
    auto CreateIdle()
    {
        return State::Create<AppStates::Idle>(mCtx, mCommissionee);
    }
    auto CreateAbortingCommissionableDiscovery(const SdkStates::CommissionableNodeDiscovery & previous)
    {
        return State::Create<AppStates::AbortingCommissionableDiscovery>(previous);
    }
    auto CreateFinishingPase(const SdkStates::InitiatingPase & previous)
    {
        return State::Create<AppStates::FinishingPase>(previous);
    }
    auto CreatePaseComplete()
    {
        return State::Create<AppStates::PaseComplete>(mCtx, mCommissionee);
    }
    auto CreateFailSafeArmed()
    {
        return State::Create<AppStates::FailSafeArmed>(mCtx, mCommissionee);
    }
    auto CreateAttestationVerification(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<AppStates::AttestationVerification>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateAttestationVerified(SdkEvents::AttestationInformation attestationInformation)
    {
        return State::Create<AppStates::AttestationVerified>(mCtx, mCommissionee, attestationInformation);
    }
    auto CreateOpCSRResponseReceived(SdkEvents::NocsrInformation nocsrInformation)
    {
        return State::Create<AppStates::OpCSRResponseReceived>(mCtx, mCommissionee, nocsrInformation);
    }
    auto CreateSigningCertificates(SdkEvents::NocsrInformation nocsrInformation)
    {
        return State::Create<AppStates::SigningCertificates>(mCtx, mCommissionee, nocsrInformation, mIssuer, mFabricIndex, mNodeId);
    }
    auto CreateCertificatesSigned(SdkEvents::OperationalCredentials opCreds)
    {
        return State::Create<AppStates::CertificatesSigned>(mCtx, mCommissionee, opCreds);
    }
    auto CreateOpCredsWritten()
    {
        return State::Create<AppStates::OpCredsWritten>(mCtx, mCommissionee);
    }
    auto CreateNetworkFeatureMapRead(SdkEvents::NetworkFeatureMap featureMap)
    {
        return State::Create<AppStates::NetworkFeatureMapRead>(mCtx, mCommissionee, featureMap, mOperationalDataset, mSsid, mWiFiCredentials);
    }
    auto CreateNetworkAdded(SdkEvents::NetworkId networkId)
    {
        return State::Create<AppStates::NetworkAdded>(mCtx, mCommissionee, networkId);
    }
    auto CreateNetworkEnabled()
    {
        return State::Create<AppStates::NetworkEnabled>(mCtx, mCommissionee);
    }
    auto CreateCaseComplete()
    {
        return State::Create<AppStates::CaseComplete>(mCtx, mCommissionee);
    }
    auto CreateCommissioningComplete()
    {
        return State::Create<AppStates::CommissioningComplete>(mCtx, mCommissionee, mOnSuccess);
    }
    auto CreateFailed()
    {
        return State::Create<AppStates::Failed>(mCtx, mCommissionee, mOnFailure);
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
