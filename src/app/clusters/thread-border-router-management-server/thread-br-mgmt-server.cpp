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
#include "app/MessageDef/StatusIB.h"
#include "app/clusters/general-commissioning-server/general-commissioning-server.h"
#include "app/data-model/Nullable.h"
#include "app/server/Server.h"
#include "lib/core/CHIPError.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/Span.h"
#include "lib/support/ThreadOperationalDataset.h"
#include "platform/CHIPDeviceEvent.h"
#include "platform/PlatformManager.h"
#include "protocols/interaction_model/StatusCode.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

using Protocols::InteractionModel::Status;

static bool CheckOverCASESession(CommandHandlerInterface::HandlerContext & ctx)
{
    Messaging::ExchangeContext * exchangeCtx = ctx.mCommandHandler.GetExchangeContext();
    if (!exchangeCtx || !exchangeCtx->HasSessionHandle() || !exchangeCtx->GetSessionHandle()->IsSecureSession() ||
        exchangeCtx->GetSessionHandle()->AsSecureSession()->GetSecureSessionType() != Transport::SecureSession::Type::kCASE)
    {
        ChipLogError(Zcl, "This command MUST be over a valid CASE session");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedAccess);
        return false;
    }
    return true;
}

static bool CheckFailSafeArmed(CommandHandlerInterface::HandlerContext & ctx)
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    if (failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()))
    {
        return true;
    }
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::FailsafeRequired);
    return false;
}

static bool CheckDelegate(CommandHandlerInterface::HandlerContext & ctx, Delegate * delegate)
{
    if (!delegate)
    {
        ChipLogError(Zcl, "Thread Border Router Management server not initialized");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
    }
    return delegate;
}

void ServerInstance::HandleGetDatasetRequest(HandlerContext & ctx, Delegate::DatasetType type)
{
    VerifyOrReturn(CheckOverCASESession(ctx));
    VerifyOrReturn(CheckDelegate(ctx, mDelegate));

    Commands::DatasetResponse::Type response;
    Thread::OperationalDataset dataset;
    CHIP_ERROR err = mDelegate->GetDataset(dataset, type);
    if (err != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath,
                                      err == CHIP_IM_GLOBAL_STATUS(NotFound) ? StatusIB(err).mStatus : Status::Failure);
        return;
    }
    response.dataset = dataset.AsByteSpan();
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ServerInstance::HandleSetActiveDatasetRequest(HandlerContext & ctx,
                                                   const Commands::SetActiveDatasetRequest::DecodableType & req)
{
    // The SetActiveDatasetRequest command SHALL be FailSafeArmed. Upon receiving this command, the Thread BR will set its
    // active dataset. If the dataset is set successfully, OnActivateDatasetComplete will be called with CHIP_NO_ERROR, prompting
    // the Thread BR to respond with a success status and disarm the FailSafe timer. If an error occurs while setting the active
    // dataset, the Thread BR should respond with a failure status. In this case, when the FailSafe timer expires, the active
    // dataset set by this command will be reverted. If the FailSafe timer expires before the Thread BR responds, the Thread BR will
    // respond with a timeout status and the active dataset should also be reverted.
    VerifyOrReturn(CheckFailSafeArmed(ctx));
    VerifyOrReturn(CheckDelegate(ctx, mDelegate));
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    Thread::OperationalDataset activeDataset;
    Thread::OperationalDataset currentActiveDataset;
    uint64_t currentActiveDatasetTimestamp;
    if (activeDataset.Init(req.activeDataset) != CHIP_NO_ERROR)
    {
        // If any of the parameters in the ActiveDataset is invalid, the command SHALL fail with a status code
        // of INVALID_COMMAND.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    if (mDelegate->GetDataset(currentActiveDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR &&
        currentActiveDataset.GetActiveTimestamp(currentActiveDatasetTimestamp) == CHIP_NO_ERROR)
    {
        // If this command is invoked when the ActiveDatasetTimestamp attribute is not null, the command SHALL
        // fail with a status code of INVALID_IN_STATE.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
        return;
    }

    mPath               = ctx.mRequestPath;
    mAsyncCommandHandle = CommandHandler::Handle(&ctx.mCommandHandler);
    ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
    mBreadcrumb    = req.breadcrumb;
    CHIP_ERROR err = CHIP_NO_ERROR;
    if ((err = mDelegate->SetActiveDataset(activeDataset, this)) != CHIP_NO_ERROR)
    {
        OnActivateDatasetComplete(err);
    }
}

void ServerInstance::HandleSetPendingDatasetRequest(HandlerContext & ctx,
                                                    const Commands::SetPendingDatasetRequest::DecodableType & req)
{
    VerifyOrReturn(CheckDelegate(ctx, mDelegate));
    bool panChangeSupported;
    if (mDelegate->GetPanChangeSupported(panChangeSupported) != CHIP_NO_ERROR || !panChangeSupported)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        return;
    }
    Thread::OperationalDataset pendingDataset;
    if (pendingDataset.Init(req.pendingDataset) != CHIP_NO_ERROR)
    {
        // If any of the parameters in the PendingDataset is invalid, the command SHALL fail with a status code
        // of INVALID_COMMAND.
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }
    CHIP_ERROR err = mDelegate->SetPendingDataset(pendingDataset);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, StatusIB(err).mStatus);
}

void ServerInstance::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::GetActiveDatasetRequest::Id:
        HandleCommand<Commands::GetActiveDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleGetActiveDatasetRequest(ctx, req); });
        break;
    case Commands::GetPendingDatasetRequest::Id:
        HandleCommand<Commands::GetPendingDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleGetPendingDatasetRequest(ctx, req); });
        break;
    case Commands::SetActiveDatasetRequest::Id:
        HandleCommand<Commands::SetActiveDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleSetActiveDatasetRequest(ctx, req); });
        break;
    case Commands::SetPendingDatasetRequest::Id:
        HandleCommand<Commands::SetPendingDatasetRequest::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandleSetPendingDatasetRequest(ctx, req); });
        break;
    default:
        break;
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
    return aEncoder.Encode(borderRouterNameSpan);
}

CHIP_ERROR ServerInstance::ReadBorderAgentID(AttributeValueEncoder & aEncoder)
{
    uint8_t borderAgentId[kBorderAgentIdLength];
    MutableByteSpan borderAgentIdSpan(borderAgentId);
    ReturnErrorOnFailure(mDelegate->GetBorderAgentId(borderAgentIdSpan));
    return aEncoder.Encode(borderAgentIdSpan);
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

CHIP_ERROR ServerInstance::ReadActiveDatasetTimestamp(AttributeValueEncoder & aEncoder)
{
    Thread::OperationalDataset activeDataset;
    uint64_t activeDatasetTimestamp;
    if (mDelegate->GetDataset(activeDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR &&
        activeDataset.GetActiveTimestamp(activeDatasetTimestamp) == CHIP_NO_ERROR)
    {
        return aEncoder.Encode(DataModel::MakeNullable(activeDatasetTimestamp));
    }
    return aEncoder.EncodeNull();
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
    case Attributes::BorderAgentID::Id:
        status = ReadBorderAgentID(aEncoder);
        break;
    case Attributes::ThreadVersion::Id:
        status = ReadThreadVersion(aEncoder);
        break;
    case Attributes::InterfaceEnabled::Id:
        status = ReadInterfaceEnabled(aEncoder);
        break;
    case Attributes::ActiveDatasetTimestamp::Id:
        status = ReadActiveDatasetTimestamp(aEncoder);
        break;
    default:
        break;
    }
    return status;
}

void ServerInstance::CommitSavedBreadcrumb()
{
    if (mBreadcrumb.HasValue())
    {
        GeneralCommissioning::SetBreadcrumb(mBreadcrumb.Value());
    }
    mBreadcrumb.ClearValue();
}

void ServerInstance::OnActivateDatasetComplete(CHIP_ERROR error)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }
    if (error == CHIP_NO_ERROR)
    {
        // The successful completion of the activation process SHALL disarm the fail-safe timer.
        auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
        failSafeContext.DisarmFailSafe();
        CommitSavedBreadcrumb();
    }
    else
    {
        ChipLogError(Zcl, "Failed on activating the active dataset for Thread BR: %" CHIP_ERROR_FORMAT, error.Format());
    }
    commandHandle->AddStatus(mPath, StatusIB(error).mStatus);
}

void ServerInstance::OnFailSafeTimerExpired()
{
    if (mDelegate)
    {
        mDelegate->RevertActiveDataset();
    }
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }
    commandHandle->AddStatus(mPath, Status::Timeout);
}

void ServerInstance::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    ServerInstance * _this = reinterpret_cast<ServerInstance *>(arg);
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        _this->OnFailSafeTimerExpired();
    }
}

CHIP_ERROR ServerInstance::Init()
{
    ReturnErrorCodeIf(!mDelegate, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
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
