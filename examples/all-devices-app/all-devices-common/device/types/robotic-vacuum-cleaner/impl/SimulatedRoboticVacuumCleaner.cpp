/*
 *
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

#include <clusters/RvcCleanMode/Enums.h>
#include <clusters/RvcRunMode/Enums.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcNamedPipeSimulation.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationLogic.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationTopology.h>
#include <device/types/robotic-vacuum-cleaner/impl/SimulatedRoboticVacuumCleaner.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

namespace chip::app {

using namespace Clusters;
using namespace Clusters::ServiceArea;
using namespace chip::app::all_devices::rvc_simulation;
using namespace chip::app::all_devices::rvc_simulation::Topology;

namespace {

using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

const ModeTagStructType kRunModeIdleTags[]     = { { .value = to_underlying(RvcRunMode::ModeTag::kIdle) } };
const ModeTagStructType kRunModeCleaningTags[] = { { .value = to_underlying(RvcRunMode::ModeTag::kCleaning) } };
const ModeTagStructType kRunModeMappingTags[]  = { { .value = to_underlying(RvcRunMode::ModeTag::kMapping) } };

struct RunModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};

const RunModeOption kRunModeOptions[] = {
    { "Idle"_span, kRunModeIdle, Span<const ModeTagStructType>(kRunModeIdleTags) },
    { "Cleaning"_span, kRunModeCleaning, Span<const ModeTagStructType>(kRunModeCleaningTags) },
    { "Mapping"_span, kRunModeMapping, Span<const ModeTagStructType>(kRunModeMappingTags) },
};

const ModeTagStructType kCleanModeQuickTags[]      = { { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kQuick) } };
const ModeTagStructType kCleanModeAutoTags[]       = { { .value = to_underlying(RvcCleanMode::ModeTag::kAuto) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) } };
const ModeTagStructType kCleanModeDeepCleanTags[]  = { { .value = to_underlying(RvcCleanMode::ModeTag::kMop) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kDeepClean) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) } };
const ModeTagStructType kCleanModeQuietTags[]      = { { .value = to_underlying(RvcCleanMode::ModeTag::kQuiet) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) } };
const ModeTagStructType kCleanModeMaxVacTags[]     = { { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kDeepClean) } };
const ModeTagStructType kCleanModeVacThenMopTags[] = { { .value = to_underlying(RvcCleanMode::ModeTag::kVacuum) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kMop) },
                                                       { .value = to_underlying(RvcCleanMode::ModeTag::kVacuumThenMop) } };

struct CleanModeOption
{
    CharSpan label;
    uint8_t value;
    Span<const ModeTagStructType> tags;
};

const CleanModeOption kCleanModeOptions[] = {
    { "Quick"_span, 0, Span<const ModeTagStructType>(kCleanModeQuickTags) },
    { "Auto"_span, 1, Span<const ModeTagStructType>(kCleanModeAutoTags) },
    { "Deep Clean"_span, 2, Span<const ModeTagStructType>(kCleanModeDeepCleanTags) },
    { "Quiet"_span, 3, Span<const ModeTagStructType>(kCleanModeQuietTags) },
    { "Max Vac"_span, 4, Span<const ModeTagStructType>(kCleanModeMaxVacTags) },
    { "Vacuum Then Mop"_span, 5, Span<const ModeTagStructType>(kCleanModeVacThenMopTags) },
};

bool IsOperating(RvcOperationalState::RvcOperationalStateCluster & cluster)
{
    return cluster.GetCurrentOperationalState() == to_underlying(OperationalState::OperationalStateEnum::kRunning);
}

} // namespace

SimulatedRoboticVacuumCleaner::SimulatedRoboticVacuumCleaner(const Context & context) :
    RoboticVacuumCleaner(RoboticVacuumCleaner::Config{
        .operationalStateDelegate = *this,
        .runModeDelegate          = mRunModeAppDelegate,
        .runModeStartupValue      = kRunModeIdle,
        .diagnosticDataProvider   = context.diagnosticDataProvider,
    }),
    mTimerDelegate(context.timerDelegate)
{}

SimulatedRoboticVacuumCleaner::~SimulatedRoboticVacuumCleaner()
{
    CancelTimer();
}

void SimulatedRoboticVacuumCleaner::Unregister(CodeDrivenDataModelProvider & provider)
{
    CancelTimer();
    RoboticVacuumCleaner::Unregister(provider);
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    ServiceAreaCluster::OptionalAttributeSet serviceAreaOptionalAttributes;
    serviceAreaOptionalAttributes.Set<Attributes::SupportedMaps::Id>();
    serviceAreaOptionalAttributes.Set<Attributes::CurrentArea::Id>();
    serviceAreaOptionalAttributes.Set<Attributes::EstimatedEndTime::Id>();
    serviceAreaOptionalAttributes.Set<Attributes::Progress::Id>();

    mServiceAreaCluster.Create(endpoint, mServiceAreaStorageDelegate, *this,
                               BitMask<Feature>(Feature::kMaps, Feature::kProgressReporting), serviceAreaOptionalAttributes);
    ReturnErrorOnFailure(provider.AddCluster(mServiceAreaCluster.Registration()));

    mCleanModeCluster.Create(endpoint, Clusters::ModeBase::kRvcCleanMode,
                             Clusters::ModeBaseCluster::Config{
                                 .feature                = BitMask<Clusters::ModeBase::Feature>(),
                                 .optionalAttributeSet   = {},
                                 .appDelegate            = mCleanModeAppDelegate,
                                 .onOffValueForStartUp   = false,
                                 .diagnosticDataProvider = GetDiagnosticDataProvider(),
                             });
    ReturnErrorOnFailure(provider.AddCluster(mCleanModeCluster.Registration()));

    ReturnErrorOnFailure(Init());

    mCleanModeCluster.Cluster().UpdateCurrentMode(kCleanModeQuick);
    RegisterRvcNamedPipeSimulation(GetEndpointId(), this);
    return CHIP_NO_ERROR;
}

void SimulatedRoboticVacuumCleaner::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    UnregisterRvcNamedPipeSimulation(GetEndpointId());
    if (mCleanModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCleanModeCluster.Cluster()));
        mCleanModeCluster.Destroy();
    }
    if (mServiceAreaCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mServiceAreaCluster.Cluster()));
        mServiceAreaCluster.Destroy();
    }
}

void SimulatedRoboticVacuumCleaner::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void SimulatedRoboticVacuumCleaner::StartActivityTimer()
{
    CancelTimer();
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kActivitySimulationDurationSec)));
}

void SimulatedRoboticVacuumCleaner::StartChargingTimer()
{
    CancelTimer();
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kChargingSimulationDurationSec)));
}

void SimulatedRoboticVacuumCleaner::TimerFired()
{
    uint8_t state = OperationalState().GetCurrentOperationalState();
    if (state == to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: simulated activity finished.");
        HandleActivityComplete();
    }
    else if (state == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
    {
        ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: simulated charging finished.");
        HandleCharged();
    }
}

// -- OperationalState::Delegate --

CHIP_ERROR SimulatedRoboticVacuumCleaner::GetOperationalStateAtIndex(size_t index,
                                                                     OperationalState::GenericOperationalState & operationalState)
{
    static const OperationalState::GenericOperationalState kSupportedStates[] = {
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped),
                                                  MakeOptional("Stopped"_span)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning),
                                                  MakeOptional("Running"_span)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused),
                                                  MakeOptional("Paused"_span)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kError),
                                                  MakeOptional("Error"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger),
                                                  MakeOptional("SeekingCharger"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging),
                                                  MakeOptional("Charging"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked),
                                                  MakeOptional("Docked"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kEmptyingDustBin),
                                                  MakeOptional("EmptyingDustBin"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCleaningMop),
                                                  MakeOptional("CleaningMop"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kFillingWaterTank),
                                                  MakeOptional("FillingWaterTank"_span)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kUpdatingMaps),
                                                  MakeOptional("UpdatingMaps"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

void SimulatedRoboticVacuumCleaner::ClearDockChargingTracking()
{
    mPhysicalDockState = PhysicalDockState::kOffDock;
}

void SimulatedRoboticVacuumCleaner::SetDeviceToIdleState()
{
    switch (mPhysicalDockState)
    {
    case PhysicalDockState::kOnDockCharging:
        LogErrorOnFailure(
            OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
        break;
    case PhysicalDockState::kOnDock:
        LogErrorOnFailure(
            OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
        break;
    case PhysicalDockState::kOffDock:
        LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped)));
        break;
    }
}

void SimulatedRoboticVacuumCleaner::UpdateServiceAreaProgressOnExit()
{
    VerifyOrReturn(GetServiceAreaCluster().HasFeature(Feature::kProgressReporting));

    uint32_t i = 0;
    Structs::ProgressStruct::Type progressElement;
    while (GetServiceAreaCluster().GetProgressElementByIndex(i, progressElement))
    {
        if (progressElement.status == OperationalStatusEnum::kOperating ||
            progressElement.status == OperationalStatusEnum::kPending)
        {
            GetServiceAreaCluster().SetProgressStatus(progressElement.areaID, OperationalStatusEnum::kSkipped);
        }
        i++;
    }
}

void SimulatedRoboticVacuumCleaner::ExitActiveCleaningServiceArea()
{
    GetServiceAreaCluster().SetCurrentArea(DataModel::NullNullable);
    GetServiceAreaCluster().SetEstimatedEndTime(DataModel::NullNullable);
    UpdateServiceAreaProgressOnExit();
}

void SimulatedRoboticVacuumCleaner::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: Pause command received.");
    const uint8_t stateBeforePause = OperationalState().GetCurrentOperationalState();
    CancelTimer();
    LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused)));
    if (OperationalState().GetCurrentOperationalState() == to_underlying(OperationalState::OperationalStateEnum::kPaused))
    {
        mStateBeforePause = stateBeforePause;
    }
    err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
}

void SimulatedRoboticVacuumCleaner::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: Resume command received.");

    uint8_t targetState  = to_underlying(OperationalState::OperationalStateEnum::kRunning);
    uint8_t currentState = OperationalState().GetCurrentOperationalState();

    if (currentState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
        currentState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        uint8_t runMode = RunMode().GetCurrentMode();
        if (runMode != kRunModeCleaning && runMode != kRunModeMapping)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }
    }
    else if (currentState == to_underlying(OperationalState::OperationalStateEnum::kPaused) &&
             mStateBeforePause == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        targetState = to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
    }
    else if (currentState != to_underlying(OperationalState::OperationalStateEnum::kPaused))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    if (targetState == to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ClearDockChargingTracking();
    }

    auto error = OperationalState().SetOperationalState(targetState);
    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));

    if (error == CHIP_NO_ERROR && targetState == to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        StartActivityTimer();
    }
}

void SimulatedRoboticVacuumCleaner::HandleStartStateCallback(OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: Start command received.");
    auto error = OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    if (error == CHIP_NO_ERROR)
    {
        SetAttributesAtCleanStart();
        StartActivityTimer();
    }
}

void SimulatedRoboticVacuumCleaner::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: Stop command received.");
    CancelTimer();
    LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped)));
    err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
}

void SimulatedRoboticVacuumCleaner::HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: Go Home command received.");

    switch (OperationalState().GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(OperationalState::OperationalStateEnum::kPaused):
    case to_underlying(OperationalState::OperationalStateEnum::kRunning): {
        if (OperationalState().GetCurrentOperationalState() == to_underlying(OperationalState::OperationalStateEnum::kStopped) &&
            RunMode().GetCurrentMode() != kRunModeIdle)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }

        CancelTimer();
        RunMode().UpdateCurrentMode(kRunModeIdle);

        const uint8_t currentState = OperationalState().GetCurrentOperationalState();
        if (currentState == to_underlying(OperationalState::OperationalStateEnum::kRunning) ||
            (currentState == to_underlying(OperationalState::OperationalStateEnum::kPaused) &&
             mStateBeforePause == to_underlying(OperationalState::OperationalStateEnum::kRunning)))
        {
            ExitActiveCleaningServiceArea();
        }

        auto error =
            OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                         : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

// -- ServiceArea::Delegate --

CHIP_ERROR SimulatedRoboticVacuumCleaner::Init()
{
    SetMapTopology();
    GetServiceAreaCluster().SetCurrentArea(kAreaIdC);
    return CHIP_NO_ERROR;
}

void SimulatedRoboticVacuumCleaner::SetMapTopology()
{
    ApplyDefaultMapTopology(GetServiceAreaCluster());
}

bool SimulatedRoboticVacuumCleaner::IsSetSelectedAreasAllowed(MutableCharSpan & statusText)
{
    if (IsOperating(OperationalState()))
    {
        CopyCharSpanToMutableCharSpanWithTruncation("cannot set the Selected Areas while the device is running"_span, statusText);
        return false;
    }
    return true;
}

bool SimulatedRoboticVacuumCleaner::IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas,
                                                          SelectAreasStatus & locationStatus, MutableCharSpan & statusText)
{
    if (selectedAreas.empty())
    {
        return true;
    }

    auto & cluster = GetServiceAreaCluster();

    if (!cluster.HasFeature(Feature::kMaps) || cluster.GetNumberOfSupportedMaps() <= 1)
    {
        return true;
    }

    AreaStructureWrapper tempArea;
    uint32_t ignoredIndex;
    if (!cluster.GetSupportedAreaById(selectedAreas[0], ignoredIndex, tempArea))
    {
        locationStatus = SelectAreasStatus::kUnsupportedArea;
        CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
        return false;
    }

    if (tempArea.mapID.IsNull())
    {
        locationStatus = SelectAreasStatus::kInvalidSet;
        CopyCharSpanToMutableCharSpanWithTruncation("all selected areas must be in the same map"_span, statusText);
        return false;
    }

    const uint32_t mapId = tempArea.mapID.Value();

    for (const auto & areaId : selectedAreas.SubSpan(1))
    {
        if (!cluster.GetSupportedAreaById(areaId, ignoredIndex, tempArea))
        {
            locationStatus = SelectAreasStatus::kUnsupportedArea;
            CopyCharSpanToMutableCharSpanWithTruncation("unable to find selected area in supported areas"_span, statusText);
            return false;
        }

        if (tempArea.mapID.IsNull() || tempArea.mapID.Value() != mapId)
        {
            locationStatus = SelectAreasStatus::kInvalidSet;
            CopyCharSpanToMutableCharSpanWithTruncation("all selected areas must be in the same map"_span, statusText);
            return false;
        }
    }

    return true;
}

bool SimulatedRoboticVacuumCleaner::HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText)
{
    if (GetServiceAreaCluster().GetCurrentArea() != skippedArea)
    {
        CopyCharSpanToMutableCharSpanWithTruncation("the skipped area does not match the current area"_span, skipStatusText);
        return false;
    }

    if (!IsOperating(OperationalState()))
    {
        CopyCharSpanToMutableCharSpanWithTruncation("skip area is only accepted when the device is running"_span, skipStatusText);
        return false;
    }

    bool finished = false;
    GoToNextArea(OperationalStatusEnum::kSkipped, finished);

    if (finished)
    {
        HandleActivityComplete();
    }

    return true;
}

bool SimulatedRoboticVacuumCleaner::IsSupportedAreasChangeAllowed()
{
    return !IsOperating(OperationalState());
}

bool SimulatedRoboticVacuumCleaner::IsSupportedMapChangeAllowed()
{
    return !IsOperating(OperationalState());
}

void SimulatedRoboticVacuumCleaner::SetAttributesAtCleanStart()
{
    auto & cluster = GetServiceAreaCluster();

    if (cluster.GetNumberOfSupportedAreas() == 0)
    {
        return;
    }

    if (cluster.GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper firstArea;
        cluster.GetSupportedAreaByIndex(0, firstArea);

        cluster.SetCurrentArea(firstArea.areaID);

        if (cluster.HasFeature(Feature::kProgressReporting))
        {
            cluster.AddPendingProgressElement(firstArea.areaID);
            cluster.SetProgressStatus(firstArea.areaID, OperationalStatusEnum::kOperating);
        }
    }
    else
    {
        uint32_t areaId;
        cluster.GetSelectedAreaByIndex(0, areaId);

        cluster.SetCurrentArea(areaId);

        if (cluster.HasFeature(Feature::kProgressReporting))
        {
            cluster.AddPendingProgressElement(areaId);
            cluster.SetProgressStatus(areaId, OperationalStatusEnum::kOperating);

            uint32_t i = 1;
            while (cluster.GetSelectedAreaByIndex(i, areaId))
            {
                cluster.AddPendingProgressElement(areaId);
                i++;
            }
        }
    }
}

void SimulatedRoboticVacuumCleaner::GoToNextArea(OperationalStatusEnum currentAreaOpState, bool & finished)
{
    finished       = true;
    auto & cluster = GetServiceAreaCluster();

    auto currentAreaIdN = cluster.GetCurrentArea();

    if (currentAreaIdN.IsNull())
    {
        ChipLogError(Zcl, "GoToNextArea: Cannot go to the next area when the current area is null.");
        return;
    }

    if (currentAreaOpState != OperationalStatusEnum::kCompleted && currentAreaOpState != OperationalStatusEnum::kSkipped)
    {
        ChipLogError(Zcl, "GoToNextArea: currentAreaOpState must be either completed or skipped.");
        return;
    }

    auto currentAreaId = currentAreaIdN.Value();
    AreaStructureWrapper currentArea;
    uint32_t currentAreaIndex;
    if (!cluster.GetSupportedAreaById(currentAreaId, currentAreaIndex, currentArea))
    {
        ChipLogError(Zcl, "GoToNextArea: current area %u not found in supported areas.", static_cast<unsigned>(currentAreaId));
        return;
    }
    auto currentAreaMapId = currentArea.mapID;

    if (cluster.HasFeature(Feature::kProgressReporting))
    {
        cluster.SetProgressStatus(currentAreaId, currentAreaOpState);
    }

    if (cluster.GetNumberOfSelectedAreas() == 0)
    {
        AreaStructureWrapper nextArea;
        uint32_t nextIndex = currentAreaIndex + 1;
        while (cluster.GetSupportedAreaByIndex(nextIndex, nextArea))
        {
            if (!currentAreaMapId.IsNull() && nextArea.mapID == currentAreaMapId.Value())
            {
                cluster.SetCurrentArea(nextArea.areaID);

                if (cluster.HasFeature(Feature::kProgressReporting))
                {
                    cluster.SetProgressStatus(nextArea.areaID, OperationalStatusEnum::kOperating);
                }

                finished = false;
                return;
            }

            ++nextIndex;
        }
    }
    else
    {
        uint32_t selectedAreaId;
        uint32_t selectedAreaIndex = 0;
        while (cluster.GetSelectedAreaByIndex(selectedAreaIndex, selectedAreaId))
        {
            if (selectedAreaId == currentAreaId)
            {
                break;
            }
            ++selectedAreaIndex;
        }

        uint32_t nextSelectedAreaId;
        uint32_t nextSelectedAreaIndex = selectedAreaIndex + 1;
        if (cluster.GetSelectedAreaByIndex(nextSelectedAreaIndex, nextSelectedAreaId))
        {
            cluster.SetCurrentArea(nextSelectedAreaId);

            if (cluster.HasFeature(Feature::kProgressReporting))
            {
                cluster.SetProgressStatus(nextSelectedAreaId, OperationalStatusEnum::kOperating);
            }

            finished = false;
        }
    }
}

// -- RunMode / CleanMode logic (fed by the thin AppDelegate adapters above) --

CHIP_ERROR SimulatedRoboticVacuumCleaner::RunModeAppDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kRunModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kRunModeOptions[modeIndex].label, label);
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::RunModeAppDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kRunModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kRunModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::RunModeAppDelegate::GetModeTagsByIndex(uint8_t modeIndex,
                                                                                 DataModel::List<ModeTagStructType> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kRunModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kRunModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::CleanModeAppDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kCleanModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kCleanModeOptions[modeIndex].label, label);
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::CleanModeAppDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kCleanModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kCleanModeOptions[modeIndex].value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedRoboticVacuumCleaner::CleanModeAppDelegate::GetModeTagsByIndex(uint8_t modeIndex,
                                                                                   DataModel::List<ModeTagStructType> & modeTags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kCleanModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & tags = kCleanModeOptions[modeIndex].tags;
    VerifyOrReturnError(modeTags.size() >= tags.size(), CHIP_ERROR_INVALID_ARGUMENT);
    std::copy(tags.begin(), tags.end(), modeTags.begin());
    modeTags.reduce_size(tags.size());
    return CHIP_NO_ERROR;
}

void SimulatedRoboticVacuumCleaner::HandleRunModeChangeToMode(uint8_t newMode,
                                                              ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: RunMode ChangeToMode(%u) received.", newMode);

    uint8_t currentState = OperationalState().GetCurrentOperationalState();
    uint8_t currentMode  = RunMode().GetCurrentMode();

    switch (currentState)
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging):
        // We could be in the charging state with a run mode != idle.
        if (currentMode != kRunModeIdle && newMode != kRunModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue("Change to the mapping or cleaning mode is only allowed from idle"_span);
            return;
        }
        if (newMode == kRunModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kSuccess);
            return;
        }
        ClearDockChargingTracking();
        LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning)));
        SetAttributesAtCleanStart();
        StartActivityTimer();
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;

    case to_underlying(OperationalState::OperationalStateEnum::kRunning):
        if (newMode != kRunModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue("Change to the mapping or cleaning mode is only allowed from idle"_span);
            return;
        }
        CancelTimer();
        LogErrorOnFailure(
            OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));
        UpdateServiceAreaProgressOnExit();
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;

    default:
        break;
    }

    // If we fall through, the change is not supported in the current state.
    response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
    response.statusText.SetValue("This change is not allowed at this time"_span);
}

void SimulatedRoboticVacuumCleaner::HandleCleanModeChangeToMode(uint8_t /* newMode */,
                                                                ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(Zcl, "SimulatedRoboticVacuumCleaner: CleanMode ChangeToMode received.");

    if (RunMode().GetCurrentMode() != kRunModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue("Change of the cleaning mode is only allowed in Idle."_span);
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

// -- Named-pipe simulation entry points --

void SimulatedRoboticVacuumCleaner::HandleCharged()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
    {
        ChipLogError(Zcl, "'Charged' is only accepted when the device is in the 'Charging' state.");
        return;
    }

    mPhysicalDockState = PhysicalDockState::kOnDock;

    if (RunMode().GetCurrentMode() == kRunModeIdle)
    {
        CancelTimer();
        LogErrorOnFailure(
            OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
    }
    else
    {
        ClearDockChargingTracking();
        LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning)));
        StartActivityTimer();
    }
}

void SimulatedRoboticVacuumCleaner::HandleCharging()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        ChipLogError(Zcl, "'Charging' is only accepted when the device is in the 'Docked' state.");
        return;
    }

    mPhysicalDockState = PhysicalDockState::kOnDockCharging;
    LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
    StartChargingTimer();
}

void SimulatedRoboticVacuumCleaner::HandleDocked()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kStopped))
    {
        ChipLogError(Zcl, "'Docked' is only accepted when the device is in the 'Stopped' state.");
        return;
    }

    mPhysicalDockState = PhysicalDockState::kOnDock;
    LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
}

void SimulatedRoboticVacuumCleaner::HandleChargerFound()
{
    if (OperationalState().GetCurrentOperationalState() !=
        to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        ChipLogError(Zcl, "'ChargerFound' is only accepted when the device is in the 'SeekingCharger' state.");
        return;
    }

    mPhysicalDockState = PhysicalDockState::kOnDockCharging;
    LogErrorOnFailure(OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
    StartChargingTimer();
}

void SimulatedRoboticVacuumCleaner::HandleLowCharge()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(Zcl, "'LowCharge' is only accepted when the device is in the 'Running' state.");
        return;
    }

    CancelTimer();
    LogErrorOnFailure(
        OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));
}

void SimulatedRoboticVacuumCleaner::HandleActivityComplete()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(Zcl, "'ActivityComplete' is only accepted when the device is in the 'Running' state.");
        return;
    }

    CancelTimer();
    RunMode().UpdateCurrentMode(kRunModeIdle);

    Optional<DataModel::Nullable<uint32_t>> totalTime(DataModel::Nullable<uint32_t>(100));
    Optional<DataModel::Nullable<uint32_t>> pausedTime(DataModel::Nullable<uint32_t>(10));
    OperationalState().OnOperationCompletionDetected(0, totalTime, pausedTime);

    LogErrorOnFailure(
        OperationalState().SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));

    ExitActiveCleaningServiceArea();
}

void SimulatedRoboticVacuumCleaner::HandleAreaComplete()
{
    bool finished = false;
    GoToNextArea(ServiceArea::OperationalStatusEnum::kCompleted, finished);

    if (finished)
    {
        HandleActivityComplete();
    }
}

void SimulatedRoboticVacuumCleaner::HandleClearError()
{
    if (OperationalState().GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kError))
    {
        ChipLogError(Zcl, "'ClearError' is only accepted when the device is in the 'Error' state.");
        return;
    }

    RunMode().UpdateCurrentMode(kRunModeIdle);
    SetDeviceToIdleState();
}

void SimulatedRoboticVacuumCleaner::HandleReset()
{
    CancelTimer();
    ClearDockChargingTracking();
    mStateBeforePause = 0;
    ResetRvcSimulation(RunMode(), OperationalState(), CleanMode(), GetServiceAreaCluster());
}

void SimulatedRoboticVacuumCleaner::HandleErrorEvent(const std::string & error)
{
    uint8_t errorStateId;

    if (error == "UnableToStartOrResume")
    {
        errorStateId = to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume);
    }
    else if (error == "UnableToCompleteOperation")
    {
        errorStateId = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);
    }
    else if (error == "CommandInvalidInState")
    {
        errorStateId = to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState);
    }
    else if (error == "FailedToFindChargingDock")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kFailedToFindChargingDock);
    }
    else if (error == "Stuck")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kStuck);
    }
    else if (error == "DustBinMissing")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing);
    }
    else if (error == "DustBinFull")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinFull);
    }
    else if (error == "WaterTankEmpty")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankEmpty);
    }
    else if (error == "WaterTankMissing")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing);
    }
    else if (error == "WaterTankLidOpen")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankLidOpen);
    }
    else if (error == "MopCleaningPadMissing")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing);
    }
    else if (error == "BatteryLow" || error == "LowBattery")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kLowBattery);
    }
    else if (error == "CannotReachTargetArea")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kCannotReachTargetArea);
    }
    else if (error == "DirtyWaterTankFull")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kDirtyWaterTankFull);
    }
    else if (error == "DirtyWaterTankMissing")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kDirtyWaterTankMissing);
    }
    else if (error == "WheelsJammed")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kWheelsJammed);
    }
    else if (error == "BrushJammed")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kBrushJammed);
    }
    else if (error == "NavigationSensorObscured")
    {
        errorStateId = to_underlying(RvcOperationalState::ErrorStateEnum::kNavigationSensorObscured);
    }
    else
    {
        ChipLogError(Zcl, "Unhandled ErrorEvent 'Error' value: %s", error.c_str());
        return;
    }

    CancelTimer();
    OperationalState::Structs::ErrorStateStruct::Type err;
    err.errorStateID = errorStateId;
    OperationalState().OnOperationalErrorDetected(err);
}

} // namespace chip::app
