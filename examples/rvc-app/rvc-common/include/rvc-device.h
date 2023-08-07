#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include "rvc-mode-delegates.h"
#include "rvc-operational-state-delegate.h"

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
    RvcDevice():
        mRunModeDelegate(),
        mRunModeInstance(&mRunModeDelegate, 0x1, RvcRunMode::Id, chip::to_underlying(RvcRunMode::Feature::kOnOff)),
        mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, 0x1, RvcCleanMode::Id, chip::to_underlying(RvcCleanMode::Feature::kOnOff)),
        mOperationalStateDelegate(), mOperationalStateInstance(&mOperationalStateDelegate, 0x01, RvcOperationalState::Id)
    {
        // todo set start-up modes and state?

        // set callback functions
        mRunModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcRunChangeToMode, this);
        mCleanModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcCleanChangeToMode, this);
        mOperationalStateDelegate.SetPauseCallback(&RvcDevice::HandleOpStatePauseCallback, this);
        mOperationalStateDelegate.SetResumeCallback(&RvcDevice::HandleOpStateResumeCallback, this);
    }

    /**
     * set up all the clusters and callback functions.
     */
    void Init();

    void HandleRvcRunChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    void HandleRvcCleanChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    void HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err);

    void HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err);
};

} // namespace Clusters
} // namespace app
} // namespace chip
