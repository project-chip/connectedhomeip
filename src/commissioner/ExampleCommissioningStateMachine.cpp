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

#include <commissioner/ExampleCommissioningStateMachine.h>

namespace chip {
namespace Commissioner {
namespace ExampleCommissioningStateMachine {

void StateFactory::Init(OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId, ByteSpan operationalDataset, ByteSpan ssid,
                        ByteSpan wiFiCredentials)
{
    mIssuer             = issuer;
    mFabricIndex        = fabricIndex;
    mNodeId             = nodeId;
    mOperationalDataset = operationalDataset;
    mSsid               = ssid;
    mWiFiCredentials    = wiFiCredentials;
}

void StateFactory::SetCallbacks(OnSuccess onSuccess, OnFailure onFailure)
{
    mOnSuccess = onSuccess;
    mOnFailure = onFailure;
}

chip::StateMachine::Optional<State> Transitions::operator()(const State & state, const Event & event)
{
    if (state.Is<Idle>() && event.Is<OnboardingPayload>())
    {
        return mFactory.CreateParsingOnboardingPayload(event.Get<OnboardingPayload>());
    }
    else if ((state.Is<Idle>() || state.Is<ParsingOnboardingPayload>()) && event.Is<ParsedPayload>())
    {
        CHIP_ERROR err = mCtx.ScheduleTimeout(kCommissionableDiscoveryTimeout);
        return err == CHIP_NO_ERROR ? mFactory.CreateCommissionableNodeDiscovery(event.Get<ParsedPayload>())
                                    : mFactory.CreateFailed();
    }
    else if (state.Is<CommissionableNodeDiscovery>() && event.Is<Timeout>())
    {
        return mFactory.CreateAbortingCommissionableDiscovery(state.Get<CommissionableNodeDiscovery>());
    }
    else if (state.Is<CommissionableNodeDiscovery>() && event.Is<Success>())
    {
        return mFactory.CreateInitiatingPase(state.Get<CommissionableNodeDiscovery>());
    }
    else if (state.Is<AbortingCommissionableDiscovery>() && event.Is<Success>())
    {
        return mFactory.CreateFailed();
    }
    else if (state.Is<InitiatingPase>() && event.Is<Await>())
    {
        return mFactory.CreateAwaitingCommissionableDiscovery(state.Get<InitiatingPase>());
    }
    else if (state.Is<InitiatingPase>() && event.Is<Timeout>())
    {
        return mFactory.CreateFinishingPase(state.Get<InitiatingPase>());
    }
    else if (state.Is<InitiatingPase>() && event.Is<Success>())
    {
        return mFactory.CreatePaseComplete();
    }
    else if (state.Is<AwaitingCommissionableDiscovery>() && event.Is<Timeout>())
    {
        return mFactory.CreateAbortingCommissionableDiscovery(state.Get<AwaitingCommissionableDiscovery>());
    }
    else if (state.Is<AwaitingCommissionableDiscovery>() && event.Is<Success>())
    {
        return mFactory.CreateInitiatingPase(state.Get<AwaitingCommissionableDiscovery>());
    }
    else if (state.Is<FinishingPase>() && event.Is<Success>())
    {
        this->mCtx.CancelTimeout();
        return mFactory.CreatePaseComplete();
    }
    else if (state.Is<PaseComplete>() && event.Is<ArmFailSafe>())
    {
        return mFactory.CreateInvokingArmFailSafe(event.Get<ArmFailSafe>());
    }
    else if (state.Is<InvokingArmFailSafe>() && (event.Is<Success>() || event.Is<Failure>())) // TODO(#13327): accept failure; until #13327 is resolved, commissionees will return ArmFailSafe instead of ArmFailSafeResponse
    {
        return mFactory.CreateFailSafeArmed();
    }
    else if (state.Is<FailSafeArmed>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateInvokingAttestationRequest(event.Get<AttestationInformation>());
    }
    else if (state.Is<InvokingAttestationRequest>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateInvokingDacCertificateChainRequest(event.Get<AttestationInformation>());
    }
    else if (state.Is<InvokingDacCertificateChainRequest>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateInvokingPaiCertificateChainRequest(event.Get<AttestationInformation>());
    }
    else if (state.Is<InvokingPaiCertificateChainRequest>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateCapturingAttestationChallenge(event.Get<AttestationInformation>());
    }
    else if (state.Is<CapturingAttestationChallenge>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateAttestationVerification(event.Get<AttestationInformation>());
    }
    else if (state.Is<AttestationVerification>() && event.Is<AttestationInformation>())
    {
        return mFactory.CreateAttestationVerified(event.Get<AttestationInformation>());
    }
    else if (state.Is<AttestationVerified>() && event.Is<NocsrInformation>())
    {
        return mFactory.CreateInvokingOpCSRRequest(event.Get<NocsrInformation>());
    }
    else if (state.Is<InvokingOpCSRRequest>() && event.Is<NocsrInformation>())
    {
        return mFactory.CreateOpCSRResponseReceived(event.Get<NocsrInformation>());
    }
    else if (state.Is<OpCSRResponseReceived>() && event.Is<NocsrInformation>())
    {
        return mFactory.CreateSigningCertificates(event.Get<NocsrInformation>());
    }
    else if (state.Is<SigningCertificates>() && event.Is<OperationalCredentials>())
    {
        return mFactory.CreateCertificatesSigned(event.Get<OperationalCredentials>());
    }
    else if (state.Is<CertificatesSigned>() && event.Is<OperationalCredentials>())
    {
        return mFactory.CreateInvokingAddTrustedRootCertificate(event.Get<OperationalCredentials>());
    }
    else if (state.Is<InvokingAddTrustedRootCertificate>() && event.Is<OperationalCredentials>())
    {
        return mFactory.CreateInvokingAddNOC(event.Get<OperationalCredentials>());
    }
    else if (state.Is<InvokingAddNOC>() && event.Is<OperationalCredentials>())
    {
        return mFactory.CreateOpCredsWritten();
    }
    else if (state.Is<OpCredsWritten>() && event.Is<InitiateNetworkConfiguration>())
    {
        return mFactory.CreateReadingNetworkFeatureMap();
    }
    else if (state.Is<OpCredsWritten>() && event.Is<SkipNetworkConfiguration>())
    {
        return mFactory.CreateNetworkEnabled();
    }
    else if (state.Is<ReadingNetworkFeatureMap>() && event.Is<NetworkFeatureMap>())
    {
        return mFactory.CreateNetworkFeatureMapRead(event.Get<NetworkFeatureMap>());
    }
    else if (state.Is<NetworkFeatureMapRead>() && event.Is<AddOrUpdateWiFiNetwork>())
    {
        return mFactory.CreateInvokingAddOrUpdateWiFiNetwork(event.Get<AddOrUpdateWiFiNetwork>());
    }
    else if (state.Is<NetworkFeatureMapRead>() && event.Is<AddOrUpdateThreadNetwork>())
    {
        return mFactory.CreateInvokingAddOrUpdateThreadNetwork(event.Get<AddOrUpdateThreadNetwork>());
    }
    else if ((state.Is<InvokingAddOrUpdateWiFiNetwork>() || state.Is<InvokingAddOrUpdateThreadNetwork>()) && event.Is<NetworkId>())
    {
        return mFactory.CreateNetworkAdded(event.Get<NetworkId>());
    }
    else if (state.Is<NetworkAdded>() && event.Is<NetworkId>())
    {
        return mFactory.CreateInvokingConnectNetwork(event.Get<NetworkId>());
    }
    else if (state.Is<InvokingConnectNetwork>() && event.Is<Success>())
    {
        return mFactory.CreateNetworkEnabled();
    }
    else if (state.Is<NetworkEnabled>() && event.Is<InitiateOperationalDiscovery>())
    {
        return mFactory.CreateOperationalDiscovery();
    }
    else if (state.Is<OperationalDiscovery>() && event.Is<OperationalRecord>())
    {
        return mFactory.CreateInitiatingCase(event.Get<OperationalRecord>());
    }
    else if (state.Is<InitiatingCase>() && event.Is<Success>())
    {
        return mFactory.CreateCaseComplete();
    }
    else if (state.Is<CaseComplete>() && event.Is<InvokeCommissioningComplete>())
    {
        return mFactory.CreateInvokingCommissioningComplete();
    }
    else if (state.Is<InvokingCommissioningComplete>() &&
             (event.Is<Success>() || event.Is<Failure>())) // TODO(#13327): accept failure; until #13327 is resolved, commissionees will return CommissioningComplete instead of CommissioningCompleteResponse
    {
        return mFactory.CreateCommissioningComplete();
    }
    else if (event.Is<Failure>())
    {
        this->mCtx.CancelTimeout();
        return mFactory.CreateFailed();
    }
    else if (event.Is<Shutdown>())
    {
        this->mCtx.CancelTimeout();
        return mFactory.CreateIdle();
    }
    else
    {
        return {};
    }
}

void ExampleCommissioningStateMachine::Init(SystemState * systemState, OpCredsIssuer * issuer, FabricIndex fabricIndex,
                                            NodeId nodeId, ByteSpan operationalDataset, ByteSpan ssid, ByteSpan wiFiCredentials)
{
    mTransitions.mFactory.Init(issuer, fabricIndex, nodeId, operationalDataset, ssid, wiFiCredentials);
    mCommissionee.Init(systemState);
}

void ExampleCommissioningStateMachine::Init(SystemState * systemState, OpCredsIssuer * issuer, FabricIndex fabricIndex,
                                            NodeId nodeId)
{
    Init(systemState, issuer, fabricIndex, nodeId, ByteSpan{}, ByteSpan{}, ByteSpan{});
}

void ExampleCommissioningStateMachine::Shutdown()
{
    this->Dispatch(Event::Create<chip::Commissioner::ExampleCommissioningStateMachine::Shutdown>());
    mCommissionee.Shutdown();
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(Event event, OnSuccess onSuccess, OnFailure onFailure)
{
    if (!GetState().Is<Idle>())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mTransitions.mFactory.SetCallbacks(onSuccess, onFailure);
    this->Dispatch(event);
    return GetState().Is<Idle>() ? CHIP_ERROR_INCORRECT_STATE : CHIP_NO_ERROR;
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(const char * onboardingPayload, OnSuccess onSuccess, OnFailure onFailure)
{
    return Commission(Event::Create<OnboardingPayload>(OnboardingPayload{ onboardingPayload }), onSuccess, onFailure);
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(chip::SetupPayload & onboardingPayload, OnSuccess onSuccess,
                                                        OnFailure onFailure)
{
    auto allocatedPayload = Platform::MakeShared<chip::SetupPayload>(onboardingPayload);
    if (allocatedPayload.get() == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return Commission(Event::Create<decltype(allocatedPayload)>(allocatedPayload), onSuccess, onFailure);
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(RendezvousInformationFlags flags, uint16_t discriminator,
                                                        uint32_t setUpPINCode, OnSuccess onSuccess, OnFailure onFailure)
{
    chip::SetupPayload onboardingPayload;
    onboardingPayload.rendezvousInformation = flags;
    onboardingPayload.discriminator         = discriminator;
    onboardingPayload.setUpPINCode          = setUpPINCode;
    return Commission(onboardingPayload, onSuccess, onFailure);
}

CHIP_ERROR ExampleCommissioningStateMachine::GrabCommissionee(chip::Controller::DeviceController & controller)
{
    VerifyOrReturnError(mCommissionee.mOperationalAddress.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mCommissionee.mOperationalId.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    controller.EmplaceOperationalDevice(mCommissionee.mOperationalAddress.Value(), mCommissionee.mMrpConfig,
                                        mCommissionee.mOperationalId.Value(), mCommissionee.mCaseSession);
    mCommissionee.mCaseSession.Release();
    return CHIP_NO_ERROR;
}

void ExampleCommissioningStateMachine::DispatchTimeout(System::Layer * aLayer, void * appState)
{
    ExampleCommissioningStateMachine * instance = static_cast<ExampleCommissioningStateMachine *>(appState);
    instance->Dispatch(Event::Create<Timeout>());
}

CHIP_ERROR ExampleCommissioningStateMachine::ScheduleTimeout(System::Clock::Timeout aDelay)
{
    VerifyOrReturnError(mCommissionee.mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mCommissionee.mSystemState->SystemLayer()->StartTimer(aDelay, DispatchTimeout, this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleCommissioningStateMachine::CancelTimeout()
{
    VerifyOrReturnError(mCommissionee.mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissionee.mSystemState->SystemLayer()->CancelTimer(DispatchTimeout, this);
    return CHIP_NO_ERROR;
}

} // namespace ExampleCommissioningStateMachine
} // namespace Commissioner
} // namespace chip
