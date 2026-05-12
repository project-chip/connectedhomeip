/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include "OperationalStateCluster.h"
#include <app/data-model/Decode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OperationalState/Metadata.h>
#include <clusters/OvenCavityOperationalState/Metadata.h>
#include <clusters/RvcOperationalState/Metadata.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;

using Status = Protocols::InteractionModel::Status;

// ---------------------------------------------------------------------------
// OperationalStateCluster — constructors
// ---------------------------------------------------------------------------

OperationalStateCluster::OperationalStateCluster(EndpointId endpointId, Delegate * delegate, const Config & config) :
    OperationalStateCluster(endpointId, OperationalState::Id, OperationalState::kRevision, delegate, config)
{}

OperationalStateCluster::OperationalStateCluster(EndpointId endpointId, ClusterId clusterId, uint32_t revision, Delegate * delegate,
                                                 const Config & config) :
    DefaultServerCluster({ endpointId, clusterId }),
    mDelegate(delegate), mRevision(revision), mConfig(config)
{
    mCountdownTime.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
}

OperationalStateCluster::~OperationalStateCluster() = default;

// ---------------------------------------------------------------------------
// Attribute setters / getters
// ---------------------------------------------------------------------------

CHIP_ERROR OperationalStateCluster::SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase)
{
    if (!aPhase.IsNull() && !IsSupportedPhase(aPhase.Value()))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    bool changed = SetAttributeValue(mCurrentPhase, aPhase, OperationalState::Attributes::CurrentPhase::Id);
    if (changed)
    {
        UpdateCountdownTimeFromClusterLogic();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateCluster::SetOperationalState(uint8_t aOpState)
{
    if (aOpState == to_underlying(OperationalStateEnum::kError) || !IsSupportedOperationalState(aOpState))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool countdownUpdateNeeded = false;

    if (mOperationalError.errorStateID != to_underlying(ErrorStateEnum::kNoError))
    {
        mOperationalError.Set(to_underlying(ErrorStateEnum::kNoError));
        countdownUpdateNeeded = true;
        NotifyAttributeChanged(OperationalState::Attributes::OperationalError::Id);
    }

    bool stateChanged = SetAttributeValue(mOperationalState, aOpState, OperationalState::Attributes::OperationalState::Id);
    if (stateChanged)
    {
        countdownUpdateNeeded = true;
    }

    if (countdownUpdateNeeded)
    {
        UpdateCountdownTimeFromClusterLogic();
    }
    return CHIP_NO_ERROR;
}

void OperationalStateCluster::GetCurrentOperationalError(GenericOperationalError & error) const
{
    error.Set(mOperationalError.errorStateID, mOperationalError.errorStateLabel, mOperationalError.errorStateDetails);
}

void OperationalStateCluster::OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: OnOperationalErrorDetected");

    if (mOperationalState != to_underlying(OperationalStateEnum::kError))
    {
        mOperationalState = to_underlying(OperationalStateEnum::kError);
        NotifyAttributeChanged(OperationalState::Attributes::OperationalState::Id);
    }

    if (!mOperationalError.IsEqual(aError))
    {
        mOperationalError.Set(aError.errorStateID, aError.errorStateLabel, aError.errorStateDetails);
        NotifyAttributeChanged(OperationalState::Attributes::OperationalError::Id);
    }

    UpdateCountdownTimeFromClusterLogic();

    if (mContext == nullptr)
    {
        ChipLogError(Zcl, "OperationalStateCluster: cannot emit OperationalError event, no context");
        return;
    }
    GenericErrorEvent event(mPath.mClusterId, aError);
    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value())
    {
        ChipLogError(Zcl, "OperationalStateCluster: failed to emit OperationalError event");
    }
}

void OperationalStateCluster::OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                                            const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime,
                                                            const Optional<DataModel::Nullable<uint32_t>> & aPausedTime)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: OnOperationCompletionDetected");

    if (mContext == nullptr)
    {
        ChipLogError(Zcl, "OperationalStateCluster: cannot emit OperationCompletion event, no context");
        return;
    }
    GenericOperationCompletionEvent event(mPath.mClusterId, aCompletionErrorCode, aTotalOperationalTime, aPausedTime);
    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value())
    {
        ChipLogError(Zcl, "OperationalStateCluster: failed to emit OperationCompletion event");
    }

    UpdateCountdownTimeFromClusterLogic();
}

void OperationalStateCluster::ReportOperationalStateListChange()
{
    NotifyAttributeChanged(OperationalState::Attributes::OperationalStateList::Id);
}

void OperationalStateCluster::ReportPhaseListChange()
{
    NotifyAttributeChanged(OperationalState::Attributes::PhaseList::Id);
    UpdateCountdownTimeFromClusterLogic();
}

void OperationalStateCluster::UpdateCountdownTime(bool fromDelegate)
{
    DataModel::Nullable<uint32_t> newCountdownTime = mDelegate->GetCountdownTime();
    auto now                                       = System::SystemClock().GetMonotonicTimestamp();
    bool markDirty                                 = false;

    if (fromDelegate)
    {
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate & candidate) -> bool {
            if (candidate.lastDirtyValue.IsNull() || candidate.newValue.IsNull())
            {
                return false;
            }
            uint32_t lastDirtyValue           = candidate.lastDirtyValue.Value();
            uint32_t newValue                 = candidate.newValue.Value();
            constexpr uint32_t kDeltaToReport = 10;
            return (newValue < lastDirtyValue) && ((lastDirtyValue - newValue) > kDeltaToReport);
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
        NotifyAttributeChanged(OperationalState::Attributes::CountdownTime::Id);
    }
}

bool OperationalStateCluster::IsSupportedPhase(uint8_t aPhase)
{
    char buffer[kMaxPhaseNameLength];
    MutableCharSpan phase(buffer);
    return mDelegate->GetOperationalPhaseAtIndex(aPhase, phase) == CHIP_NO_ERROR;
}

bool OperationalStateCluster::IsSupportedOperationalState(uint8_t aState)
{
    GenericOperationalState opState;
    for (uint8_t i = 0; mDelegate->GetOperationalStateAtIndex(i, opState) == CHIP_NO_ERROR; i++)
    {
        if (opState.operationalStateID == aState)
        {
            return true;
        }
    }
    ChipLogDetail(Zcl, "OperationalStateCluster: no operational state with value %u", aState);
    return false;
}

// ---------------------------------------------------------------------------
// ReadAttribute
// ---------------------------------------------------------------------------

DataModel::ActionReturnStatus OperationalStateCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    using namespace OperationalState::Attributes;
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(static_cast<uint16_t>(mRevision));
    case FeatureMap::Id:
        return encoder.Encode(uint32_t(0));
    case PhaseList::Id: {
        char firstBuf[kMaxPhaseNameLength];
        MutableCharSpan firstPhase(firstBuf);
        if (mDelegate->GetOperationalPhaseAtIndex(0, firstPhase) == CHIP_ERROR_NOT_FOUND)
        {
            return encoder.EncodeNull();
        }
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            constexpr uint8_t kMaxPhaseCount = 32; // spec §1.14.5.1: max 32 entries
            for (uint8_t i = 0; i < kMaxPhaseCount; i++)
            {
                char buf[kMaxPhaseNameLength];
                MutableCharSpan phase(buf);
                CHIP_ERROR err = mDelegate->GetOperationalPhaseAtIndex(i, phase);
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(listEncoder.Encode(phase));
            }
            return CHIP_NO_ERROR;
        });
    }
    case CurrentPhase::Id:
        return encoder.Encode(mCurrentPhase);
    case CountdownTime::Id:
        return encoder.Encode(mDelegate->GetCountdownTime());
    case OperationalStateList::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            GenericOperationalState opState;
            for (size_t i = 0;; i++)
            {
                CHIP_ERROR err = mDelegate->GetOperationalStateAtIndex(i, opState);
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(listEncoder.Encode(opState));
            }
        });
    case Attributes::OperationalState::Id:
        return encoder.Encode(mOperationalState);
    case OperationalError::Id:
        return encoder.Encode(mOperationalError);
    default:
        return Status::UnsupportedAttribute;
    }
}

// ---------------------------------------------------------------------------
// Attributes
// ---------------------------------------------------------------------------

CHIP_ERROR OperationalStateCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    static constexpr DataModel::AttributeEntry kOptional[] = {
        OperationalState::Attributes::CountdownTime::kMetadataEntry,
    };

    return listBuilder.Append(Span(OperationalState::Attributes::kMandatoryMetadata), Span(kOptional), mConfig.optionalAttributes);
}

// ---------------------------------------------------------------------------
// AcceptedCommands — base OperationalState cluster
// ---------------------------------------------------------------------------

CHIP_ERROR OperationalStateCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        OperationalState::Commands::Pause::kMetadataEntry,
        OperationalState::Commands::Stop::kMetadataEntry,
        OperationalState::Commands::Start::kMetadataEntry,
        OperationalState::Commands::Resume::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

CHIP_ERROR OperationalStateCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    // OperationalCommandResponse (0x04) is sent in response to every accepted command.
    // RVC and OvenCavity derived clusters share the same command ID value.
    static constexpr CommandId kGeneratedCommands[] = { OperationalState::Commands::OperationalCommandResponse::Id };
    return builder.ReferenceExisting(kGeneratedCommands);
}

// ---------------------------------------------------------------------------
// InvokeCommand
// ---------------------------------------------------------------------------

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                    chip::TLV::TLVReader & input_arguments,
                                                                                    CommandHandler * handler)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: InvokeCommand 0x%08" PRIx32, request.path.mCommandId);
    switch (request.path.mCommandId)
    {
    case OperationalState::Commands::Pause::Id:
        return HandlePauseState(request, input_arguments, handler);
    case OperationalState::Commands::Stop::Id:
        return HandleStopState(request, input_arguments, handler);
    case OperationalState::Commands::Start::Id:
        return HandleStartState(request, input_arguments, handler);
    case OperationalState::Commands::Resume::Id:
        return HandleResumeState(request, input_arguments, handler);
    default:
        return HandleDerivedClusterCommand(request, input_arguments, handler);
    }
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandlePauseState(const DataModel::InvokeRequest & request,
                                                                                       chip::TLV::TLVReader & input,
                                                                                       CommandHandler * handler)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: HandlePauseState");

    OperationalState::Commands::Pause::DecodableType req;
    if (DataModel::Decode(input, req) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else if (opState >= DerivedClusterNumberSpaceStart && opState < VendorNumberSpaceStart)
    {
        if (!IsDerivedClusterStatePauseCompatible(opState))
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        }
    }

    if (err.errorStateID == 0 && opState != to_underlying(OperationalStateEnum::kPaused))
    {
        mDelegate->HandlePauseStateCallback(err);
    }

    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandleStopState(const DataModel::InvokeRequest & request,
                                                                                      chip::TLV::TLVReader & input,
                                                                                      CommandHandler * handler)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: HandleStopState");

    OperationalState::Commands::Stop::DecodableType req;
    if (DataModel::Decode(input, req) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    if (GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kStopped))
    {
        mDelegate->HandleStopStateCallback(err);
    }

    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandleStartState(const DataModel::InvokeRequest & request,
                                                                                       chip::TLV::TLVReader & input,
                                                                                       CommandHandler * handler)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: HandleStartState");

    OperationalState::Commands::Start::DecodableType req;
    if (DataModel::Decode(input, req) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    if (GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kRunning))
    {
        mDelegate->HandleStartStateCallback(err);
    }

    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandleResumeState(const DataModel::InvokeRequest & request,
                                                                                        chip::TLV::TLVReader & input,
                                                                                        CommandHandler * handler)
{
    ChipLogDetail(Zcl, "OperationalStateCluster: HandleResumeState");

    OperationalState::Commands::Resume::DecodableType req;
    if (DataModel::Decode(input, req) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else if (opState >= DerivedClusterNumberSpaceStart && opState < VendorNumberSpaceStart)
    {
        if (!IsDerivedClusterStateResumeCompatible(opState))
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        }
    }

    if (err.errorStateID == 0 && opState != to_underlying(OperationalStateEnum::kRunning))
    {
        mDelegate->HandleResumeStateCallback(err);
    }

    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// RvcOperationalStateCluster
// ---------------------------------------------------------------------------

RvcOperationalState::RvcOperationalStateCluster::RvcOperationalStateCluster(
    EndpointId endpointId, RvcOperationalState::Delegate * delegate,
    const OperationalState::OperationalStateCluster::Config & config) :
    OperationalState::OperationalStateCluster(endpointId, RvcOperationalState::Id, RvcOperationalState::kRevision,
                                              static_cast<OperationalState::Delegate *>(delegate), config),
    mRvcDelegate(delegate)
{}

CHIP_ERROR
RvcOperationalState::RvcOperationalStateCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        RvcOperationalState::Commands::Pause::kMetadataEntry,
        RvcOperationalState::Commands::Resume::kMetadataEntry,
        RvcOperationalState::Commands::GoHome::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

bool RvcOperationalState::RvcOperationalStateCluster::IsDerivedClusterStatePauseCompatible(uint8_t aState)
{
    return aState == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
}

bool RvcOperationalState::RvcOperationalStateCluster::IsDerivedClusterStateResumeCompatible(uint8_t aState)
{
    return (aState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
            aState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
}

std::optional<DataModel::ActionReturnStatus>
RvcOperationalState::RvcOperationalStateCluster::HandleDerivedClusterCommand(const DataModel::InvokeRequest & request,
                                                                             chip::TLV::TLVReader & input, CommandHandler * handler)
{
    ChipLogDetail(Zcl, "RvcOperationalStateCluster: HandleDerivedClusterCommand 0x%08" PRIx32, request.path.mCommandId);
    switch (request.path.mCommandId)
    {
    case RvcOperationalState::Commands::GoHome::Id:
        return HandleGoHomeCommand(request, input, handler);
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus>
RvcOperationalState::RvcOperationalStateCluster::HandleGoHomeCommand(const DataModel::InvokeRequest & request,
                                                                     chip::TLV::TLVReader & input, CommandHandler * handler)
{
    ChipLogDetail(Zcl, "RvcOperationalStateCluster: HandleGoHomeCommand");

    RvcOperationalState::Commands::GoHome::DecodableType req;
    if (DataModel::Decode(input, req) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    OperationalState::GenericOperationalError err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
        opState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
    }

    if (err.errorStateID == 0 && opState != to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        mRvcDelegate->HandleGoHomeCommandCallback(err);
    }

    RvcOperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// OvenCavityOperationalStateCluster
// ---------------------------------------------------------------------------

OvenCavityOperationalState::OvenCavityOperationalStateCluster::OvenCavityOperationalStateCluster(
    EndpointId endpointId, OperationalState::Delegate * delegate,
    const OperationalState::OperationalStateCluster::Config & config) :
    OperationalState::OperationalStateCluster(endpointId, OvenCavityOperationalState::Id, OvenCavityOperationalState::kRevision,
                                              delegate, config)
{}

CHIP_ERROR OvenCavityOperationalState::OvenCavityOperationalStateCluster::AcceptedCommands(
    const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        OvenCavityOperationalState::Commands::Stop::kMetadataEntry,
        OvenCavityOperationalState::Commands::Start::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}
