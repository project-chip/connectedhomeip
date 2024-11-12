/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "commissioner-control-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <memory>

using namespace chip;
using namespace chip::app;

using chip::Protocols::InteractionModel::Status;

namespace {

NodeId GetNodeId(const CommandHandler & commandHandler)
{
    auto descriptor = commandHandler.GetSubjectDescriptor();

    if (descriptor.authMode != Access::AuthMode::kCase)
    {
        return kUndefinedNodeId;
    }
    return descriptor.subject;
}

void AddReverseOpenCommissioningWindowResponse(CommandHandler & commandHandler, const ConcreteCommandPath & path,
                                               const Clusters::CommissionerControl::CommissioningWindowParams & params)
{
    Clusters::CommissionerControl::Commands::ReverseOpenCommissioningWindow::Type response;
    response.commissioningTimeout = params.commissioningTimeout;
    response.discriminator        = params.discriminator;
    response.iterations           = params.iterations;
    response.PAKEPasscodeVerifier = params.PAKEPasscodeVerifier;
    response.salt                 = params.salt;

    commandHandler.AddResponse(path, response);
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

CommissionerControlServer::CommissionerControlServer(Delegate * delegate, EndpointId endpointId) :
    CommandHandlerInterface(MakeOptional(endpointId), Id)
{
    mDelegate = delegate;
}

CommissionerControlServer::~CommissionerControlServer()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
}

CHIP_ERROR CommissionerControlServer::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

Status CommissionerControlServer::GetSupportedDeviceCategoriesValue(
    EndpointId endpoint, BitMask<SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const
{
    Status status = Attributes::SupportedDeviceCategories::Get(endpoint, supportedDeviceCategories);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "CommissionerControl: reading  supportedDeviceCategories, err:0x%x", to_underlying(status));
    }
    return status;
}

Status
CommissionerControlServer::SetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                                             const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories)
{
    Status status = Status::Success;

    if ((status = Attributes::SupportedDeviceCategories::Set(endpoint, supportedDeviceCategories)) != Status::Success)
    {
        ChipLogProgress(Zcl, "CommissionerControl: writing supportedDeviceCategories, err:0x%x", to_underlying(status));
        return status;
    }

    return status;
}

CHIP_ERROR
CommissionerControlServer::GenerateCommissioningRequestResultEvent(EndpointId endpoint,
                                                                   const Events::CommissioningRequestResult::Type & result)
{
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(result, endpoint, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "CommissionerControl: Unable to emit CommissioningRequestResult event: %" CHIP_ERROR_FORMAT,
                     error.Format());
    }

    return error;
}

void CommissionerControlServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "CommissionerControl: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::RequestCommissioningApproval::Id:
        ChipLogDetail(Zcl, "CommissionerControl: Entering RequestCommissioningApproval");

        CommandHandlerInterface::HandleCommand<Commands::RequestCommissioningApproval::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleRequestCommissioningApproval(ctx, req); });
        break;

    case Commands::CommissionNode::Id:
        ChipLogDetail(Zcl, "CommissionerControl: Entering CommissionNode");

        CommandHandlerInterface::HandleCommand<Commands::CommissionNode::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleCommissionNode(ctx, req); });
        break;
    }
}

void CommissionerControlServer::HandleRequestCommissioningApproval(
    HandlerContext & ctx, const Commands::RequestCommissioningApproval::DecodableType & req)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status  = Status::Success;

    ChipLogProgress(Zcl, "Received command to request commissioning approval");

    auto sourceNodeId = GetNodeId(ctx.mCommandHandler);

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commissioning approval request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedAccess);
        return;
    }

    auto fabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();
    auto requestId   = req.requestID;
    auto vendorId    = req.vendorID;
    auto productId   = req.productID;

    // The label assigned from req need to be stored in CommissionerControl::Delegate which ensure that the backing buffer
    // of it has a valid lifespan during fabric sync setup process.
    auto & label = req.label;

    // Create a CommissioningApprovalRequest struct and populate it with the command data
    Clusters::CommissionerControl::CommissioningApprovalRequest request = { .requestId    = requestId,
                                                                            .vendorId     = vendorId,
                                                                            .productId    = productId,
                                                                            .clientNodeId = sourceNodeId,
                                                                            .fabricIndex  = fabricIndex,
                                                                            .label        = label };

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Handle commissioning approval request
    err = mDelegate->HandleCommissioningApprovalRequest(request);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleRequestCommissioningApproval error: %" CHIP_ERROR_FORMAT, err.Format());
        status = StatusIB(err).mStatus;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CommissionerControlServer::HandleCommissionNode(HandlerContext & ctx, const Commands::CommissionNode::DecodableType & req)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Zcl, "Received command to commission node");

    auto sourceNodeId = GetNodeId(ctx.mCommandHandler);

    // Constraint on responseTimeoutSeconds is [30; 120] seconds
    if ((req.responseTimeoutSeconds < 30) || (req.responseTimeoutSeconds > 120))
    {
        ChipLogError(Zcl, "Invalid responseTimeoutSeconds for CommissionNode.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commission node request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedAccess);
        return;
    }

    auto requestId                 = req.requestID;
    auto delegate                  = mDelegate;
    auto commissioningWindowParams = std::make_unique<Clusters::CommissionerControl::CommissioningWindowParams>();

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Validate the commission node command.
    err = mDelegate->ValidateCommissionNodeCommand(sourceNodeId, requestId);
    SuccessOrExit(err);

    // Populate the parameters for the commissioning window
    err = mDelegate->GetCommissioningWindowParams(*commissioningWindowParams);
    SuccessOrExit(err);

    // Add the response for the commissioning window.
    AddReverseOpenCommissioningWindowResponse(ctx.mCommandHandler, ctx.mRequestPath, *commissioningWindowParams);

    // Schedule the deferred reverse commission node task
    DeviceLayer::SystemLayer().ScheduleLambda([delegate, params = commissioningWindowParams.release()]() {
        if (delegate != nullptr)
        {
            CHIP_ERROR error = delegate->HandleCommissionNode(*params);
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleCommissionNode error: %" CHIP_ERROR_FORMAT, error.Format());
            }
        }
        else
        {
            ChipLogError(Zcl, "No delegate available for HandleCommissionNode");
        }

        delete params;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleCommissionNode error: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, StatusIB(err).mStatus);
    }
}

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterCommissionerControlPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing Commissioner Control cluster.");
}
