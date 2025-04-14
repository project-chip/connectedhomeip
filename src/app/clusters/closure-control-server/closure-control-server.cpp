/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "closure-control-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureControl::Attributes;
using namespace chip::app::Clusters::ClosureControl::Commands;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeatures) const
{
    return mFeatures.Has(aFeatures);
}

bool Instance::SupportsOptAttr(OptionalAttribute aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

bool Instance::IsSupportedState(MainStateEnum aMainState)
{
    switch (aMainState)
    {
    case MainStateEnum::kCalibrating:
        return HasFeature(Feature::kCalibration);
    case MainStateEnum::kProtected:
        return HasFeature(Feature::kProtection);
    case MainStateEnum::kDisengaged:
        return HasFeature(Feature::kManuallyOperable);
    default:
        // Remaining MainState have Mandatory conformance,so will be supported.
        return true;
    }
    return true;
}

// Closure Control MainState and supoorted commands are given below
//  1. Calibrating        -> Calibrate and Stop commands supported
//  2. Stopped            -> Calibrate, MoveTo and Stop commands supported
//  3. Moving             -> MoveTo and Stop command supported
//  4. WaitingForMotion   -> MoveTo and Stop command supported
//  5. Disengaged         -> No commands supported
//  6. Protected          -> No commands supported
//  7. SetupRequired      -> No commands supported
//  8. Error              -> Only MoveTo command supported
bool Instance::CheckCommandStateCompatibility(CommandId cmd, MainStateEnum state)
{
    // None of the commands are supported in Disengaged, Protected and SetupRequired States
    if ((state == MainStateEnum::kDisengaged) || (state == MainStateEnum::kProtected) || (state == MainStateEnum::kSetupRequired))
    {
        return false;
    }

    switch (cmd)
    {
    case Commands::Stop::Id:
        // The Stop command is supported in Calibrating ,Stopped, Moving and WaitingForMotion states, not supported in the Error
        // state.
        if (state == MainStateEnum::kError)
        {
            return false;
        }
        else
        {
            return true;
        }
        break;

    case Commands::MoveTo::Id:
        // The MoveTo command is supported in Stopped, Error, Moving and WaitingForMotion states, not supported in the Calibrating
        // State.
        if (state == MainStateEnum::kCalibrating)
        {
            return false;
        }
        else
        {
            return true;
        }
        break;

    case Commands::Calibrate::Id:
        // The Calibrate command is supported in Calibrating and Stopped states, not supported in Error, Moving and WaitingForMotion
        // states.
        if ((state == MainStateEnum::kCalibrating) || (state == MainStateEnum::kStopped))
        {
            return true;
        }
        else
        {
            return false;
        }
        break;
    default:
        return false;
    }
    return false;
}

void Instance::ReportCurrentErrorListChange()
{
    MatterReportingAttributeChangeCallback(
        ConcreteAttributePath(mDelegate.GetEndpointId(), ClosureControl::Id, Attributes::CurrentErrorList::Id));
}

CHIP_ERROR Instance::SetMainState(MainStateEnum aMainState)
{
    if (!IsSupportedState(aMainState))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // If the Main State has changed, trigger the attribute change callback
    if (mMainState != aMainState)
    {
        mMainState = aMainState;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureControl::Id, Attributes::MainState::Id);
        UpdateCountdownTimeFromClusterLogic();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOverallState(const GenericOverallState & aOverallState)
{
    // If the overall state has changed, trigger the attribute change callback
    if (!(mOverallState == aOverallState))
    {
        mOverallState = aOverallState;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureControl::Id, Attributes::OverallState::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOverallTarget(const GenericOverallTarget & aOverallTarget)
{
    // If the overall target has changed, trigger the attribute change callback
    if (!(mOverallTarget == aOverallTarget))
    {
        mOverallTarget = aOverallTarget;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureControl::Id, Attributes::OverallTarget::Id);
    }

    return CHIP_NO_ERROR;
}

MainStateEnum Instance::GetMainState() const
{
    return mMainState;
}

const GenericOverallState & Instance::GetOverallState() const
{
    return mOverallState;
}

const GenericOverallTarget & Instance::GetOverallTarget() const
{
    return mOverallTarget;
}

void Instance::UpdateCountdownTime(bool fromDelegate)
{
    app::DataModel::Nullable<uint32_t> newCountdownTime = mDelegate.GetCountdownTime();
    auto now                                            = System::SystemClock().GetMonotonicTimestamp();

    bool markDirty = false;

    if (fromDelegate)
    {
        // Updates from delegate are reduce-reported to every 1s max (choice of this implementation), in addition
        // to default change-from-null, change-from-zero and increment policy.
        System::Clock::Milliseconds64 reportInterval = System::Clock::Milliseconds64(1000);
        auto predicate                               = mCountdownTime.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
        markDirty = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }
    else
    {
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate &) -> bool { return true; };
        markDirty      = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }

    if (markDirty)
    {
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureControl::Id, Attributes::CountdownTime::Id);
    }
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ClosureControl::Id);

    switch (aPath.mAttributeId)
    {
    case CountdownTime::Id:
        // Optional Attribute
        if (SupportsOptAttr(OptionalAttribute::kCountdownTime))
        {
            return aEncoder.Encode(mDelegate.GetCountdownTime());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    case MainState::Id:
        return aEncoder.Encode(GetMainState());

    case CurrentErrorList::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->EncodeCurrentErrorList(encoder); });

    case OverallState::Id:
        return aEncoder.Encode(GetOverallState());

    case OverallTarget::Id:
        return aEncoder.Encode(GetOverallTarget());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeatures);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::EncodeCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mDelegate.StartCurrentErrorListRead());

    for (size_t i = 0; true; i++)
    {
        ClosureErrorEnum error;

        err = mDelegate.GetCurrentErrorListAtIndex(i, error);
        // Convert end of list to CHIP_NO_ERROR
        VerifyOrExit(err != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED, err = CHIP_NO_ERROR);

        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        // Encode the error
        err = encoder.Encode(error);

        // Check if another error occurred before trying to encode
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    ReturnErrorOnFailure(mDelegate.EndCurrentErrorListRead());
    return err;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == ClosureControl::Id);

    switch (aPath.mAttributeId)
    {
    case CountdownTime::Id:
        if (SupportsOptAttr(OptionalAttribute::kCountdownTime))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    case MainState::Id:
    case CurrentErrorList::Id:
    case OverallState::Id:
    case OverallTarget::Id:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

// CommandHandlerInterface
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;
    Status status = Status::UnsupportedCommand;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Stop::Id:
        if (!HasFeature(Feature::kInstantaneous))
        {
            HandleCommand<Stop::DecodableType>(handlerContext, [this, &status](HandlerContext & ctx, const auto & commandData) {
                status = HandleStop(ctx, commandData);
            });
        }
        break;
    case MoveTo::Id:
        HandleCommand<MoveTo::DecodableType>(handlerContext, [this, &status](HandlerContext & ctx, const auto & commandData) {
            status = HandleMoveTo(ctx, commandData);
        });
        break;
    case Calibrate::Id:
        if (HasFeature(Feature::kCalibration))
        {
            HandleCommand<Calibrate::DecodableType>(
                handlerContext,
                [this, &status](HandlerContext & ctx, const auto & commandData) { status = HandleCalibrate(ctx, commandData); });
        }
        break;
    }

    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
}

Status Instance::HandleStop(HandlerContext & ctx, const Commands::Stop::DecodableType & commandData)
{
    MainStateEnum state = GetMainState();

    Status status = Status::Failure;

    VerifyOrReturnValue(CheckCommandStateCompatibility(Commands::Stop::Id, state), Status::InvalidInState);
    if (state == MainStateEnum::kStopped)
    {
        return Status::Success;
    }

    if ((state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
    {
        status = mDelegate.Stop();
        SetMainState(MainStateEnum::kStopped);
    }

    return status;
}

Status Instance::HandleMoveTo(HandlerContext & ctx, const Commands::MoveTo::DecodableType & commandData)
{
    MainStateEnum state = GetMainState();
    VerifyOrReturnValue(CheckCommandStateCompatibility(Commands::Stop::Id, state), Status::InvalidInState);

    // If all command parameters don't have a value, return InvalidCommand
    VerifyOrReturnValue(commandData.position.HasValue() || commandData.latch.HasValue() || commandData.speed.HasValue(),
                        Status::InvalidCommand);

    if (commandData.position.HasValue())
    {
        VerifyOrReturnError((commandData.position.Value() != TargetPositionEnum::kUnknownEnumValue), Status::ConstraintError);
        // If Positioning(PS) feature or not, it SHALL return a status code SUCCESS.
        VerifyOrReturnError(HasFeature(Feature::kPositioning), Status::Success);
    }

    if (commandData.latch.HasValue())
    {
        VerifyOrReturnError(commandData.latch.Value() != TargetLatchEnum::kUnknownEnumValue, Status::ConstraintError);

        // If MotionLatching (LT) feature or not, the server SHALL return a status code SUCCESS
        VerifyOrReturnError(HasFeature(Feature::kMotionLatching), Status::Success);

        // If manual intervention is required to latch, respond with INVALID_ACTION
        VerifyOrReturnError(mDelegate.IsLatchManual() == true, Status::InvalidAction);
    }

    if (commandData.speed.HasValue())
    {
        VerifyOrReturnError(commandData.speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        // If Speed (SP) feature or not, the server SHALL return a status code SUCCESS
        VerifyOrReturnError(HasFeature(Feature::kSpeed), Status::Success);
    }

    if (mDelegate.CheckErrorOnDevice())
    {
        // If the device is in an error state, set the MainState to Error
        VerifyOrReturnError(SetMainState(MainStateEnum::kError) == CHIP_NO_ERROR, Status::Failure);
        // Return Status Failure
        return Status::Failure;
    }

    if (mDelegate.IsDeviceReadyToMove())
    {
        // If the device is ready to move, set MainState to Moving
        SetMainState(MainStateEnum::kMoving);
    }
    else
    {
        // If device need pre-stage before moving, then set MainState to Waiting for Moving
        SetMainState(MainStateEnum::kWaitingForMotion);
    }

    // TODO: Check if the device is in a state to move and set MainState to Moving or Waiting for Moving
    return mDelegate.MoveTo(commandData.position, commandData.latch, commandData.speed);
}

Status Instance::HandleCalibrate(HandlerContext & ctx, const Commands::Calibrate::DecodableType & commandData)
{
    MainStateEnum state = GetMainState();
    Status status       = Status::Failure;
    VerifyOrReturnValue(CheckCommandStateCompatibility(Commands::Calibrate::Id, state), Status::InvalidInState);

    if (state == MainStateEnum::kCalibrating)
    {
        return Status::Success;
    }

    if ((state == MainStateEnum::kStopped))
    {
        status = mDelegate.Calibrate();
        SetMainState(MainStateEnum::kCalibrating);
    }

    return status;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureControlPluginServerInitCallback() {}
void MatterClosureControlPluginServerShutdownCallback() {}
