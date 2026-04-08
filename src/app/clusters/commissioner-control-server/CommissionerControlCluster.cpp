/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/clusters/commissioner-control-server/CommissionerControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/CommissionerControl/Metadata.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

namespace chip::app::Clusters {

using namespace CommissionerControl;
using namespace CommissionerControl::Attributes;

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

CommissionerControlCluster::CommissionerControlCluster(EndpointId endpointId, Delegate & delegate) :
    DefaultServerCluster({ endpointId, CommissionerControl::Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus CommissionerControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(CommissionerControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case SupportedDeviceCategories::Id:
        return encoder.Encode(mSupportedDeviceCategories);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR CommissionerControlCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), {});
}

std::optional<DataModel::ActionReturnStatus> CommissionerControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                       TLV::TLVReader & input_arguments,
                                                                                       CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::RequestCommissioningApproval::Id: {
        Commands::RequestCommissioningApproval::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleRequestCommissioningApproval(handler, request.path, data);
    }
    case Commands::CommissionNode::Id: {
        Commands::CommissionNode::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleCommissionNode(handler, request.path, data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR CommissionerControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements(
        { Commands::RequestCommissioningApproval::kMetadataEntry, Commands::CommissionNode::kMetadataEntry });
}

CHIP_ERROR CommissionerControlCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<CommandId> & builder)
{
    return builder.AppendElements({
        Commands::ReverseOpenCommissioningWindow::Id,
    });
}

void CommissionerControlCluster::SetSupportedDeviceCategories(
    const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories)
{
    SetAttributeValue(mSupportedDeviceCategories, supportedDeviceCategories, SupportedDeviceCategories::Id);
}

void CommissionerControlCluster::GenerateCommissioningRequestResultEvent(const Events::CommissioningRequestResult::Type & result)
{
    VerifyOrReturn(mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(result, mPath.mEndpointId);
}

std::optional<DataModel::ActionReturnStatus> CommissionerControlCluster::HandleRequestCommissioningApproval(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const CommissionerControl::Commands::RequestCommissioningApproval::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    using namespace chip::Protocols::InteractionModel;

    ChipLogProgress(Zcl, "Received command to request commissioning approval");

    auto sourceNodeId = GetNodeId(*commandObj);

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commissioning approval request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        return Status::UnsupportedAccess;
    }

    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto requestId   = commandData.requestID;
    auto vendorId    = commandData.vendorID;
    auto productId   = commandData.productID;

    // The label assigned from req need to be stored in CommissionerControl::Delegate which ensure that the backing buffer
    // of it has a valid lifespan during fabric sync setup process.
    auto & label = commandData.label;

    // Create a CommissioningApprovalRequest struct and populate it with the command data
    Clusters::CommissionerControl::CommissioningApprovalRequest request = { .requestId    = requestId,
                                                                            .vendorId     = vendorId,
                                                                            .productId    = productId,
                                                                            .clientNodeId = sourceNodeId,
                                                                            .fabricIndex  = fabricIndex,
                                                                            .label        = label };

    // Handle commissioning approval request
    err = mDelegate.HandleCommissioningApprovalRequest(request);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleRequestCommissioningApproval error: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
CommissionerControlCluster::HandleCommissionNode(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                 const CommissionerControl::Commands::CommissionNode::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    using namespace chip::Protocols::InteractionModel;

    ChipLogProgress(Zcl, "Received command to commission node");

    auto sourceNodeId = GetNodeId(*commandObj);

    // Constraint on responseTimeoutSeconds is [30; 120] seconds
    if ((commandData.responseTimeoutSeconds < 30) || (commandData.responseTimeoutSeconds > 120))
    {
        ChipLogError(Zcl, "Invalid responseTimeoutSeconds for CommissionNode.");
        return Status::ConstraintError;
    }

    // Check if the command is executed via a CASE session
    if (sourceNodeId == kUndefinedNodeId)
    {
        ChipLogError(Zcl, "Commission node request not executed via CASE session, failing with UNSUPPORTED_ACCESS");
        return Status::UnsupportedAccess;
    }

    auto requestId                 = commandData.requestID;
    auto commissioningWindowParams = std::make_unique<Clusters::CommissionerControl::CommissioningWindowParams>();

    // Validate the commission node command.
    err = mDelegate.ValidateCommissionNodeCommand(sourceNodeId, requestId);
    SuccessOrExit(err);

    // Populate the parameters for the commissioning window
    err = mDelegate.GetCommissioningWindowParams(*commissioningWindowParams);
    SuccessOrExit(err);

    // Add the response for the commissioning window.
    AddReverseOpenCommissioningWindowResponse(*commandObj, commandPath, *commissioningWindowParams);

    // Schedule the deferred reverse commission node task
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, params = commissioningWindowParams.release()]() {
        CHIP_ERROR error = mDelegate.HandleCommissionNode(*params);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleCommissionNode error: %" CHIP_ERROR_FORMAT, error.Format());
        }

        delete params;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleCommissionNode error: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return std::nullopt;
}

} // namespace chip::app::Clusters
