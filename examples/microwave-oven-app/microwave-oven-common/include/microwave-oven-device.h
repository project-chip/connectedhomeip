#pragma once

#include "microwave-oven-control-delegate.h"
#include "operational-state-delegate.h"
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {

class MicrowaveOvenDevice
{
private:
    MicrowaveOvenControl::ExampleMicrowaveOvenControlDelegate mMicrowaveOvenControlDelegate;
    MicrowaveOvenControl::Instance mMicrowaveOvenControlInstance;

    OperationalState::OperationalStateDelegate mOperationalStateDelegate;
    OperationalState::Instance mOperationalStateInstance;

public:
    /**
     * This class is responsible for initialising all the microwave oven device clusters and managing the interactions between them
     * as required by the specific "business logic". See the state machine diagram.
     * @param aClustersEndpoint The endpoint ID where all the microwave oven clusters exist.
     */
    explicit MicrowaveOvenDevice(EndpointId aClustersEndpoint) :
        mMicrowaveOvenControlDelegate(),
        mMicrowaveOvenControlInstance(&mMicrowaveOvenControlDelegate, aClustersEndpoint, MicrowaveOvenControl::Id),
        mOperationalStateDelegate(), mOperationalStateInstance(&mOperationalStateDelegate, aClustersEndpoint, OperationalState::Id)
    {
        // set callback functions
        mMicrowaveOvenControlDelegate.SetMicrowaveOvenControlSetCookingParametersCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenSetCookingParametersCommandCallback, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
        mMicrowaveOvenControlDelegate.SetMicrowaveOvenControlAddMoreTimeCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenAddMoreTimeCommandCallback, this, std::placeholders::_1));
        mOperationalStateDelegate.SetOpStatePauseCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenOpStatePauseCallback, this, std::placeholders::_1));
        mOperationalStateDelegate.SetOpStateResumeCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenOpStateResumeCallback, this, std::placeholders::_1));
        mOperationalStateDelegate.SetOpStateStartCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenOpStateStartCallback, this, std::placeholders::_1));
        mOperationalStateDelegate.SetOpStateStopCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenOpStateStopCallback, this, std::placeholders::_1));
        mOperationalStateDelegate.SetOpStateGetCountdownTimeCallback(
            std::bind(&MicrowaveOvenDevice::HandleMicrowaveOvenOpStateGetCountdownTime, this));
    }

    /**
     * Init all the clusters used by this device.
     */
    void Init(EndpointId aEndpoint);

    /**
     * handle command for microwave oven control: set cooking parameters
     */
    Protocols::InteractionModel::Status HandleMicrowaveOvenSetCookingParametersCommandCallback(uint8_t cookMode, uint32_t cookTime,
                                                                                               uint8_t powerSetting);

    /**
     * handle command for microwave oven control: add more time
     */
    Protocols::InteractionModel::Status HandleMicrowaveOvenAddMoreTimeCommandCallback(uint32_t finalCookTime);

    /**
     * handle command for operational state: pause
     * @param[out] err: get operational error after callback.
     * this method is called to set operational state to target state.
     * if success, 'err' is set to NoError, otherwise it will be set to UnableToCompleteOperation.
     */
    void HandleMicrowaveOvenOpStatePauseCallback(OperationalState::GenericOperationalError & err);

    /**
     * handle command for operational state: resume
     * @param[out] err: get operational error after callback.
     * this method is called to set operational state to target state.
     * if success, 'err' is set to NoError, otherwise it will be set to UnableToCompleteOperation.
     */
    void HandleMicrowaveOvenOpStateResumeCallback(OperationalState::GenericOperationalError & err);

    /**
     * handle command for operational state: start
     * @param[out] err: get operational error after callback.
     * this method is called to set operational state to target state.
     * if success, 'err' is set to NoError, otherwise it will be set to UnableToCompleteOperation.
     */
    void HandleMicrowaveOvenOpStateStartCallback(OperationalState::GenericOperationalError & err);

    /**
     * handle command for operational state: stop
     * @param[out] err: get operational error after callback.
     * this method is called to set operational state to target state.
     * if success, 'err' is set to NoError, otherwise it will be set to UnableToCompleteOperation.
     */
    void HandleMicrowaveOvenOpStateStopCallback(OperationalState::GenericOperationalError & err);

    /**
     * handle command for operational state: get count down time
     * return actual cook time.
     */
    app::DataModel::Nullable<uint32_t> HandleMicrowaveOvenOpStateGetCountdownTime();
};

} // namespace Clusters
} // namespace app
} // namespace chip
