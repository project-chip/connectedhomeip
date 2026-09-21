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

// helper to return tags for the closurepanel
#include "LoggingClosure.h"

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

LoggingClosure::LoggingClosure(TimerDelegate & Tdelegate, Clusters::IdentifyDelegate & Idelegate, Closure::Config CConfig,
                               Credentials::GroupDataProvider & groupDataProvider, FabricTable & fabricTable,
                               std::vector<PanelList> panels, TestEventTriggerDelegate & testEventTriggerDelegate) :
    Closure(CConfig, Tdelegate, Idelegate, *this), OnOffContext({ groupDataProvider, fabricTable, Tdelegate, Idelegate }),
    mPanelList(std::move(panels)), mTimerDelegate(Tdelegate), mTestEventTriggerDelegate(testEventTriggerDelegate)
{}

LoggingClosure::~LoggingClosure()
{
    CancelTimer();
}
Protocols::InteractionModel::Status LoggingClosure::HandleStopCommand()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::HandleStopCommand()");
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
LoggingClosure::HandleMoveToCommand(const Optional<Clusters::ClosureControl::TargetPositionEnum> & position,
                                    const Optional<bool> & latch, const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::HandleMoveToCommand() -> position=%hhu latch=%d speed=%u",
                    position.ValueOr(Clusters::ClosureControl::TargetPositionEnum::kUnknownEnumValue), latch.ValueOr(false),
                    to_underlying(speed.ValueOr(Clusters::Globals::ThreeLevelAutoEnum::kAuto)));
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

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kTimeoutnDurationSec)));
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status LoggingClosure::HandleCalibrateCommand()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::HandleCalibrateCommand()");
    mPendingCurrentState.reset();
    LogErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kTimeoutnDurationSec)));
    return Protocols::InteractionModel::Status::Success;
}

bool LoggingClosure::IsReadyToMove()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::IsReadyToMove()");
    return true;
}
ElapsedS LoggingClosure::GetCalibrationCountdownTime()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::GetCalibrationCountdownTime()");
    return 0u;
}
ElapsedS LoggingClosure::GetMovingCountdownTime()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::GetMovingCountdownTime()");
    return 0u;
}
ElapsedS LoggingClosure::GetWaitingForMotionCountdownTime()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::GetMovingCountdownTime()");
    return 0u;
}
bool LoggingClosure::RegistersAccessDevicePanel() const
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

void LoggingClosure::TimerFired()
{
    ChipLogProgress(DeviceLayer, "LoggingClosure::TimerFired()");
    if (mPendingCurrentState.has_value())
    {
        LogErrorOnFailure(ClosureControlCluster().SetOverallCurrentState(DataModel::MakeNullable(*mPendingCurrentState)));
        mPendingCurrentState.reset();
    }
    LogErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kStopped));
    LogErrorOnFailure(ClosureControlCluster().GenerateMovementCompletedEvent());
}
CHIP_ERROR LoggingClosure::HandleEventTrigger(uint64_t eventTrigger)
{
    eventTrigger = clearEndpointInEventTrigger(eventTrigger);
    switch (eventTrigger)
    {
    case kTriggerError:
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleEventTrigger() -> Error");
        ReturnErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kError));
        return ClosureControlCluster().AddErrorToCurrentErrorList(Clusters::ClosureControl::ClosureErrorEnum::kBlockedBySensor);
    case kTriggerProtected:
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleEventTrigger() -> Protected");
        return Closure::ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kProtected);
    case kTriggerDisengaged:
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleEventTrigger() -> Disengaged");
        return Closure::ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kDisengaged);
    case kTriggerClear:
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleEventTrigger() -> Clear");
        ReturnErrorOnFailure(ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kStopped));
        ClosureControlCluster().ClearCurrentErrorList();
        return CHIP_NO_ERROR;
    case kTriggerSetupRequired:
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleEventTrigger() -> SetupRequired");
        return ClosureControlCluster().SetMainState(Clusters::ClosureControl::MainStateEnum::kSetupRequired);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT; // not ours — lets any other registered handler try instead
    }
}

void LoggingClosure::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}
CHIP_ERROR LoggingClosure::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{

    for (const auto & panel : mPanelList)
    {
        auto ClosurePanel = std::make_unique<LoggingClosurePanel>(panel.config);
        // the tag list will be provided in the devicefactory.h
        composition.tagList = panel.tags;
        ReturnErrorOnFailure(ClosurePanel->Register(allocator, provider, composition));
        mLoggingClosurePanel.push_back(std::move(ClosurePanel));
    }

    mLoggingOnOffLights = std::make_unique<LoggingOnOffLight>(OnOffContext);
    ReturnErrorOnFailure(
        mLoggingOnOffLights->Register(allocator.Allocate(), provider, EndpointComposition::WithParent(GetEndpointId())));

    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(this));
    return CHIP_NO_ERROR;
}

void LoggingClosure::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    mTestEventTriggerDelegate.RemoveHandler(this);

    for (size_t i = 0; i < mLoggingClosurePanel.size(); i++)
    {
        mLoggingClosurePanel[i]->Unregister(provider);
    }
    if (mLoggingOnOffLights)
    {
        mLoggingOnOffLights->Unregister(provider);
    }
}

} // namespace app
} // namespace chip
