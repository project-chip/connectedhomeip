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

#include "thread-border-router-management-server.h"

#include "app-common/zap-generated/cluster-objects.h"
#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "app/AttributeAccessInterfaceRegistry.h"
#include "app/AttributeValueEncoder.h"
#include "app/CommandHandler.h"
#include "app/CommandHandlerInterface.h"
#include "app/CommandHandlerInterfaceRegistry.h"
#include "app/InteractionModelEngine.h"
#include "app/MessageDef/StatusIB.h"
#include "app/clusters/general-commissioning-server/general-commissioning-server.h"
#include "app/data-model/Nullable.h"
#include "lib/core/CHIPError.h"
#include "lib/core/Optional.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/Span.h"
#include "lib/support/ThreadOperationalDataset.h"
#include "platform/CHIPDeviceEvent.h"
#include "platform/PlatformManager.h"
#include "protocols/interaction_model/StatusCode.h"
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

using Protocols::InteractionModel::Status;

bool ServerInstance::IsCommandOverCASESession(CommandHandlerInterface::HandlerContext & ctx)
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (mSkipCASESessionCheck)
    {
        return true;
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
    Messaging::ExchangeContext * exchangeCtx = ctx.mCommandHandler.GetExchangeContext();
    return exchangeCtx && exchangeCtx->HasSessionHandle() && exchangeCtx->GetSessionHandle()->IsSecureSession() &&
        exchangeCtx->GetSessionHandle()->AsSecureSession()->GetSecureSessionType() == Transport::SecureSession::Type::kCASE;
}

Status ServerInstance::HandleGetDatasetRequest(CommandHandlerInterface::HandlerContext & ctx, Delegate::DatasetType type,
                                               Thread::OperationalDataset & dataset)
{
    VerifyOrDie(mDelegate);
    VerifyOrReturnValue(IsCommandOverCASESession(ctx), Status::UnsupportedAccess);

    CHIP_ERROR err = mDelegate->GetDataset(dataset, type);
    if (err != CHIP_NO_ERROR)
    {
        return err == CHIP_IM_GLOBAL_STATUS(NotFound) ? StatusIB(err).mStatus : Status::Failure;
    }
    return Status::Success;
}

Status ServerInstance::HandleSetActiveDatasetRequest(CommandHandlerInterface::HandlerContext & ctx,
                                                     const Commands::SetActiveDatasetRequest::DecodableType & req)
{
    // The SetActiveDatasetRequest command SHALL be FailSafeArmed. Upon receiving this command, the Thread BR will set its
    // active dataset. If the dataset is set successfully, OnActivateDatasetComplete will be called with CHIP_NO_ERROR, prompting
    // the Thread BR to respond with a success status. If an error occurs while setting the active dataset, the Thread BR should
    // respond with a failure status. In this case, when the FailSafe timer expires, the active dataset set by this command will be
    // reverted. If the FailSafe timer expires before the Thread BR responds, the Thread BR will respond with a timeout status and
    // the active dataset should also be reverted.
    VerifyOrDie(mDelegate);
    VerifyOrReturnValue(IsCommandOverCASESession(ctx), Status::UnsupportedAccess);
    VerifyOrReturnValue(mFailsafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()), Status::FailsafeRequired);

    Thread::OperationalDataset activeDataset;
    Thread::OperationalDataset currentActiveDataset;
    uint64_t currentActiveDatasetTimestamp = 0;
    // If any of the parameters in the ActiveDataset is invalid, the command SHALL fail with a status code
    // of INVALID_COMMAND.
    VerifyOrReturnValue(activeDataset.Init(req.activeDataset) == CHIP_NO_ERROR, Status::InvalidCommand);

    // If this command is invoked when the ActiveDatasetTimestamp attribute is not null, the command SHALL
    // fail with a status code of INVALID_IN_STATE.
    if ((mDelegate->GetDataset(currentActiveDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR) &&
        (currentActiveDataset.GetActiveTimestamp(currentActiveDatasetTimestamp) == CHIP_NO_ERROR))
    {
        return Status::InvalidInState;
    }
    // If there is a back end command process, return status BUSY.
    if (mAsyncCommandHandle.Get())
    {
        return Status::Busy;
    }
    ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
    mAsyncCommandHandle = CommandHandler::Handle(&ctx.mCommandHandler);
    mBreadcrumb         = req.breadcrumb;
    mSetActiveDatasetSequenceNumber++;
    mDelegate->SetActiveDataset(activeDataset, mSetActiveDatasetSequenceNumber, this);
    return Status::Success;
}

Status ServerInstance::HandleSetPendingDatasetRequest(CommandHandlerInterface::HandlerContext & ctx,
                                                      const Commands::SetPendingDatasetRequest::DecodableType & req)
{
    VerifyOrDie(mDelegate);
    VerifyOrReturnValue(IsCommandOverCASESession(ctx), Status::UnsupportedAccess);
    if (!mDelegate->GetPanChangeSupported())
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

void AddDatasetResponse(CommandHandlerInterface::HandlerContext & ctx, Status status, const Thread::OperationalDataset & dataset)
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
            Status status = HandleGetActiveDatasetRequest(ctx, dataset);
            AddDatasetResponse(ctx, status, dataset);
        });
        break;
    case Commands::GetPendingDatasetRequest::Id:
        HandleCommand<Commands::GetPendingDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            Thread::OperationalDataset dataset;
            Status status = HandleGetPendingDatasetRequest(ctx, dataset);
            AddDatasetResponse(ctx, status, dataset);
        });
        break;
    case Commands::SetActiveDatasetRequest::Id:
        HandleCommand<Commands::SetActiveDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            mPath         = ctx.mRequestPath;
            Status status = HandleSetActiveDatasetRequest(ctx, req);
            if (status != Status::Success)
            {
                // If status is not Success, we should immediately report the status. Otherwise the async work will report the
                // status to the client.
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            }
        });
        break;
    case Commands::SetPendingDatasetRequest::Id:
        HandleCommand<Commands::SetPendingDatasetRequest::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            Status status = HandleSetPendingDatasetRequest(ctx, req);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        });
        break;
    default:
        break;
    }
}

void ServerInstance::ReadFeatureMap(BitFlags<Feature> & outFeatureMap)
{
    if (mDelegate->GetPanChangeSupported())
    {
        outFeatureMap.Set(Feature::kPANChange);
    }
}

CHIP_ERROR ServerInstance::ReadBorderRouterName(MutableCharSpan & outBorderRouterName)
{
    mDelegate->GetBorderRouterName(outBorderRouterName);
    VerifyOrReturnValue(outBorderRouterName.size() <= kBorderRouterNameMaxLength, CHIP_IM_GLOBAL_STATUS(Failure));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerInstance::ReadBorderAgentID(MutableByteSpan & outBorderAgentId)
{
    VerifyOrReturnValue((mDelegate->GetBorderAgentId(outBorderAgentId) == CHIP_NO_ERROR) &&
                            (outBorderAgentId.size() == kBorderAgentIdLength),
                        CHIP_IM_GLOBAL_STATUS(Failure));
    return CHIP_NO_ERROR;
}

std::optional<uint64_t> ServerInstance::ReadActiveDatasetTimestamp()
{
    uint64_t activeDatasetTimestampValue = 0;
    Thread::OperationalDataset activeDataset;
    if ((mDelegate->GetDataset(activeDataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR) &&
        (activeDataset.GetActiveTimestamp(activeDatasetTimestampValue) == CHIP_NO_ERROR))
    {
        return std::make_optional(activeDatasetTimestampValue);
    }
    return std::nullopt;
}

std::optional<uint64_t> ServerInstance::ReadPendingDatasetTimestamp()
{
    uint64_t pendingDatasetTimestampValue = 0;
    Thread::OperationalDataset pendingDataset;
    if ((mDelegate->GetDataset(pendingDataset, Delegate::DatasetType::kPending) == CHIP_NO_ERROR) &&
        (pendingDataset.GetActiveTimestamp(pendingDatasetTimestampValue) == CHIP_NO_ERROR))
    {
        return std::make_optional(pendingDatasetTimestampValue);
    }
    return std::nullopt;
}

CHIP_ERROR ServerInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != ThreadBorderRouterManagement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrDie(mDelegate);
    CHIP_ERROR status = CHIP_NO_ERROR;
    switch (aPath.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id: {
        BitFlags<Feature> featureMap;
        ReadFeatureMap(featureMap);
        status = aEncoder.Encode(featureMap);
        break;
    }
    case Attributes::BorderRouterName::Id: {
        char borderRouterNameBuf[kBorderRouterNameMaxLength] = { 0 };
        MutableCharSpan borderRouterName(borderRouterNameBuf);
        status = ReadBorderRouterName(borderRouterName);
        // If there are any internal errors, the status will be returned and the client will get an error report.
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(borderRouterName);
        }
        break;
    }
    case Attributes::BorderAgentID::Id: {
        uint8_t borderAgentIDBuf[kBorderAgentIdLength] = { 0 };
        MutableByteSpan borderAgentID(borderAgentIDBuf);
        status = ReadBorderAgentID(borderAgentID);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(borderAgentID);
        }
        break;
    }
    case Attributes::ThreadVersion::Id: {
        uint16_t threadVersion = mDelegate->GetThreadVersion();
        status                 = aEncoder.Encode(threadVersion);
        break;
    }
    case Attributes::InterfaceEnabled::Id: {
        bool interfaceEnabled = mDelegate->GetInterfaceEnabled();
        status                = aEncoder.Encode(interfaceEnabled);
        break;
    }
    case Attributes::ActiveDatasetTimestamp::Id: {
        std::optional<uint64_t> activeDatasetTimestamp = ReadActiveDatasetTimestamp();
        status = activeDatasetTimestamp.has_value() ? aEncoder.Encode(activeDatasetTimestamp.value()) : aEncoder.EncodeNull();
        break;
    }
    case Attributes::PendingDatasetTimestamp::Id: {
        std::optional<uint64_t> pendingDatasetTimestamp = ReadPendingDatasetTimestamp();
        status = pendingDatasetTimestamp.has_value() ? aEncoder.Encode(pendingDatasetTimestamp.value()) : aEncoder.EncodeNull();
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

void ServerInstance::OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }
    if (mSetActiveDatasetSequenceNumber != sequenceNum)
    {
        // Previous SetActiveDatasetRequest was handled.
        return;
    }
    if (error == CHIP_NO_ERROR)
    {
        // TODO: SPEC Issue #10022
        CommitSavedBreadcrumb();
    }
    else
    {
        ChipLogError(Zcl, "Failed on activating the active dataset for Thread BR: %" CHIP_ERROR_FORMAT, error.Format());
    }
    commandHandle->AddStatus(mPath, StatusIB(error).mStatus);
}

void ServerInstance::ReportAttributeChanged(AttributeId attributeId)
{
    MatterReportingAttributeChangeCallback(mServerEndpointId, Id, attributeId);
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
    else if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        _this->mDelegate->CommitActiveDataset();
    }
}

CHIP_ERROR ServerInstance::Init()
{
    ReturnErrorCodeIf(!mDelegate, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(chip::app::AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    return mDelegate->Init(this);
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThreadBorderRouterManagementPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in Instance::Init()
}
