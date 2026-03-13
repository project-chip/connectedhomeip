/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/reporting/reporting.h>
#include <protocols/interaction_model/StatusCode.h>

#include "ActionsDelegate.h"
#include "ActionsStructs.h"
namespace chip {
namespace app {
namespace Clusters {
namespace Actions {
class ActionsServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the Actions cluster on all endpoints.
    ActionsServer(EndpointId aEndpointId, Delegate & aDelegate) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Actions::Id),
        CommandHandlerInterface(MakeOptional(aEndpointId), Actions::Id), mDelegate(aDelegate), mEndpointId(aEndpointId)
    {}

    ~ActionsServer();

    /**
     * Initialise the Actions server instance.
     * @return Returns an error if the given endpoint and cluster have not been enabled in zap, if the
     * AttributeAccessInterface or AttributeAccessInterface registration fails returns an error.
     */
    CHIP_ERROR Init();

    /**
     * Unregisters the CommandHandlerInterface and AttributeAccessInterface.
     */
    void Shutdown();

    /**
     * @brief
     *   Called when the state of an action is changed.
     */
    void OnStateChanged(EndpointId aEndpoint, uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState);

    /**
     * @brief
     *   Called when an action fails.
     */
    void OnActionFailed(EndpointId aEndpoint, uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState,
                        ActionErrorEnum aActionError);

    void SetDefaultDelegate(EndpointId aEndpointId, Delegate * aDelegate);

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * A notification from an application to the sever that an ActionList is modified..
     *
     * @param aEndpoint The endpoint ID where the action should be updated
     */
    void ActionListModified(EndpointId aEndpoint);

    /**
     * A notification from an application to the sever that an EndpointList is modified..
     *
     * @param aEndpoint The endpoint ID where the action should be updated
     */
    void EndpointListModified(EndpointId aEndpoint);

private:
    Delegate & mDelegate;
    EndpointId mEndpointId;
    static ActionsServer sInstance;
    static constexpr size_t kMaxEndpointListLength = 256u;
    static constexpr size_t kMaxActionListLength   = 256u;

    CHIP_ERROR ReadActionListAttribute(const ConcreteReadAttributePath & aPath,
                                       const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    CHIP_ERROR ReadEndpointListAttribute(const ConcreteReadAttributePath & aPath,
                                         const AttributeValueEncoder::ListEncodeHelper & aEncoder);
    bool HaveActionWithId(EndpointId aEndpointId, uint16_t aActionId, uint16_t & aActionIndex);

    // TODO: We should move to non-global dirty marker.
    void MarkDirty(EndpointId aEndpointId, AttributeId aAttributeId)
    {
        MatterReportingAttributeChangeCallback(aEndpointId, Id, aAttributeId);
    }
    // Cannot use CommandHandlerInterface::HandleCommand directly because we need to do the HaveActionWithId() check before
    // handling a command.
    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleInstantAction(HandlerContext & ctx, const Commands::InstantAction::DecodableType & commandData);
    void HandleInstantActionWithTransition(HandlerContext & ctx,
                                           const Commands::InstantActionWithTransition::DecodableType & commandData);
    void HandleStartAction(HandlerContext & ctx, const Commands::StartAction::DecodableType & commandData);
    void HandleStartActionWithDuration(HandlerContext & ctx, const Commands::StartActionWithDuration::DecodableType & commandData);
    void HandleStopAction(HandlerContext & ctx, const Commands::StopAction::DecodableType & commandData);
    void HandlePauseAction(HandlerContext & ctx, const Commands::PauseAction::DecodableType & commandData);
    void HandlePauseActionWithDuration(HandlerContext & ctx, const Commands::PauseActionWithDuration::DecodableType & commandData);
    void HandleResumeAction(HandlerContext & ctx, const Commands::ResumeAction::DecodableType & commandData);
    void HandleEnableAction(HandlerContext & ctx, const Commands::EnableAction::DecodableType & commandData);
    void HandleEnableActionWithDuration(HandlerContext & ctx,
                                        const Commands::EnableActionWithDuration::DecodableType & commandData);
    void HandleDisableAction(HandlerContext & ctx, const Commands::DisableAction::DecodableType & commandData);
    void HandleDisableActionWithDuration(HandlerContext & ctx,
                                         const Commands::DisableActionWithDuration::DecodableType & commandData);
};
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
