/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <lib/support/BytesCircularBuffer.h>
#include <lib/support/ScopedBuffer.h>

#include <array>

// We store our times as millisecond times, to save space.
using TimeInBufferType = chip::System::Clock::Milliseconds32::rep;

CHIP_ERROR DiagnosticLogsCommandHandler::PushLog(const chip::ByteSpan & payload)
{
    auto now = std::chrono::duration_cast<chip::System::Clock::Milliseconds32>(chip::Server::GetInstance().TimeSinceInit());
    TimeInBufferType timeMs = now.count();
    chip::ByteSpan payloadTime(reinterpret_cast<uint8_t *>(&timeMs), sizeof(timeMs));
    return mBuffer.Push(payloadTime, payload);
}

void DiagnosticLogsCommandHandler::InvokeCommand(HandlerContext & handlerContext)
{
    HandleCommand<chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsRequest::DecodableType>(
        handlerContext, [&](auto & _u, auto & payload) {
            if (payload.requestedProtocol == chip::app::Clusters::DiagnosticLogs::LogsTransferProtocol::kUnknownEnumValue)
            {
                handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                         chip::Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }

            switch (payload.intent)
            {
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kEndUserSupport: {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                if (mBuffer.IsEmpty())
                {
                    response.status = chip::app::Clusters::DiagnosticLogs::LogsStatus::kNoLogs;
                    handlerContext.mCommandHandler.AddResponse(handlerContext.mRequestPath, response);
                    break;
                }

                size_t logSize = mBuffer.GetFrontSize();
                TimeInBufferType timeFromBuffer;
                VerifyOrDie(logSize > sizeof(timeFromBuffer));

                chip::Platform::ScopedMemoryBuffer<uint8_t> buf;
                if (!buf.Calloc(logSize))
                {
                    response.status = chip::app::Clusters::DiagnosticLogs::LogsStatus::kBusy;
                    handlerContext.mCommandHandler.AddResponse(handlerContext.mRequestPath, response);
                    break;
                }

                // The entry is | time (4 bytes) | content (var size) |
                chip::MutableByteSpan entry(buf.Get(), logSize);
                CHIP_ERROR err = mBuffer.ReadFront(entry);
                VerifyOrDie(err == CHIP_NO_ERROR);
                memcpy(&timeFromBuffer, buf.Get(), sizeof(timeFromBuffer));

                auto timestamp = chip::System::Clock::Milliseconds32(timeFromBuffer);

                response.status     = chip::app::Clusters::DiagnosticLogs::LogsStatus::kSuccess;
                response.logContent = chip::ByteSpan(buf.Get() + sizeof(timeFromBuffer), logSize - sizeof(timeFromBuffer));
                response.timeSinceBoot.SetValue(chip::System::Clock::Microseconds64(timestamp).count());
                handlerContext.mCommandHandler.AddResponse(handlerContext.mRequestPath, response);
            }
            break;
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kNetworkDiag: {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                response.status = chip::app::Clusters::DiagnosticLogs::LogsStatus::kNoLogs;
                handlerContext.mCommandHandler.AddResponse(handlerContext.mRequestPath, response);
            }
            break;
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kCrashLogs: {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                response.status = chip::app::Clusters::DiagnosticLogs::LogsStatus::kNoLogs;
                handlerContext.mCommandHandler.AddResponse(handlerContext.mRequestPath, response);
            }
            break;
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kUnknownEnumValue: {
                handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                         chip::Protocols::InteractionModel::Status::InvalidCommand);
                break;
            }
            }
        });
}

bool emberAfDiagnosticLogsClusterRetrieveLogsRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsRequest::DecodableType & commandData)
{
    // TODO: Shouldn't the default "no-op" impl return some sort of error?
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterDiagnosticLogsPluginServerInitCallback() {}
