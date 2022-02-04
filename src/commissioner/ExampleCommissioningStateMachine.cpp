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

void StateFactory::Init(OpCredsIssuer * issuer, FabricIndex fabricIndex, NodeId nodeId,
                        CommissioningParameters & commissioningParameters)
{
    mIssuer                  = issuer;
    mFabricIndex             = fabricIndex;
    mNodeId                  = nodeId;
    mCommissioningParameters = commissioningParameters;
}

void StateFactory::SetCallbacks(OnSuccess onSuccess, OnFailure onFailure)
{
    mOnSuccess = onSuccess;
    mOnFailure = onFailure;
}

chip::StateMachine::Optional<State> Transitions::operator()(const State & state, const Event & event)
{
    if (state.Is<AppStates::Idle>() && event.Is<SdkEvents::RawOnboardingPayload>())
    {
        return mFactory.CreateParsingOnboardingPayload(event.Get<SdkEvents::RawOnboardingPayload>());
    }
    else if ((state.Is<AppStates::Idle>() || state.Is<SdkStates::ParsingOnboardingPayload>()) &&
             event.Is<SdkEvents::OnboardingPayload>())
    {
        CHIP_ERROR err = mCtx.ScheduleTimeout(kCommissionableDiscoveryTimeout);
        return err == CHIP_NO_ERROR ? mFactory.CreateCommissionableNodeDiscovery(event.Get<SdkEvents::OnboardingPayload>())
                                    : mFactory.CreateFailed();
    }
    else if (state.Is<SdkStates::CommissionableNodeDiscovery>() && event.Is<AppEvents::Timeout>())
    {
        return mFactory.CreateAbortingCommissionableDiscovery(state.Get<SdkStates::CommissionableNodeDiscovery>());
    }
    else if (state.Is<SdkStates::CommissionableNodeDiscovery>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateInitiatingPase(state.Get<SdkStates::CommissionableNodeDiscovery>());
    }
    else if (state.Is<AppStates::AbortingCommissionableDiscovery>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateFailed();
    }
    else if (state.Is<SdkStates::InitiatingPase>() && event.Is<SdkEvents::Await>())
    {
        return mFactory.CreateAwaitingCommissionableDiscovery(state.Get<SdkStates::InitiatingPase>());
    }
    else if (state.Is<SdkStates::InitiatingPase>() && event.Is<AppEvents::Timeout>())
    {
        return mFactory.CreateFinishingPase(state.Get<SdkStates::InitiatingPase>());
    }
    else if (state.Is<SdkStates::InitiatingPase>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreatePaseComplete();
    }
    else if (state.Is<SdkStates::AwaitingCommissionableDiscovery>() && event.Is<AppEvents::Timeout>())
    {
        return mFactory.CreateAbortingCommissionableDiscovery(state.Get<SdkStates::AwaitingCommissionableDiscovery>());
    }
    else if (state.Is<SdkStates::AwaitingCommissionableDiscovery>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateInitiatingPase(state.Get<SdkStates::AwaitingCommissionableDiscovery>());
    }
    else if (state.Is<AppStates::FinishingPase>() && event.Is<SdkEvents::Success>())
    {
        this->mCtx.CancelTimeout();
        return mFactory.CreatePaseComplete();
    }
    else if (state.Is<AppStates::PaseComplete>() && event.Is<SdkEvents::ArmFailSafe>())
    {
        return mFactory.CreateInvokingArmFailSafe(event.Get<SdkEvents::ArmFailSafe>());
    }
    else if (state.Is<SdkStates::InvokingArmFailSafe>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateFailSafeArmed();
    }
    else if (state.Is<AppStates::FailSafeArmed>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateInvokingAttestationRequest(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<SdkStates::InvokingAttestationRequest>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateInvokingDacCertificateChainRequest(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<SdkStates::InvokingDacCertificateChainRequest>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateInvokingPaiCertificateChainRequest(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<SdkStates::InvokingPaiCertificateChainRequest>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateCapturingAttestationChallenge(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<SdkStates::CapturingAttestationChallenge>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateAttestationVerification(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<AppStates::AttestationVerification>() && event.Is<SdkEvents::AttestationInformation>())
    {
        return mFactory.CreateAttestationVerified(event.Get<SdkEvents::AttestationInformation>());
    }
    else if (state.Is<AppStates::AttestationVerified>() && event.Is<SdkEvents::NocsrInformation>())
    {
        return mFactory.CreateInvokingOpCSRRequest(event.Get<SdkEvents::NocsrInformation>());
    }
    else if (state.Is<SdkStates::InvokingOpCSRRequest>() && event.Is<SdkEvents::NocsrInformation>())
    {
        return mFactory.CreateOpCSRResponseReceived(event.Get<SdkEvents::NocsrInformation>());
    }
    else if (state.Is<AppStates::OpCSRResponseReceived>() && event.Is<SdkEvents::NocsrInformation>())
    {
        return mFactory.CreateSigningCertificates(event.Get<SdkEvents::NocsrInformation>());
    }
    else if (state.Is<AppStates::SigningCertificates>() && event.Is<SdkEvents::OperationalCredentials>())
    {
        return mFactory.CreateCertificatesSigned(event.Get<SdkEvents::OperationalCredentials>());
    }
    else if (state.Is<AppStates::CertificatesSigned>() && event.Is<SdkEvents::OperationalCredentials>())
    {
        return mFactory.CreateInvokingAddTrustedRootCertificate(event.Get<SdkEvents::OperationalCredentials>());
    }
    else if (state.Is<SdkStates::InvokingAddTrustedRootCertificate>() && event.Is<SdkEvents::OperationalCredentials>())
    {
        return mFactory.CreateInvokingAddNOC(event.Get<SdkEvents::OperationalCredentials>());
    }
    else if (state.Is<SdkStates::InvokingAddNOC>() && event.Is<SdkEvents::OperationalCredentials>())
    {
        return mFactory.CreateOpCredsWritten();
    }
    else if (state.Is<AppStates::OpCredsWritten>() && event.Is<AppEvents::InitiateNetworkConfiguration>())
    {
        return mFactory.CreateReadingNetworkFeatureMap();
    }
    else if (state.Is<AppStates::OpCredsWritten>() && event.Is<AppEvents::SkipNetworkConfiguration>())
    {
        return mFactory.CreateNetworkEnabled();
    }
    else if (state.Is<SdkStates::ReadingNetworkFeatureMap>() && event.Is<SdkEvents::NetworkFeatureMap>())
    {
        return mFactory.CreateNetworkFeatureMapRead(event.Get<SdkEvents::NetworkFeatureMap>());
    }
    else if (state.Is<AppStates::NetworkFeatureMapRead>() && event.Is<SdkEvents::AddOrUpdateWiFiNetwork>())
    {
        return mFactory.CreateInvokingAddOrUpdateWiFiNetwork(event.Get<SdkEvents::AddOrUpdateWiFiNetwork>());
    }
    else if (state.Is<AppStates::NetworkFeatureMapRead>() && event.Is<SdkEvents::AddOrUpdateThreadNetwork>())
    {
        return mFactory.CreateInvokingAddOrUpdateThreadNetwork(event.Get<SdkEvents::AddOrUpdateThreadNetwork>());
    }
    else if ((state.Is<SdkStates::InvokingAddOrUpdateWiFiNetwork>() || state.Is<SdkStates::InvokingAddOrUpdateThreadNetwork>()) &&
             event.Is<SdkEvents::NetworkId>())
    {
        return mFactory.CreateNetworkAdded(event.Get<SdkEvents::NetworkId>());
    }
    else if (state.Is<AppStates::NetworkAdded>() && event.Is<SdkEvents::NetworkId>())
    {
        return mFactory.CreateInvokingConnectNetwork(event.Get<SdkEvents::NetworkId>());
    }
    else if (state.Is<SdkStates::InvokingConnectNetwork>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateNetworkEnabled();
    }
    else if (state.Is<AppStates::NetworkEnabled>() && event.Is<AppEvents::InitiateOperationalDiscovery>())
    {
        return mFactory.CreateOperationalDiscovery();
    }
    else if (state.Is<SdkStates::OperationalDiscovery>() && event.Is<SdkEvents::OperationalRecord>())
    {
        return mFactory.CreateInitiatingCase(event.Get<SdkEvents::OperationalRecord>());
    }
    else if (state.Is<SdkStates::InitiatingCase>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateCaseComplete();
    }
    else if (state.Is<AppStates::CaseComplete>() && event.Is<AppEvents::InvokeCommissioningComplete>())
    {
        return mFactory.CreateInvokingCommissioningComplete();
    }
    else if (state.Is<SdkStates::InvokingCommissioningComplete>() && event.Is<SdkEvents::Success>())
    {
        return mFactory.CreateCommissioningComplete();
    }
    else if (event.Is<SdkEvents::Failure>())
    {
        this->mCtx.CancelTimeout();
        return mFactory.CreateFailed();
    }
    else if (event.Is<AppEvents::Shutdown>())
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
                                            NodeId nodeId, CommissioningParameters commissioningParameeters)
{
    mTransitions.mFactory.Init(issuer, fabricIndex, nodeId, commissioningParameeters);
    mCommissionee.Init(systemState);
}

void ExampleCommissioningStateMachine::Shutdown()
{
    this->Dispatch(Event::Create<AppEvents::Shutdown>());
    mCommissionee.Shutdown();
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(Event event, OnSuccess onSuccess, OnFailure onFailure)
{
    if (!GetState().Is<AppStates::Idle>())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mTransitions.mFactory.SetCallbacks(onSuccess, onFailure);
    this->Dispatch(event);
    return GetState().Is<AppStates::Idle>() ? CHIP_ERROR_INCORRECT_STATE : CHIP_NO_ERROR;
}

CHIP_ERROR ExampleCommissioningStateMachine::Commission(const char * onboardingPayload, OnSuccess onSuccess, OnFailure onFailure)
{
    return Commission(Event::Create<SdkEvents::RawOnboardingPayload>(SdkEvents::RawOnboardingPayload{ onboardingPayload }),
                      onSuccess, onFailure);
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
    instance->Dispatch(Event::Create<AppEvents::Timeout>());
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
