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

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
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
    case MainStateEnum::kPendingFallback:
        return HasFeature(Feature::kFallback);
    default:
        return true;
    }
    return true;
}

CHIP_ERROR Instance::SetMainState(const MainStateEnum & aMainState)
{
    if (!IsSupportedState(aMainState))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    
    MainStateEnum oldMainState = mMainState;
    mMainState                         = aMainState;
    if (mMainState != oldMainState)
    {
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), mClusterId, Attributes::MainState::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOverallState(const DataModel::Nullable<Structs::OverallStateStruct::Type> & aOverallState)
{
    mOverallState = aOverallState;
    MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), mClusterId, Attributes::OverallState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOverallTarget(const DataModel::Nullable<Structs::OverallTargetStruct::Type> & aOverallTarget)
{
    mOverallTarget = aOverallTarget;
    MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), mClusterId, Attributes::OverallTarget::Id);
    return CHIP_NO_ERROR;
}

MainStateEnum Instance::GetMainState() const
{
    return mMainState;
}

DataModel::Nullable<Structs::OverallStateStruct::Type> Instance::GetOverallState() const
{
    return mOverallState;
}

DataModel::Nullable<Structs::OverallTargetStruct::Type> Instance::GetOverallTarget() const
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
        markDirty                                    = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }
    else
    {
        auto predicate = [](const decltype(mCountdownTime)::SufficientChangePredicateCandidate &) -> bool { return true; };
        markDirty      = (mCountdownTime.SetValue(newCountdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }

    if (markDirty)
    {
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), mClusterId, Attributes::CountdownTime::Id);
    }
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ClosureControl::Id);

    switch (aPath.mAttributeId)
    {
    case CountdownTime::Id:
        if (SupportsOptAttr(OptionalAttributes::kCountdownTime))
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
    case RestingProcedure::Id:
        if (HasFeature(Feature::kFallback))
        {
            return aEncoder.Encode(mDelegate.GetRestingProcedure());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    case TriggerCondition::Id:
        if (HasFeature(Feature::kFallback))
        {
            return aEncoder.Encode(mDelegate.GetTriggerCondition());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    case TriggerPosition::Id:
        if (HasFeature(Feature::kFallback))
        {
            return aEncoder.Encode(mDelegate.GetTriggerPosition());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    case WaitingDelay::Id:
        if (HasFeature(Feature::kFallback))
        {
            return aEncoder.Encode(mDelegate.GetWaitingDelay());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    case KickoffTimer::Id:
        if (HasFeature(Feature::kFallback))
        {
            return aEncoder.Encode(mDelegate.GetKickoffTimer());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
        // TODO CHECK CLUSTER REVISION
    }
    /* Allow all other unhandled attributes to fall through to Ember */
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
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // Convert end of list to CHIP_NO_ERROR
            err = CHIP_NO_ERROR;
            goto exit;
        }

        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        err = encoder.Encode(error);
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    err = mDelegate.EndCurrentErrorListRead();
    return err;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    default:
        // Unknown attribute; return error.  None of the other attributes for
        // this cluster are writable, so should not be ending up in this code to
        // start with.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

// CommandHandlerInterface
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Stop::Id:
        if (!HasFeature(Feature::kInstantaneous))
        {
            HandleCommand<Stop::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleStop(ctx, commandData); });
        }
        else
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        return;
    case MoveTo::Id:
        HandleCommand<MoveTo::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleMoveTo(ctx, commandData); });
        return;
    case Calibrate::Id:
        if (HasFeature(Feature::kCalibration))
        {
            HandleCommand<Calibrate::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleCalibrate(ctx, commandData); });
        }
        else
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        return;
    case ConfigureFallback::Id:
        if (HasFeature(Feature::kFallback))
        {
            HandleCommand<ConfigureFallback::DecodableType>(handlerContext, [this](HandlerContext & ctx, const auto & commandData) {
                HandleConfigureFallback(ctx, commandData);
            });
        }
        else
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        return;
    case CancelFallback::Id:
        if (HasFeature(Feature::kFallback))
        {
            HandleCommand<CancelFallback::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleCancelFallback(ctx, commandData); });
        }
        else
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        return;
    }
}

void Instance::HandleStop(HandlerContext & ctx, const Commands::Stop::DecodableType & commandData)
{
    // No parameters for this command
    // Call the delegate
    Status status = mDelegate.Stop();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleMoveTo(HandlerContext & ctx, const Commands::MoveTo::DecodableType & commandData)
{
    const Optional<TagPositionEnum> tag               = commandData.tag;
    const Optional<TagLatchEnum> latch                = commandData.latch;
    const Optional<Globals::ThreeLevelAutoEnum> speed = commandData.speed;

    // Call the delegate
    Status status = mDelegate.MoveTo(tag, latch, speed);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleCalibrate(HandlerContext & ctx, const Commands::Calibrate::DecodableType & commandData)
{
    // No parameters for this command
    // Call the delegate
    Status status = mDelegate.Calibrate();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleConfigureFallback(HandlerContext & ctx, const Commands::ConfigureFallback::DecodableType & commandData)
{
    const Optional<RestingProcedureEnum> restingProcedure = commandData.restingProcedure;
    const Optional<TriggerConditionEnum> triggerCondition = commandData.triggerCondition;
    const Optional<TriggerPositionEnum> triggerPosition   = commandData.triggerPosition;
    const Optional<uint32_t> waitingDelay                 = commandData.waitingDelay;

    // Call the delegate
    Status status = mDelegate.ConfigureFallback(restingProcedure, triggerCondition, triggerPosition, waitingDelay);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleCancelFallback(HandlerContext & ctx, const Commands::CancelFallback::DecodableType & commandData)
{
    // No parameters for this command
    // Call the delegate
    Status status = mDelegate.CancelFallback();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureControlPluginServerInitCallback() {}
