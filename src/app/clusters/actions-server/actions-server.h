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

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

static constexpr size_t kActionNameMaxSize       = 128u;
static constexpr size_t kEndpointListNameMaxSize = 128u;

static constexpr size_t kEndpointListMaxSize = 256u;

class Delegate;

struct ActionStructStorage : public Structs::ActionStruct::Type
{
    ActionStructStorage(){};

    ActionStructStorage(uint16_t aAction, const CharSpan & aActionName, ActionTypeEnum aActionType, uint16_t aEpListID,
                        BitMask<CommandBits> aCommands, ActionStateEnum aActionState)
    {
        Set(aAction, aActionName, aActionType, aEpListID, aCommands, aActionState);
    }

    ActionStructStorage(const ActionStructStorage & aAction) { *this = aAction; }

    ActionStructStorage & operator=(const ActionStructStorage & aAction)
    {
        Set(aAction.actionID, aAction.name, aAction.type, aAction.endpointListID, aAction.supportedCommands, aAction.state);
        return *this;
    }

    void Set(uint16_t aAction, const CharSpan & aActionName, ActionTypeEnum aActionType, uint16_t aEpListID,
             BitMask<CommandBits> aCommands, ActionStateEnum aActionState)
    {
        actionID          = aAction;
        type              = aActionType;
        endpointListID    = aEpListID;
        supportedCommands = aCommands;
        state             = aActionState;
        MutableCharSpan actionName(mBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(aActionName, actionName);
        name = actionName;
    }

private:
    char mBuffer[kActionNameMaxSize];
};

struct EndpointListStorage : public Structs::EndpointListStruct::Type
{
    EndpointListStorage(){};

    EndpointListStorage(uint16_t aEpListId, const CharSpan & aEpListName, EndpointListTypeEnum aEpListType,
                        const DataModel::List<const EndpointId> & aEndpointList)
    {
        Set(aEpListId, aEpListName, aEpListType, aEndpointList);
    }

    EndpointListStorage(const EndpointListStorage & aEpList) { *this = aEpList; }

    EndpointListStorage & operator=(const EndpointListStorage & aEpList)
    {
        Set(aEpList.endpointListID, aEpList.name, aEpList.type, aEpList.endpoints);
        return *this;
    }

    void Set(uint16_t aEpListId, const CharSpan & aEpListName, EndpointListTypeEnum aEpListType,
             const DataModel::List<const EndpointId> & aEndpointList)
    {
        endpointListID    = aEpListId;
        type              = aEpListType;
        size_t epListSize = std::min(aEndpointList.size(), MATTER_ARRAY_SIZE(mEpList));

        for (size_t index = 0; index < epListSize; index++)
        {
            mEpList[index] = aEndpointList[index];
        }
        endpoints = DataModel::List<const EndpointId>(Span(mEpList, epListSize));
        MutableCharSpan epListName(mBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(aEpListName, epListName);
        name = epListName;
    }

private:
    char mBuffer[kEndpointListNameMaxSize];
    EndpointId mEpList[kEndpointListMaxSize];
};

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

class Delegate
{
public:
    virtual ~Delegate() = default;

    /**
     * Get the action at the Nth index from list of actions.
     * @param index The index of the action to be returned. It is assumed that actions are indexable from 0 and with no gaps.
     * @param action A reference to the ActionStructStorage which should be initialized via copy/assignments or calling Set().
     * @return Returns a CHIP_NO_ERROR if there was no error and the action was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is past the end of the list of actions.
     */
    virtual CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) = 0;

    /**
     * Get the EndpointList at the Nth index from list of endpointList.
     * @param index The index of the endpointList to be returned. It is assumed that endpoint lists are indexable from 0 and with no
     * gaps.
     * @param action A reference to the EndpointListStorage which should be initialized via copy/assignments or calling Set().
     * @return Returns a CHIP_NO_ERROR if there was no error and the epList was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is past the end of the list of endpointLists.
     */
    virtual CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) = 0;

    /**
     * Check whether there is an action with the given actionId in the list of actions.
     * @param aActionId The action ID to search for.
     * @param aActionIndex A reference to the index at which an action with matching aActionId.
     * @return Returns a true if matching action is found otherwise false.
     */
    virtual bool HaveActionWithId(uint16_t aActionId, uint16_t & aActionIndex) = 0;

    /**
     * The implementations of the Handle* command callbacks below are expected to call OnStateChanged or
     * OnActionFailed as needed to generate the events required by the spec.
     */

    /**
     * @brief Callback that will be called to handle an InstantAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle an InstantActionWithTransition command.
     * @param actionId The actionId of an action.
     * @param transitionTime The time for transition from the current state to the new state.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                                  Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a StartAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a StartActionWithDuration command.
     * @param actionId The actionId of an action.
     * @param duration The time for which an action shall be in start state.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                              Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a StopAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a PauseAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a PauseActionWithDuration command.
     * @param actionId The actionId of an action.
     * @param duration The time for which an action shall be in pause state.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                              Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a ResumeAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle an EnableAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle an EnableActionWithDuration command.
     * @param actionId The actionId of an action.
     * @param duration The time for which an action shall be in active state.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                               Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a DisableAction command.
     * @param actionId The actionId of an action.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) = 0;

    /**
     * @brief Callback that will be called to handle a DisableActionWithDuration command.
     * @param actionId The actionId of an action.
     * @param duration The time for which an action shall be in disable state.
     * It should report Status::Success if successful and may report other Status codes if it fails.
     */
    virtual Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                                Optional<uint32_t> invokeId) = 0;
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
