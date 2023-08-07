#include "rvc-device.h"

using namespace chip::app::Clusters;

void RvcDevice::Init()
{
    mRvcRunModeInstance.Init();
    mRvcCleanModeInstance.Init();
    mRvcOperationalStateInstance.Init();
}

void RvcDevice::HandleRvcRunChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentMode = mRvcRunModeInstance.GetCurrentMode();

    // Our business logic states that we can only switch into the mapping state from the idle state.
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

        mRvcRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mRvcOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString(err.AsString()));
            return;
        }
        break;
    }
    case RvcRunMode::ModeCleaning: {
        mRvcRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mRvcOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        if (err != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString(err.AsString()));
            return;
        }
    }
        break;
    case RvcRunMode::ModeIdle: {
        mRvcRunModeInstance.UpdateCurrentMode(NewMode);
        // Set operational state to running
        ChipError err =
            mRvcOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
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
    uint8_t rvcRunCurrentMode = mRvcRunModeInstance.GetCurrentMode();

    if (rvcRunCurrentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(RvcCleanMode::StatusCode::kCleaningInProgress);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Change of the cleaning mode is only allowed in Idle."));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}
