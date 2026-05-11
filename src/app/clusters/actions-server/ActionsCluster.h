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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/actions-server/ActionsDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/Actions/Attributes.h>
#include <clusters/Actions/Events.h>

namespace chip::app::Clusters {

class ActionsCluster : public DefaultServerCluster
{
public:
    using OptionalAttributesSet = OptionalAttributeSet<Actions::Attributes::SetupURL::Id>;

    ActionsCluster(EndpointId endpointId, Actions::Delegate & delegate, OptionalAttributesSet optionalAttributes = {},
                   std::optional<CharSpan> setupURL = std::nullopt) :
        DefaultServerCluster({ endpointId, Actions::Id }),
        mDelegate(delegate), mOptionalAttributes(optionalAttributes), mSetupURL(setupURL)
    {}

    ~ActionsCluster() = default;

    void ActionListModified();

    void EndpointListsModified();

    /**
     * Public helper API for the Application/Delegate to call asynchronously to emit a Matter event to the fabric.
     * Use these when an action's state changes or an action fails.
     */
    void GenerateEvent(const Actions::Events::StateChanged::Type & event);
    void GenerateEvent(const Actions::Events::ActionFailed::Type & event);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    Actions::Delegate & mDelegate;
    const OptionalAttributesSet mOptionalAttributes;
    const std::optional<CharSpan> mSetupURL;

    CHIP_ERROR ReadActionListAttribute(const DataModel::ReadAttributeRequest & request,
                                       const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    CHIP_ERROR ReadEndpointListAttribute(const DataModel::ReadAttributeRequest & request,
                                         const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    Protocols::InteractionModel::Status ValidateActionExists(uint16_t actionID, uint16_t & outActionIndex);

    Protocols::InteractionModel::Status ValidateCommandSupported(uint16_t actionIndex, CommandId commandId);
};

} // namespace chip::app::Clusters
