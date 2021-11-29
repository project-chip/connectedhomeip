/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the Liceunse at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <log-source-common/DiagnosticLogsCommandHandler.h>

CHIP_ERROR DiagnosticLogsCommandHandler::PushLog(const chip::ByteSpan & payload)
{
    chip::System::Clock::Milliseconds32 now = chip::System::SystemClock().GetMonotonicTimestamp();
    uint32_t timeMs                         = now.count();
    chip::ByteSpan payloadTime(reinterpret_cast<uint8_t *>(&timeMs), sizeof(timeMs));
    return mBuffer.Push(payloadTime, payload);
}

void DiagnosticLogsCommandHandler::InvokeCommand(HandlerContext & handlerContext)
{
    HandleCommand<chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsRequest::DecodableType>(
        handlerContext, [&](auto & _u, auto & payload) {
            switch (payload.intent)
            {
#ifdef CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kEndUserSupport:
#else // CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case EmberAfLogsIntent::EMBER_ZCL_LOGS_INTENT_END_USER_SUPPORT:
#endif
            {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                if (mBuffer.IsEmpty())
                {
                    response.status = EmberAfLogsStatus::EMBER_ZCL_LOGS_STATUS_NO_LOGS;
                    handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, response);
                    break;
                }

                size_t logSize = mBuffer.GetFrontSize();
                chip::System::Clock::Milliseconds32::rep timeMs;
                VerifyOrDie(logSize > sizeof(timeMs));

                std::unique_ptr<uint8_t, decltype(&chip::Platform::MemoryFree)> buf(
                    reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(logSize)), &chip::Platform::MemoryFree);
                if (!buf)
                {
                    response.status = EmberAfLogsStatus::EMBER_ZCL_LOGS_STATUS_BUSY;
                    handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, response);
                    break;
                }

                // The entry is | time (4 bytes) | content (var size) |
                chip::MutableByteSpan entry(buf.get(), logSize);
                CHIP_ERROR err = mBuffer.ReadFront(entry);
                VerifyOrDie(err != CHIP_NO_ERROR);
                timeMs = *reinterpret_cast<decltype(timeMs) *>(buf.get());

                response.status    = EmberAfLogsStatus::EMBER_ZCL_LOGS_STATUS_SUCCESS;
                response.content   = chip::ByteSpan(buf.get() + sizeof(timeMs), logSize - sizeof(timeMs));
                response.timeStamp = timeMs;
                handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, response);
            }
            break;
#ifdef CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kNetworkDiag:
#else // CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case EmberAfLogsIntent::EMBER_ZCL_LOGS_INTENT_NETWORK_DIAG:
#endif
            {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                response.status = EmberAfLogsStatus::EMBER_ZCL_LOGS_STATUS_DENIED;
                handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, response);
            }
            break;
#ifdef CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case chip::app::Clusters::DiagnosticLogs::LogsIntent::kCrashLogs:
#else // CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
            case EmberAfLogsIntent::EMBER_ZCL_LOGS_INTENT_CRASH_LOGS:
#endif
            {
                chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::Type response;
                response.status = EmberAfLogsStatus::EMBER_ZCL_LOGS_STATUS_DENIED;
                handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, response);
            }
            break;
            }
        });
}
