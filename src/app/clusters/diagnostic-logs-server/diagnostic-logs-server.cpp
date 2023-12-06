/**
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>

#include "diagnostic-logs-server.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/config.h>
#include <lib/support/ScopedBuffer.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;

#ifdef EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT
static constexpr size_t kDiagnosticLogsLogProviderDelegateTableSize =
    EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kDiagnosticLogsLogProviderDelegateTableSize < kEmberInvalidEndpointIndex,
              "DiagnosticLogs: log provider delegate table size error");

namespace {

LogProviderDelegate * gLogProviderDelegateTable[kDiagnosticLogsLogProviderDelegateTableSize] = { nullptr };

LogProviderDelegate * GetLogProviderDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Id, EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kDiagnosticLogsLogProviderDelegateTableSize ? nullptr : gLogProviderDelegateTable[ep]);
}

bool IsLogProviderDelegateNull(LogProviderDelegate * logProviderDelegate, EndpointId endpoint)
{
    if (logProviderDelegate == nullptr)
    {
        ChipLogProgress(Zcl, "Diagnosticlogs: no log provider delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}

} // anonymous namespace

DiagnosticLogsServer DiagnosticLogsServer::sInstance;

void DiagnosticLogsServer::SetLogProviderDelegate(EndpointId endpoint, LogProviderDelegate * logProviderDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, DiagnosticLogs::Id, EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kDiagnosticLogsLogProviderDelegateTableSize)
    {
        gLogProviderDelegateTable[ep] = logProviderDelegate;
    }
}

DiagnosticLogsServer & DiagnosticLogsServer::Instance()
{
    return sInstance;
}

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

bool DiagnosticLogsServer::IsBDXProtocolRequested(TransferProtocolEnum requestedProtocol)
{
    return requestedProtocol == TransferProtocolEnum::kBdx;
}

bool DiagnosticLogsServer::HasValidFileDesignator(CharSpan transferFileDesignator)
{
    return (transferFileDesignator.size() <= kMaxFileDesignatorLen);
}

CHIP_ERROR DiagnosticLogsServer::HandleLogRequestForBDXProtocol(Messaging::ExchangeContext * exchangeCtx, EndpointId endpointId,
                                                                IntentEnum intent, CharSpan fileDesignator)
{

    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mIntent = intent;
    ScopedNodeId scopedPeerNodeId;
    auto sessionHandle = exchangeCtx->GetSessionHandle();

    LogProviderDelegate * logProviderDelegate = GetLogProviderDelegate(endpointId);

    VerifyOrReturnError(!(IsLogProviderDelegateNull(logProviderDelegate, endpointId)), CHIP_ERROR_INCORRECT_STATE);

    // If there is already an existing mDiagnosticLogsBDXTransferHandler, we will return a busy error.
    if (mDiagnosticLogsBDXTransferHandler != nullptr)
    {
        return CHIP_ERROR_BUSY;
    }

    // TODO: Need to resolve #30539. The spec says we should check the log size to see if it fits in the response payload.
    // If it fits, we send the content in the response and not initiate BDX.
    if (sessionHandle->IsSecureSession())
    {
        scopedPeerNodeId = sessionHandle->AsSecureSession()->GetPeer();
    }

    mDiagnosticLogsBDXTransferHandler = new DiagnosticLogsBDXTransferHandler();
    CHIP_ERROR error                  = mDiagnosticLogsBDXTransferHandler->InitializeTransfer(
        exchangeCtx->GetExchangeMgr(), exchangeCtx->GetSessionHandle(), scopedPeerNodeId.GetFabricIndex(),
        scopedPeerNodeId.GetNodeId(), logProviderDelegate, intent, fileDesignator);
    // TODO: Fix #30540 - If we fail to initialize a BDX session, we should call HandleLogRequestForResponsePayload.
    return error;
}

void DiagnosticLogsServer::SendCommandResponse(StatusEnum status)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();

    if (commandHandle == nullptr)
    {
        ChipLogError(Zcl, "SendCommandResponse - commandHandler is null");
        return;
    }

    Commands::RetrieveLogsResponse::Type response;
    response.status = status;
    commandHandle->AddResponse(mRequestPath, response);
}

void DiagnosticLogsServer::SetAsyncCommandHandleAndPath(CommandHandler * commandObj, const ConcreteCommandPath & commandPath)
{
    mAsyncCommandHandle = CommandHandler::Handle(commandObj);
    mRequestPath        = commandPath;
}

#endif

void DiagnosticLogsServer::HandleLogRequestForResponsePayload(CommandHandler * commandHandler, ConcreteCommandPath path,
                                                              IntentEnum intent)
{
    Commands::RetrieveLogsResponse::Type response;
    mIntent = intent;

    EndpointId endpoint                       = path.mEndpointId;
    LogProviderDelegate * logProviderDelegate = GetLogProviderDelegate(endpoint);

    if (IsLogProviderDelegateNull(logProviderDelegate, endpoint))
    {
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    Platform::ScopedMemoryBuffer<uint8_t> buffer;

    if (!buffer.Alloc(kMaxLogContentSize))
    {
        ChipLogError(Zcl, "buffer not allocated");
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    mLogSessionHandle = logProviderDelegate->StartLogCollection(intent);

    if (mLogSessionHandle == kInvalidLogSessionHandle)
    {
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    MutableByteSpan mutableBuffer;

    mutableBuffer = MutableByteSpan(buffer.Get(), kMaxLogContentSize);

    bool isEOF = false;

    // Get the log chunk of size up to kMaxLogContentSize to send in the response payload.
    CHIP_ERROR err = logProviderDelegate->GetNextChunk(mLogSessionHandle, mutableBuffer, isEOF);

    if (err != CHIP_NO_ERROR)
    {
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    response.logContent = ByteSpan(mutableBuffer.data(), mutableBuffer.size());
    response.status     = StatusEnum::kSuccess;
    commandHandler->AddResponse(path, response);

    logProviderDelegate->EndLogCollection(mLogSessionHandle);
    mLogSessionHandle = kInvalidLogSessionHandle;
}

static void HandleRetrieveLogRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                     TransferProtocolEnum protocol, IntentEnum intent, Optional<CharSpan> transferFileDesignator)
{

    if (protocol == TransferProtocolEnum::kResponsePayload)
    {
        DiagnosticLogsServer::Instance().HandleLogRequestForResponsePayload(commandObj, commandPath, intent);
    }
#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
    // TODO: Fix #30540 - If BDX is not supported, we should send whatever fits in the logContent of the ResponsePayload.
    else
    {
        Commands::RetrieveLogsResponse::Type response;
        if (!transferFileDesignator.HasValue() ||
            !DiagnosticLogsServer::Instance().HasValidFileDesignator(transferFileDesignator.Value()))
        {
            ChipLogError(Zcl, "HandleRetrieveLogRequest - fileDesignator not valid for BDX protocol");
            commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        if (DiagnosticLogsServer::Instance().IsBDXProtocolRequested(protocol))
        {
            CHIP_ERROR err = DiagnosticLogsServer::Instance().HandleLogRequestForBDXProtocol(
                commandObj->GetExchangeContext(), commandPath.mEndpointId, intent, transferFileDesignator.Value());
            if (err != CHIP_NO_ERROR)
            {
                LogErrorOnFailure(err);
                // TODO: Fix #30540 - If a BDX session can't be started, we should send whatever fits in the logContent of the
                // ResponsePayload.
                return;
            }
            DiagnosticLogsServer::Instance().SetAsyncCommandHandleAndPath(std::move(commandObj), std::move(commandPath));
        }
    }
#endif
}

bool emberAfDiagnosticLogsClusterRetrieveLogsRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                             const Commands::RetrieveLogsRequest::DecodableType & commandData)
{
    if (commandData.requestedProtocol == TransferProtocolEnum::kUnknownEnumValue ||
        commandData.intent == IntentEnum::kUnknownEnumValue)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return true;
    }
    HandleRetrieveLogRequest(commandObj, commandPath, commandData.requestedProtocol, commandData.intent,
                             commandData.transferFileDesignator);
    return true;
}

void MatterDiagnosticLogsPluginServerInitCallback() {}
#endif // #ifdef EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT
