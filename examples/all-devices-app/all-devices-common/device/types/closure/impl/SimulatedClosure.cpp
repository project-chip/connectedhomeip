/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "SimulatedClosure.h"

namespace {

// TargetPositionEnum (command input) and CurrentPositionEnum (state output) are separate,
// independently-generated enums whose values do not line up numerically past the first two.
chip::app::Clusters::ClosureControl::CurrentPositionEnum
MapToCurrentPosition(chip::app::Clusters::ClosureControl::TargetPositionEnum target)
{
    using namespace chip::app::Clusters::ClosureControl;
    switch (target)
    {
    case TargetPositionEnum::kMoveToFullyClosed:
        return CurrentPositionEnum::kFullyClosed;
    case TargetPositionEnum::kMoveToFullyOpen:
        return CurrentPositionEnum::kFullyOpened;
    case TargetPositionEnum::kMoveToPedestrianPosition:
        return CurrentPositionEnum::kOpenedForPedestrian;
    case TargetPositionEnum::kMoveToVentilationPosition:
        return CurrentPositionEnum::kOpenedForVentilation;
    case TargetPositionEnum::kMoveToSignaturePosition:
        return CurrentPositionEnum::kOpenedAtSignature;
    default:
        return CurrentPositionEnum::kUnknownEnumValue;
    }
}

} // namespace

namespace chip {
namespace app {

SimulatedClosure::SimulatedClosure(TimerDelegate & Tdelegate, Clusters::IdentifyDelegate & Idelegate, Config config,
                                   Credentials::GroupDataProvider & groupDataProvider, FabricTable & fabricTable,
                                   TestEventTriggerDelegate & testEventTriggerDelegate) :
    Closure(config.closure, Tdelegate, Idelegate, *this),
    OnOffContext({ groupDataProvider, fabricTable, Tdelegate, Idelegate }), mPanelList(std::move(config.panels)),
    mTimerDelegate(Tdelegate), mTestEventTriggerDelegate(testEventTriggerDelegate)
{}

SimulatedClosure::~SimulatedClosure()
{
    CancelTimer();
}

Protocols::InteractionModel::Status SimulatedClosure::HandleStopCommand()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleStopCommand()");
    CancelTimer();
    mPendingCurrentState.reset();
    for (auto & panel : mSimulatedClosurePanel)
    {
        panel->CancelTimer();
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
SimulatedClosure::HandleMoveToCommand(const Optional<Clusters::ClosureControl::TargetPositionEnum> & position,
                                      const Optional<bool> & latch, const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleMoveToCommand() -> position=%u latch=%d speed=%u",
                    to_underlying(position.ValueOr(Clusters::ClosureControl::TargetPositionEnum::kUnknownEnumValue)),
                    latch.ValueOr(false), to_underlying(speed.ValueOr(Clusters::Globals::ThreeLevelAutoEnum::kAuto)));
    DataModel::Nullable<Clusters::ClosureControl::GenericOverallCurrentState> overallCurrentState =
        ClosureControlCluster().GetOverallCurrentState();
    Clusters::ClosureControl::GenericOverallCurrentState fallback =
        overallCurrentState.IsNull() ? Clusters::ClosureControl::GenericOverallCurrentState() : overallCurrentState.Value();

    auto newPosition =
        position.HasValue() ? MakeOptional(DataModel::MakeNullable(MapToCurrentPosition(position.Value()))) : fallback.position;
    auto newLatch = latch.HasValue() ? MakeOptional(DataModel::MakeNullable(latch.Value())) : fallback.latch;

    // A closure is secure only when every supported securing mechanism is engaged.
    const BitFlags<Clusters::ClosureControl::Feature> featureMap = ClosureControlCluster().GetFeatureMap();
    bool isSecure                                                = true;
    if (featureMap.Has(Clusters::ClosureControl::Feature::kPositioning))
    {
        isSecure &= newPosition.HasValue() && !newPosition.Value().IsNull() &&
            newPosition.Value().Value() == Clusters::ClosureControl::CurrentPositionEnum::kFullyClosed;
    }
    if (featureMap.Has(Clusters::ClosureControl::Feature::kMotionLatching))
    {
        isSecure &= newLatch.HasValue() && !newLatch.Value().IsNull() && newLatch.Value().Value();
    }

    mPendingCurrentState = Clusters::ClosureControl::GenericOverallCurrentState(
        newPosition, newLatch, speed.HasValue() ? MakeOptional(speed.Value()) : fallback.speed, DataModel::MakeNullable(isSecure));
    CancelTimer();
    VerifyOrReturnValue(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kTimeoutDurationSec)).Handle([](CHIP_ERROR err) {
        ChipLogError(DeviceLayer, "SimulatedClosure: failed to start move timer: %" CHIP_ERROR_FORMAT, err.Format());
    }),
                        Protocols::InteractionModel::Status::Failure);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedClosure::HandleCalibrateCommand()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleCalibrateCommand()");
    mPendingCurrentState.reset();
    CancelTimer();
    VerifyOrReturnValue(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kTimeoutDurationSec)).Handle([](CHIP_ERROR err) {
        ChipLogError(DeviceLayer, "SimulatedClosure: failed to start Calibrate timer : %" CHIP_ERROR_FORMAT, err.Format());
    }),
                        Protocols::InteractionModel::Status::Failure);
    return Protocols::InteractionModel::Status::Success;
}

bool SimulatedClosure::IsReadyToMove()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::IsReadyToMove()");
    return true;
}

ElapsedS SimulatedClosure::GetCalibrationCountdownTime()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::GetCalibrationCountdownTime()");
    return kTimeoutDurationSec;
}

ElapsedS SimulatedClosure::GetMovingCountdownTime()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::GetMovingCountdownTime()");
    return kTimeoutDurationSec;
}

ElapsedS SimulatedClosure::GetWaitingForMotionCountdownTime()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::GetWaitingForMotionCountdownTime()");
    return kTimeoutDurationSec;
}

bool SimulatedClosure::RegistersAccessDevicePanel() const
{
    for (const auto & panel : mPanelList)
    {
        if (panel.config.withAccess)
        {
            return true;
        }
    }
    return false;
}

void SimulatedClosure::TimerFired()
{
    ChipLogProgress(DeviceLayer, "SimulatedClosure::TimerFired()");
    if (mPendingCurrentState.has_value())
    {
        Clusters::ClosureControl::GenericOverallCurrentState pendingState = *mPendingCurrentState;
        LogErrorOnFailure(ClosureControlCluster().SetOverallCurrentState(DataModel::MakeNullable(pendingState)));
        mPendingCurrentState.reset();
    }
    LogErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kStopped));
    LogErrorOnFailure(ClosureControlCluster().GenerateMovementCompletedEvent());
}

CHIP_ERROR SimulatedClosure::HandleEventTrigger(uint64_t eventTrigger)
{
    const auto triggerEndpoint = static_cast<EndpointId>((eventTrigger >> 32) & 0xFFFF);
    VerifyOrReturnError(triggerEndpoint == kRootEndpointId || triggerEndpoint == GetEndpointId(), CHIP_ERROR_INVALID_ARGUMENT);

    eventTrigger = clearEndpointInEventTrigger(eventTrigger);
    switch (eventTrigger)
    {
    case kTriggerError:
        ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleEventTrigger() -> Error");
        ReturnErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kError));
        return ClosureControlCluster().AddErrorToCurrentErrorList(Clusters::ClosureControl::ClosureErrorEnum::kBlockedBySensor);
    case kTriggerProtected:
        ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleEventTrigger() -> Protected");
        return Closure::ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kProtected);
    case kTriggerDisengaged:
        ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleEventTrigger() -> Disengaged");
        return Closure::ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kDisengaged);
    case kTriggerClear:
        ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleEventTrigger() -> Clear");
        ReturnErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kStopped));
        ClosureControlCluster().ClearCurrentErrorList();
        return CHIP_NO_ERROR;
    case kTriggerSetupRequired:
        ChipLogProgress(DeviceLayer, "SimulatedClosure::HandleEventTrigger() -> SetupRequired");
        return ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kSetupRequired);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT; // not ours — lets any other registered handler try instead
    }
}

void SimulatedClosure::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedClosure::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                           EndpointComposition composition)
{
    for (const auto & panel : mPanelList)
    {
        auto ClosurePanel = std::make_unique<SimulatedClosurePanel>(panel.config, mTimerDelegate);
        // Each panel is a part of this closure endpoint and carries its own tags.
        EndpointComposition panelComposition(GetEndpointId(), composition.pattern, panel.tags);
        ReturnErrorOnFailure(ClosurePanel->Register(allocator, provider, panelComposition));
        mSimulatedClosurePanel.push_back(std::move(ClosurePanel));
    }

    auto onOffLights    = std::make_unique<LoggingOnOffLight>(OnOffContext);
    mOnOffLightPart     = onOffLights.get();
    mLoggingOnOffLights = std::move(onOffLights);
    ReturnErrorOnFailure(mLoggingOnOffLights->Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));

    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(this));
    return CHIP_NO_ERROR;
}

void SimulatedClosure::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    mTestEventTriggerDelegate.RemoveHandler(this);

    // Parts whose registration was rolled back already have an invalid endpoint id: skip them,
    // unregistering them a second time would fail in RemoveEndpoint().
    for (size_t i = 0; i < mSimulatedClosurePanel.size(); i++)
    {
        if (mSimulatedClosurePanel[i]->GetEndpointId() != kInvalidEndpointId)
        {
            mSimulatedClosurePanel[i]->Unregister(provider);
        }
    }
    mSimulatedClosurePanel.clear();

    if (mOnOffLightPart != nullptr && mOnOffLightPart->GetEndpointId() != kInvalidEndpointId)
    {
        mLoggingOnOffLights->Unregister(provider);
    }
    mLoggingOnOffLights.reset();
    mOnOffLightPart = nullptr;
}

} // namespace app
} // namespace chip
