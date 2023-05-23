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

#include "OperationalStateDataProvider.h"

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/operational-state-server/operational-state-delegate.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

class OperationalStateServer : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    template <typename T>
    using DataModelListTemplate          = app::DataModel::List<T>;
    using OperationalStateStructType     = app::Clusters::OperationalState::Structs::OperationalStateStruct::Type;
    using OperationalStateStructTypeList = app::DataModel::List<OperationalStateStructType>;
    using PhaseList                      = chip::app::DataModel::List<const chip::CharSpan>;
    /**
     * Init the operational state server.
     * @param void
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR Init();

    // Inherited from CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    /// IM-level implementation of read
    ///
    /// Returns appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * Set operational state list.
     * @param operationalStateList The operational state list for which to save.
     *  Template for save operational state alias cluster
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    template <typename T>
    CHIP_ERROR SetOperationalStateList(const DataModelListTemplate<T> & operationalStateList);

    /**
     * Get operational state list.
     * @param operationalStateList The pointer to save operational state list.
     * @param size The number of operational state list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetOperationalStateList(OperationalStateStructDynamicList ** operationalStateList, size_t & size);

    /**
     * Rlease OperationalStateStructDynamicList
     * @param operationalStateList The pointer for which to clear the OperationalStateStructDynamicList.
     * @return void
     */
    void ReleaseOperationalStateList(OperationalStateStructDynamicList * operationalStateList);

    /**
     * Clear operational state list.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearOperationalStateList();

    /**
     * Set phase list.
     * @param phaseList The phase list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetPhaseList(const PhaseList & phaseList);

    /**
     * Get phase list.
     * @param phaseList The pointer to load phase list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetPhaseList(PhaseListCharSpan ** phaseList, size_t & size);

    /**
     * Rlease PhaseListCharSpan
     * @param phaseList The pointer for which to clear the PhaseListCharSpan.
     * @return void
     */
    void ReleasePhaseList(PhaseListCharSpan * phaseList);

    /**
     * Clear phase list.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearPhaseStateList();

    /**
     * Set operational state.
     * @param opState The operational state for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetOperationalState(OperationalStateStruct & opState);

    /**
     * Get operational state.
     * @param void.
     * @return the reference of operational state.
     */
    OperationalStateStruct & GetOperationalState();

    /**
     * Set operational error.
     * @param the reference of operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetOperationalError(OperationalErrorStateStruct & opErrState);

private:
    EndpointId endpointId;
    ClusterId clusterId;
    Delegate * delegate;
    OperationalStateDataProvider mOperationalStateDataProvider;
    OperationalStateStruct operationalState;
    OperationalErrorStateStruct operationalError;

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

public:
    /**
     * Creates a operational state cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeSelect aliased cluster to be instantiated.
     * @param aDelegate A pointer to a delegate that will handle application layer logic.
     */
    OperationalStateServer(EndpointId aEndpointId, ClusterId aClusterId, Delegate * aDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId)
    {

        endpointId = aEndpointId;
        clusterId  = aClusterId;
        delegate   = aDelegate;
    }

    // Inherited from CommandHandlerInterface
    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    ~OperationalStateServer() override {}
};

template <typename T>
CHIP_ERROR OperationalStateServer::SetOperationalStateList(const DataModelListTemplate<T> & operationalStateList)
{
    return mOperationalStateDataProvider.StoreOperationalStateList<T>(endpointId, clusterId, operationalStateList);
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
