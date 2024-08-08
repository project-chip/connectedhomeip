/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/****************************************************************************'
 * @file
 * @brief Implementation for the Operational State Server Cluster
 ***************************************************************************/
#include "operational-state-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::OperationalState::Attributes;

using Status = Protocols::InteractionModel::Status;

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId), AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId),
    mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(aClusterId)
{
    mDelegate->SetInstance(this);
    mCountdownTime.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
}

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId) : Instance(aDelegate, aEndpointId, OperationalState::Id) {}

Instance::~Instance()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, mClusterId))
    {
        ChipLogError(Zcl, "Operational State: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase)
{
    if (!aPhase.IsNull())
    {
        if (!IsSupportedPhase(aPhase.Value()))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    DataModel::Nullable<uint8_t> oldPhase = mCurrentPhase;
    mCurrentPhase                         = aPhase;
    if (mCurrentPhase != oldPhase)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CurrentPhase::Id);
        UpdateCountdownTimeFromClusterLogic();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOperationalState(uint8_t aOpState)
{
    // Error is only allowed to be set by OnOperationalErrorDetected.
    if (aOpState == to_underlying(OperationalStateEnum::kError) || !IsSupportedOperationalState(aOpState))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool countdownTimeUpdateNeeded = false;
    if (mOperationalError.errorStateID != to_underlying(ErrorStateEnum::kNoError))
    {
        mOperationalError.Set(to_underlying(ErrorStateEnum::kNoError));
        countdownTimeUpdateNeeded = true;
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::OperationalError::Id);
    }

    uint8_t oldState  = mOperationalState;
    mOperationalState = aOpState;
    if (mOperationalState != oldState)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::OperationalState::Id);
        countdownTimeUpdateNeeded = true;
    }

    if (countdownTimeUpdateNeeded)
    {
        UpdateCountdownTimeFromClusterLogic();
    }
    return CHIP_NO_ERROR;
}

DataModel::Nullable<uint8_t> Instance::GetCurrentPhase() const
{
    return mCurrentPhase;
}

uint8_t Instance::GetCurrentOperationalState() const
{
    return mOperationalState;
}

void Instance::GetCurrentOperationalError(GenericOperationalError & error) const
{
    error.Set(mOperationalError.errorStateID, mOperationalError.errorStateLabel, mOperationalError.errorStateDetails);
}

void Instance::OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError)
{
    ChipLogDetail(Zcl, "OperationalStateServer: OnOperationalErrorDetected");
    // Set the OperationalState attribute to Error
    if (mOperationalState != to_underlying(OperationalStateEnum::kError))
    {
        mOperationalState = to_underlying(OperationalStateEnum::kError);
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::OperationalState::Id);
    }

    // Set the OperationalError attribute
    if (!mOperationalError.IsEqual(aError))
    {
        mOperationalError.Set(aError.errorStateID, aError.errorStateLabel, aError.errorStateDetails);
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::OperationalError::Id);
    }

    UpdateCountdownTimeFromClusterLogic();

    // Generate an ErrorDetected event
    GenericErrorEvent event(mClusterId, aError);
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, mEndpointId, eventNumber);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OperationalStateServer: Failed to record OperationalError event: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void Instance::OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                             const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime,
                                             const Optional<DataModel::Nullable<uint32_t>> & aPausedTime)
{
    ChipLogDetail(Zcl, "OperationalStateServer: OnOperationCompletionDetected");

    GenericOperationCompletionEvent event(mClusterId, aCompletionErrorCode, aTotalOperationalTime, aPausedTime);
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, mEndpointId, eventNumber);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OperationalStateServer: Failed to record OperationCompletion event: %" CHIP_ERROR_FORMAT,
                     error.Format());
    }

    UpdateCountdownTimeFromClusterLogic();
}

void Instance::ReportOperationalStateListChange()
{
    MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, mClusterId, Attributes::OperationalStateList::Id));
}

void Instance::ReportPhaseListChange()
{
    MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, mClusterId, Attributes::PhaseList::Id));
    UpdateCountdownTimeFromClusterLogic();
}

void Instance::UpdateCountdownTime(bool fromDelegate)
{
    app::DataModel::Nullable<uint32_t> newCountdownTime = mDelegate->GetCountdownTime();
    auto now                                            = System::SystemClock().GetMonotonicTimestamp();

    bool markDirty = false;

    if (fromDelegate)
    {
        // Updates from delegate are reduce-reported to every 10s max (choice of this implementation), in addition
        // to default change-from-null, change-from-zero and increment policy.
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate & candidate) -> bool {
            if (candidate.lastDirtyValue.IsNull() || candidate.newValue.IsNull())
            {
                return false;
            }

            uint32_t lastDirtyValue           = candidate.lastDirtyValue.Value();
            uint32_t newValue                 = candidate.newValue.Value();
            uint32_t kNumSecondsDeltaToReport = 10;
            return (newValue < lastDirtyValue) && ((lastDirtyValue - newValue) > kNumSecondsDeltaToReport);
        };
        markDirty = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }
    else
    {
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate &) -> bool { return true; };
        markDirty      = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }

    if (markDirty)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CountdownTime::Id);
    }
}

bool Instance::IsSupportedPhase(uint8_t aPhase)
{
    char buffer[kMaxPhaseNameLength];
    MutableCharSpan phase(buffer);
    if (mDelegate->GetOperationalPhaseAtIndex(aPhase, phase) != CHIP_ERROR_NOT_FOUND)
    {
        return true;
    }
    return false;
}

bool Instance::IsSupportedOperationalState(uint8_t aState)
{
    GenericOperationalState opState;
    for (uint8_t i = 0; mDelegate->GetOperationalStateAtIndex(i, opState) != CHIP_ERROR_NOT_FOUND; i++)
    {
        if (opState.operationalStateID == aState)
        {
            return true;
        }
    }
    ChipLogDetail(Zcl, "Cannot find an operational state with value %u", aState);
    return false;
}

// private

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "OperationalState: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::Pause::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Pause state");

        HandleCommand<Commands::Pause::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandlePauseState(ctx, req); });
        break;

    case Commands::Resume::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Resume state");

        HandleCommand<Commands::Resume::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleResumeState(ctx, req); });
        break;

    case Commands::Start::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Start state");

        HandleCommand<Commands::Start::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleStartState(ctx, req); });
        break;

    case Commands::Stop::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Stop state");

        HandleCommand<Commands::Stop::DecodableType>(handlerContext,
                                                     [this](HandlerContext & ctx, const auto & req) { HandleStopState(ctx, req); });
        break;
    default:
        ChipLogDetail(Zcl, "OperationalState: Entering handling derived cluster commands");

        InvokeDerivedClusterCommand(handlerContext);
        break;
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "OperationalState: Reading");
    switch (aPath.mAttributeId)
    {
    case OperationalState::Attributes::OperationalStateList::Id: {
        return aEncoder.EncodeList([delegate = mDelegate](const auto & encoder) -> CHIP_ERROR {
            GenericOperationalState opState;
            size_t index   = 0;
            CHIP_ERROR err = CHIP_NO_ERROR;
            while ((err = delegate->GetOperationalStateAtIndex(index, opState)) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encoder.Encode(opState));
                index++;
            }
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                return CHIP_NO_ERROR;
            }
            return err;
        });
        break;
    }

    case OperationalState::Attributes::OperationalState::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetCurrentOperationalState()));
        break;
    }

    case OperationalState::Attributes::OperationalError::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mOperationalError));
        break;
    }

    case OperationalState::Attributes::PhaseList::Id: {

        char buffer[kMaxPhaseNameLength];
        MutableCharSpan phase(buffer);
        size_t index = 0;

        if (mDelegate->GetOperationalPhaseAtIndex(index, phase) == CHIP_ERROR_NOT_FOUND)
        {
            return aEncoder.EncodeNull();
        }

        return aEncoder.EncodeList([delegate = mDelegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                char buffer2[kMaxPhaseNameLength];
                MutableCharSpan phase2(buffer2);
                auto err = delegate->GetOperationalPhaseAtIndex(i, phase2);
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(phase2));
            }
        });
        break;
    }

    case OperationalState::Attributes::CurrentPhase::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetCurrentPhase()));
        break;
    }

    case OperationalState::Attributes::CountdownTime::Id: {
        // Read through to get value closest to reality.
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetCountdownTime()));
        break;
    }
    }
    return CHIP_NO_ERROR;
}

void Instance::HandlePauseState(HandlerContext & ctx, const Commands::Pause::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandlePauseState");

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    // Handle Operational State Pause-incompatible states.
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }

    // Handle Pause-incompatible states for derived clusters.
    if (opState >= DerivedClusterNumberSpaceStart && opState < VendorNumberSpaceStart)
    {
        if (!IsDerivedClusterStatePauseCompatible(opState))
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        }
    }

    // If the error is still NoError, we can call the delegate's handle function.
    // If the current state is Paused we can skip this call.
    if (err.errorStateID == 0 && opState != to_underlying(OperationalStateEnum::kPaused))
    {
        mDelegate->HandlePauseStateCallback(err);
    }

    Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void Instance::HandleStopState(HandlerContext & ctx, const Commands::Stop::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStopState");

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState != to_underlying(OperationalStateEnum::kStopped))
    {
        mDelegate->HandleStopStateCallback(err);
    }

    Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void Instance::HandleStartState(HandlerContext & ctx, const Commands::Start::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStartState");

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState != to_underlying(OperationalStateEnum::kRunning))
    {
        mDelegate->HandleStartStateCallback(err);
    }

    Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void Instance::HandleResumeState(HandlerContext & ctx, const Commands::Resume::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleResumeState");

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    // Handle Operational State Resume-incompatible states.
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }

    // Handle Resume-incompatible states for derived clusters.
    if (opState >= DerivedClusterNumberSpaceStart && opState < VendorNumberSpaceStart)
    {
        if (!IsDerivedClusterStateResumeCompatible(opState))
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        }
    }

    // If the error is still NoError, we can call the delegate's handle function.
    // If the current state is Running we can skip this call.
    if (err.errorStateID == 0 && opState != to_underlying(OperationalStateEnum::kRunning))
    {
        mDelegate->HandleResumeStateCallback(err);
    }

    Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

// RvcOperationalState

bool RvcOperationalState::Instance::IsDerivedClusterStatePauseCompatible(uint8_t aState)
{
    return aState == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
}

bool RvcOperationalState::Instance::IsDerivedClusterStateResumeCompatible(uint8_t aState)
{
    return (aState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
            aState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
}

// This function is called by the base operational state cluster when a command in the derived cluster number-space is received.
void RvcOperationalState::Instance::InvokeDerivedClusterCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "RvcOperationalState: InvokeDerivedClusterCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case RvcOperationalState::Commands::GoHome::Id:
        ChipLogDetail(Zcl, "RvcOperationalState: Entering handling GoHome command");

        CommandHandlerInterface::HandleCommand<Commands::GoHome::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleGoHomeCommand(ctx, req); });
        break;
    }
}

void RvcOperationalState::Instance::HandleGoHomeCommand(HandlerContext & ctx, const Commands::GoHome::DecodableType & req)
{
    ChipLogDetail(Zcl, "RvcOperationalState: HandleGoHomeCommand");

    GenericOperationalError err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    // Handle the case of the device being in an invalid state
    if (opState == to_underlying(OperationalStateEnum::kCharging) || opState == to_underlying(OperationalStateEnum::kDocked))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
    }

    if (err.errorStateID == 0 && opState != to_underlying(OperationalStateEnum::kSeekingCharger))
    {
        mDelegate->HandleGoHomeCommandCallback(err);
    }

    Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}
