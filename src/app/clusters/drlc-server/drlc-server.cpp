/**
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

#include "drlc-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>

namespace {
// Spec-defined constraints
constexpr uint8_t kMaxRandomStartMinutes    = 60;
constexpr uint8_t kMaxRandomDurationMinutes = 60;
constexpr uint8_t kMaxDefaultRandomStart    = 60;
constexpr uint8_t kMaxDefaultRandomDuration = 60;
constexpr uint16_t kMaxTransitionDuration   = 1440;
constexpr int16_t kMinTemperature           = -27315;
constexpr int8_t kMinLoadAdjustment         = -100;
constexpr int8_t kMaxLoadAdjustment         = 100;
constexpr uint8_t kMaxPercentValue          = 100;
} // anonymous namespace

namespace chip {

using Status = Protocols::InteractionModel::Status;

namespace app {
namespace Clusters {
namespace DemandResponseLoadControl {

using namespace Attributes;

template <bool ClearLoadControlEventsRequestSupported>
Instance<ClearLoadControlEventsRequestSupported>::Instance(EndpointId aEndpointId, Delegate & aDelegate) :
    AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
    mDelegate(aDelegate), mEndpoint(aEndpointId)
{}

template <bool ClearLoadControlEventsRequestSupported>
Instance<ClearLoadControlEventsRequestSupported>::~Instance()
{
    // Just in case, make sure there are no dangling pointers to us.
    Shutdown();
}

template <bool ClearLoadControlEventsRequestSupported>
CHIP_ERROR Instance<ClearLoadControlEventsRequestSupported>::Init()
{
    registerAttributeAccessOverride(this);
    InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
    return CHIP_NO_ERROR;
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::Shutdown()
{
    unregisterAttributeAccessOverride(this);
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
}

template <bool ClearLoadControlEventsRequestSupported>
CHIP_ERROR Instance<ClearLoadControlEventsRequestSupported>::Read(const ConcreteReadAttributePath & aPath,
                                                                  AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case LoadControlPrograms::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            size_t index = 0;
            do
            {
                LoadControlProgram program;
                CHIP_ERROR err = mDelegate.GetLoadControlProgram(index, program);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    // Done encoding the list.
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(program));
                ++index;
            } while (true);
        });
    case NumberOfLoadControlPrograms::Id:
        return aEncoder.Encode(mDelegate.GetNumberOfLoadControlPrograms());
    case Attributes::Events::Id:
        // TODO: Figure out the right API for handling the list
        // of transitions inside a LoadControlEventStruct.  We could make the
        // list size a compile-time constant, but maybe we can figure out
        // something more configurable.
        return aEncoder.EncodeEmptyList();
    case ActiveEvents::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            size_t index = 0;
            do
            {
                uint8_t eventIDBuffer[kEventIDSize];
                FixedSpan<uint8_t, kEventIDSize> eventID(eventIDBuffer);
                CHIP_ERROR err = mDelegate.GetActiveEventID(index, eventID);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    // Done encoding the list.
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(ByteSpan(eventID)));
                ++index;
            } while (true);
        });
    case NumberOfEventsPerProgram::Id:
        return aEncoder.Encode(mDelegate.GetNumberOfEventsPerProgram());
    case NumberOfTransitions::Id:
        return aEncoder.Encode(mDelegate.GetNumberOfTransitions());
    case DefaultRandomStart::Id:
        return aEncoder.Encode(mDelegate.GetDefaultRandomStart());
    case DefaultRandomDuration::Id:
        return aEncoder.Encode(mDelegate.GetDefaultRandomDuration());
    default:
        // Unknown attribute; return error.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

template <bool ClearLoadControlEventsRequestSupported>
CHIP_ERROR Instance<ClearLoadControlEventsRequestSupported>::Write(const ConcreteDataAttributePath & aPath,
                                                                   AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == DemandResponseLoadControl::Id);

    switch (aPath.mAttributeId)
    {
    case DefaultRandomStart::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        if (newValue > kMaxDefaultRandomStart)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        uint8_t oldValue = mDelegate.GetDefaultRandomStart();
        ReturnErrorOnFailure(mDelegate.SetDefaultRandomStart(newValue));

        if (oldValue != newValue)
        {
            MatterReportingAttributeChangeCallback(aPath);
        }
        return CHIP_NO_ERROR;
    }
    case DefaultRandomDuration::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        if (newValue > kMaxDefaultRandomDuration)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        uint8_t oldValue = mDelegate.GetDefaultRandomDuration();
        ReturnErrorOnFailure(mDelegate.SetDefaultRandomDuration(newValue));

        if (oldValue != newValue)
        {
            MatterReportingAttributeChangeCallback(aPath);
        }
        return CHIP_NO_ERROR;
    }
    default:
        // Unknown attribute; return error.  None of the other attributes for
        // this cluster are writable, so should not be ending up in this code to
        // start with.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

template <bool ClearLoadControlEventsRequestSupported>
CHIP_ERROR Instance<ClearLoadControlEventsRequestSupported>::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                                                                       CommandIdCallback callback, void * context)
{
    using namespace Commands;

    for (auto && id : {
             RegisterLoadControlProgramRequest::Id,
             UnregisterLoadControlProgramRequest::Id,
             AddLoadControlEventRequest::Id,
             RemoveLoadControlEventRequest::Id,
         })
    {
        if (callback(id, context) == Loop::Break)
        {
            return CHIP_NO_ERROR;
        }
    }

    if constexpr (ClearLoadControlEventsRequestSupported)
    {
        callback(ClearLoadControlEventsRequest::Id, context);
    }

    return CHIP_NO_ERROR;
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case RegisterLoadControlProgramRequest::Id:
        HandleCommand<RegisterLoadControlProgramRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleRegisterLoadControlProgramRequest(ctx, commandData); });
        return;
    case UnregisterLoadControlProgramRequest::Id:
        HandleCommand<UnregisterLoadControlProgramRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) {
                HandleUnregisterLoadControlProgramRequest(ctx, commandData);
            });
        return;
    case AddLoadControlEventRequest::Id:
        HandleCommand<AddLoadControlEventRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleAddLoadControlEventRequest(ctx, commandData); });
        return;
    case RemoveLoadControlEventRequest::Id:
        HandleCommand<RemoveLoadControlEventRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleRemoveLoadControlEventRequest(ctx, commandData); });
        return;
    case ClearLoadControlEventsRequest::Id:
        if constexpr (ClearLoadControlEventsRequestSupported)
        {
            HandleCommand<ClearLoadControlEventsRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleClearLoadControlEventsRequest(ctx, commandData); });
        }
        return;
    }
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::HandleRegisterLoadControlProgramRequest(
    HandlerContext & ctx, const Commands::RegisterLoadControlProgramRequest::DecodableType & commandData)
{
    // Check spec-defined constraints on the fields of the struct.
    auto & program = commandData.loadControlProgram;
    if (program.programID.size() != kProgramIDSize || program.name.size() > LoadControlProgram::kMaxNameSize ||
        (!program.randomStartMinutes.IsNull() && program.randomStartMinutes.Value() > kMaxRandomStartMinutes) ||
        (!program.randomDurationMinutes.IsNull() && program.randomDurationMinutes.Value() > kMaxRandomDurationMinutes))
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Check whether this is an insert or a replace.
    bool hasMatchingID = false;
    size_t index       = 0;
    do
    {
        LoadControlProgram existingProgram;
        CHIP_ERROR err = mDelegate.GetLoadControlProgram(index, existingProgram);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // No matches found.
            break;
        }

        if (err != CHIP_NO_ERROR)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, StatusIB(err).mStatus);
            return;
        }

        if (program.programID.data_equal(existingProgram.GetProgramID()))
        {
            hasMatchingID = true;
            break;
        }

        ++index;
    } while (true);

    Status status;
    if (hasMatchingID)
    {
        status = mDelegate.ReplaceLoadControlProgram(index, program);
    }
    else
    {
        if (index == mDelegate.GetNumberOfLoadControlPrograms())
        {
            // Spec says RESOURCE_EXHAUSTED in this case.
            status = Status::ResourceExhausted;
        }
        else
        {
            status = mDelegate.AddLoadControlProgram(program);
        }
    }

    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(mEndpoint, Id, LoadControlPrograms::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::HandleUnregisterLoadControlProgramRequest(
    HandlerContext & ctx, const Commands::UnregisterLoadControlProgramRequest::DecodableType & commandData)
{
    // TODO: Figure out what happens if the provided ID does not match an
    // existing program.  See
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8540

    // TODO: What is the actual constraint on LoadControlProgramID?  See
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8541

    auto & id = commandData.loadControlProgramID;
    if (id.size() > kProgramIDSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    Status status = mDelegate.RemoveLoadControlProgram(id);

    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(mEndpoint, Id, LoadControlPrograms::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::HandleAddLoadControlEventRequest(
    HandlerContext & ctx, const Commands::AddLoadControlEventRequest::DecodableType & commandData)
{
    auto transitionIter = commandData.event.transitions.begin();

    // Check spec-defined constraints on the fields of the struct.
    auto constraintsSatisfied = [this, &commandData, &transitionIter]() -> bool {
        auto & event = commandData.event;
        if (event.eventID.size() != kEventIDSize ||
            (!event.programID.IsNull() && event.programID.Value().size() != kProgramIDSize) ||
            event.criticality == CriticalityLevelEnum::kUnknownEnumValue)
        {
            return false;
        }

        size_t transitionCount = 0;
        while (transitionIter.Next())
        {
            ++transitionCount;

            auto transition = transitionIter.GetValue();
            if (transition.duration > kMaxTransitionDuration)
            {
                return false;
            }

            // TODO: Decide on the best way to check against the feature map here
            // and reject attempts to add controls that we don't claim to support.
            // We could have the feature map as a template parameter, but that leads
            // to a bunch of code duplication if multiple DRLC clusters with
            // different features are in fact used...  We could store it in a
            // member, but then we end up paying some codesize costs for the cases
            // that we don't actually support.  Maybe we should have CHIP_CONFIG
            // defines for the set of features we might support, and then do runtime
            // checks.
            size_t providedControls = 0;
            if (transition.temperatureControl.HasValue())
            {
                ++providedControls;
                auto & control = transition.temperatureControl.Value();
                if ((control.coolingTempSetpoint.HasValue() && !control.coolingTempSetpoint.Value().IsNull() &&
                     control.coolingTempSetpoint.Value().Value() < kMinTemperature) ||
                    (control.heatingTempSetpoint.HasValue() && !control.heatingTempSetpoint.Value().IsNull() &&
                     control.heatingTempSetpoint.Value().Value() < kMinTemperature))
                {
                    return false;
                }
            }

            if (transition.averageLoadControl.HasValue())
            {
                ++providedControls;
                auto loadAdjustment = transition.averageLoadControl.Value().loadAdjustment;
                if (loadAdjustment < kMinLoadAdjustment || loadAdjustment > kMaxLoadAdjustment)
                {
                    return false;
                }
            }

            if (transition.dutyCycleControl.HasValue())
            {
                ++providedControls;
                uint8_t dutyCycle = transition.dutyCycleControl.Value().dutyCycle;
                if (dutyCycle > kMaxPercentValue)
                {
                    return false;
                }
            }

            if (transition.powerSavingsControl.HasValue())
            {
                ++providedControls;
                uint8_t powerSavings = transition.powerSavingsControl.Value().powerSavings;
                if (powerSavings > kMaxPercentValue)
                {
                    return false;
                }
            }

            if (transition.heatingSourceControl.HasValue())
            {
                ++providedControls;
                auto heatingSource = transition.heatingSourceControl.Value().heatingSource;
                if (heatingSource == HeatingSourceEnum::kUnknownEnumValue)
                {
                    return false;
                }
            }

            if (providedControls > 1)
            {
                // Spec says at most one of these can be provided.
                return false;
            }
        }

        if (transitionCount > mDelegate.GetNumberOfTransitions())
        {
            return false;
        }

        return true;
    }();

    if (!constraintsSatisfied)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if (transitionIter.GetStatus() != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, StatusIB(transitionIter.GetStatus()).mStatus);
        return;
    }

    // TODO: Figure out how to check whether we have a synchronized clock and
    // return INVALID_IN_STATE if not.

    // TODO: Check existing Events list to see whether the number of events that
    // have the provided ProgramID equals GetNumberOfEventsPerProgram(), and if
    // so return RESOURCE_EXHAUSTED. But see
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8543

    // TODO: Check existing Events for an EventID match, and if there is one
    // return INVALID_COMMAND. But see
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8542

    Status status = mDelegate.AddLoadControlEvent(commandData.event);

    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(mEndpoint, Id, Attributes::Events::Id);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::HandleRemoveLoadControlEventRequest(
    HandlerContext & ctx, const Commands::RemoveLoadControlEventRequest::DecodableType & commandData)
{
    auto & id = commandData.eventID;
    if (id.size() != kEventIDSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // TODO: Really not clear what the requirements are around
    // CancelControlBitmap, to the point where I am not sure what the right
    // delegate API here would be.  See
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8544
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
}

template <bool ClearLoadControlEventsRequestSupported>
void Instance<ClearLoadControlEventsRequestSupported>::HandleClearLoadControlEventsRequest(
    HandlerContext & ctx, const Commands::ClearLoadControlEventsRequest::DecodableType & commandData)
{
    Status status = mDelegate.ClearLoadControlEvents();
    if (status == Status::Success)
    {
        MatterReportingAttributeChangeCallback(mEndpoint, Id, Attributes::Events::Id);
    }
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

// Ensure the out-of-line bits are instantiated.
template class Instance<false>;
template class Instance<true>;

CHIP_ERROR
LoadControlProgram::Init(const FixedByteSpan<kProgramIDSize> & aProgramID, const CharSpan & aName,
                         const Nullable<uint8_t> & aRandomStartMinutes, const Nullable<uint8_t> & aRandomDurationMinutes)
{
    static_assert(std::remove_reference_t<decltype(aProgramID)>::size() == sizeof(mProgramIDBuffer));
    if (aName.size() > sizeof(mNameBuffer))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    memcpy(mProgramIDBuffer, aProgramID.data(), aProgramID.size());
    memcpy(mNameBuffer, aName.data(), aName.size());

    programID             = ByteSpan(mProgramIDBuffer);
    name                  = CharSpan(mNameBuffer, aName.size());
    randomStartMinutes    = aRandomStartMinutes;
    randomDurationMinutes = aRandomDurationMinutes;
    return CHIP_NO_ERROR;
}

} // namespace DemandResponseLoadControl
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterDemandResponseLoadControlPluginServerInitCallback()
{
    // Nothing here; there is no global state for this cluster.
}
