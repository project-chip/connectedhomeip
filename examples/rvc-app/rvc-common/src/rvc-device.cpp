#include "rvc-device.h"

using namespace chip::app::Clusters;

void RvcDevice::Init()
{
    mRunModeInstance.Init();
    mCleanModeInstance.Init();
    mOperationalStateInstance.Init();
}

void RvcDevice::SetDeviceToIdleState()
{
    if (mCharging)
    {
        mDocked = true;
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
    }
    else if (mDocked)
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
    }
    else
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    }
}

void RvcDevice::HandleRvcRunChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentState = mOperationalStateInstance.GetCurrentOperationalState();
    uint8_t currentMode  = mRunModeInstance.GetCurrentMode();

    switch (currentState)
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging): {
        // We could be in the charging state with an RvcRun mode != idle.
        if (currentMode != RvcRunMode::ModeIdle && newMode != RvcRunMode::ModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }

        mCharging = false;
        mDocked   = false;
        mRunModeInstance.UpdateCurrentMode(newMode);
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;
    }
    break;
    case to_underlying(OperationalState::OperationalStateEnum::kRunning): {
        if (newMode != RvcRunMode::ModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }

        mRunModeInstance.UpdateCurrentMode(newMode);
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;
    }
    break;
    }

    // If we fall through at any point, it's because the change is not supported in the current state.
    response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
    response.statusText.SetValue(chip::CharSpan::fromCharString("This change is not allowed at this time"));
}

void RvcDevice::HandleRvcCleanChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t rvcRunCurrentMode = mRunModeInstance.GetCurrentMode();

    if (rvcRunCurrentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Change of the cleaning mode is only allowed in Idle."));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

void RvcDevice::HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    // This method is only called if the device is in a Pause-compatible state, i.e. `Running` or `SeekingCharger`.
    mStateBeforePause = mOperationalStateInstance.GetCurrentOperationalState();
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
}

void RvcDevice::HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    uint8_t targetState = to_underlying(OperationalState::OperationalStateEnum::kRunning);

    switch (mOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked): {
        if (mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeCleaning &&
            mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeMapping)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }
    }
    break;
    case to_underlying(OperationalState::OperationalStateEnum::kPaused): {
        if (mStateBeforePause == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
        {
            targetState = to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
        }
    }
    break;
    default:
        // This method is only called if the device is in a resume-compatible state, i.e. `Charging`, `Docked` or
        // `Paused`. Therefore, we do not expect to ever enter this branch.
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    auto error = mOperationalStateInstance.SetOperationalState(targetState);

    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
}

void RvcDevice::HandleOpStateGoHomeCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    switch (mOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped): {
        if (mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeIdle)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }

        auto error = mOperationalStateInstance.SetOperationalState(
            to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));

        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                         : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
    break;
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

void RvcDevice::HandleChargedMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() !=
        to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Charged' command is only accepted when the device is in the 'Charging' state.");
        return;
    }

    mCharging = false;

    if (mRunModeInstance.GetCurrentMode() == RvcRunMode::ModeIdle)
    {
        if (mDocked) // assuming that we can't be charging the device while it is not docked.
        {
            mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
        }
        else
        {
            mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
        }
    }
    else
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    }
}

void RvcDevice::HandleChargingMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Charging' command is only accepted when the device is in the 'Docked' state.");
        return;
    }

    mCharging = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
}

void RvcDevice::HandleDockedMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kStopped))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Docked' command is only accepted when the device is in the 'Stopped' state.");
        return;
    }

    mDocked = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
}

void RvcDevice::HandleChargerFoundMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() !=
        to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        ChipLogError(NotSpecified,
                     "RVC App: The 'ChargerFound' command is only accepted when the device is in the 'SeekingCharger' state.");
        return;
    }

    mCharging = true;
    mDocked   = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
}

void RvcDevice::HandleLowChargeMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(NotSpecified, "RVC App: The 'LowCharge' command is only accepted when the device is in the 'Running' state.");
        return;
    }

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
}

void RvcDevice::HandleActivityCompleteEvent()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(NotSpecified,
                     "RVC App: The 'ActivityComplete' command is only accepted when the device is in the 'Running' state.");
        return;
    }

    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);

    Optional<DataModel::Nullable<uint32_t>> a(DataModel::Nullable<uint32_t>(100));
    Optional<DataModel::Nullable<uint32_t>> b(DataModel::Nullable<uint32_t>(10));
    mOperationalStateInstance.OnOperationCompletionDetected(0, a, b);

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
}

void RvcDevice::HandleErrorEvent(const std::string & error)
{
    detail::Structs::ErrorStateStruct::Type err;

    if (error == "UnableToStartOrResume")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume);
    }
    else if (error == "UnableToCompleteOperation")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);
    }
    else if (error == "CommandInvalidInState")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState);
    }
    else if (error == "FailedToFindChargingDock")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kFailedToFindChargingDock);
    }
    else if (error == "Stuck")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kStuck);
    }
    else if (error == "DustBinMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing);
    }
    else if (error == "DustBinFull")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinFull);
    }
    else if (error == "WaterTankEmpty")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankEmpty);
    }
    else if (error == "WaterTankMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing);
    }
    else if (error == "WaterTankLidOpen")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankLidOpen);
    }
    else if (error == "MopCleaningPadMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: The 'Error' key of the 'ErrorEvent' message is not valid.");
        return;
    }

    mOperationalStateInstance.OnOperationalErrorDetected(err);
}

void RvcDevice::HandleClearErrorMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kError))
    {
        ChipLogError(NotSpecified, "RVC App: The 'ClearError' command is only excepted when the device is in the 'Error' state.");
        return;
    }

    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
    SetDeviceToIdleState();
}

void RvcDevice::HandleResetMessage()
{
    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
    mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    mCleanModeInstance.UpdateCurrentMode(RvcCleanMode::ModeQuick);
}
