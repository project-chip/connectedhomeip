#pragma once

#include "rvc-mode-delegates.h"
#include "rvc-operational-state-delegate.h"
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {

const EndpointId RvcRunModeEndpoint          = 1;
const EndpointId RvcCleanModeEndpoint        = 1;
const EndpointId RvcOperationalStateEndpoint = 1;

class RvcDevice
{
private:
    RvcRunMode::RvcRunModeDelegate mRunModeDelegate;
    ModeBase::Instance mRunModeInstance;

    RvcCleanMode::RvcCleanModeDelegate mCleanModeDelegate;
    ModeBase::Instance mCleanModeInstance;

    RvcOperationalState::RvcOperationalStateDelegate mOperationalStateDelegate;
    OperationalState::Instance mOperationalStateInstance;

    bool mDocked = false;
    bool mCharging = false;

public:
    RvcDevice() :
        mRunModeDelegate(), mRunModeInstance(&mRunModeDelegate, RvcRunModeEndpoint, RvcRunMode::Id, 0), mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, RvcCleanModeEndpoint, RvcCleanMode::Id, 0), mOperationalStateDelegate(),
        mOperationalStateInstance(&mOperationalStateDelegate, RvcOperationalStateEndpoint, RvcOperationalState::Id)
    {
        // set the current-mode at start-up
        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);

        // Hypothetically, the device checks if it is physically docked or charging
        SetDeviceToIdleState();

        // set callback functions
        mRunModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcRunChangeToMode, this);
        mCleanModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcCleanChangeToMode, this);
        mOperationalStateDelegate.SetPauseCallback(&RvcDevice::HandleOpStatePauseCallback, this);
        mOperationalStateDelegate.SetResumeCallback(&RvcDevice::HandleOpStateResumeCallback, this);
    }

    /**
     * Init all the clusters used by this device.
     */
    void Init();

    /**
     * Sets the device to and idle state, that is either the STOPPED, DOCKED or CHARGING state, depending on physical information.
     * Note: in this example this is based on the mDocked and mChanging boolean variables.
     */
    void SetDeviceToIdleState();

    /**
     * Handles the RvcRunMode command requesting a mode change.
     */
    void HandleRvcRunChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcCleanMode command requesting a mode change.
     */
    void HandleRvcCleanChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcOperationalState pause command.
     */
    void HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Handles the RvcOperationalState resume command.
     */
    void HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Updates the state machine when the device becomes fully-charged.
     */
    void HandleChargedMessage();

    void HandleChargingMessage();

    void HandleDockedMessage();

    void HandleChargerFoundMessage();

    void HandleLowChargeMessage();

    void HandleActivityCompleteEvent();

    void HandleErrorEvent();

    void HandleClearErrorMessage();
};

} // namespace Clusters
} // namespace app
} // namespace chip
