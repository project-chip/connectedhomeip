/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/general-diagnostics-server/general-diagnostics-logic.h>
#include <app/server/Server.h>
#include <lib/core/CHIPError.h>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR GeneralDiagnosticsLogic::ReadNetworkInterfaces(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::NetworkInterface * netifs;

    if (GetDiagnosticDataProvider().GetNetworkInterfaces(&netifs) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&netifs](const auto & encoder) -> CHIP_ERROR {
            for (DeviceLayer::NetworkInterface * ifp = netifs; ifp != nullptr; ifp = ifp->Next)
            {
                ReturnErrorOnFailure(encoder.Encode(*ifp));
            }

            return CHIP_NO_ERROR;
        });

        GetDiagnosticDataProvider().ReleaseNetworkInterfaces(netifs);
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }

    return err;
}

DataModel::ActionReturnStatus
GeneralDiagnosticsLogic::HandleTestEventTrigger(const GeneralDiagnostics::Commands::TestEventTrigger::DecodableType & commandData)
{
    auto * triggerDelegate = GetTriggerDelegateOnMatchingKey(commandData.enableKey);
    if (triggerDelegate == nullptr)
    {
        return chip::Protocols::InteractionModel::Status::ConstraintError;
    }

    return triggerDelegate->HandleEventTriggers(commandData.eventTrigger);
}

std::optional<DataModel::ActionReturnStatus>
GeneralDiagnosticsLogic::HandleTimeSnapshot(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                            const GeneralDiagnostics::Commands::TimeSnapshot::DecodableType & commandData)
{
    ChipLogError(Zcl, "Received TimeSnapshot command!");

    GeneralDiagnostics::Commands::TimeSnapshotResponse::Type response;

    System::Clock::Microseconds64 posix_time_us{ 0 };

    // Only consider real time if time sync cluster is actually enabled. Avoids
    // likelihood of frequently reporting unsynced time.
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
    CHIP_ERROR posix_time_err = System::SystemClock().GetClock_RealTime(posix_time_us);
    if (posix_time_err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get POSIX real time: %" CHIP_ERROR_FORMAT, posix_time_err.Format());
        posix_time_us = System::Clock::Microseconds64{ 0 };
    }
#endif // ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER

    System::Clock::Milliseconds64 system_time_ms =
        std::chrono::duration_cast<System::Clock::Milliseconds64>(Server::GetInstance().TimeSinceInit());

    response.systemTimeMs = static_cast<uint64_t>(system_time_ms.count());
    if (posix_time_us.count() != 0)
    {
        response.posixTimeMs.SetNonNull(
            static_cast<uint64_t>(std::chrono::duration_cast<System::Clock::Milliseconds64>(posix_time_us).count()));
    }
    handler.AddResponse(commandPath, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> GeneralDiagnosticsLogic::HandlePayloadTestRequest(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const GeneralDiagnostics::Commands::PayloadTestRequest::DecodableType & commandData)
{
    // Max allowed is 2048.
    if (commandData.count > 2048)
    {
        return chip::Protocols::InteractionModel::Status::ConstraintError;
    }

    // Ensure Test Event triggers are enabled and key matches.
    auto * triggerDelegate = GetTriggerDelegateOnMatchingKey(commandData.enableKey);
    if (triggerDelegate == nullptr)
    {
        return chip::Protocols::InteractionModel::Status::ConstraintError;
    }

    GeneralDiagnostics::Commands::PayloadTestResponse::Type response;
    Platform::ScopedMemoryBufferWithSize<uint8_t> payload;
    if (!payload.Calloc(commandData.count))
    {
        return chip::Protocols::InteractionModel::Status::ResourceExhausted;
    }

    memset(payload.Get(), commandData.value, payload.AllocatedSize());
    response.payload = ByteSpan{ payload.Get(), payload.AllocatedSize() };

    if (handler.AddResponseData(commandPath, response) != CHIP_NO_ERROR)
    {
        return chip::Protocols::InteractionModel::Status::ResourceExhausted;
    }
    return std::nullopt;
}

TestEventTriggerDelegate * GeneralDiagnosticsLogic::GetTriggerDelegateOnMatchingKey(ByteSpan enableKey)
{
    if (enableKey.size() != TestEventTriggerDelegate::kEnableKeyLength)
    {
        return nullptr;
    }

    if (IsByteSpanAllZeros(enableKey))
    {
        return nullptr;
    }

    auto * triggerDelegate = chip::Server::GetInstance().GetTestEventTriggerDelegate();

    if (triggerDelegate == nullptr || !triggerDelegate->DoesEnableKeyMatch(enableKey))
    {
        return nullptr;
    }

    return triggerDelegate;
}

bool GeneralDiagnosticsLogic::IsByteSpanAllZeros(const ByteSpan & byteSpan)
{
    for (unsigned char it : byteSpan)
    {
        if (it != 0)
        {
            return false;
        }
    }
    return true;
}

} // namespace Clusters
} // namespace app
} // namespace chip
