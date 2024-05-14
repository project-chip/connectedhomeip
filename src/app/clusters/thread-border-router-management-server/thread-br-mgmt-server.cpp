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

#include "thread-br-mgmt-server.h"

#include "app-common/zap-generated/cluster-objects.h"
#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "app/AttributeAccessInterfaceRegistry.h"
#include "app/AttributeValueEncoder.h"
#include "app/CommandHandler.h"
#include "app/CommandHandlerInterface.h"
#include "app/InteractionModelEngine.h"
#include "app/clusters/general-commissioning-server/general-commissioning-server.h"
#include "app/data-model/List.h"
#include "app/data-model/Nullable.h"
#include "app/server/Server.h"
#include "lib/core/CHIPError.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/Span.h"
#include "lib/support/ThreadOperationalDataset.h"
#include "lib/support/TypeTraits.h"
#include "protocols/interaction_model/StatusCode.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

static Protocols::InteractionModel::Status MapChipErrorToIMStatus(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return Protocols::InteractionModel::Status::ResourceExhausted;
    }
    else if (err == CHIP_ERROR_NO_SHARED_TRUSTED_ROOT)
    {
        return Protocols::InteractionModel::Status::UnsupportedAccess;
    }
    else if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }
    else if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return Protocols::InteractionModel::Status::InvalidInState;
    }
    else if (err == CHIP_ERROR_NOT_FOUND)
    {
        return Protocols::InteractionModel::Status::NotFound;
    }
    return Protocols::InteractionModel::Status::Failure;
}

static CHIP_ERROR AddStatusOnError(CommandHandlerInterface::HandlerContext & ctx, CHIP_ERROR err)
{
    if (CHIP_NO_ERROR != err)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, MapChipErrorToIMStatus(err));
    }
    return err;
}

static bool CheckOverCASESession(CommandHandlerInterface::HandlerContext & ctx)
{
    chip::Messaging::ExchangeContext * exchangeCtx = ctx.mCommandHandler.GetExchangeContext();
    if (!exchangeCtx || !exchangeCtx->HasSessionHandle() || !exchangeCtx->GetSessionHandle()->IsSecureSession() ||
        exchangeCtx->GetSessionHandle()->AsSecureSession()->GetSecureSessionType() != Transport::SecureSession::Type::kCASE)
    {
        ChipLogError(Zcl, "This command MUST be over a valid CASE session");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedAccess);
        return false;
    }
    return true;
}

static bool CheckFailSafeArmed(CommandHandlerInterface::HandlerContext & ctx)
{
    auto & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();
    if (failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()))
    {
        return true;
    }
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::FailsafeRequired);
    return false;
}

void ServerInstance::HandleGetActiveDatasetRequest(HandlerContext & ctx,
                                                   const Commands::GetActiveDatasetRequest::DecodableType & req)
{
    VerifyOrReturn(CheckOverCASESession(ctx));

    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_UNINITIALIZED));
    }

    bool interfaceEnabled;
    ReturnOnFailure(AddStatusOnError(ctx, mDelegate->GetInterfaceEnabled(interfaceEnabled)));
    if (!interfaceEnabled)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        return;
    }
    Commands::DatasetResponse::Type response;
    Thread::OperationalDataset activeDataset;
    ReturnOnFailure(AddStatusOnError(ctx, mDelegate->GetActiveDataset(activeDataset)));
    response.dataset = activeDataset.AsByteSpan();
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ServerInstance::HandleGetPendingDatasetRequest(HandlerContext & ctx,
                                                    const Commands::GetPendingDatasetRequest::DecodableType & req)
{
    Commands::DatasetResponse::Type response;
    VerifyOrReturn(CheckOverCASESession(ctx));

    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_UNINITIALIZED));
    }

    Thread::OperationalDataset pendingDataset;
    ReturnOnFailure(AddStatusOnError(ctx, mDelegate->GetPendingDataset(pendingDataset)));
    response.dataset = pendingDataset.AsByteSpan();
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ServerInstance::HandleSetActiveDatasetRequest(HandlerContext & ctx,
                                                   const Commands::SetActiveDatasetRequest::DecodableType & req)
{
    VerifyOrReturn(CheckOverCASESession(ctx));
    VerifyOrReturn(CheckFailSafeArmed(ctx));

    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_UNINITIALIZED));
    }
    Thread::OperationalDataset activeDataset;
    Thread::OperationalDataset currentActiveDataset;
    uint64_t currentActiveDatasetTimestamp;
    if (activeDataset.Init(req.activeDataset) != CHIP_NO_ERROR)
    {
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_INVALID_ARGUMENT));
    }
    if (mDelegate->GetActiveDataset(currentActiveDataset) == CHIP_NO_ERROR &&
        currentActiveDataset.GetActiveTimestamp(currentActiveDatasetTimestamp) == CHIP_NO_ERROR)
    {
        // When the ActiveDatasetTimestamp attribute is not null, the command SHALL fail with a status
        // code of INVALID_IN_STATE
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_INCORRECT_STATE));
    }
    if (mDelegate->SetActiveDataset(activeDataset) != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
    }
    else
    {
        GeneralCommissioning::SetBreadcrumb(req.breadcrumb);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
    }
}

void ServerInstance::HandleSetPendingDatasetRequest(HandlerContext & ctx,
                                                    const Commands::SetPendingDatasetRequest::DecodableType & req)
{
    VerifyOrReturn(CheckOverCASESession(ctx));

    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_UNINITIALIZED));
    }
    bool panChangeSupported;
    if (mDelegate->GetPanChangeSupported(panChangeSupported) != CHIP_NO_ERROR || !panChangeSupported)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
        return;
    }
    Thread::OperationalDataset pendingDataset;
    if (pendingDataset.Init(req.pendingDataset) != CHIP_NO_ERROR)
    {
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_INVALID_ARGUMENT));
        return;
    }
    if (mDelegate->SetPendingDataset(pendingDataset) != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
    }
}

void ServerInstance::OnTopologyRequestFinished(Protocols::InteractionModel::Status status, uint8_t snapshot,
                                               const Span<Delegate::ThreadNode> & threadNodes)
{
    Commands::TopologyResponse::Type response;
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (status != Protocols::InteractionModel::Status::Success)
    {
        response.status = to_underlying(status);
        commandHandle->AddResponse(mCommandPath, response);
        return;
    }
    if (threadNodes.size() < mTopologyReq.startIndex)
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        commandHandle->AddResponse(mCommandPath, response);
        return;
    }
    uint16_t threadNodesCount = threadNodes.size() - mTopologyReq.startIndex;
    threadNodesCount          = threadNodesCount > mTopologyReq.count ? mTopologyReq.count : threadNodesCount;
    response.snapshot         = snapshot;
    response.numberOfDevices  = threadNodes.size();
    response.status           = to_underlying(Protocols::InteractionModel::Status::Success);
    if (threadNodesCount == 0)
    {
        // return a response with a NULL ThreadTopology
        commandHandle->AddResponse(mCommandPath, response);
        return;
    }
    Platform::ScopedMemoryBuffer<Structs::ThreadNodeStruct::Type> threadNodesStuctsBuffer;
    threadNodesStuctsBuffer.Alloc(threadNodesCount);
    if (!threadNodesStuctsBuffer.Get())
    {
        response.status = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
        commandHandle->AddResponse(mCommandPath, response);
        return;
    }
    for (size_t index = 0; index < threadNodesCount; ++index)
    {
        threadNodesStuctsBuffer[index] = threadNodes[mTopologyReq.startIndex + index];
    }
    response.threadTopology =
        DataModel::List<const Structs::ThreadNodeStruct::Type>(threadNodesStuctsBuffer.Get(), threadNodesCount);
    commandHandle->AddResponse(mCommandPath, response);
}

void ServerInstance::HandleTopologyRequest(HandlerContext & ctx, const Commands::TopologyRequest::DecodableType & req)
{
    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ReturnOnFailure(AddStatusOnError(ctx, CHIP_ERROR_UNINITIALIZED));
    }
    mAsyncCommandHandle = CommandHandler::Handle(&ctx.mCommandHandler);
    mTopologyReq        = req;
    ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
    CHIP_ERROR err = mDelegate->GetTopology(req.snapshot, this);
    if (err != CHIP_NO_ERROR)
    {
        OnTopologyRequestFinished(MapChipErrorToIMStatus(err), 0, Span<Delegate::ThreadNode>());
    }
}

void ServerInstance::InvokeCommand(HandlerContext & ctx)
{
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctx.SetCommandHandled();
        return;
    }
    mCommandPath = ctx.mRequestPath;
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::GetActiveDatasetRequest::Id:
        HandleCommand<Commands::GetActiveDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleGetActiveDatasetRequest(ctx, req); });
        return;
    case Commands::GetPendingDatasetRequest::Id:
        HandleCommand<Commands::GetPendingDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleGetPendingDatasetRequest(ctx, req); });
        return;
    case Commands::SetActiveDatasetRequest::Id:
        HandleCommand<Commands::SetActiveDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleSetActiveDatasetRequest(ctx, req); });
        return;
    case Commands::SetPendingDatasetRequest::Id:
        HandleCommand<Commands::SetPendingDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleSetPendingDatasetRequest(ctx, req); });
        return;
    case Commands::TopologyRequest::Id:
        HandleCommand<Commands::TopologyRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleTopologyRequest(ctx, req); });
        return;
    }
}

CHIP_ERROR ServerInstance::ReadFeatureMap(AttributeValueEncoder & aEncoder)
{
    BitFlags<Feature> featureMap;
    bool panChangeSupported;
    ReturnErrorOnFailure(mDelegate->GetPanChangeSupported(panChangeSupported));
    if (panChangeSupported)
    {
        featureMap.Set(Feature::kPANChange);
    }
    return aEncoder.Encode(featureMap);
}

CHIP_ERROR ServerInstance::ReadBorderRouterName(AttributeValueEncoder & aEncoder)
{
    char borderRouterName[kBorderRouterNameMaxLength];
    MutableCharSpan borderRouterNameSpan(borderRouterName);
    ReturnErrorOnFailure(mDelegate->GetBorderRouterName(borderRouterNameSpan));
    return aEncoder.Encode(chip::CharSpan(borderRouterName, borderRouterNameSpan.size()));
}

CHIP_ERROR ServerInstance::ReadBorderAgentID(AttributeValueEncoder & aEncoder)
{
    uint8_t borderAgentId[kBorderAgentIdLength];
    MutableByteSpan borderAgentIdSpan(borderAgentId);
    ReturnErrorOnFailure(mDelegate->GetBorderAgentId(borderAgentIdSpan));
    return aEncoder.Encode(chip::ByteSpan(borderAgentId));
}

CHIP_ERROR ServerInstance::ReadThreadVersion(AttributeValueEncoder & aEncoder)
{
    uint16_t threadVersion;
    ReturnErrorOnFailure(mDelegate->GetThreadVersion(threadVersion));
    return aEncoder.Encode(threadVersion);
}

CHIP_ERROR ServerInstance::ReadInterfaceEnabled(AttributeValueEncoder & aEncoder)
{
    bool interfaceEnabled;
    ReturnErrorOnFailure(mDelegate->GetInterfaceEnabled(interfaceEnabled));
    return aEncoder.Encode(interfaceEnabled);
}

CHIP_ERROR ServerInstance::ReadThreadNode(AttributeValueEncoder & aEncoder)
{
    Delegate::ThreadNode threadNode;
    ReturnErrorOnFailure(mDelegate->GetThreadNode(threadNode));
    return aEncoder.Encode(threadNode);
}

CHIP_ERROR ServerInstance::ReadActiveDatasetTimestamp(AttributeValueEncoder & aEncoder)
{
    Thread::OperationalDataset activeDataset;
    uint64_t activeDatasetTimestamp;
    if (mDelegate->GetActiveDataset(activeDataset) == CHIP_NO_ERROR &&
        activeDataset.GetActiveTimestamp(activeDatasetTimestamp) == CHIP_NO_ERROR)
    {
        return aEncoder.Encode(DataModel::MakeNullable(activeDatasetTimestamp));
    }
    return aEncoder.Encode(DataModel::Nullable<uint64_t>());
}

CHIP_ERROR ServerInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != ThreadBorderRouterManagement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!mDelegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }
    CHIP_ERROR status = CHIP_NO_ERROR;
    switch (aPath.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id:
        status = ReadFeatureMap(aEncoder);
        break;
    case Attributes::BorderRouterName::Id:
        status = ReadBorderRouterName(aEncoder);
        break;
    case Attributes::BorderAgentId::Id:
        status = ReadBorderAgentID(aEncoder);
        break;
    case Attributes::ThreadVersion::Id:
        status = ReadThreadVersion(aEncoder);
        break;
    case Attributes::InterfaceEnabled::Id:
        status = ReadInterfaceEnabled(aEncoder);
        break;
    case Attributes::ThreadNode::Id:
        status = ReadThreadNode(aEncoder);
        break;
    case Attributes::ActiveDatasetTimestamp::Id:
        status = ReadActiveDatasetTimestamp(aEncoder);
        break;
    default:
        break;
    }
    return status;
}

CHIP_ERROR ServerInstance::Init()
{
    ReturnErrorCodeIf(!mDelegate, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->Init();
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThreadBorderRouterManagementPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in Instance::Init()
}
