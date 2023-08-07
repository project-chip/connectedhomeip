#include "rvc-device.h"

using namespace chip::app::Clusters;

void RvcDevice::Init()
{
    mRunModeInstance.Init();
    mCleanModeInstance.Init();
    mOperationalStateInstance.Init();
}

void RvcDevice::HandleRvcRunChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentMode = mRunModeInstance.GetCurrentMode();

    switch (NewMode)
    {
    case RvcRunMode::ModeMapping: {
        // change to mapping only allowed from the Idle state.
        if (currentMode != RvcRunMode::ModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString("Change to the mapping mode is only allowed from idle"));
            return;
        }

        mRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString(err.AsString()));
            return;
        }
        break;
    }
    case RvcRunMode::ModeCleaning: {
        mRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString(err.AsString()));
            return;
        }
    }
        break;
    case RvcRunMode::ModeIdle: {
        mRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString(err.AsString()));
            return;
        }
    }

    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

void RvcDevice::HandleRvcCleanChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t rvcRunCurrentMode = mRunModeInstance.GetCurrentMode();

    if (rvcRunCurrentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(RvcCleanMode::StatusCode::kCleaningInProgress);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Change of the cleaning mode is only allowed in Idle."));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

void RvcDevice::HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void RvcDevice::HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}
