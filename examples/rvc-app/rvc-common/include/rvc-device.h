#pragma once

#include "rvc-mode-delegates.h"
#include "rvc-operational-state-delegate.h"
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice
{
private:
    RvcRunMode::RvcRunModeDelegate mRunModeDelegate;
    ModeBase::Instance mRunModeInstance;

    RvcCleanMode::RvcCleanModeDelegate mCleanModeDelegate;
    ModeBase::Instance mCleanModeInstance;

    RvcOperationalState::RvcOperationalStateDelegate mOperationalStateDelegate;
    RvcOperationalState::Instance mOperationalStateInstance;

    bool mDocked   = false;
    bool mCharging = false;

    uint8_t mStateBeforePause = 0;

public:
    /**
     * This class is responsible for initialising all the RVC clusters and manging the interactions between them as required by
     * the specific "business logic". See the state machine diagram.
     * @param aRvcClustersEndpoint The endpoint ID where all the RVC clusters exist.
     */
    explicit RvcDevice(EndpointId aRvcClustersEndpoint) :
        mRunModeDelegate(), mRunModeInstance(&mRunModeDelegate, aRvcClustersEndpoint, RvcRunMode::Id, 0), mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, aRvcClustersEndpoint, RvcCleanMode::Id, 0), mOperationalStateDelegate(),
        mOperationalStateInstance(&mOperationalStateDelegate, aRvcClustersEndpoint)
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
        mOperationalStateDelegate.SetGoHomeCallback(&RvcDevice::HandleOpStateGoHomeCallback, this);
    }

    /**
     * Init all the clusters used by this device.
     */
    void Init();

    /**
     * Sets the device to an idle state, that is either the STOPPED, DOCKED or CHARGING state, depending on physical information.
     * Note: in this example this is based on the mDocked and mChanging boolean variables.
     */
    void SetDeviceToIdleState();

    /**
     * Handles the RvcRunMode command requesting a mode change.
     */
    void HandleRvcRunChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcCleanMode command requesting a mode change.
     */
    void HandleRvcCleanChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcOperationalState pause command.
     */
    void HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Handles the RvcOperationalState resume command.
     */
    void HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Handles the RvcOperationalState GoHome command.
     */
    void HandleOpStateGoHomeCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Updates the state machine when the device becomes fully-charged.
     */
    void HandleChargedMessage();

    void HandleChargingMessage();

    void HandleDockedMessage();

    void HandleChargerFoundMessage();

    void HandleLowChargeMessage();

    void HandleActivityCompleteEvent();

    /**
     * Sets the device to an error state with the error state ID matching the error name given.
     * @param error The error name. Could be one of UnableToStartOrResume, UnableToCompleteOperation, CommandInvalidInState,
     * FailedToFindChargingDock, Stuck, DustBinMissing, DustBinFull, WaterTankEmpty, WaterTankMissing, WaterTankLidOpen or
     * MopCleaningPadMissing.
     */
    void HandleErrorEvent(const std::string & error);

    void HandleClearErrorMessage();

    void HandleResetMessage();
};

} // namespace Clusters
} // namespace app
} // namespace chip
