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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/DataModelTypes.h>

#include "ActionStructs.h"
#include "ActionsDelegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

static constexpr size_t kMaxEndpointListLength = 256u;
static constexpr size_t kMaxActionListLength   = 256u;

class ActionsCluster : public DefaultServerCluster
{
public:
    ActionsCluster(EndpointId endpointId, Delegate & delegate) :
        DefaultServerCluster({ endpointId, Actions::Id }), mDelegate(delegate)
    {}

    ~ActionsCluster() override = default;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    void Shutdown(ClusterShutdownType type) override;

    void ActionListModified();

    void EndpointListsModified();

    void OnStateChanged(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState);

    void OnActionFailed(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState, ActionErrorEnum aActionError);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    Delegate & mDelegate;
    ServerClusterContext * mContext = nullptr;

    CHIP_ERROR ReadActionListAttribute(const DataModel::ReadAttributeRequest & request,
                                       const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    CHIP_ERROR ReadEndpointListAttribute(const DataModel::ReadAttributeRequest & request,
                                         const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    bool HaveActionWithId(uint16_t aActionId, uint16_t & aActionIndex);

    void HandleInstantAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                             const Commands::InstantAction::DecodableType & commandData);

    void HandleInstantActionWithTransition(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                           const Commands::InstantActionWithTransition::DecodableType & commandData);

    void HandleStartAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                           const Commands::StartAction::DecodableType & commandData);

    void HandleStartActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                       const Commands::StartActionWithDuration::DecodableType & commandData);

    void HandleStopAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                          const Commands::StopAction::DecodableType & commandData);

    void HandlePauseAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                           const Commands::PauseAction::DecodableType & commandData);

    void HandlePauseActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                       const Commands::PauseActionWithDuration::DecodableType & commandData);

    void HandleResumeAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                            const Commands::ResumeAction::DecodableType & commandData);

    void HandleEnableAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                            const Commands::EnableAction::DecodableType & commandData);

    void HandleEnableActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                        const Commands::EnableActionWithDuration::DecodableType & commandData);

    void HandleDisableAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                             const Commands::DisableAction::DecodableType & commandData);

    void HandleDisableActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                         const Commands::DisableActionWithDuration::DecodableType & commandData);

    /**
     * @brief Unified helper for attribute change notifications.
     * @param attributeId The ID of the attribute that changed.
     *
     * This method replaces direct calls to MatterReportingAttributeChangeCallback
     * and uses the DefaultServerCluster's NotifyAttributeChanged mechanism.
     */
    void OnClusterAttributeChanged(AttributeId attributeId);
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
