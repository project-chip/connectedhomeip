/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <commissioner/Commissionee.h>
#include <commissioner/Discoverer.h>
#include <commissioner/Events.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/support/StateMachine.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>

namespace chip {
namespace Commissioner {
namespace States {

constexpr uint16_t kCommissioningEndpoint = 0; // TODO: should get the endpoint information from the descriptor cluster.

template <typename TContext>
struct Base
{
    Base(TContext & ctx, Commissionee & commissionee, const char * name) : mCtx(ctx), mCommissionee(commissionee), mName(name) {}
    void LogTransition(const char * previous) { ChipLogDetail(Controller, "%s -> %s", previous, GetName()); }
    const char * GetName() { return mName; }
    void Enter() {}
    void Exit() {}

protected:
    TContext & mCtx;
    Commissionee & mCommissionee;
    const char * mName;
};

template <typename TContext>
struct ParsingOnboardingPayload : Base<TContext>
{
    ParsingOnboardingPayload(TContext & ctx, Commissionee & commissionee, Events::OnboardingPayload & payload) :
        Base<TContext>(ctx, commissionee, "ParsingOnboardingPayload"), mPayload(payload)
    {}

    void Enter()
    {
        CHIP_ERROR err;
        bool isQRCode                            = strncmp(mPayload.mPayload, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
        Platform::SharedPtr<SetupPayload> parsed = Platform::MakeShared<SetupPayload>();
        VerifyOrExit(parsed.get() != nullptr, err = CHIP_ERROR_NO_MEMORY);
        if (isQRCode)
        {
            SuccessOrExit(err = chip::QRCodeSetupPayloadParser(mPayload.mPayload).populatePayload(*parsed.get()));
        }
        else
        {
            SuccessOrExit(err = chip::ManualSetupPayloadParser(mPayload.mPayload).populatePayload(*parsed.get()));
        }
    exit:
        if (err == CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Platform::SharedPtr<SetupPayload>>(parsed));
        }
        else
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::OnboardingPayload mPayload;
};

template <typename TContext>
struct CommissionableNodeDiscovery : Base<TContext>, CommissionableNodeDiscoverer::Delegate
{
    CommissionableNodeDiscovery(TContext & ctx, Commissionee & commissionee, Platform::SharedPtr<SetupPayload> payload) :
        Base<TContext>(ctx, commissionee, "CommissionableNodeDiscovery"), mPayload(payload)
    {}

    void Enter()
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        mDiscoverer =
            Platform::MakeShared<CommissionableNodeDiscoverer::Discoverer>(*this->mCommissionee.mSystemState, mPayload, this);
        VerifyOrExit(mDiscoverer.get() != nullptr, err = CHIP_ERROR_NO_MEMORY);
        SuccessOrExit(err = mDiscoverer.get()->Init());
        SuccessOrExit(err = mDiscoverer.get()->Discover());
    exit:
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    void OnDiscovery() override { this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>()); }
    void OnDiscovererShutdown() override {}

    Platform::SharedPtr<SetupPayload> mPayload;
    Platform::SharedPtr<CommissionableNodeDiscoverer::Discoverer> mDiscoverer;
};

template <typename TContext>
struct AwaitingCommissionableDiscovery : CommissionableNodeDiscovery<TContext>
{
    AwaitingCommissionableDiscovery(const CommissionableNodeDiscovery<TContext> & previous) :
        CommissionableNodeDiscovery<TContext>(previous)
    {
        this->mName = "AwaitingCommissionableDiscovery";
    }
    void Enter() { this->mDiscoverer.get()->SetDelegate(this); }

protected:
    void OnDiscovery() override { this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>()); }
    void OnDiscovererShutdown() override {}
};

template <typename TContext>
struct PasscodeAuthenticatedSessionEstablishment : CommissionableNodeDiscovery<TContext>, SessionEstablishmentDelegate
{
    PasscodeAuthenticatedSessionEstablishment(const CommissionableNodeDiscovery<TContext> & previous) :
        CommissionableNodeDiscovery<TContext>(previous)
    {
        this->mName = "PasscodeAuthenticatedSessionEstablishment";
    }

    void Enter()
    {
        this->mDiscoverer.get()->SetDelegate(this);
        while ((mStatus = this->mDiscoverer.get()->GetNextCandidate(this->mCommissionee)) == CHIP_NO_ERROR)
        {
            VerifyOrReturn((mStatus = TryPase()) != CHIP_NO_ERROR);
        }
        if (mStatus == CHIP_ERROR_NOT_FOUND)
        {
            // CHIP_ERROR_NOT_FOUND from the discoverer means we should keep looking.
            // This is not ncessarily a failure.
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Await>());
        }
        else
        {
            this->mDiscoverer.get()->Shutdown();
        }
    }

    void ShutdownDiscoverer() { this->mDiscoverer.get()->Shutdown(); }

protected:
    CHIP_ERROR TryPase()
    {
        CHIP_ERROR err                        = CHIP_NO_ERROR;
        uint16_t sessionId                    = 0;
        Messaging::ExchangeContext * exchange = nullptr;
        Optional<SessionHandle> session;

        mPairing = Platform::MakeShared<PASESession>();
        VerifyOrExit(mPairing.get() != nullptr, err = CHIP_ERROR_NO_MEMORY);
        session = this->mCommissionee.mSystemState->SessionMgr()->CreateUnauthenticatedSession(
            this->mCommissionee.mCommissionableNodeAddress.Value(), this->mCommissionee.mMrpConfig.ValueOr(gDefaultMRPConfig));
        VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NO_MEMORY);
        exchange = this->mCommissionee.mSystemState->ExchangeMgr()->NewContext(session.Value(), this->mPairing.get());
        VerifyOrExit(exchange != nullptr, err = CHIP_ERROR_INTERNAL);
        SuccessOrExit(err = this->mCommissionee.mSystemState->GetSessionIDAllocator()->Allocate(sessionId));
        SuccessOrExit(err = mPairing.get()->Pair(this->mCommissionee.mCommissionableNodeAddress.Value(),
                                                 this->mPayload.get()->setUpPINCode, sessionId, this->mCommissionee.mMrpConfig,
                                                 exchange, this));
    exit:
        return err;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
#if CONFIG_NETWORK_LAYER_BLE
        this->mCommissionee.CloseBle();
#endif
        Enter(); // try next candidate, if any exists
    }

    void OnSessionEstablished() override
    {
        mStatus = this->mCommissionee.mSystemState->SessionMgr()->NewPairing(
            this->mCommissionee.mPaseSession, Optional<Transport::PeerAddress>::Value(mPairing.get()->GetPeerAddress()),
            kUndefinedNodeId, mPairing.get(), CryptoContext::SessionRole::kInitiator, kUndefinedFabricId);
        if (mStatus != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(mStatus));
        }
        else
        {
            ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake");
        }
        // We can't dustruct the PASESession object until the stack unwinds
        // because the exchange isn't yet closed, the exchange has a pointer
        // to the PASESession object, and ACK for the secure_channel Status
        // Report hasn't been sent yet.  Therefore, schedule dispatch, and
        // implicit destruction, for the next iteration of the event loop.
        this->mCommissionee.mSystemState->SystemLayer()->ScheduleWork(Dispose, this);
    }

    static void Dispose(System::Layer * layer, void * context)
    {
        auto state = static_cast<PasscodeAuthenticatedSessionEstablishment *>(context);
        state->ShutdownDiscoverer();
    }

    void OnDiscovery() override {}
    void OnDiscovererShutdown() override
    {
        if (mStatus == CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>());
        }
        else
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

    Platform::SharedPtr<PASESession> mPairing;
    CHIP_ERROR mStatus = CHIP_NO_ERROR;
};

template <typename TContext>
struct InvokingArmFailSafe : Base<TContext>
{
    using ArmFailSafe = app::Clusters::GeneralCommissioning::Commands::ArmFailSafe::Type;
    using Response    = ArmFailSafe::ResponseType;
    InvokingArmFailSafe(TContext & ctx, Commissionee & commissionee, ArmFailSafe request) :
        Base<TContext>(ctx, commissionee, "InvokingArmFailSafe"), mRequest(request)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>());
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err =
            this->mCommissionee.template PaseInvoke<ArmFailSafe>(kCommissioningEndpoint, mRequest, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    ArmFailSafe mRequest;
};

template <typename TContext>
struct InvokingAttestationRequest : Base<TContext>
{
    using AttestationRequest = app::Clusters::OperationalCredentials::Commands::AttestationRequest::Type;
    using Response           = AttestationRequest::ResponseType;
    InvokingAttestationRequest(TContext & ctx, Commissionee & commissionee,
                               Events::AttestationInformation & attestationInformation) :
        Base<TContext>(ctx, commissionee, "InvokingAttestationRequest"),
        mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            CHIP_ERROR err = CHIP_NO_ERROR;
            SuccessOrExit(err = mAttestationInformation.AttestationElements()->Set(response.attestationElements));
            SuccessOrExit(err = mAttestationInformation.Signature()->Set(response.signature));
        exit:
            if (err == CHIP_NO_ERROR)
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::AttestationInformation>(mAttestationInformation));
            }
            else
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
            }
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<AttestationRequest>(
            kCommissioningEndpoint, AttestationRequest{ mAttestationInformation.Nonce()->Get() }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::AttestationInformation mAttestationInformation;
};

template <typename TContext>
struct InvokingDacCertificateChainRequest : Base<TContext>
{
    using CertificateChainRequest = app::Clusters::OperationalCredentials::Commands::CertificateChainRequest::Type;
    using Response                = CertificateChainRequest::ResponseType;
    InvokingDacCertificateChainRequest(TContext & ctx, Commissionee & commissionee,
                                       Events::AttestationInformation & attestationInformation) :
        Base<TContext>(ctx, commissionee, "InvokingDacCertificateChainRequest"),
        mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            if (mAttestationInformation.Dac()->Set(response.certificate) == CHIP_NO_ERROR)
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::AttestationInformation>(mAttestationInformation));
            }
            else
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
            }
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<CertificateChainRequest>(
            kCommissioningEndpoint, CertificateChainRequest{ Credentials::CertificateType::kDAC }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::AttestationInformation mAttestationInformation;
};

template <typename TContext>
struct InvokingPaiCertificateChainRequest : Base<TContext>
{
    using CertificateChainRequest = app::Clusters::OperationalCredentials::Commands::CertificateChainRequest::Type;
    using Response                = CertificateChainRequest::ResponseType;
    InvokingPaiCertificateChainRequest(TContext & ctx, Commissionee & commissionee,
                                       Events::AttestationInformation & attestationInformation) :
        Base<TContext>(ctx, commissionee, "InvokingPaiCertificateChainRequest"),
        mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            if (mAttestationInformation.Pai()->Set(response.certificate) == CHIP_NO_ERROR)
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::AttestationInformation>(mAttestationInformation));
            }
            else
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
            }
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke(
            kCommissioningEndpoint, CertificateChainRequest{ Credentials::CertificateType::kPAI }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::AttestationInformation mAttestationInformation;
};

template <typename TContext>
struct CapturingAttestationChallenge : Base<TContext>
{
    CapturingAttestationChallenge(TContext & ctx, Commissionee & commissionee,
                                  Events::AttestationInformation & attestationInformation) :
        Base<TContext>(ctx, commissionee, "CapturingAttestationChallenge"),
        mAttestationInformation(attestationInformation)
    {}
    void Enter()
    {
        const ByteSpan challenge = this->mCommissionee.mSystemState->SessionMgr()
                                       ->GetSecureSession(this->mCommissionee.mPaseSession.Get())
                                       ->GetCryptoContext()
                                       .GetAttestationChallenge();
        if (mAttestationInformation.Challenge()->Set(challenge) == CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::AttestationInformation>(mAttestationInformation));
        }
        else
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::AttestationInformation mAttestationInformation;
};

template <typename TContext>
struct InvokingOpCSRRequest : Base<TContext>
{
    using OpCSRRequest = app::Clusters::OperationalCredentials::Commands::OpCSRRequest::Type;
    using Response     = OpCSRRequest::ResponseType;
    InvokingOpCSRRequest(TContext & ctx, Commissionee & commissionee, Events::NocsrInformation nocsrInformation) :
        Base<TContext>(ctx, commissionee, "InvokingOpCSRRequest"), mNocsrInformation(nocsrInformation)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            CHIP_ERROR err;
            SuccessOrExit(err = mNocsrInformation.NocsrElements()->Set(response.NOCSRElements));
            SuccessOrExit(err = mNocsrInformation.Signature()->Set(response.attestationSignature));
        exit:
            if (err == CHIP_NO_ERROR)
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::NocsrInformation>(mNocsrInformation));
            }
            else
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
            }
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<OpCSRRequest>(
            kCommissioningEndpoint, OpCSRRequest{ mNocsrInformation.Nonce()->Get() }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::NocsrInformation mNocsrInformation;
};

template <typename TContext>
struct InvokingAddTrustedRootCertificate : Base<TContext>
{
    using AddTrustedRootCertificate = app::Clusters::OperationalCredentials::Commands::AddTrustedRootCertificate::Type;
    using Response                  = AddTrustedRootCertificate::ResponseType;
    InvokingAddTrustedRootCertificate(TContext & ctx, Commissionee & commissionee, Events::OperationalCredentials opCreds) :
        Base<TContext>(ctx, commissionee, "InvokingAddTrustedRootCertificate"), mOpCreds(opCreds)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::OperationalCredentials>(mOpCreds));
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<AddTrustedRootCertificate>(
            kCommissioningEndpoint, AddTrustedRootCertificate{ mOpCreds.Rcac()->Get() }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::OperationalCredentials mOpCreds;
};

template <typename TContext>
struct InvokingAddNOC : Base<TContext>
{
    using AddNOC   = app::Clusters::OperationalCredentials::Commands::AddNOC::Type;
    using Response = AddNOC::ResponseType;
    InvokingAddNOC(TContext & ctx, Commissionee & commissionee, Events::OperationalCredentials opCreds) :
        Base<TContext>(ctx, commissionee, "InvokingAddNOC"), mOpCreds(opCreds)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            PeerId operationalId;
            CHIP_ERROR err;
            SuccessOrExit(err = mOpCreds.GetOperationalId(operationalId));
            this->mCommissionee.mOperationalId.SetValue(operationalId);
            ChipLogProgress(Controller, "AddNOC succeeded for node %016" PRIX64 "-%016" PRIX64,
                            this->mCommissionee.mOperationalId.Value().GetCompressedFabricId(),
                            this->mCommissionee.mOperationalId.Value().GetNodeId());
        exit:
            if (err == CHIP_NO_ERROR)
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::OperationalCredentials>(mOpCreds));
            }
            else
            {
                this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
            }
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        AddNOC request = {
            .NOCValue      = mOpCreds.Noc()->Get(),
            .ICACValue     = mOpCreds.Icac()->Get().size() ? Optional<ByteSpan>(mOpCreds.Icac()->Get()) : Optional<ByteSpan>{},
            .IPKValue      = mOpCreds.Ipk()->Get(),
            .caseAdminNode = mOpCreds.mAdminSubject,
            .adminVendorId = mOpCreds.mAdminVendorId,
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<AddNOC>(kCommissioningEndpoint, request, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::OperationalCredentials mOpCreds;
};

template <typename TContext>
struct ReadingNetworkCommissioningClusterFeatureMap : Base<TContext>
{
    ClusterId clusterId     = app::Clusters::NetworkCommissioning::Id;
    AttributeId attributeId = app::Clusters::NetworkCommissioning::Attributes::FeatureMap::Id;
    using FeatureMap        = app::Clusters::NetworkCommissioning::Attributes::FeatureMap::TypeInfo::DecodableType;
    ReadingNetworkCommissioningClusterFeatureMap(TContext & ctx, Commissionee & commissionee) :
        Base<TContext>(ctx, commissionee, "ReadingNetworkCommissioningClusterFeatureMap")
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteAttributePath & commandPath, const FeatureMap & aData) {
            this->mCtx.Dispatch(TContext::Event::template Create<FeatureMap>(aData));
        };

        auto onFailure = [this](const app::ConcreteAttributePath * commandPath, app::StatusIB status, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };

        CHIP_ERROR err =
            this->mCommissionee.template PaseRead<FeatureMap>(kCommissioningEndpoint, clusterId, attributeId, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }
};

template <typename TContext>
struct InvokingAddOrUpdateWiFiNetwork : Base<TContext>
{
    using AddOrUpdateWiFiNetwork = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type;
    using Response               = AddOrUpdateWiFiNetwork::ResponseType;
    InvokingAddOrUpdateWiFiNetwork(TContext & ctx, Commissionee & commissionee, AddOrUpdateWiFiNetwork request) :
        Base<TContext>(ctx, commissionee, "InvokingAddOrUpdateWiFiNetwork"), mRequest(request)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::NetworkId>(mNetworkId));
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err;
        SuccessOrExit(err = mNetworkId.Set(mRequest.ssid)); // deep-copy network ID for next state
        SuccessOrExit(err = this->mCommissionee.template PaseInvoke<AddOrUpdateWiFiNetwork>(kCommissioningEndpoint, mRequest,
                                                                                            onSuccess, onFailure));
    exit:
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    AddOrUpdateWiFiNetwork mRequest;
    Events::NetworkId mNetworkId;
};

template <typename TContext>
struct InvokingAddOrUpdateThreadNetwork : Base<TContext>
{
    using AddOrUpdateThreadNetwork = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type;
    using Response                 = AddOrUpdateThreadNetwork::ResponseType;
    InvokingAddOrUpdateThreadNetwork(TContext & ctx, Commissionee & commissionee, AddOrUpdateThreadNetwork request) :
        Base<TContext>(ctx, commissionee, "InvokingAddOrUpdateThreadNetwork"), mRequest(request)
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::NetworkId>(mNetworkId));
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err;
        SuccessOrExit(
            err = mNetworkId.FromThreadOperationalDataset(mRequest.operationalDataset)); // deep-copy ext pan ID for next state
        SuccessOrExit(err = this->mCommissionee.template PaseInvoke<AddOrUpdateThreadNetwork>(kCommissioningEndpoint, mRequest,
                                                                                              onSuccess, onFailure));
    exit:
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    AddOrUpdateThreadNetwork mRequest;
    Events::NetworkId mNetworkId;
};

template <typename TContext>
struct InvokingConnectNetwork : Base<TContext>
{
    using ConnectNetwork = app::Clusters::NetworkCommissioning::Commands::ConnectNetwork::Type;
    using Response       = ConnectNetwork::ResponseType;
    InvokingConnectNetwork(TContext & ctx, Commissionee & commissionee, Events::NetworkId networkId) :
        Base<TContext>(ctx, commissionee, "InvokingConnectNetwork"), mNetworkId(networkId)
    {}

    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>());
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.template PaseInvoke<ConnectNetwork>(
            kCommissioningEndpoint, ConnectNetwork{ mNetworkId.Get() }, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

protected:
    Events::NetworkId mNetworkId;
};

template <typename TContext>
struct OperationalDiscovery : Base<TContext>, chip::Dnssd::ResolverDelegate
{
    OperationalDiscovery(TContext & ctx, Commissionee & commissionee) : Base<TContext>(ctx, commissionee, "OperationalDiscovery") {}

    void Enter()
    {
#if !CHIP_DEVICE_CONFIG_ENABLE_DNSSD
        this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
    }
#else
        CHIP_ERROR err;
        SuccessOrExit(err = mDnsResolver.Init(this->mCommissionee.mSystemState->UDPEndPointManager()));
        mDnsResolver.SetResolverDelegate(this);
        SuccessOrExit(err = mDnsResolver.ResolveNodeId(this->mCommissionee.mOperationalId.Value(), Inet::IPAddressType::kAny));
    exit:
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

    void Exit()
    {
        mDnsResolver.Shutdown();
        // TODO: Setting the instance delegate to nullptr is a workaround for
        //       #13227.  If we do not do this, minimal mdns can call back into
        //       our allocated delegate proxy after it is freed.
        chip::Dnssd::Resolver::Instance().SetResolverDelegate(nullptr);
    }

protected:
    void OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData) override {}

    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        auto record = Platform::MakeShared<Dnssd::ResolvedNodeData>(nodeData);
        VerifyOrExit(record.get() != nullptr, err = CHIP_ERROR_NO_MEMORY);
        this->mCtx.Dispatch(TContext::Event::template Create<decltype(record)>(record));
    exit:
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }

    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogError(Controller, "Operational discovery failed for node %016" PRIX64 "-%016" PRIX64,
                     this->mCommissionee.mOperationalId.Value().GetCompressedFabricId(),
                     this->mCommissionee.mOperationalId.Value().GetNodeId());
        this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
    }

    Dnssd::ResolverProxy mDnsResolver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD
};

template <typename TContext>
struct CertificateAuthenticatedSessionEstablishment : Base<TContext>, SessionEstablishmentDelegate
{
    CertificateAuthenticatedSessionEstablishment(TContext & ctx, Commissionee & commissionee,
                                                 Platform::SharedPtr<Dnssd::ResolvedNodeData> record) :
        Base<TContext>(ctx, commissionee, "CertificateAuthenticatedSessionEstablishment"),
        mRecord(record)
    {}

    void Enter()
    {
        while (GetNextCandidate() == CHIP_NO_ERROR)
        {
            VerifyOrReturn(TryCase() != CHIP_NO_ERROR);
        }
        this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
    }

    void DispatchStatus()
    {
        auto status =
            mSuccess ? TContext::Event::template Create<Events::Success>() : TContext::Event::template Create<Events::Failure>();
        this->mCtx.Dispatch(status);
    }

protected:
    CHIP_ERROR GetNextCandidate()
    {
        if (mIdx >= mRecord.get()->mNumIPs)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        Inet::InterfaceId interfaceId =
            mRecord.get()->mAddress[mIdx].IsIPv6LinkLocal() ? mRecord.get()->mInterfaceId : Inet::InterfaceId::Null();
#if 0
        // TODO(#13326): connecting via TCP does not work
        if (mRecord.get()->supportsTcp)
        {
            this->mCommissionee.mOperationalAddress.SetValue(Transport::PeerAddress::TCP(mRecord.get()->mAddress[mIdx], mRecord.get()->mPort, interfaceId));
            this->mCommissionee.mMrpConfig.ClearValue();
        }
        else
#endif
        {
            this->mCommissionee.mOperationalAddress.SetValue(
                Transport::PeerAddress::UDP(mRecord.get()->mAddress[mIdx], mRecord.get()->mPort, interfaceId));
            this->mCommissionee.mMrpConfig.SetValue(mRecord.get()->GetMRPConfig());
        }
#if CHIP_PROGRESS_LOGGING
        char addressStr[Transport::PeerAddress::kMaxToStringSize];
        this->mCommissionee.mOperationalAddress.Value().ToString(addressStr);
        ChipLogProgress(Controller, "Operational identity discovered at %s", addressStr);
#endif // CHIP_PROGRESS_LOGGING
        ++mIdx;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR TryCase()
    {
        CHIP_ERROR err                        = CHIP_NO_ERROR;
        uint16_t sessionId                    = 0;
        Messaging::ExchangeContext * exchange = nullptr;
        Optional<SessionHandle> session;
        FabricInfo * fabric;

        fabric = this->mCommissionee.mSystemState->Fabrics()->FindFabricWithCompressedId(
            this->mCommissionee.mOperationalId.Value().GetCompressedFabricId());
        VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        mPairing = Platform::MakeShared<CASESession>();
        VerifyOrExit(mPairing.get() != nullptr, err = CHIP_ERROR_NO_MEMORY);
        session = this->mCommissionee.mSystemState->SessionMgr()->CreateUnauthenticatedSession(
            this->mCommissionee.mOperationalAddress.Value(), this->mCommissionee.mMrpConfig.ValueOr(gDefaultMRPConfig));
        VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NO_MEMORY);
        exchange = this->mCommissionee.mSystemState->ExchangeMgr()->NewContext(session.Value(), this->mPairing.get());
        VerifyOrExit(exchange != nullptr, err = CHIP_ERROR_INTERNAL);
        SuccessOrExit(err = this->mCommissionee.mSystemState->GetSessionIDAllocator()->Allocate(sessionId));
        SuccessOrExit(err = mPairing.get()->EstablishSession(this->mCommissionee.mOperationalAddress.Value(), fabric,
                                                             this->mCommissionee.mOperationalId.Value().GetNodeId(), sessionId,
                                                             exchange, this, this->mCommissionee.mMrpConfig));
    exit:
        return err;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
        Enter(); // try next candidate, if any exists
    }

    void OnSessionEstablished() override
    {
        CHIP_ERROR err = this->mCommissionee.mSystemState->SessionMgr()->NewPairing(
            this->mCommissionee.mCaseSession, this->mCommissionee.mOperationalAddress,
            this->mCommissionee.mOperationalId.Value().GetNodeId(), mPairing.get(), CryptoContext::SessionRole::kInitiator,
            kUndefinedFabricId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        }
        else
        {
            ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake");
            mSuccess = true;
        }
        // We can't dustruct the CASESession object until the stack unwinds
        // because the exchange isn't yet closed, the exchange has a pointer
        // to the CASESession object, and ACK for the secure_channel Status
        // Report hasn't been sent yet.  Therefore, schedule dispatch, and
        // implicit destruction, for the next iteration of the event loop.
        this->mCommissionee.mSystemState->SystemLayer()->ScheduleWork(Dispose, this);
    }

    static void Dispose(System::Layer * layer, void * context)
    {
        auto state = static_cast<CertificateAuthenticatedSessionEstablishment *>(context);
        state->DispatchStatus();
    }

    Platform::SharedPtr<Dnssd::ResolvedNodeData> mRecord;
    Platform::SharedPtr<CASESession> mPairing;
    size_t mIdx   = 0;
    bool mSuccess = false;
};

template <typename TContext>
struct InvokingCommissioningComplete : Base<TContext>
{
    using CommissioningComplete = app::Clusters::GeneralCommissioning::Commands::CommissioningComplete::Type;
    using Response              = CommissioningComplete::ResponseType;
    InvokingCommissioningComplete(TContext & ctx, Commissionee & commissionee) :
        Base<TContext>(ctx, commissionee, "InvokingCommissioningComplete")
    {}
    void Enter()
    {
        auto onSuccess = [this](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                const Response & response) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Success>());
        };
        auto onFailure = [this](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        };
        CHIP_ERROR err = this->mCommissionee.CaseInvoke(kCommissioningEndpoint, CommissioningComplete{}, onSuccess, onFailure);
        if (err != CHIP_NO_ERROR)
        {
            this->mCtx.Dispatch(TContext::Event::template Create<Events::Failure>());
        }
    }
};

template <typename TContext>
struct Failed : Base<TContext>
{
    Failed(TContext & ctx, Commissionee & commissionee) : Base<TContext>(ctx, commissionee, "Failed") {}
};

} // namespace States
} // namespace Commissioner
} // namespace chip
