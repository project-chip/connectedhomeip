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
#include "crypto/RandUtils.h"
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

static bool IsOverCASESession(CommandHandlerInterface::HandlerContext & ctx)
{
    Messaging::ExchangeContext * exchangeCtx = ctx.mCommandHandler.GetExchangeContext();
    return exchangeCtx && exchangeCtx->HasSessionHandle() && exchangeCtx->GetSessionHandle()->IsSecureSession() &&
        exchangeCtx->GetSessionHandle()->AsSecureSession()->GetSecureSessionType() == Transport::SecureSession::Type::kCASE;
}

static bool IsFailSafeArmed(CommandHandlerInterface::HandlerContext & ctx)
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    return failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex());
}

Status ServerInstance::HandleGetDatasetRequest(bool isOverCASESession, Delegate::DatasetType type,
                                               Thread::OperationalDataset & dataset)
{
    if (!isOverCASESession)
    {
        return Status::UnsupportedAccess;
    }

    CHIP_ERROR err = mDelegate->GetDataset(dataset, type);
    if (err != CHIP_NO_ERROR)
    {
        return err == CHIP_IM_GLOBAL_STATUS(NotFound) ? StatusIB(err).mStatus : Status::Failure;
    }
    return Status::Success;
}

Status ServerInstance::HandleSetActiveDatasetRequest(bool failSafeArmed,
                                                     const Commands::SetActiveDatasetRequest::DecodableType & req)
{
    // The SetActiveDatasetRequest command SHALL be FailSafeArmed. Upon receiving this command, the Thread BR will set its
    // active dataset. If the dataset is set successfully, OnActivateDatasetComplete will be called with CHIP_NO_ERROR, prompting
    // the Thread BR to respond with a success status and disarm the FailSafe timer. If an error occurs while setting the active
    // dataset, the Thread BR should respond with a failure status. In this case, when the FailSafe timer expires, the active
    // dataset set by this command will be reverted. If the FailSafe timer expires before the Thread BR responds, the Thread BR will
    // respond with a timeout status and the active dataset should also be reverted.
    ReturnErrorCodeIf(!failSafeArmed, Status::FailsafeRequired);
    ReturnErrorCodeIf(!mDelegate, Status::InvalidInState);

    Thread::OperationalDataset activeDataset;
    Thread::OperationalDataset currentActiveDataset;
    uint64_t currentActiveDatasetTimestamp;
    // If any of the parameters in the ActiveDataset is invalid, the command SHALL fail with a status code
    // of INVALID_COMMAND.
    ReturnErrorCodeIf(activeDataset.Init(req.activeDataset) != CHIP_NO_ERROR, Status::InvalidCommand);

    // If this command is invoked when the ActiveDatasetTimestamp attribute is not null, the command SHALL
    // fail with a status code of INVALID_IN_STATE.
    if (mDelegate->GetDataset(currentActiveDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR &&
        currentActiveDataset.GetActiveTimestamp(currentActiveDatasetTimestamp) == CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    mBreadcrumb    = req.breadcrumb;
    mRandomNumber  = Crypto::GetRandU32();
    CHIP_ERROR err = mDelegate->SetActiveDataset(activeDataset, mRandomNumber, this);
    if (err != CHIP_NO_ERROR)
    {
        mRandomNumber = 0;
    }
    return StatusIB(err).mStatus;
}

Status ServerInstance::HandleSetPendingDatasetRequest(const Commands::SetPendingDatasetRequest::DecodableType & req)
{
    ReturnErrorCodeIf(!mDelegate, Status::InvalidInState);
    bool panChangeSupported;
    if (mDelegate->GetPanChangeSupported(panChangeSupported) != CHIP_NO_ERROR || !panChangeSupported)
    {
        return Status::UnsupportedCommand;
    }
    Thread::OperationalDataset pendingDataset;
    // If any of the parameters in the PendingDataset is invalid, the command SHALL fail with a status code
    // of INVALID_COMMAND.
    ReturnErrorCodeIf(pendingDataset.Init(req.pendingDataset) != CHIP_NO_ERROR, Status::InvalidCommand);
    CHIP_ERROR err = mDelegate->SetPendingDataset(pendingDataset);
    return StatusIB(err).mStatus;
}

void AddDatasetResponse(CommandHandlerInterface::HandlerContext & ctx, Status status, Thread::OperationalDataset & dataset)
{
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }
    Commands::DatasetResponse::Type response;
    response.dataset = dataset.AsByteSpan();
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ServerInstance::InvokeCommand(HandlerContext & ctxt)
{
    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::GetActiveDatasetRequest::Id:
        HandleCommand<Commands::GetActiveDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            Thread::OperationalDataset dataset;
            Status status = HandleGetActiveDatasetRequest(IsOverCASESession(ctx), dataset);
            AddDatasetResponse(ctx, status, dataset);
        });
        break;
    case Commands::GetPendingDatasetRequest::Id:
        HandleCommand<Commands::GetPendingDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            Thread::OperationalDataset dataset;
            Status status = HandleGetPendingDatasetRequest(IsOverCASESession(ctx), dataset);
            AddDatasetResponse(ctx, status, dataset);
        });
        break;
    case Commands::SetActiveDatasetRequest::Id:
        HandleCommand<Commands::SetActiveDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            if (!mAsyncCommandHandle.Get())
            {
                mPath               = ctx.mRequestPath;
                mAsyncCommandHandle = CommandHandler::Handle(&ctx.mCommandHandler);
                ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
                Status status = HandleSetActiveDatasetRequest(IsFailSafeArmed(ctx), req);
                if (status != Status::Success)
                {
                    OnActivateDatasetComplete(mRandomNumber, ChipError(ChipError::SdkPart::kIMGlobalStatus, to_underlying(status)));
                }
            }
            else
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
            }
        });
        break;
    case Commands::SetPendingDatasetRequest::Id:
        HandleCommand<Commands::SetPendingDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, HandleSetPendingDatasetRequest(req));
        });
        break;
    default:
        break;
    }
}

CHIP_ERROR ServerInstance::ReadFeatureMap(BitFlags<Feature> & featureMap)
{
    bool panChangeSupported;
    ReturnErrorOnFailure(mDelegate->GetPanChangeSupported(panChangeSupported));
    if (panChangeSupported)
    {
        featureMap.Set(Feature::kPANChange);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerInstance::ReadBorderRouterName(MutableCharSpan & borderRouterName)
{
    ReturnErrorOnFailure(mDelegate->GetBorderRouterName(borderRouterName));
    ReturnErrorCodeIf(borderRouterName.size() > kBorderRouterNameMaxLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerInstance::ReadBorderAgentID(MutableByteSpan & borderAgentId)
{
    ReturnErrorOnFailure(mDelegate->GetBorderAgentId(borderAgentId));
    ReturnErrorCodeIf(borderAgentId.size() != kBorderAgentIdLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerInstance::ReadThreadVersion(uint16_t & threadVersion)
{
    return mDelegate->GetThreadVersion(threadVersion);
}

CHIP_ERROR ServerInstance::ReadInterfaceEnabled(bool & interfaceEnabled)
{
    return mDelegate->GetInterfaceEnabled(interfaceEnabled);
}

CHIP_ERROR ServerInstance::ReadActiveDatasetTimestamp(Optional<uint64_t> & activeDatasetTimestamp)
{
    uint64_t activeDatasetTimestampValue;
    Thread::OperationalDataset activeDataset;
    if (mDelegate->GetDataset(activeDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR &&
        activeDataset.GetActiveTimestamp(activeDatasetTimestampValue) == CHIP_NO_ERROR)
    {
        activeDatasetTimestamp.SetValue(activeDatasetTimestampValue);
    }
    else
    {
        activeDatasetTimestamp.ClearValue();
    }
    return CHIP_NO_ERROR;
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
    case Globals::Attributes::FeatureMap::Id: {
        BitFlags<Feature> featureMap;
        status = ReadFeatureMap(featureMap);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(featureMap);
        }
        break;
    }
    case Attributes::BorderRouterName::Id: {
        char borderRouterNameBuf[kBorderRouterNameMaxLength + 1];
        MutableCharSpan borderRouterName(borderRouterNameBuf);
        status = ReadBorderRouterName(borderRouterName);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(borderRouterName);
        }
        break;
    }
    case Attributes::BorderAgentID::Id: {
        uint8_t borderAgentIDBuf[kBorderAgentIdLength];
        MutableByteSpan borderAgentID(borderAgentIDBuf);
        status = ReadBorderAgentID(borderAgentID);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(borderAgentID);
        }
        break;
    }
    case Attributes::ThreadVersion::Id: {
        uint16_t threadVersion;
        status = ReadThreadVersion(threadVersion);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(threadVersion);
        }
        break;
    }
    case Attributes::InterfaceEnabled::Id: {
        bool interfaceEnabled;
        status = ReadInterfaceEnabled(interfaceEnabled);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(interfaceEnabled);
        }
        break;
    }
    case Attributes::ActiveDatasetTimestamp::Id: {
        Optional<uint64_t> activeDatasetTimestamp;
        status = ReadActiveDatasetTimestamp(activeDatasetTimestamp);
        if (status == CHIP_NO_ERROR)
        {
            status = activeDatasetTimestamp.HasValue() ? aEncoder.Encode(DataModel::MakeNullable(activeDatasetTimestamp.Value()))
                                                       : aEncoder.EncodeNull();
        }
        break;
    }
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

void ServerInstance::OnActivateDatasetComplete(uint32_t randomNumber, CHIP_ERROR error)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }
    if (mRandomNumber != randomNumber)
    {
        // Previous SetActiveDatasetRequest was handled.
        return;
    }
    mRandomNumber = 0;
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
    // Reset the RandomNumeber so that the OnActivateDatasetComplete will not handle the previous SetActiveDatasetRequest command.
    mRandomNumber = 0;
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
