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
    RvcRunMode::RvcRunModeDelegate mRvcRunModeDelegate;
    ModeBase::Instance mRvcRunModeInstance;

    RvcCleanMode::RvcCleanModeDelegate mRvcCleanModeDelegate;
    ModeBase::Instance mRvcCleanModeInstance;

    RvcOperationalState::RvcOperationalStateDelegate mRvcOperationalStateDelegate;
    OperationalState::Instance mRvcOperationalStateInstance;

public:
    RvcDevice():
        mRvcRunModeDelegate(),
        mRvcRunModeInstance(&mRvcRunModeDelegate, 0x1, RvcRunMode::Id, chip::to_underlying(RvcRunMode::Feature::kOnOff)),
        mRvcCleanModeDelegate(),
        mRvcCleanModeInstance(&mRvcCleanModeDelegate, 0x1, RvcCleanMode::Id, chip::to_underlying(RvcCleanMode::Feature::kOnOff)),
        mRvcOperationalStateDelegate(),
        mRvcOperationalStateInstance(&mRvcOperationalStateDelegate, 0x01, RvcOperationalState::Id)
    {
        // todo set start-up modes and state?

        // set callback functions
        mRvcRunModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcRunChangeToMode, this);
        mRvcCleanModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcCleanChangeToMode, this);

    }

    /**
     * set up all the clusters and callback functions.
     */
    void Init();

    void HandleRvcRunChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    void HandleRvcCleanChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response);
};

} // namespace Clusters
} // namespace app
} // namespace chip
