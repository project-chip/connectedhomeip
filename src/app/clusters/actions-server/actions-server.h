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
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

class Delegate;

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the Actions cluster on all endpoints.
    Instance() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Actions::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), Actions::Id)
    {}

    static Instance * GetInstance();
    void SetDefaultDelegate(Delegate * aDelegate);

private:
    Delegate * mDelegate;
    static Instance instance;

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    CHIP_ERROR ReadActionListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadEndpointListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR FindActionIdInActionList(uint16_t actionId);

    // CommandHandlerInterface
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

class Delegate
{
public:
    virtual ~Delegate() = default;

    /**
     * Get the action at the Nth index from list of actions.
     * @param index The index of the action to be returned. It is assumed that actions are indexable from 0 and with no gaps.
     * @param action A reference to the action struct which copies existing and initialised buffer at index.
     * @return Returns a CHIP_NO_ERROR if there was no error and the action was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index in beyond the list of available actions.
     */
    virtual CHIP_ERROR ReadActionAtIndex(uint16_t index, Structs::ActionStruct::Type & action);

    /**
     * Get the EndpointList at the Nth index from list of endpointList.
     * @param index The index of the endpointList to be returned. It is assumed that actions are indexable from 0 and with no gaps.
     * @param epList A reference to the endpointList struct which copies existing and initialised buffer at index.
     * @return Returns a CHIP_NO_ERROR if there was no error and the epList was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index in beyond the list of available endpointList.
     */
    virtual CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, Structs::EndpointListStruct::Type & epList);

    /**
     * Find the action with matching actionId in the list of action.
     * @param actionId The action to be find in the list of action.
     * @return Returns a CHIP_NO_ERROR if matching action is found.
     * CHIP_ERROR_NOT_FOUND if the matching action does not found in the list of action.
     */
    virtual CHIP_ERROR FindActionIdInActionList(uint16_t actionId);

    /**
     * On reciept of each and every command,
     * if the InvokeID data field is provided by the client when invoking a command, the server SHALL generate a StateChanged event
     * when the action changes to a new state or an ActionFailed event when execution of the action fails.
     *
     * @return If the command refers to an action which currently is not in a state where the command applies, a response SHALL be
     * generated with the StatusCode INVALID_COMMAND.
     */

    /**
     * When an InstantAction command is recieved, an action (state change) on the involved endpoints shall trigger,
     * in a "fire and forget" manner. Afterwards, the action’s state SHALL be Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When an InstantActionWithTransition command is recieved, an action (state change) on the involved endpoints shall trigger,
     * with a specified time to transition from the current state to the new state. During the transition, the action’s state SHALL
     * be Active. Afterwards, the action’s state SHALL be Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @param transitionTime The time for transition from the current state to the new state.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId);

    /**
     * When a StartAction command is recieved, the commencement of an action on the involved endpoints shall trigger. Afterwards,
     * the action’s state SHALL be Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When a StartActionWithDuration command is recieved, the commencement of an action on the involved endpoints shall trigger,
     * and SHALL change the action’s state to Active. After the specified Duration, the action will stop, and the action’s state
     * SHALL change to Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @param duration The time for which an action shall be in start state.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);

    /**
     * When a StopAction command is recieved, the ongoing action on the involved endpoints shall stop. Afterwards, the action’s
     * state SHALL be Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When a PauseAction command is recieved, the ongoing action on the involved endpoints shall pause and SHALL change the
     * action’s state to Paused.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When a PauseActionWithDuration command is recieved, pauses an ongoing action, and SHALL change the action’s state to Paused.
     * After the specified Duration, the ongoing action will be automatically resumed. which SHALL change the action’s state to
     * Active.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @param duration The time for which an action shall be in pause state.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);

    /**
     * When a ResumeAction command is recieved, the previously paused action shall resume and SHALL change the action’s state to
     * Active.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When an EnableAction command is recieved, it enables a certain action or automation. Afterwards, the action’s state SHALL be
     * Active.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When an EnableActionWithDuration command is recieved, it enables a certain action or automation, and SHALL change the
     * action’s state to be Active. After the specified Duration, the action or automation will stop, and the action’s state SHALL
     * change to Disabled.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @param duration The time for which an action shall be in active state.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);

    /**
     * When a DisableAction command is recieved, it disables a certain action or automation, and SHALL change the action’s state to
     * Inactive.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId);

    /**
     * When a DisableActionWithDuration command is recieved, it disables a certain action or automation, and SHALL change the
     * action’s state to Disabled. After the specified Duration, the action or automation will re-start, and the action’s state
     * SHALL change to either Inactive or Active, depending on the actions.
     *
     * @param actionId The id of an action on which an action shall takes place.
     * @param duration The time for which an action shall be in disable state.
     * @return Returns a Success if an action took place successfully otherwise, suitable error.
     */
    virtual Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
