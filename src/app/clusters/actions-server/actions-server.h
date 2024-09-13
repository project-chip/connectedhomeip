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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

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

    virtual CHIP_ERROR ReadActionAtIndex(uint16_t index, Structs::ActionStruct::Type & action);
    virtual CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, Structs::EndpointListStruct::Type & epList);
    virtual CHIP_ERROR FindActionIdInActionList(uint16_t actionId);

    virtual Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId);
    virtual Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);
    virtual Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);
    virtual Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);
    virtual Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId);
    virtual Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId);
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
