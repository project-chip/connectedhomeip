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
    OperationalState::Instance mOperationalStateInstance;

public:
    /**
     * This class is responsible for initialising all the RVC clusters and manging the interactions between them as required by
     * the specific "business logic". See the state machine diagram.
     * @param aRvcClustersEndpoint The endpoint ID where all the RVC clusters exist.
     */
    explicit RvcDevice(EndpointId aRvcClustersEndpoint) :
        mRunModeDelegate(), mRunModeInstance(&mRunModeDelegate, aRvcClustersEndpoint, RvcRunMode::Id, 0), mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, aRvcClustersEndpoint, RvcCleanMode::Id, 0), mOperationalStateDelegate(),
        mOperationalStateInstance(&mOperationalStateDelegate, aRvcClustersEndpoint, RvcOperationalState::Id)
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
};

} // namespace Clusters
} // namespace app
} // namespace chip
