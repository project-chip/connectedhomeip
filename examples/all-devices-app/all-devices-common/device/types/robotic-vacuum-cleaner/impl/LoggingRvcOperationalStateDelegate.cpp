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

#include <device/types/robotic-vacuum-cleaner/impl/LoggingRvcOperationalStateDelegate.h>
#include <device/types/robotic-vacuum-cleaner/impl/LoggingServiceAreaDelegate.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationTopology.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::OperationalState {

using namespace chip::app::all_devices::rvc_simulation::Topology;

CHIP_ERROR LoggingRvcOperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    static const GenericOperationalState kSupportedStates[] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped), MakeOptional("Stopped"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning), MakeOptional("Running"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused), MakeOptional("Paused"_span)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError), MakeOptional("Error"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger),
                                MakeOptional("SeekingCharger"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging), MakeOptional("Charging"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked), MakeOptional("Docked"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kEmptyingDustBin),
                                MakeOptional("EmptyingDustBin"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCleaningMop),
                                MakeOptional("CleaningMop"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kFillingWaterTank),
                                MakeOptional("FillingWaterTank"_span)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kUpdatingMaps),
                                MakeOptional("UpdatingMaps"_span)),
    };

    if (index >= MATTER_ARRAY_SIZE(kSupportedStates))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = kSupportedStates[index];
    return CHIP_NO_ERROR;
}

void LoggingRvcOperationalStateDelegate::ClearDockChargingTracking()
{
    mCharging = false;
    mDocked   = false;
}

void LoggingRvcOperationalStateDelegate::SetDeviceToIdleState()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCharging)
    {
        mDocked = true;
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
    }
    else if (mDocked)
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
    }
    else
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)));
    }
}

void LoggingRvcOperationalStateDelegate::UpdateServiceAreaProgressOnExit()
{
    if (mServiceAreaDelegate != nullptr)
    {
        mServiceAreaDelegate->UpdateProgressOnExit();
    }
}

void LoggingRvcOperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingRvcOperationalStateDelegate: Pause command received.");
    const uint8_t stateBeforePause = mCluster ? mCluster->GetCurrentOperationalState() : 0;
    LoggingOperationalStateDelegate::HandlePauseStateCallback(err);
    if (err.errorStateID == to_underlying(ErrorStateEnum::kNoError) && mCluster != nullptr &&
        mCluster->GetCurrentOperationalState() == to_underlying(OperationalStateEnum::kPaused))
    {
        mStateBeforePause = stateBeforePause;
    }
}

void LoggingRvcOperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingRvcOperationalStateDelegate: Resume command received.");
    if (!mCluster)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    uint8_t targetState  = to_underlying(OperationalStateEnum::kRunning);
    uint8_t currentState = mCluster->GetCurrentOperationalState();

    if (currentState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
        currentState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        uint8_t runMode = mRunModeCluster ? mRunModeCluster->GetCurrentMode() : kRunModeIdle;
        if (runMode != kRunModeCleaning && runMode != kRunModeMapping)
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
            return;
        }
    }
    else if (currentState == to_underlying(OperationalStateEnum::kPaused) &&
             mStateBeforePause == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        targetState = to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
    }
    else if (currentState != to_underlying(OperationalStateEnum::kPaused))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    auto error = mCluster->SetOperationalState(targetState);
    err.Set((error == CHIP_NO_ERROR) ? to_underlying(ErrorStateEnum::kNoError)
                                     : to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
}

void LoggingRvcOperationalStateDelegate::HandleGoHomeCommandCallback(GenericOperationalError & err)
{
    ChipLogProgress(Zcl, "LoggingRvcOperationalStateDelegate: Go Home command received.");
    if (!mCluster || !mRunModeCluster)
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }

    switch (mCluster->GetCurrentOperationalState())
    {
    case to_underlying(OperationalStateEnum::kStopped):
    case to_underlying(OperationalStateEnum::kPaused):
    case to_underlying(OperationalStateEnum::kRunning): {
        if (mCluster->GetCurrentOperationalState() == to_underlying(OperationalStateEnum::kStopped) &&
            mRunModeCluster->GetCurrentMode() != kRunModeIdle)
        {
            err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
            return;
        }

        mRunModeCluster->UpdateCurrentMode(kRunModeIdle);

        auto error = mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        err.Set((error == CHIP_NO_ERROR) ? to_underlying(ErrorStateEnum::kNoError)
                                         : to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    default:
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

void LoggingRvcOperationalStateDelegate::HandleCharged()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
    {
        ChipLogError(Zcl, "'Charged' is only accepted when the device is in the 'Charging' state.");
        return;
    }

    mCharging = false;

    if (mRunModeCluster && mRunModeCluster->GetCurrentMode() == kRunModeIdle)
    {
        if (mDocked)
        {
            LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
        }
        else
        {
            LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)));
        }
    }
    else
    {
        LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)));
    }
}

void LoggingRvcOperationalStateDelegate::HandleCharging()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        ChipLogError(Zcl, "'Charging' is only accepted when the device is in the 'Docked' state.");
        return;
    }

    mCharging = true;
    LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
}

void LoggingRvcOperationalStateDelegate::HandleDocked()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kStopped))
    {
        ChipLogError(Zcl, "'Docked' is only accepted when the device is in the 'Stopped' state.");
        return;
    }

    mDocked = true;
    LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)));
}

void LoggingRvcOperationalStateDelegate::HandleChargerFound()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        ChipLogError(Zcl, "'ChargerFound' is only accepted when the device is in the 'SeekingCharger' state.");
        return;
    }

    mCharging = true;
    mDocked   = true;
    LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging)));
}

void LoggingRvcOperationalStateDelegate::HandleLowCharge()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kRunning))
    {
        ChipLogError(Zcl, "'LowCharge' is only accepted when the device is in the 'Running' state.");
        return;
    }

    LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));
}

void LoggingRvcOperationalStateDelegate::HandleActivityComplete()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kRunning))
    {
        ChipLogError(Zcl, "'ActivityComplete' is only accepted when the device is in the 'Running' state.");
        return;
    }

    if (mRunModeCluster)
    {
        mRunModeCluster->UpdateCurrentMode(kRunModeIdle);
    }

    Optional<DataModel::Nullable<uint32_t>> totalTime(DataModel::Nullable<uint32_t>(100));
    Optional<DataModel::Nullable<uint32_t>> pausedTime(DataModel::Nullable<uint32_t>(10));
    mCluster->OnOperationCompletionDetected(0, totalTime, pausedTime);

    LogErrorOnFailure(mCluster->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)));

    if (mServiceAreaCluster)
    {
        mServiceAreaCluster->SetCurrentArea(DataModel::NullNullable);
        mServiceAreaCluster->SetEstimatedEndTime(DataModel::NullNullable);
    }
    UpdateServiceAreaProgressOnExit();
}

void LoggingRvcOperationalStateDelegate::HandleAreaComplete()
{
    VerifyOrReturn(mServiceAreaDelegate != nullptr);

    bool finished = false;
    mServiceAreaDelegate->GoToNextArea(ServiceArea::OperationalStatusEnum::kCompleted, finished);

    if (finished)
    {
        HandleActivityComplete();
    }
}

void LoggingRvcOperationalStateDelegate::HandleClearError()
{
    VerifyOrReturn(mCluster != nullptr);

    if (mCluster->GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kError))
    {
        ChipLogError(Zcl, "'ClearError' is only accepted when the device is in the 'Error' state.");
        return;
    }

    if (mRunModeCluster)
    {
        mRunModeCluster->UpdateCurrentMode(kRunModeIdle);
    }
    SetDeviceToIdleState();
}

void LoggingRvcOperationalStateDelegate::HandleErrorEvent(const std::string & error)
{
    VerifyOrReturn(mCluster != nullptr);

    uint8_t errorStateId;

    if (error == "UnableToStartOrResume")
    {
        errorStateId = to_underlying(ErrorStateEnum::kUnableToStartOrResume);
    }
    else if (error == "UnableToCompleteOperation")
    {
        errorStateId = to_underlying(ErrorStateEnum::kUnableToCompleteOperation);
    }
    else if (error == "CommandInvalidInState")
    {
        errorStateId = to_underlying(ErrorStateEnum::kCommandInvalidInState);
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

    OperationalState::Structs::ErrorStateStruct::Type err;
    err.errorStateID = errorStateId;
    mCluster->OnOperationalErrorDetected(err);
}

} // namespace chip::app::Clusters::OperationalState
