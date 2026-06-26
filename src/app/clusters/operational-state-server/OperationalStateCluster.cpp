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

namespace {
// operationalStateID is a uint8_t, so there can be at most 256 distinct states. This bound also
// guards the delegate-iteration loops against a buggy delegate that never returns CHIP_ERROR_NOT_FOUND.
constexpr size_t kMaxOperationalStateCount = 256;
} // namespace

// ---------------------------------------------------------------------------
// OperationalStateCluster — constructors
// ---------------------------------------------------------------------------

OperationalStateCluster::OperationalStateCluster(EndpointId endpointId, Delegate * delegate, const Config & config) :
    OperationalStateCluster(endpointId, OperationalState::Id, OperationalState::kRevision, delegate, config)
{}

OperationalStateCluster::OperationalStateCluster(EndpointId endpointId, ClusterId clusterId, uint16_t revision, Delegate * delegate,
                                                 const Config & config) :
    DefaultServerCluster({ endpointId, clusterId }),
    mDelegate(delegate), mRevision(revision), mConfig(config)
{
    mCountdownTime.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
}

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

    VerifyOrReturn(mContext != nullptr);
    GenericErrorEvent event(mPath.mClusterId, aError);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void OperationalStateCluster::OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                                            const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime,
                                                            const Optional<DataModel::Nullable<uint32_t>> & aPausedTime)
{
    VerifyOrReturn(mContext != nullptr);
    GenericOperationCompletionEvent event(mPath.mClusterId, aCompletionErrorCode, aTotalOperationalTime, aPausedTime);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);

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
        // When the update is coming from the cluster logic (e.g. due to a state change),
        // we should report if the value has changed since the last report.
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate & candidate) -> bool {
            return candidate.lastDirtyValue != candidate.newValue;
        };
        markDirty = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
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
    for (size_t i = 0; i < kMaxOperationalStateCount && mDelegate->GetOperationalStateAtIndex(i, opState) == CHIP_NO_ERROR; i++)
    {
        if (opState.operationalStateID == aState)
        {
            return true;
        }
    }
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
        return encoder.Encode(mRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
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
            for (size_t i = 0; i < kMaxOperationalStateCount; i++)
            {
                CHIP_ERROR err = mDelegate->GetOperationalStateAtIndex(i, opState);
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(listEncoder.Encode(opState));
            }
            return CHIP_NO_ERROR;
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
    static_assert(OperationalState::Commands::OperationalCommandResponse::Id ==
                      RvcOperationalState::Commands::OperationalCommandResponse::Id,
                  "Derived clusters must share the same command ID for OperationalCommandResponse");
    static_assert(OperationalState::Commands::OperationalCommandResponse::Id ==
                      OvenCavityOperationalState::Commands::OperationalCommandResponse::Id,
                  "Derived clusters must share the same command ID for OperationalCommandResponse");

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
    switch (request.path.mCommandId)
    {
    case OperationalState::Commands::Pause::Id:
        return HandlePauseOrResumeState(request.path, input_arguments, handler, PauseOrResume::kPause);
    case OperationalState::Commands::Resume::Id:
        return HandlePauseOrResumeState(request.path, input_arguments, handler, PauseOrResume::kResume);
    case OperationalState::Commands::Stop::Id:
        return HandleStartOrStopState(request.path, input_arguments, handler, StartOrStop::kStop);
    case OperationalState::Commands::Start::Id:
        return HandleStartOrStopState(request.path, input_arguments, handler, StartOrStop::kStart);
    default:
        return HandleDerivedClusterCommand(request.path, input_arguments, handler);
    }
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

namespace {

// Builds the standard OperationalCommandResponse carrying `err` and adds it to `handler`.
std::optional<DataModel::ActionReturnStatus> AddCommandResponse(const ConcreteCommandPath & path, CommandHandler * handler,
                                                                const GenericOperationalError & err)
{
    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(path, response);
    return std::nullopt;
}

} // namespace

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandlePauseOrResumeState(const ConcreteCommandPath & path,
                                                                                               chip::TLV::TLVReader & input,
                                                                                               CommandHandler * handler,
                                                                                               PauseOrResume action)
{
    // Pause/Resume are fieldless commands; just confirm there is no trailing payload.
    if (input.VerifyEndOfContainer() != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    const bool isPause = (action == PauseOrResume::kPause);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t opState   = GetCurrentOperationalState();
    uint8_t noOpState = isPause ? to_underlying(OperationalStateEnum::kPaused) : to_underlying(OperationalStateEnum::kRunning);

    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kError))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else if (opState >= DerivedClusterNumberSpaceStart && opState < VendorNumberSpaceStart)
    {
        bool compat = isPause ? IsDerivedClusterStatePauseCompatible(opState) : IsDerivedClusterStateResumeCompatible(opState);
        if (!compat)
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        }
    }

    if (err.errorStateID == 0 && opState != noOpState)
    {
        if (isPause)
        {
            mDelegate->HandlePauseStateCallback(err);
        }
        else
        {
            mDelegate->HandleResumeStateCallback(err);
        }
    }

    return AddCommandResponse(path, handler, err);
}

std::optional<DataModel::ActionReturnStatus> OperationalStateCluster::HandleStartOrStopState(const ConcreteCommandPath & path,
                                                                                             chip::TLV::TLVReader & input,
                                                                                             CommandHandler * handler,
                                                                                             StartOrStop action)
{
    // Start/Stop are fieldless commands; just confirm there is no trailing payload.
    if (input.VerifyEndOfContainer() != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    const bool isStart = (action == StartOrStop::kStart);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    uint8_t noOpState = isStart ? to_underlying(OperationalStateEnum::kRunning) : to_underlying(OperationalStateEnum::kStopped);

    if (GetCurrentOperationalState() != noOpState)
    {
        if (isStart)
        {
            mDelegate->HandleStartStateCallback(err);
        }
        else
        {
            mDelegate->HandleStopStateCallback(err);
        }
    }

    return AddCommandResponse(path, handler, err);
}
