/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include "ActionsStructs.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

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
     * @param epList A reference to the EndpointListStorage which should be initialized via copy/assignments or calling Set().
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
