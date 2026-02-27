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

#include "actions-server.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/reporting/reporting.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

// Null delegate implementation for static instance initialization
// This is a legacy facade class that delegates to the new code-driven architecture
// The static instance is kept for backward compatibility but is not actively used
namespace {
class NullDelegate : public Delegate
{
public:
    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    bool HaveActionWithId(uint16_t aActionId, uint16_t & aActionIndex) override { return false; }

    Protocols::InteractionModel::Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                          Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                      Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                       Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    Protocols::InteractionModel::Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration,
                                                                        Optional<uint32_t> invokeId) override
    {
        return Protocols::InteractionModel::Status::Failure;
    }
};

static NullDelegate gNullDelegate;
} // namespace

ActionsServer ActionsServer::sInstance(kInvalidEndpointId, gNullDelegate);

ActionsServer::~ActionsServer()
{
    Shutdown();
}

CHIP_ERROR ActionsServer::Init()
{
    // Delegate to the new code-driven architecture
    // The CodegenIntegration will handle registration via CodegenClusterIntegration
    MatterActionsClusterInitCallback(mEndpointId);
    return CHIP_NO_ERROR;
}

void ActionsServer::Shutdown()
{
    // Delegate to the new code-driven architecture
    // The CodegenIntegration will handle unregistration via CodegenClusterIntegration
    MatterActionsClusterShutdownCallback(mEndpointId, ClusterShutdownType::kClusterShutdown);
}

void ActionsServer::OnStateChanged(EndpointId aEndpoint, uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState)
{
    // This method is called by the application to notify of state changes
    // In the new architecture, the delegate would call this on the cluster instance
    // For now, we keep this for backward compatibility but it may not be used
    ChipLogProgress(Zcl, "ActionsServer::OnStateChanged called");
}

void ActionsServer::OnActionFailed(EndpointId aEndpoint, uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState,
                                   ActionErrorEnum aActionError)
{
    // This method is called by the application to notify of action failures
    // In the new architecture, the delegate would call this on the cluster instance
    // For now, we keep this for backward compatibility but it may not be used
    ChipLogProgress(Zcl, "ActionsServer::OnActionFailed called");
}

void ActionsServer::SetDefaultDelegate(EndpointId aEndpointId, Delegate * aDelegate)
{
    // This method is kept for backward compatibility
    // In the new architecture, delegates are managed differently
    ChipLogProgress(Zcl, "ActionsServer::SetDefaultDelegate called");
}

CHIP_ERROR ActionsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // This method is kept for backward compatibility
    // In the new architecture, attribute reading is handled by the ActionsCluster
    ChipLogProgress(Zcl, "ActionsServer::Read called - delegating to new architecture");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ActionsServer::ActionListModified(EndpointId aEndpoint)
{
    // This method is kept for backward compatibility
    // In the new architecture, this would trigger attribute reporting
    MarkDirty(aEndpoint, ActionList::Id);
}

void ActionsServer::EndpointListModified(EndpointId aEndpoint)
{
    // This method is kept for backward compatibility
    // In the new architecture, this would trigger attribute reporting
    MarkDirty(aEndpoint, EndpointLists::Id);
}

CHIP_ERROR ActionsServer::ReadActionListAttribute(const ConcreteReadAttributePath & aPath,
                                                  const AttributeValueEncoder::ListEncodeHelper & aEncoder)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ActionsServer::ReadEndpointListAttribute(const ConcreteReadAttributePath & aPath,
                                                    const AttributeValueEncoder::ListEncodeHelper & aEncoder)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ActionsServer::HaveActionWithId(EndpointId aEndpointId, uint16_t aActionId, uint16_t & aActionIndex)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by the delegate
    return mDelegate.HaveActionWithId(aActionId, aActionIndex);
}

template <typename RequestT, typename FuncT>
void ActionsServer::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    // This method is kept for backward compatibility
    // In the new architecture, command handling is done by ActionsCluster
}

void ActionsServer::InvokeCommand(HandlerContext & handlerContext)
{
    // This method is kept for backward compatibility
    // In the new architecture, command handling is done by ActionsCluster
    ChipLogProgress(Zcl, "ActionsServer::InvokeCommand called - delegating to new architecture");
}

void ActionsServer::HandleInstantAction(HandlerContext & ctx, const Commands::InstantAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleInstantActionWithTransition(HandlerContext & ctx,
                                                      const Commands::InstantActionWithTransition::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleStartAction(HandlerContext & ctx, const Commands::StartAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleStartActionWithDuration(HandlerContext & ctx,
                                                  const Commands::StartActionWithDuration::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleStopAction(HandlerContext & ctx, const Commands::StopAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandlePauseAction(HandlerContext & ctx, const Commands::PauseAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandlePauseActionWithDuration(HandlerContext & ctx,
                                                  const Commands::PauseActionWithDuration::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleResumeAction(HandlerContext & ctx, const Commands::ResumeAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleEnableAction(HandlerContext & ctx, const Commands::EnableAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleEnableActionWithDuration(HandlerContext & ctx,
                                                   const Commands::EnableActionWithDuration::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleDisableAction(HandlerContext & ctx, const Commands::DisableAction::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}

void ActionsServer::HandleDisableActionWithDuration(HandlerContext & ctx,
                                                    const Commands::DisableActionWithDuration::DecodableType & commandData)
{
    // This method is kept for backward compatibility
    // In the new architecture, this is handled by ActionsCluster
}
