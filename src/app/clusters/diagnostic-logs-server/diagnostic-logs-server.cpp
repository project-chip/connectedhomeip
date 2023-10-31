/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/ScopedBuffer.h>
#include <messaging/ExchangeContext.h>

#include <fstream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kDiagnosticLogsLogProviderDelegateTableSize =
    EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kDiagnosticLogsLogProviderDelegateTableSize < kEmberInvalidEndpointIndex,
              "DiagnosticLogs: log provider delegate table size error");

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

LogProviderDelegate * gLogProviderDelegateTable[kDiagnosticLogsLogProviderDelegateTableSize] = { nullptr };

LogProviderDelegate * GetLogProviderDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, DiagnosticLogs::Id, EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kDiagnosticLogsLogProviderDelegateTableSize ? nullptr : gLogProviderDelegateTable[ep]);
}

bool isLogProviderDelegateNull(LogProviderDelegate * logProviderDelegate, EndpointId endpoint)
{
    if (logProviderDelegate == nullptr)
    {
        ChipLogProgress(Zcl, "Diagnosticlogs: no log provider delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip

DiagnosticLogsServer DiagnosticLogsServer::sInstance;

void DiagnosticLogsServer::SetDefaultLogProviderDelegate(EndpointId endpoint, LogProviderDelegate * logProviderDelegate)
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
    return (transferFileDesignator.size() > 0 && transferFileDesignator.size() <= kLogFileDesignatorMaxLen);
}

CHIP_ERROR DiagnosticLogsServer::HandleLogRequestForBDXProtocol(Messaging::ExchangeContext * exchangeCtx, EndpointId endpointId,
                                                                IntentEnum intent, CharSpan fileDesignator)
{

    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mIntent = intent;

    auto scopedPeerNodeId = exchangeCtx->GetSessionHandle()->AsSecureSession()->GetPeer();

    LogProviderDelegate * logProviderDelegate = DiagnosticLogs::GetLogProviderDelegate(endpointId);

    VerifyOrReturnError(!(DiagnosticLogs::isLogProviderDelegateNull(logProviderDelegate, endpointId)), CHIP_ERROR_INCORRECT_STATE);

    mDiagnosticLogsBDXTransferHandler =  new DiagnosticLogsBDXTransferHandler();
    CHIP_ERROR error = mDiagnosticLogsBDXTransferHandler->InitializeTransfer(
        exchangeCtx, scopedPeerNodeId.GetFabricIndex(), scopedPeerNodeId.GetNodeId(), logProviderDelegate, intent, fileDesignator);
    return error;
}

void DiagnosticLogsServer::HandleBDXResponse(CHIP_ERROR error)
{
    LogErrorOnFailure(error);

    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();

    if (commandHandle == nullptr)
    {
        ChipLogError(Zcl, "HandleBDXResponse - commandHandler is null");
        return;
    }

    Commands::RetrieveLogsResponse::Type response;
    if (error == CHIP_NO_ERROR)
    {
        response.status = StatusEnum::kSuccess;
        commandHandle->AddResponse(mRequestPath, response);
    }
    else
    {
       SendErrorResponseAndReset(commandHandle, mRequestPath, StatusEnum::kNoLogs);
    }
}

void DiagnosticLogsServer::SetAsyncCommandHandleAndPath(CommandHandler * commandObj, const ConcreteCommandPath & commandPath)
{
    mAsyncCommandHandle = CommandHandler::Handle(commandObj);
    mRequestPath        = commandPath;
}

void DiagnosticLogsServer::SendErrorResponseAndReset(chip::app::CommandHandler * commandHandler, chip::app::ConcreteCommandPath path, StatusEnum status)
{
    Commands::RetrieveLogsResponse::Type response;
    if (commandHandler != nullptr)
    {
        response.status = status;
        commandHandler->AddResponse(path, response);
    }
    //mDiagnosticLogsBDXTransferHandler->Reset();
    //delete(mDiagnosticLogsBDXTransferHandler);
}

#endif

void DiagnosticLogsServer::HandleLogRequestForResponsePayload(CommandHandler * commandHandler, ConcreteCommandPath path,
                                                              IntentEnum intent)
{
    Commands::RetrieveLogsResponse::Type response;
    mIntent = intent;

    EndpointId endpoint                       = path.mEndpointId;
    LogProviderDelegate * logProviderDelegate = GetLogProviderDelegate(endpoint);

    if (isLogProviderDelegateNull(logProviderDelegate, endpoint))
    {
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    Platform::ScopedMemoryBuffer<uint8_t> buffer;

    if (!buffer.Alloc(kLogContentMaxSize))
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

    mutableBuffer = MutableByteSpan(buffer.Get(), kLogContentMaxSize);

    bool isEOF = false;

    // Get the log next chunk
    uint64_t bytesRead = logProviderDelegate->GetNextChunk(mLogSessionHandle, mutableBuffer, isEOF);

    if (bytesRead == 0)
    {
        response.status = StatusEnum::kNoLogs;
        commandHandler->AddResponse(path, response);
        return;
    }

    // log fits. Return in response commandData and call end of log collection
    if (bytesRead > 0)
    {
        if (isEOF)
        {
            response.status = StatusEnum::kSuccess;
        }
        else
        {
            response.status = StatusEnum::kExhausted;
        }
        response.logContent = ByteSpan(mutableBuffer.data(), kLogContentMaxSize);
    }
    else
    {
        response.status = StatusEnum::kNoLogs;
    }
    logProviderDelegate->EndLogCollection(mLogSessionHandle);

    commandHandler->AddResponse(path, response);
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
    else
    {
        Commands::RetrieveLogsResponse::Type response;
        if (!transferFileDesignator.HasValue() ||
            !DiagnosticLogsServer::Instance().HasValidFileDesignator(transferFileDesignator.Value()))
        {
            ChipLogError(Zcl, "HandleRetrieveLogRequest - fileDesignator not valid for BDX protocol");
            response.status = StatusEnum::kNoLogs;
            commandObj->AddResponse(commandPath, response);
            return;
        }

        if (DiagnosticLogsServer::Instance().IsBDXProtocolRequested(protocol))
        {
            CHIP_ERROR err = DiagnosticLogsServer::Instance().HandleLogRequestForBDXProtocol(
                commandObj->GetExchangeContext(), commandPath.mEndpointId, intent, transferFileDesignator.Value());
            if (err != CHIP_NO_ERROR)
            {
                LogErrorOnFailure(err);
                response.status = StatusEnum::kNoLogs;
                commandObj->AddResponse(commandPath, response);
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
