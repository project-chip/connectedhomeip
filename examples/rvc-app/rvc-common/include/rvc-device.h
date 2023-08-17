#pragma once

#include "rvc-mode-delegates.h"
#include "rvc-operational-state-delegate.h"
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {

const EndpointId RvcRunModeEndpoint = 1;
const EndpointId RvcCleanModeEndpoint = 1;
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


public:
    RvcDevice() :
        mRunModeDelegate(),
        mRunModeInstance(&mRunModeDelegate, RvcRunModeEndpoint, RvcRunMode::Id, chip::to_underlying(RvcRunMode::Feature::kOnOff)),
        mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, RvcCleanModeEndpoint, RvcCleanMode::Id, chip::to_underlying(RvcCleanMode::Feature::kOnOff)),
        mOperationalStateDelegate(),
        mOperationalStateInstance(&mOperationalStateDelegate, RvcOperationalStateEndpoint, RvcOperationalState::Id)
    {
        // set the current-mode at start-up
        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
        // Assume that the device is not docked.
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

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
};

} // namespace Clusters
} // namespace app
} // namespace chip
