/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ClosureManager.h"

#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

constexpr Percent100ths kDefaultResolution   = 100;  // 1% resolution in Percent100ths.
constexpr Percent100ths kDefaultStepValue    = 1000; // 10% step value in Percent100ths.
constexpr uint32_t kMotionCountdownTimeMs    = 1000; // 1 second for each motion.
constexpr Percent100ths kMotionPositionStep  = 2000; // 20% of the total range per motion interval.
constexpr uint32_t kCountdownTimeSeconds     = 10;
constexpr uint32_t kCalibrateCountdownTimeMs = 3000; // 3 seconds for calibrate motion

constexpr uint8_t kNamespaceClosure      = 0x44;
constexpr uint8_t kTagClosureCovering    = 0x00;
constexpr uint8_t kNamespaceCovering     = 0x46;
constexpr uint8_t kTagCoveringVenetian   = 0x03;
constexpr uint8_t kNamespaceClosurePanel = 0x45;
constexpr uint8_t kTagClosurePanelLift   = 0x00;
constexpr uint8_t kTagClosurePanelTilt   = 0x01;

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosureEndpoint1TagList[] = {
    {
        .namespaceID = kNamespaceClosure,
        .tag         = kTagClosureCovering,
        .label       = MakeOptional(DataModel::Nullable<CharSpan>("Closure.Covering"_span)),
    },
    {
        .namespaceID = kNamespaceCovering,
        .tag         = kTagCoveringVenetian,
        .label       = MakeOptional(DataModel::Nullable<CharSpan>("Covering.Venetian"_span)),
    },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint2TagList[] = {
    {
        .namespaceID = kNamespaceClosurePanel,
        .tag         = kTagClosurePanelLift,
        .label       = MakeOptional(DataModel::Nullable<CharSpan>("ClosurePanel.Lift"_span)),
    },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint3TagList[] = {
    {
        .namespaceID = kNamespaceClosurePanel,
        .tag         = kTagClosurePanelTilt,
        .label       = MakeOptional(DataModel::Nullable<CharSpan>("ClosurePanel.Tilt"_span)),
    },
};

ClosureManager ClosureManager::sInstance;

void ClosureManager::Init()
{
    ChipLogDetail(AppServer, "ClosureManager::Init");

    SuccessOrDie(mClosureEndpoint1.Init());

    ClosureDimension::ClusterConformance conformanceEndpoint2;
    conformanceEndpoint2.FeatureMap()
        .Set(ClosureDimension::Feature::kPositioning)
        .Set(ClosureDimension::Feature::kMotionLatching)
        .Set(ClosureDimension::Feature::kUnit)
        .Set(ClosureDimension::Feature::kLimitation)
        .Set(ClosureDimension::Feature::kSpeed)
        .Set(ClosureDimension::Feature::kTranslation);

    ClosureDimension::ClusterInitParameters clusterInitParametersEndpoint2;
    clusterInitParametersEndpoint2.resolution           = kDefaultResolution;
    clusterInitParametersEndpoint2.stepValue            = kDefaultStepValue;
    clusterInitParametersEndpoint2.unit                 = ClosureUnitEnum::kMillimeter;
    clusterInitParametersEndpoint2.translationDirection = TranslationDirectionEnum::kDownward;
    clusterInitParametersEndpoint2.overflow             = OverflowEnum::kTopInside;
    clusterInitParametersEndpoint2.modulationType       = ModulationTypeEnum::kVentilation;
    clusterInitParametersEndpoint2.latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);

    SuccessOrDie(mClosurePanelEndpoint2.Init(conformanceEndpoint2, clusterInitParametersEndpoint2));

    ClosureDimension::ClusterConformance conformanceEndpoint3;
    conformanceEndpoint3.FeatureMap()
        .Set(ClosureDimension::Feature::kPositioning)
        .Set(ClosureDimension::Feature::kMotionLatching)
        .Set(ClosureDimension::Feature::kUnit)
        .Set(ClosureDimension::Feature::kLimitation)
        .Set(ClosureDimension::Feature::kSpeed)
        .Set(ClosureDimension::Feature::kRotation);

    ClosureDimension::ClusterInitParameters clusterInitParametersEndpoint3;
    clusterInitParametersEndpoint3.resolution     = kDefaultResolution;
    clusterInitParametersEndpoint3.stepValue      = kDefaultStepValue;
    clusterInitParametersEndpoint3.unit           = ClosureUnitEnum::kDegree;
    clusterInitParametersEndpoint3.rotationAxis   = RotationAxisEnum::kCenteredVertical;
    clusterInitParametersEndpoint3.overflow       = OverflowEnum::kTopInside;
    clusterInitParametersEndpoint3.modulationType = ModulationTypeEnum::kVentilation;
    clusterInitParametersEndpoint3.latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);

    SuccessOrDie(mClosurePanelEndpoint3.Init(conformanceEndpoint3, clusterInitParametersEndpoint3));

    SuccessOrDie(SetTagList(/* endpoint= */ kClosureEndpoint1,
                            Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosureEndpoint1TagList)));

    SuccessOrDie(SetTagList(/* endpoint= */ kClosurePanelEndpoint2,
                            Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint2TagList)));
    SuccessOrDie(SetTagList(/* endpoint= */ kClosurePanelEndpoint3,
                            Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint3TagList)));

    VerifyOrDie(SetClosureControlInitialState(mClosureEndpoint1) == CHIP_NO_ERROR);
    VerifyOrDie(SetClosurePanelInitialState(mClosurePanelEndpoint2) == CHIP_NO_ERROR);
    VerifyOrDie(SetClosurePanelInitialState(mClosurePanelEndpoint3) == CHIP_NO_ERROR);

    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate)
    {
        CHIP_ERROR err = pTestEventDelegate->AddHandler(&mClosureEndpoint1.GetDelegate());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to add handler for delegate: %s", ErrorStr(err));
        }
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot add handler for delegate");
    }

    ChipLogDetail(AppServer, "ClosureManager::Init done!");
}

void ClosureManager::Shutdown()
{
    ChipLogDetail(AppServer, "ClosureManager::Shutdown");

    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate)
    {
        pTestEventDelegate->RemoveHandler(&mClosureEndpoint1.GetDelegate());
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot remove handler for delegate");
    }
}

Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    ChipLogDetail(AppServer, "ClosureManager::OnCalibrateCommand");
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);

    mEp1CurrentAction              = ClosureAction::kCalibrateAction;
    mIsCalibrationActionInProgress = true;

    // For sample application, we are using a timer to simulate the hardware calibration action.
    // In a real application, this would be replaced with actual calibration logic and call HandleClosureActionComplete.
    VerifyOrReturnValue(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kCalibrateCountdownTimeMs),
                                                              HandleEp1ClosureActionTimer, nullptr) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));

    ChipLogProgress(AppServer, "ClosureManager: Calibration action started for endpoint 1");
    return Status::Success;
}

CHIP_ERROR ClosureManager::SetClosureControlInitialState(ClosureControlEndpoint & closureControlEndpoint)
{
    ChipLogDetail(AppServer, "ClosureManager::SetClosureControlInitialState");
    BitFlags<ClosureControl::Feature> featureMap = closureControlEndpoint.GetClusterInstance().GetFeatureMap();
    Optional<DataModel::Nullable<CurrentPositionEnum>> currentPosition = featureMap.Has(ClosureControl::Feature::kPositioning)
        ? MakeOptional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed))
        : NullOptional;
    Optional<DataModel::Nullable<bool>> currentLatch =
        featureMap.Has(ClosureControl::Feature::kMotionLatching) ? MakeOptional(DataModel::MakeNullable(true)) : NullOptional;
    Optional<Globals::ThreeLevelAutoEnum> speed =
        featureMap.Has(ClosureControl::Feature::kSpeed) ? MakeOptional(Globals::ThreeLevelAutoEnum::kAuto) : NullOptional;
    DataModel::Nullable<GenericOverallCurrentState> overallState(
        GenericOverallCurrentState(currentPosition, currentLatch, speed, DataModel::MakeNullable(true)));
    ReturnErrorOnFailure(closureControlEndpoint.GetClusterInstance().SetOverallCurrentState(overallState));

    Optional<DataModel::Nullable<TargetPositionEnum>> targetPosition =
        featureMap.Has(ClosureControl::Feature::kPositioning) ? MakeOptional(DataModel::NullNullable) : NullOptional;
    Optional<DataModel::Nullable<bool>> targetLatch =
        featureMap.Has(ClosureControl::Feature::kMotionLatching) ? MakeOptional(DataModel::NullNullable) : NullOptional;
    DataModel::Nullable<GenericOverallTargetState> overallTarget(GenericOverallTargetState(targetPosition, targetLatch, speed));
    ReturnErrorOnFailure(closureControlEndpoint.GetClusterInstance().SetOverallTargetState(overallTarget));

    if (featureMap.Has(ClosureControl::Feature::kPositioning) && !featureMap.Has(ClosureControl::Feature::kInstantaneous))
    {
        ReturnErrorOnFailure(closureControlEndpoint.GetClusterInstance().SetCountdownTimeFromDelegate(DataModel::NullNullable));
    }
    ReturnErrorOnFailure(closureControlEndpoint.GetClusterInstance().SetMainState(MainStateEnum::kStopped));

    return CHIP_NO_ERROR;
}
CHIP_ERROR ClosureManager::SetClosurePanelInitialState(ClosureDimensionEndpoint & closurePanelEndpoint)
{
    ChipLogDetail(AppServer, "ClosureManager::SetClosurePanelInitialState");
    ClosureDimension::ClusterConformance conformance = closurePanelEndpoint.GetClusterInstance().GetConformance();

    Optional<DataModel::Nullable<Percent100ths>> currentPosition = conformance.HasFeature(ClosureDimension::Feature::kPositioning)
        ? MakeOptional(DataModel::MakeNullable<Percent100ths>(10000))
        : NullOptional;
    Optional<DataModel::Nullable<bool>> currentLatch = conformance.HasFeature(ClosureDimension::Feature::kMotionLatching)
        ? MakeOptional(DataModel::MakeNullable(true))
        : NullOptional;
    Optional<Globals::ThreeLevelAutoEnum> speed =
        conformance.HasFeature(ClosureDimension::Feature::kSpeed) ? MakeOptional(Globals::ThreeLevelAutoEnum::kAuto) : NullOptional;
    DataModel::Nullable<GenericDimensionStateStruct> currentState(
        GenericDimensionStateStruct(currentPosition, currentLatch, speed));
    ReturnErrorOnFailure(closurePanelEndpoint.GetClusterInstance().SetCurrentState(currentState));

    Optional<DataModel::Nullable<Percent100ths>> targetPosition =
        conformance.HasFeature(ClosureDimension::Feature::kPositioning) ? MakeOptional(DataModel::NullNullable) : NullOptional;
    Optional<DataModel::Nullable<bool>> targetLatch =
        conformance.HasFeature(ClosureDimension::Feature::kMotionLatching) ? MakeOptional(DataModel::NullNullable) : NullOptional;
    DataModel::Nullable<GenericDimensionStateStruct> targetState(GenericDimensionStateStruct(targetPosition, targetLatch, speed));
    ReturnErrorOnFailure(closurePanelEndpoint.GetClusterInstance().SetTargetState(targetState));

    if (conformance.HasFeature(ClosureDimension::Feature::kUnit))
    {
        if (conformance.HasFeature(ClosureDimension::Feature::kRotation))
        {
            ReturnErrorOnFailure(
                closurePanelEndpoint.GetClusterInstance().SetUnitRange(ClosureDimension::Structs::UnitRangeStruct::Type{
                    .min = static_cast<int16_t>(0), .max = static_cast<int16_t>(360) })); // degree
        }
        else
        {
            ReturnErrorOnFailure(
                closurePanelEndpoint.GetClusterInstance().SetUnitRange(ClosureDimension::Structs::UnitRangeStruct::Type{
                    .min = static_cast<int16_t>(0), .max = static_cast<int16_t>(10000) })); // millimeter
        }
    }
    if (conformance.HasFeature(ClosureDimension::Feature::kLimitation))
    {
        ClosureDimension::Structs::RangePercent100thsStruct::Type limitRange{ .min = static_cast<Percent100ths>(0),
                                                                              .max = static_cast<Percent100ths>(10000) };
        ReturnErrorOnFailure(closurePanelEndpoint.GetClusterInstance().SetLimitRange(limitRange));
    }

    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position,
                                                                    const Optional<bool> & latch,
                                                                    const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    ChipLogDetail(AppServer, "ClosureManager::OnMoveToCommand");
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    mEp2CurrentAction    = ClosureAction::kInvalidAction;
    mEp2MotionInProgress = false;
    mEp3CurrentAction    = ClosureAction::kInvalidAction;
    mEp3MotionInProgress = false;

    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetClusterInstance().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetClusterInstance().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetClusterInstance().GetTargetState(ep2TargetState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetClusterInstance().GetTargetState(ep3TargetState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturnError(!ep2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturnError(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    GenericDimensionStateStruct ep2Target = ep2TargetState.IsNull() ? GenericDimensionStateStruct() : ep2TargetState.Value();
    GenericDimensionStateStruct ep3Target = ep3TargetState.IsNull() ? GenericDimensionStateStruct() : ep3TargetState.Value();
    ClosureDimension::ClusterConformance ep2Conformance = mClosurePanelEndpoint2.GetClusterInstance().GetConformance();
    ClosureDimension::ClusterConformance ep3Conformance = mClosurePanelEndpoint3.GetClusterInstance().GetConformance();
    if (position.HasValue())
    {
        Percent100ths ep2Position;
        Percent100ths ep3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kMoveToFullyClosed:
            ep2Position = static_cast<Percent100ths>(10000);
            ep3Position = static_cast<Percent100ths>(10000);
            break;
        case TargetPositionEnum::kMoveToFullyOpen:
            ep2Position = static_cast<Percent100ths>(0);
            ep3Position = static_cast<Percent100ths>(0);
            break;
        case TargetPositionEnum::kMoveToPedestrianPosition:
            ep2Position = static_cast<Percent100ths>(6000);
            ep3Position = static_cast<Percent100ths>(6000);
            break;
        case TargetPositionEnum::kMoveToSignaturePosition:
            ep2Position = static_cast<Percent100ths>(4000);
            ep3Position = static_cast<Percent100ths>(4000);
            break;
        case TargetPositionEnum::kMoveToVentilationPosition:
            ep2Position = static_cast<Percent100ths>(2000);
            ep3Position = static_cast<Percent100ths>(2000);
            break;
        default:
            ChipLogError(AppServer, "Invalid target position received in OnMoveToCommand");
            return Status::Failure;
        }

        if (ep2Conformance.HasFeature(ClosureDimension::Feature::kPositioning))
        {
            ep2Target.position.SetValue(DataModel::MakeNullable(ep2Position));
        }
        if (ep3Conformance.HasFeature(ClosureDimension::Feature::kPositioning))
        {
            ep3Target.position.SetValue(DataModel::MakeNullable(ep3Position));
        }
    }

    if (latch.HasValue())
    {
        if (ep2Conformance.HasFeature(ClosureDimension::Feature::kMotionLatching))
        {
            ep2Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
        }
        if (ep3Conformance.HasFeature(ClosureDimension::Feature::kMotionLatching))
        {
            ep3Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
        }
    }

    if (speed.HasValue())
    {
        if (ep2Conformance.HasFeature(ClosureDimension::Feature::kSpeed))
        {
            ep2Target.speed.SetValue(speed.Value());
        }
        if (ep3Conformance.HasFeature(ClosureDimension::Feature::kSpeed))
        {
            ep3Target.speed.SetValue(speed.Value());
        }
    }

    VerifyOrReturnError(mClosurePanelEndpoint2.GetClusterInstance().SetTargetState(DataModel::MakeNullable(ep2Target)) ==
                            CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(mClosurePanelEndpoint3.GetClusterInstance().SetTargetState(DataModel::MakeNullable(ep3Target)) ==
                            CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 3"));
    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    mEp1CurrentAction    = ClosureAction::kMoveToAction;
    mEp1MotionInProgress = true;
    LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                            HandleEp1ClosureActionTimer, this));
    return Status::Success;
}

Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    ChipLogDetail(AppServer, "ClosureManager::OnStopCommand");
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    mEp1CurrentAction = ClosureAction::kStopAction;
    HandleStopActionComplete();
    return Status::Success;
}

Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & position,
                                                                       const Optional<bool> & latch,
                                                                       const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                       const EndpointId endpointId)
{
    ChipLogDetail(AppServer, "ClosureManager::OnSetTargetCommand");
    MainStateEnum ep1MainState = mClosureEndpoint1.GetClusterInstance().GetMainState();

    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    DataModel::Nullable<GenericOverallTargetState> overallTargetState =
        mClosureEndpoint1.GetClusterInstance().GetOverallTargetState();

    if (overallTargetState.IsNull())
    {
        overallTargetState.SetNonNull(GenericOverallTargetState{});
    }

    ClosureDimensionEndpoint & mClosurePanelEndpoint =
        endpointId == kClosurePanelEndpoint2 ? mClosurePanelEndpoint2 : mClosurePanelEndpoint3;
    ClosureDimension::ClusterConformance mClosurePanelConformance = mClosurePanelEndpoint.GetClusterInstance().GetConformance();

    if (position.HasValue() && mClosurePanelConformance.HasFeature(ClosureDimension::Feature::kPositioning))
    {
        overallTargetState.Value().position.SetValue(DataModel::NullNullable);
    }

    if (latch.HasValue() && mClosurePanelConformance.HasFeature(ClosureDimension::Feature::kMotionLatching))
    {
        overallTargetState.Value().latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    if (speed.HasValue() && mClosurePanelConformance.HasFeature(ClosureDimension::Feature::kSpeed))
    {
        overallTargetState.Value().speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR,
                        Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for SetTarget command for Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for SetTarget command on Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set main state for SetTarget command on Endpoint 1"));

    bool needUnlatch = false;

    if (latch.HasValue() && !latch.Value() && mClosurePanelConformance.HasFeature(ClosureDimension::Feature::kMotionLatching))
    {
        DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
        if (mClosurePanelEndpoint.GetClusterInstance().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR &&
            !panelCurrentState.IsNull() && panelCurrentState.Value().latch.HasValue() &&
            !panelCurrentState.Value().latch.Value().IsNull())
        {
            needUnlatch = panelCurrentState.Value().latch.Value().Value();
        }
    }

    ClosureAction actionToSchedule =
        needUnlatch ? ClosureManager::ClosureAction::kPanelUnLatchAction : ClosureManager::ClosureAction::kSetTargetAction;

    if (endpointId == kClosurePanelEndpoint2)
    {
        mEp2CurrentAction    = actionToSchedule;
        mEp2MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp2ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for SetTarget command on Endpoint 2");
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        mEp3CurrentAction    = actionToSchedule;
        mEp3MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp3ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for SetTarget command on Endpoint 3");
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return Status::Failure;
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                  const uint16_t & numberOfSteps,
                                                                  const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                  const EndpointId endpointId)
{
    ChipLogDetail(AppServer, "ClosureManager::OnStepCommand");
    MainStateEnum ep1MainState = mClosureEndpoint1.GetClusterInstance().GetMainState();

    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(mClosureEndpoint1.GetClusterInstance().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    DataModel::Nullable<GenericOverallTargetState> ep1Target = mClosureEndpoint1.GetClusterInstance().GetOverallTargetState();

    if (ep1Target.IsNull())
    {
        ep1Target.SetNonNull(GenericOverallTargetState{});
    }

    ep1Target.Value().position.SetValue(DataModel::NullNullable);

    VerifyOrReturnValue(mClosureEndpoint1.GetClusterInstance().SetOverallTargetState(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for Step command"));

    if (endpointId == kClosurePanelEndpoint2)
    {
        mEp2CurrentAction    = ClosureManager::ClosureAction::kStepAction;
        mEp2MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp2ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for Step command on Endpoint 2");
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        mEp3CurrentAction    = ClosureManager::ClosureAction::kStepAction;
        mEp3MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp3ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for Step command on Endpoint 3");
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return Status::Failure;
    }
    return Status::Success;
}

ClosureDimensionEndpoint * ClosureManager::GetCurrentPanelInstance(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();
    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        return &instance.mClosurePanelEndpoint2;
    }
    if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        return &instance.mClosurePanelEndpoint3;
    }
    return nullptr;
}

void ClosureManager::HandleEp1ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp1ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp1CurrentAction));

    switch (instance.mEp1CurrentAction)
    {
    case ClosureAction::kCalibrateAction:
        instance.HandleCalibrateActionComplete();
        break;
    case ClosureAction::kStopAction:
        // Add logic to handle Stop action completion
        break;
    case ClosureAction::kMoveToAction:
        instance.HandleClosureMotionAction();
        break;
    case ClosureAction::kLatchAction:
        // Add logic to handle Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp1ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp2ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp2ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp2CurrentAction));

    switch (instance.mEp2CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        instance.HandlePanelSetTargetAction(kClosurePanelEndpoint2);
        break;
    case ClosureAction::kStepAction:
        instance.HandlePanelStepAction(kClosurePanelEndpoint2);
        break;
    case ClosureAction::kPanelUnLatchAction:
        instance.HandlePanelUnlatchAction(kClosurePanelEndpoint2);
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp2ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp3ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp3ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp3CurrentAction));

    switch (instance.mEp3CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        instance.HandlePanelSetTargetAction(kClosurePanelEndpoint3);
        break;
    case ClosureAction::kStepAction:
        instance.HandlePanelStepAction(kClosurePanelEndpoint3);
        break;
    case ClosureAction::kPanelUnLatchAction:
        instance.HandlePanelUnlatchAction(kClosurePanelEndpoint3);
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp3ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandlePanelStepAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));
    StepDirectionEnum stepDirection = ep->GetDelegate().GetStepCommandTargetDirection();

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState;

    VerifyOrReturn(ep->GetClusterInstance().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Step action"));
    VerifyOrReturn(ep->GetClusterInstance().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Step action"));

    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is null, Step action Failed"));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target state  is null, Step action Failed"));
    VerifyOrReturn(panelCurrentState.Value().position.HasValue() && !panelCurrentState.Value().position.Value().IsNull(),
                   ChipLogError(AppServer, "Current or target position is not set, Step action Failed"));
    VerifyOrReturn(panelTargetState.Value().position.HasValue() && !panelTargetState.Value().position.Value().IsNull(),
                   ChipLogError(AppServer, "Current or target position is not set, Step action Failed"));

    Percent100ths currentPosition = panelCurrentState.Value().position.Value().Value();
    Percent100ths targetPosition  = panelTargetState.Value().position.Value().Value();

    ChipLogProgress(AppServer, "Current Position: %d, Target Position: %d", currentPosition, targetPosition);

    bool panelTargetReached = (currentPosition == targetPosition);
    ChipLogProgress(AppServer, "Panel Target Reached: %s", panelTargetReached ? "true" : "false");

    if (!panelTargetReached)
    {
        Percent100ths nextCurrentPosition;
        Percent100ths stepValue;
        VerifyOrReturn(ep->GetClusterInstance().GetStepValue(stepValue) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get step value for Step action"));

        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            nextCurrentPosition = std::min(static_cast<Percent100ths>(currentPosition + stepValue), targetPosition);
        }
        else
        {
            // Underflow protection: if currentPosition <= stepValue, set to 0.
            Percent100ths decreasedCurrentPosition = (currentPosition > stepValue)
                ? static_cast<Percent100ths>(currentPosition - stepValue)
                : static_cast<Percent100ths>(0);
            nextCurrentPosition                    = std::max(decreasedCurrentPosition, targetPosition);
        }

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextCurrentPosition));
        LogErrorOnFailure(ep->GetClusterInstance().SetCurrentState(panelCurrentState));

        if (endpointId == kClosurePanelEndpoint2)
        {
            instance.mEp2CurrentAction = ClosureManager::ClosureAction::kStepAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp2ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for Step Action command on Endpoint 2");
        }
        else if (endpointId == kClosurePanelEndpoint3)
        {
            instance.mEp3CurrentAction = ClosureManager::ClosureAction::kStepAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp3ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for Step Action command on Endpoint 3");
        }
        else
        {
            ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        }
        return;
    }

    HandlePanelStepActionComplete(endpointId);
}

void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
    ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(ep->GetClusterInstance().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(ep->GetClusterInstance().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    bool panelProgressPossible                      = false;
    bool positionUpdated                            = false;
    DataModel::Nullable<Percent100ths> nextPosition = DataModel::NullNullable;

    if (GetPanelNextPosition(panelCurrentState.Value(), panelTargetState.Value(), nextPosition))
    {
        VerifyOrReturn(!nextPosition.IsNull(), ChipLogError(AppServer, "Next position is not set for Endpoint %d", endpointId));

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextPosition.Value()));
        LogErrorOnFailure(ep->GetClusterInstance().SetCurrentState(panelCurrentState));

        panelProgressPossible = (nextPosition.Value() != panelTargetState.Value().position.Value().Value());
        positionUpdated       = true;
        ChipLogProgress(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", endpointId, nextPosition.Value(),
                        panelTargetState.Value().position.Value().Value());
    }

    if (panelProgressPossible)
    {
        if (endpointId == kClosurePanelEndpoint2)
        {
            mEp2CurrentAction = ClosureManager::ClosureAction::kSetTargetAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp2ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for SetTarget Action command on Endpoint 2");
        }
        else if (endpointId == kClosurePanelEndpoint3)
        {
            mEp3CurrentAction = ClosureManager::ClosureAction::kSetTargetAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp3ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for SetTarget Action command on Endpoint 3");
        }
        else
        {
            ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        }
        return;
    }

    if (ep->GetClusterInstance().GetConformance().HasFeature(ClosureDimension::Feature::kMotionLatching))
    {
        if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
            panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull())
        {
            if (!panelCurrentState.Value().latch.Value().Value() && panelTargetState.Value().latch.Value().Value())
            {
                DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState =
                    mClosureEndpoint1.GetClusterInstance().GetOverallCurrentState();
                VerifyOrReturn(!ep1OverallCurrentState.IsNull(),
                               ChipLogError(AppServer, "Overall current state is not set for Endpoint 1"));

                // In Real application, this would be replaced with actual latch logic.
                ChipLogProgress(AppServer, "Performing latch action");

                ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
                ep1OverallCurrentState.Value().secureState.SetNonNull(false);
                LogErrorOnFailure(mClosureEndpoint1.GetClusterInstance().SetOverallCurrentState(ep1OverallCurrentState));

                panelCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
                LogErrorOnFailure(ep->GetClusterInstance().SetCurrentState(panelCurrentState));

                ChipLogProgress(AppServer, "Latch action completed");
            }
        }
    }

    HandlePanelSetTargetActionComplete(endpointId, positionUpdated);
}

void ClosureManager::HandlePanelUnlatchAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    ClosureDimensionEndpoint * panelEp = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(panelEp != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(panelEp->GetClusterInstance().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(panelEp->GetClusterInstance().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull() &&
        (panelCurrentState.Value().latch.Value().Value() && !panelTargetState.Value().latch.Value().Value()))
    {
        DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState =
            mClosureEndpoint1.GetClusterInstance().GetOverallCurrentState();
        VerifyOrReturn(!ep1OverallCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint 1"));

        // In Real application, this would be replaced with actual unlatch logic.
        ChipLogProgress(AppServer, "Performing unlatch action");

        ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
        ep1OverallCurrentState.Value().secureState.SetNonNull(false);
        LogErrorOnFailure(mClosureEndpoint1.GetClusterInstance().SetOverallCurrentState(ep1OverallCurrentState));

        panelCurrentState.Value().latch.SetValue(false);
        LogErrorOnFailure(panelEp->GetClusterInstance().SetCurrentState(panelCurrentState));

        ChipLogProgress(AppServer, "Unlatched action completed");
    }

    if (endpointId == kClosurePanelEndpoint2)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return;
    }

    instance.HandlePanelSetTargetAction(endpointId);
}

void ClosureManager::HandleCalibrateActionComplete()
{
    ChipLogProgress(AppServer, "HandleCalibrateActionComplete called");

    mClosureEndpoint1.OnCalibrateActionComplete();
    mClosurePanelEndpoint2.OnCalibrateActionComplete();
    mClosurePanelEndpoint3.OnCalibrateActionComplete();
    mIsCalibrationActionInProgress = false;
    mEp1CurrentAction              = ClosureAction::kInvalidAction;
}

void ClosureManager::HandleMoveToActionComplete()
{
    ChipLogProgress(AppServer, "HandleMoveToActionComplete called");
    mClosureEndpoint1.OnMoveToActionComplete();
    mClosurePanelEndpoint2.OnMoveToActionComplete();
    mClosurePanelEndpoint3.OnMoveToActionComplete();
    mEp1MotionInProgress = false;
    mEp1CurrentAction    = ClosureAction::kInvalidAction;
}

void ClosureManager::HandleStopActionComplete()
{
    ChipLogProgress(AppServer, "HandleStopActionComplete called");
    if (mIsCalibrationActionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping calibration action");
        mClosureEndpoint1.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint2.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint3.OnStopCalibrateActionComplete();
        mIsCalibrationActionInProgress = false;
    }
    else if (mEp1MotionInProgress || mEp2MotionInProgress || mEp3MotionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping move to action");
        mClosureEndpoint1.OnStopMotionActionComplete();
        mEp1MotionInProgress = false;
        mEp1CurrentAction    = ClosureAction::kInvalidAction;
        if (mEp2MotionInProgress)
        {
            mClosurePanelEndpoint2.OnStopMotionActionComplete();
            mEp2MotionInProgress = false;
            mEp2CurrentAction    = ClosureAction::kInvalidAction;
        }
        if (mEp3MotionInProgress)
        {
            mClosurePanelEndpoint3.OnStopMotionActionComplete();
            mEp3MotionInProgress = false;
            mEp3CurrentAction    = ClosureAction::kInvalidAction;
        }
    }
    else
    {
        ChipLogDetail(AppServer, "No action in progress to stop");
    }
}

void ClosureManager::HandlePanelSetTargetActionComplete(EndpointId endpointId, bool positionUpdated)
{
    ChipLogProgress(AppServer, "HandleSetTargetActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();

    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        instance.mEp2CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp2MotionInProgress = false;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        instance.mEp3CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp3MotionInProgress = false;
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return;
    }

    if (!instance.mEp2MotionInProgress && !instance.mEp3MotionInProgress)
    {
        if (positionUpdated)
        {
            instance.mClosureEndpoint1.OnPanelMotionActionComplete();
        }
        else
        {
            (void) instance.mClosureEndpoint1.GetClusterInstance().SetMainState(ClosureControl::MainStateEnum::kStopped);
            (void) instance.mClosureEndpoint1.GetClusterInstance().SetCountdownTimeFromDelegate(DataModel::NullNullable);

            DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState =
                instance.mClosureEndpoint1.GetClusterInstance().GetOverallCurrentState();

            if (!ep1CurrentState.IsNull())
            {
                ep1CurrentState.Value().speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
                LogErrorOnFailure(instance.mClosureEndpoint1.GetClusterInstance().SetOverallCurrentState(ep1CurrentState));
            }
        }
    }

    ChipLogProgress(AppServer, "SetTarget action completed for Endpoint %d", endpointId);
}

void ClosureManager::HandlePanelStepActionComplete(EndpointId endpointId)
{
    ChipLogProgress(AppServer, "HandleStepActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();

    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        instance.mEp2CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp2MotionInProgress = false;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        instance.mEp3CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp3MotionInProgress = false;
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for Step command: %d", endpointId);
        return;
    }

    if (!instance.mEp2MotionInProgress && !instance.mEp3MotionInProgress)
    {
        instance.mClosureEndpoint1.OnPanelMotionActionComplete();
    }

    ChipLogProgress(AppServer, "Step action completed for Endpoint %d", endpointId);
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState =
        mClosureEndpoint1.GetClusterInstance().GetOverallCurrentState();
    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;

    DataModel::Nullable<GenericOverallTargetState> ep1TargetState = mClosureEndpoint1.GetClusterInstance().GetOverallTargetState();
    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;

    VerifyOrReturn(mClosurePanelEndpoint2.GetClusterInstance().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetClusterInstance().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(mClosurePanelEndpoint2.GetClusterInstance().GetTargetState(ep2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetClusterInstance().GetTargetState(ep3TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturn(!ep1CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!ep2CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!ep3CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    VerifyOrReturn(!ep1TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 1"));
    VerifyOrReturn(!ep2TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 2"));
    VerifyOrReturn(!ep3TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 3"));

    if (mClosureEndpoint1.GetClusterInstance().GetFeatureMap().Has(ClosureControl::Feature::kMotionLatching))
    {
        if (ep1CurrentState.Value().latch.HasValue() && !ep1CurrentState.Value().latch.Value().IsNull() &&
            ep1TargetState.Value().latch.HasValue() && !ep1TargetState.Value().latch.Value().IsNull())
        {
            if (ep1CurrentState.Value().latch.Value().Value() && !ep1TargetState.Value().latch.Value().Value())
            {
                // In Real application, this would be replaced with actual unlatch logic.
                ChipLogProgress(AppServer, "Performing unlatch action");
                ep1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
                ep1CurrentState.Value().secureState.SetNonNull(false);
                LogErrorOnFailure(instance.mClosureEndpoint1.GetClusterInstance().SetOverallCurrentState(ep1CurrentState));
                ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
                LogErrorOnFailure(instance.mClosurePanelEndpoint2.GetClusterInstance().SetCurrentState(ep2CurrentState));
                ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
                LogErrorOnFailure(instance.mClosurePanelEndpoint3.GetClusterInstance().SetCurrentState(ep3CurrentState));
                ChipLogProgress(AppServer, "Unlatched action completed");
            }
        }
    }

    DataModel::Nullable<Percent100ths> ep2NextPosition = DataModel::NullNullable;
    DataModel::Nullable<Percent100ths> ep3NextPosition = DataModel::NullNullable;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    if (GetPanelNextPosition(ep2CurrentState.Value(), ep2TargetState.Value(), ep2NextPosition))
    {
        VerifyOrReturn(!ep2NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        ep2CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep2NextPosition.Value()));
        LogErrorOnFailure(instance.mClosurePanelEndpoint2.GetClusterInstance().SetCurrentState(ep2CurrentState));
        isEndPoint2ProgressPossible = (ep2NextPosition.Value() != ep2TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", ep2NextPosition.Value(),
                        ep2TargetState.Value().position.Value().Value());
    }

    if (GetPanelNextPosition(ep3CurrentState.Value(), ep3TargetState.Value(), ep3NextPosition))
    {
        VerifyOrReturn(!ep3NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        ep3CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep3NextPosition.Value()));
        LogErrorOnFailure(instance.mClosurePanelEndpoint3.GetClusterInstance().SetCurrentState(ep3CurrentState));
        isEndPoint3ProgressPossible = (ep3NextPosition.Value() != ep3TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", ep3NextPosition.Value(),
                        ep3TargetState.Value().position.Value().Value());
    }

    bool isProgressPossible = isEndPoint2ProgressPossible || isEndPoint3ProgressPossible;

    ChipLogProgress(AppServer, "Motion progress possible: %s", isProgressPossible ? "true" : "false");

    if (isProgressPossible)
    {
        mEp1CurrentAction    = ClosureAction::kMoveToAction;
        mEp1MotionInProgress = true;
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                                HandleEp1ClosureActionTimer, this));
        ChipLogProgress(AppServer, "Rescheduled HandleEp1ClosureActionTimer for motion action");
        return;
    }

    if (mClosureEndpoint1.GetClusterInstance().GetFeatureMap().Has(ClosureControl::Feature::kMotionLatching))
    {
        if (ep1CurrentState.Value().latch.HasValue() && !ep1CurrentState.Value().latch.Value().IsNull() &&
            ep1TargetState.Value().latch.HasValue() && !ep1TargetState.Value().latch.Value().IsNull())
        {
            // If currently latched (false) and target is unlatched (true), unlatch first before moving
            if (!ep1CurrentState.Value().latch.Value().Value() && ep1TargetState.Value().latch.Value().Value())
            {
                // In Real application, this would be replaced with actual latch logic.
                ChipLogProgress(AppServer, "Performing latch action");
                ep1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
                if (ep1CurrentState.Value().position.HasValue() && !ep1CurrentState.Value().position.Value().IsNull())
                {
                    if (ep1CurrentState.Value().position.Value().Value() == CurrentPositionEnum::kFullyClosed)
                    {
                        ep1CurrentState.Value().secureState.SetNonNull(true);
                    }
                    else
                    {
                        ep1CurrentState.Value().secureState.SetNonNull(false);
                    }
                }
                LogErrorOnFailure(instance.mClosureEndpoint1.GetClusterInstance().SetOverallCurrentState(ep1CurrentState));
                ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
                LogErrorOnFailure(instance.mClosurePanelEndpoint2.GetClusterInstance().SetCurrentState(ep2CurrentState));
                ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
                LogErrorOnFailure(instance.mClosurePanelEndpoint3.GetClusterInstance().SetCurrentState(ep3CurrentState));
                ChipLogProgress(AppServer, "latched action complete");
            }
        }
    }

    instance.HandleMoveToActionComplete();
}

bool ClosureManager::GetPanelNextPosition(const GenericDimensionStateStruct & currentState,
                                          const GenericDimensionStateStruct & targetState,
                                          DataModel::Nullable<Percent100ths> & nextPosition)
{
    VerifyOrReturnValue(targetState.position.HasValue() && !targetState.position.Value().IsNull(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target position is not set"));

    VerifyOrReturnValue(currentState.position.HasValue() && !currentState.position.Value().IsNull(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set"));

    Percent100ths currentPosition = currentState.position.Value().Value();
    Percent100ths targetPosition  = targetState.position.Value().Value();

    if (currentPosition < targetPosition)
    {
        nextPosition.SetNonNull(std::min(static_cast<Percent100ths>(currentPosition + kMotionPositionStep), targetPosition));
    }
    else if (currentPosition > targetPosition)
    {
        Percent100ths newCurrentPosition = (currentPosition > kMotionPositionStep) ? currentPosition - kMotionPositionStep : 0;
        nextPosition.SetNonNull(std::max(newCurrentPosition, targetPosition));
    }
    else
    {
        nextPosition.SetNonNull(currentPosition);
        return false;
    }
    return true;
}
