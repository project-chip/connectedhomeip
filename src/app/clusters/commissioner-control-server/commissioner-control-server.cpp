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
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <platform/PlatformManager.h>

#include <memory>

using namespace chip;
using namespace chip::app;

using chip::Protocols::InteractionModel::Status;

namespace {

NodeId GetNodeId(const CommandHandler * commandObj)
{
    auto descriptor = commandObj->GetSubjectDescriptor();

    if (descriptor.authMode != Access::AuthMode::kCase)
    {
        return kUndefinedNodeId;
    }
    return descriptor.subject;
}

void AddReverseOpenCommissioningWindowResponse(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                               const Clusters::CommissionerControl::CommissioningWindowParams & params)
{
    Clusters::CommissionerControl::Commands::ReverseOpenCommissioningWindow::Type response;
    response.commissioningTimeout = params.commissioningTimeout;
    response.discriminator        = params.discriminator;
    response.iterations           = params.iterations;
    response.PAKEPasscodeVerifier = params.PAKEPasscodeVerifier;
    response.salt                 = params.salt;

    commandObj->AddResponse(path, response);
}

void RunDeferredCommissionNode(intptr_t commandArg)
{
    auto * params = reinterpret_cast<Clusters::CommissionerControl::CommissioningWindowParams *>(commandArg);

    Clusters::CommissionerControl::Delegate * delegate =
        Clusters::CommissionerControl::CommissionerControlServer::Instance().GetDelegate();

    if (delegate != nullptr)
    {
        CHIP_ERROR err = delegate->HandleCommissionNode(*params);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleCommissionNode error: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else
    {
        ChipLogError(Zcl, "No delegate available for HandleCommissionNode");
    }

    delete params;
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

CommissionerControlServer CommissionerControlServer::sInstance;

CommissionerControlServer & CommissionerControlServer::Instance()
{
    return sInstance;
}

CHIP_ERROR CommissionerControlServer::Init(Delegate & delegate)
{
    mDelegate = &delegate;
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
CommissionerControlServer::GenerateCommissioningRequestResultEvent(const Events::CommissioningRequestResult::Type & result)
{
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(result, kRootEndpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "CommissionerControl: Unable to emit CommissioningRequestResult event: %" CHIP_ERROR_FORMAT,
                     error.Format());
    }

    return error;
}

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfCommissionerControlClusterRequestCommissioningApprovalCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Clusters::CommissionerControl::Commands::RequestCommissioningApproval::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status  = Status::Success;

    ChipLogProgress(Zcl, "Received command to request commissioning approval");

    auto sourceNodeId = GetNodeId(commandObj);

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commissioning approval request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        commandObj->AddStatus(commandPath, Status::UnsupportedAccess);
        return true;
    }

    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto requestId   = commandData.requestID;
    auto vendorId    = commandData.vendorID;
    auto productId   = commandData.productID;

    // The label assigned from commandData need to be stored in CommissionerControl::Delegate which ensure that the backing buffer
    // of it has a valid lifespan during fabric sync setup process.
    auto & label = commandData.label;

    // Create a CommissioningApprovalRequest struct and populate it with the command data
    Clusters::CommissionerControl::CommissioningApprovalRequest request = { .requestId    = requestId,
                                                                            .vendorId     = vendorId,
                                                                            .productId    = productId,
                                                                            .clientNodeId = sourceNodeId,
                                                                            .fabricIndex  = fabricIndex,
                                                                            .label        = label };

    Clusters::CommissionerControl::Delegate * delegate =
        Clusters::CommissionerControl::CommissionerControlServer::Instance().GetDelegate();

    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Handle commissioning approval request
    err = delegate->HandleCommissioningApprovalRequest(request);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfCommissionerControlClusterRequestCommissioningApprovalCallback error: %" CHIP_ERROR_FORMAT,
                     err.Format());
        status = StatusIB(err).mStatus;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfCommissionerControlClusterCommissionNodeCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Clusters::CommissionerControl::Commands::CommissionNode::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Zcl, "Received command to commission node");

    auto sourceNodeId = GetNodeId(commandObj);

    // Constraint on responseTimeoutSeconds is [30; 120] seconds
    if ((commandData.responseTimeoutSeconds < 30) || (commandData.responseTimeoutSeconds > 120))
    {
        ChipLogError(Zcl, "Invalid responseTimeoutSeconds for CommissionNode.");
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commission node request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        commandObj->AddStatus(commandPath, Status::UnsupportedAccess);
        return true;
    }

    auto requestId = commandData.requestID;

    auto commissioningWindowParams = std::make_unique<Clusters::CommissionerControl::CommissioningWindowParams>();

    Clusters::CommissionerControl::Delegate * delegate =
        Clusters::CommissionerControl::CommissionerControlServer::Instance().GetDelegate();

    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Validate the commission node command.
    err = delegate->ValidateCommissionNodeCommand(sourceNodeId, requestId);
    SuccessOrExit(err);

    // Populate the parameters for the commissioning window
    err = delegate->GetCommissioningWindowParams(*commissioningWindowParams);
    SuccessOrExit(err);

    // Add the response for the commissioning window.
    AddReverseOpenCommissioningWindowResponse(commandObj, commandPath, *commissioningWindowParams);

    // Schedule the deferred reverse commission node task
    DeviceLayer::PlatformMgr().ScheduleWork(RunDeferredCommissionNode,
                                            reinterpret_cast<intptr_t>(commissioningWindowParams.release()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfCommissionerControlClusterCommissionNodeCallback error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, StatusIB(err).mStatus);
    }

    return true;
}

void MatterCommissionerControlPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing Commissioner Control cluster.");
}
