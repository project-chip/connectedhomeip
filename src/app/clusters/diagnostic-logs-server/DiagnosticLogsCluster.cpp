/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "DiagnosticLogsCluster.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <ctime>
#include <protocols/interaction_model/StatusCode.h>

#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include "BDXDiagnosticLogsProvider.h"

#include <clusters/DiagnosticLogs/Ids.h>
#include <clusters/DiagnosticLogs/Metadata.h>

using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;

using chip::bdx::DiagnosticLogs::kMaxFileDesignatorLen;
using chip::bdx::DiagnosticLogs::kMaxLogContentSize;

namespace chip {
namespace app {
namespace Clusters {
namespace {

Commands::RetrieveLogsResponse::Type Failure(StatusEnum status)
{
    Commands::RetrieveLogsResponse::Type response;
    response.status = status;
    return response;
}

Commands::RetrieveLogsResponse::Type Success(StatusEnum status, MutableByteSpan & logContent, const Optional<uint64_t> & timeStamp,
                                             const Optional<uint64_t> & timeSinceBoot)
{
    Commands::RetrieveLogsResponse::Type response;
    response.status        = status;
    response.logContent    = ByteSpan(logContent);
    response.UTCTimeStamp  = timeStamp;
    response.timeSinceBoot = timeSinceBoot;
    return response;
}

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
BDXDiagnosticLogsProvider gBDXDiagnosticLogsProvider;
#endif // CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

// Maintains variables & status for handling a log request payload
struct LogRequestHandler
{
    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    MutableByteSpan logContent;
    Optional<uint64_t> timeStamp;
    Optional<uint64_t> timeSinceBoot;

    LogRequestHandler()
    {
        if (buffer.Alloc(kMaxLogContentSize))
        {
            logContent = MutableByteSpan(buffer.Get(), kMaxLogContentSize);
        }
    }

    /// Returns StatusEnum::kSuccess on success and logContent/timestamp/timeSinceBoot will be populated.
    /// returns an error code on failure
    StatusEnum Process(DiagnosticLogs::DiagnosticLogsProviderDelegate * delegate, IntentEnum intent)
    {
        VerifyOrReturnError(delegate != nullptr, StatusEnum::kNoLogs);
        VerifyOrReturnError(!logContent.empty(), StatusEnum::kDenied);
        VerifyOrReturnError(delegate->GetSizeForIntent(intent) != 0, StatusEnum::kNoLogs);

        CHIP_ERROR err = delegate->GetLogForIntent(intent, logContent, timeStamp, timeSinceBoot);

        VerifyOrReturnError(err != CHIP_ERROR_NOT_FOUND, StatusEnum::kNoLogs);
        VerifyOrReturnError(err == CHIP_NO_ERROR, StatusEnum::kDenied);

        return StatusEnum::kSuccess;
    }
};

} // namespace

void DiagnosticLogsCluster::HandleLogRequestForResponsePayload(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                               IntentEnum intent, StatusEnum statusOnSuccess)
{
    LogRequestHandler handler;

    StatusEnum status = handler.Process(mDelegate, intent);
    if (status != StatusEnum::kSuccess)
    {
        commandObj->AddResponse(path, Failure(status));
    }
    else
    {
        commandObj->AddResponse(path, Success(statusOnSuccess, handler.logContent, handler.timeStamp, handler.timeSinceBoot));
    }
}

std::optional<DataModel::ActionReturnStatus>
DiagnosticLogsCluster::HandleLogRequestForBdx(CommandHandler * commandObj, const ConcreteCommandPath & path, IntentEnum intent,
                                              Optional<CharSpan> transferFileDesignator)
{
    // If the RequestedProtocol is set to BDX and there is no TransferFileDesignator the command SHALL fail with a Status Code of
    // INVALID_COMMAND.
    VerifyOrReturnError(transferFileDesignator.HasValue(), Status::InvalidCommand);

    VerifyOrReturnError(transferFileDesignator.Value().size() <= kMaxFileDesignatorLen, Status::ConstraintError);
    // If there is no delegate, there is no mechanism to read the logs. Assume those are empty and return NoLogs
    if (mDelegate == nullptr)
    {
        commandObj->AddResponse(path, Failure(StatusEnum::kNoLogs));
        return std::nullopt;
    }

    auto size = mDelegate->GetSizeForIntent(intent);
    // In the case where the size is 0 sets the Status field of the RetrieveLogsResponse to NoLogs and do not start a BDX session.
    VerifyOrReturnError(size != 0, std::nullopt, HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kNoLogs));

    // In the case where the Node is able to fit the entirety of the requested logs within the LogContent field, the Status field of
    // the RetrieveLogsResponse SHALL be set to Exhausted and a BDX session SHALL NOT be initiated.
    VerifyOrReturnError(size > kMaxLogContentSize, std::nullopt,
                        HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted));
// If the RequestedProtocol is set to BDX and either the Node does not support BDX or it is not possible for the Node
// to establish a BDX session, then the Node SHALL utilize the LogContent field of the RetrieveLogsResponse command
// to transfer as much of the current logs as it can fit within the response, and the Status field of the
// RetrieveLogsResponse SHALL be set to Exhausted.
#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
    if (gBDXDiagnosticLogsProvider.IsBusy())
    {
        commandObj->AddResponse(path, Failure(StatusEnum::kBusy));
        return std::nullopt;
    }

    if (gBDXDiagnosticLogsProvider.InitializeTransfer(commandObj, path, mDelegate, intent, transferFileDesignator.Value()) !=
        CHIP_NO_ERROR)
    {
        commandObj->AddResponse(path, Failure(StatusEnum::kDenied));
        return std::nullopt;
    }
#else
    HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted);
#endif // CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

    return std::nullopt;
}

// Implementation of virtual methods from DefaultServerCluster
DataModel::ActionReturnStatus DiagnosticLogsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id:
        // No features defined for the cluster ...
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(DiagnosticLogs::kRevision);
    }

    return Status::UnsupportedAttribute;
}

std::optional<DataModel::ActionReturnStatus> DiagnosticLogsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    using namespace DiagnosticLogs::Commands;

    switch (request.path.mCommandId)
    {
    case RetrieveLogsRequest::Id: {
        RetrieveLogsRequest::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        auto intent   = commandData.intent;
        auto protocol = commandData.requestedProtocol;
        if (intent == IntentEnum::kUnknownEnumValue || protocol == TransferProtocolEnum::kUnknownEnumValue)
        {
            return Status::InvalidCommand;
        }

        if (protocol == TransferProtocolEnum::kResponsePayload)
        {
            HandleLogRequestForResponsePayload(handler, request.path, commandData.intent);
            return std::nullopt;
        }

        return HandleLogRequestForBdx(handler, request.path, commandData.intent, commandData.transferFileDesignator);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR DiagnosticLogsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::RetrieveLogsRequest::kMetadataEntry };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR DiagnosticLogsCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = { Commands::RetrieveLogsResponse::Id };
    return builder.ReferenceExisting(kGeneratedCommands);
}

} // namespace Clusters
} // namespace app
} // namespace chip
