/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/util/af.h>
#include "operational-state-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

class Uncopyable
{
protected:
    Uncopyable() {}
    ~Uncopyable() = default;

private:
    Uncopyable(const Uncopyable &) = delete;
    Uncopyable & operator=(const Uncopyable &) = delete;
};

class Delegate;

/**
 * OperationalStateServer is a class that represents an instance of a derivation of the operational state cluster.
 * It implements CommandHandlerInterface so it can generically handle commands for any derivation cluster id.
 * todo OperationalState in OperationalStateServer is redundant given that we are in the OperationalState namespace, remove.
 */
class OperationalStateServer : public CommandHandlerInterface, public AttributeAccessInterface, public Uncopyable
{
public:
    /**
     * Creates an operational state cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the server's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the operational state aliased cluster to be instantiated.
     */
    OperationalStateServer(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId);

    ~OperationalStateServer() override;

    /**
     * Init the operational state server.
     * This function must be called after defining a OperationalStateServer class object.
     * @param void
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR Init();

    // Attribute accessors
    /**
     * Set operational phase.
     * @param phase The operational phase that should now be the current one.
     */
    CHIP_ERROR SetCurrentPhase(const app::DataModel::Nullable<uint8_t> & phase);

    /**
     * Set countdown time.
     * @param time The countdown time that should now be the current one.
     */
    CHIP_ERROR SetCountdownTime(const app::DataModel::Nullable<uint32_t> & time);

    /**
     * Set current operational state.
     * @param opState The operational state that should now be the current one.
     */
    CHIP_ERROR SetOperationalState(uint8_t opState);

    /**
     * Set operational error.
     * @param opErrState The new operational error.
     */
    CHIP_ERROR SetOperationalError(const GenericOperationalError & opErrState);

    /**
     * Get current phase
     * @param phase The app::DataModel::Nullable<uint8_t> to fill with the current phase value
     */
    app::DataModel::Nullable<uint8_t> GetCurrentPhase();

    /**
     * Get countdown time
     * @param time The app::DataModel::Nullable<uint32_t> to fill with the coutdown time value
     */
    app::DataModel::Nullable<uint32_t> GetCountdownTime();

    /**
     * Get the current operational state.
     * @return The current operational state value
     */
    uint8_t GetCurrentOperationalState() const;

    /**
     * Get current operational error.
     * @param error The GenericOperationalError to fill with the current operational error value
     */
    void GetCurrentOperationalError(GenericOperationalError & error);

    // Event triggers
    /**
     * @brief Called when the Node detects a OperationalError has been raised. Note: This function
     * also sets the OperationalState attribute to Error.
     * @param aError OperationalError which detects
     */
    void OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError);

    /**
     * @brief Called when the Node detects a OperationCompletion has been raised.
     * @param aCompletionErrorCode CompletionErrorCode
     * @param aTotalOperationalTime TotalOperationalTime
     * @param aPausedTime PausedTime
     */
    void OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                       const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime = NullOptional,
                                       const Optional<DataModel::Nullable<uint32_t>> & aPausedTime           = NullOptional) const;

private:
    Delegate * mDelegate;

    EndpointId mEndpointId;
    ClusterId mClusterId;

    // Attribute Data Store
    app::DataModel::Nullable<uint8_t> mCurrentPhase;
    app::DataModel::Nullable<uint32_t> mCountdownTime;
    uint8_t mOperationalState;
    GenericOperationalError mOperationalError;

    // Inherited from CommandHandlerInterface
    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // Inherited from CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    /// IM-level implementation of read
    ///
    /// Returns appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * Handle Command: Pause.
     */
    void HandlePauseState(HandlerContext & ctx, const Commands::Pause::DecodableType & req);

    /**
     * Handle Command: Resume.
     */
    void HandleResumeState(HandlerContext & ctx, const Commands::Resume::DecodableType & req);

    /**
     * Handle Command: Start.
     */
    void HandleStartState(HandlerContext & ctx, const Commands::Start::DecodableType & req);

    /**
     * Handle Command: Stop.
     */
    void HandleStopState(HandlerContext & ctx, const Commands::Stop::DecodableType & req);
};

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 * The delegate API assumes there will be separate delegate objects for each cluster instance.
 * (i.e. each separate operational state cluster derivation, on each separate endpoint),
 * since the delegate methods are not handed the cluster id or endpoint.
 */
class Delegate
{
protected:
    OperationalStateServer * mServer = nullptr;

public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Server object.
     * @param aServer A pointer to the Server object related to this delegate object.
     */
    void SetServer(OperationalStateServer * aServer) { mServer = aServer; }

    /**
     * Get the list of supported operational states.
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    virtual CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) = 0;

    /**
     * Get the list of supported operational phases.
     * Fills in the provided GenericOperationalPhase with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The GenericOperationalPhase is filled.
     */
    virtual CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase) = 0;

    // command callback
    /**
     * Handle Command Callback in application: Pause
     * @param[out] get operational error after callback.
     */
    virtual void HandlePauseStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] get operational error after callback.
     */
    virtual void HandleResumeStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     */
    virtual void HandleStartStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     */
    virtual void HandleStopStateCallback(GenericOperationalError & err) = 0;

};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
