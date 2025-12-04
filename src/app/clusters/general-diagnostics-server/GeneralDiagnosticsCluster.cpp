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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/DeviceLoadStatusProviderDelegate.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <clusters/GeneralDiagnostics/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

namespace {

// Max decodable count allowed is 2048.
constexpr uint16_t kMaxPayloadTestRequestCount = 2048;

bool IsTestEventTriggerEnabled()
{
    auto * triggerDelegate = Server::GetInstance().GetTestEventTriggerDelegate();
    if (triggerDelegate == nullptr)
    {
        return false;
    }
    uint8_t zeroByteSpanData[TestEventTriggerDelegate::kEnableKeyLength] = { 0 };
    return !triggerDelegate->DoesEnableKeyMatch(ByteSpan(zeroByteSpanData));
}

bool IsByteSpanAllZeros(const ByteSpan & byteSpan)
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

TestEventTriggerDelegate * GetTriggerDelegateOnMatchingKey(ByteSpan enableKey)
{
    if (enableKey.size() != TestEventTriggerDelegate::kEnableKeyLength)
    {
        return nullptr;
    }

    if (IsByteSpanAllZeros(enableKey))
    {
        return nullptr;
    }

    auto * triggerDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (triggerDelegate == nullptr || !triggerDelegate->DoesEnableKeyMatch(enableKey))
    {
        return nullptr;
    }

    return triggerDelegate;
}

template <typename T>
CHIP_ERROR EncodeValue(T value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
{
    if (readError == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        value = {};
    }
    else if (readError != CHIP_NO_ERROR)
    {
        return readError;
    }
    return encoder.Encode(value);
}

template <typename T>
CHIP_ERROR EncodeListOfValues(const T & valueList, CHIP_ERROR readError, AttributeValueEncoder & aEncoder)
{
    if (readError == CHIP_NO_ERROR)
    {
        readError = aEncoder.EncodeList([&valueList](const auto & encoder) -> CHIP_ERROR {
            for (const auto & value : valueList)
            {
                ReturnErrorOnFailure(encoder.Encode(value));
            }

            return CHIP_NO_ERROR;
        });
    }
    else
    {
        readError = aEncoder.EncodeEmptyList();
    }

    return readError;
}

DataModel::ActionReturnStatus HandleTestEventTrigger(const Commands::TestEventTrigger::DecodableType & commandData)
{
    auto * triggerDelegate = GetTriggerDelegateOnMatchingKey(commandData.enableKey);
    if (triggerDelegate == nullptr)
    {
        return Status::ConstraintError;
    }

    CHIP_ERROR handleEventTriggerResult = triggerDelegate->HandleEventTriggers(commandData.eventTrigger);
    return (handleEventTriggerResult != CHIP_NO_ERROR) ? Status::InvalidCommand : Status::Success;
}

std::optional<DataModel::ActionReturnStatus> HandleTimeSnapshot(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                                const Commands::TimeSnapshot::DecodableType & commandData)
{
    ChipLogError(Zcl, "Received TimeSnapshot command!");

    Commands::TimeSnapshotResponse::Type response;

    System::Clock::Milliseconds64 system_time_ms =
        std::chrono::duration_cast<System::Clock::Milliseconds64>(Server::GetInstance().TimeSinceInit());

    response.systemTimeMs = static_cast<uint64_t>(system_time_ms.count());
    handler.AddResponse(commandPath, response);
    return std::nullopt;
}

/*
 This builds upon the HandleTimeSnapshot function used by the default general diagnostic cluster class, but also
 considers real posix time. This is a separate function called in a separate class as there are some cases
 (that can be determined statically) where we don't want to record posix time and pay the codesize cost for it.
 */
std::optional<DataModel::ActionReturnStatus>
HandleTimeSnapshotWithPosixTime(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                const Commands::TimeSnapshot::DecodableType & commandData)
{
    ChipLogError(Zcl, "Received TimeSnapshot command!");

    Commands::TimeSnapshotResponse::Type response;

    System::Clock::Milliseconds64 posix_time_ms{ 0 };

    CHIP_ERROR posix_time_err = System::SystemClock().GetClock_RealTimeMS(posix_time_ms);
    if (posix_time_err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get POSIX real time: %" CHIP_ERROR_FORMAT, posix_time_err.Format());
        posix_time_ms = System::Clock::Milliseconds64{ 0 };
    }

    System::Clock::Milliseconds64 system_time_ms =
        std::chrono::duration_cast<System::Clock::Milliseconds64>(Server::GetInstance().TimeSinceInit());

    response.systemTimeMs = static_cast<uint64_t>(system_time_ms.count());
    if (posix_time_ms.count() != 0)
    {
        response.posixTimeMs.SetNonNull(posix_time_ms.count());
    }
    handler.AddResponse(commandPath, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
HandlePayloadTestRequest(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                         const Commands::PayloadTestRequest::DecodableType & commandData)
{
    if (commandData.count > kMaxPayloadTestRequestCount)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    // Ensure Test Event triggers are enabled and key matches.
    auto * triggerDelegate = GetTriggerDelegateOnMatchingKey(commandData.enableKey);
    if (triggerDelegate == nullptr)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    Commands::PayloadTestResponse::Type response;
    Platform::ScopedMemoryBufferWithSize<uint8_t> payload;
    if (!payload.Calloc(commandData.count))
    {
        return Protocols::InteractionModel::Status::ResourceExhausted;
    }

    memset(payload.Get(), commandData.value, payload.AllocatedSize());
    response.payload = ByteSpan{ payload.Get(), payload.AllocatedSize() };

    if (handler.AddResponseData(commandPath, response) != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::ResourceExhausted;
    }
    return std::nullopt;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR GeneralDiagnosticsCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Calling OnDeviceReboot here to maintain the event generation of the old implemenation of the
    // server init callback. We consider startup to be a boot event here.
    GeneralDiagnostics::BootReasonEnum bootReason;
    if (GetDiagnosticDataProvider().GetBootReason(bootReason) == CHIP_NO_ERROR)
    {
        OnDeviceReboot(bootReason);
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus GeneralDiagnosticsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case GeneralDiagnostics::Attributes::NetworkInterfaces::Id:
        return ReadNetworkInterfaces(encoder);
    case GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id: {
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> valueList;
        CHIP_ERROR err = GetActiveHardwareFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case GeneralDiagnostics::Attributes::ActiveRadioFaults::Id: {
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> valueList;
        CHIP_ERROR err = GetActiveRadioFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id: {
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> valueList;
        CHIP_ERROR err = GetActiveNetworkFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case GeneralDiagnostics::Attributes::RebootCount::Id: {
        uint16_t value;
        CHIP_ERROR err = GetRebootCount(value);
        return EncodeValue(value, err, encoder);
    }
    case GeneralDiagnostics::Attributes::UpTime::Id: {
        System::Clock::Seconds64 system_time_seconds =
            std::chrono::duration_cast<System::Clock::Seconds64>(Server::GetInstance().TimeSinceInit());
        return encoder.Encode(static_cast<uint64_t>(system_time_seconds.count()));
    }
    case GeneralDiagnostics::Attributes::TotalOperationalHours::Id: {
        uint32_t value;
        CHIP_ERROR err = GetTotalOperationalHours(value);
        return EncodeValue(value, err, encoder);
    }
    case GeneralDiagnostics::Attributes::BootReason::Id: {
        GeneralDiagnostics::BootReasonEnum value;
        CHIP_ERROR err = GetBootReason(value);
        return EncodeValue(value, err, encoder);
    }
    case GeneralDiagnostics::Attributes::TestEventTriggersEnabled::Id: {
        bool isTestEventTriggersEnabled = IsTestEventTriggerEnabled();
        return encoder.Encode(isTestEventTriggersEnabled);
    }
    case GeneralDiagnostics::Attributes::DeviceLoadStatus::Id: {
        static_assert(CHIP_IM_MAX_NUM_SUBSCRIPTIONS <= UINT16_MAX,
                      "The maximum number of IM subscriptions is larger than expected (should fit within a 16 bit unsigned int)");
        const DeviceLoadStatusProviderDelegate::MessageStats messageStatistics = mDeviceLoadStatusProvider->GetMessageStats();

        GeneralDiagnostics::Structs::DeviceLoadStruct::Type load = {
            .currentSubscriptions = mDeviceLoadStatusProvider->GetNumCurrentSubscriptions(),
            .currentSubscriptionsForFabric         = mDeviceLoadStatusProvider->GetNumCurrentSubscriptionsForFabric(encoder.AccessingFabricIndex()),
            .totalSubscriptionsEstablished         = mDeviceLoadStatusProvider->GetNumTotalSubscriptions(),
            .totalInteractionModelMessagesSent     = messageStatistics.interactionModelMessagesSent,
            .totalInteractionModelMessagesReceived = messageStatistics.interactionModelMessagesReceived,
        };
        return encoder.Encode(load);
    }
        // Note: Attribute ID 0x0009 was removed (#30002).

    case GeneralDiagnostics::Attributes::FeatureMap::Id: {

#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1
        mFeatureFlags.Set(Clusters::GeneralDiagnostics::Feature::kDataModelTest);
#endif // CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1

        return encoder.Encode(mFeatureFlags);
    }
    case GeneralDiagnostics::Attributes::ClusterRevision::Id:
        return encoder.Encode(GeneralDiagnostics::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> GeneralDiagnosticsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case GeneralDiagnostics::Commands::TestEventTrigger::Id: {
        GeneralDiagnostics::Commands::TestEventTrigger::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleTestEventTrigger(request_data);
    }
    case GeneralDiagnostics::Commands::TimeSnapshot::Id: {
        GeneralDiagnostics::Commands::TimeSnapshot::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleTimeSnapshot(*handler, request.path, request_data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR GeneralDiagnosticsCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    static constexpr DataModel::AttributeEntry optionalAttributeEntries[] = {
        GeneralDiagnostics::Attributes::TotalOperationalHours::kMetadataEntry,
        GeneralDiagnostics::Attributes::BootReason::kMetadataEntry,
        GeneralDiagnostics::Attributes::ActiveHardwareFaults::kMetadataEntry,
        GeneralDiagnostics::Attributes::ActiveRadioFaults::kMetadataEntry,
        GeneralDiagnostics::Attributes::ActiveNetworkFaults::kMetadataEntry,
        GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
        GeneralDiagnostics::Attributes::DeviceLoadStatus::kMetadataEntry,
    };

    if (mFeatureFlags.Has(GeneralDiagnostics::Feature::kDeviceLoad))
    {
        mOptionalAttributeSet.Set<GeneralDiagnostics::Attributes::DeviceLoadStatus::Id>();
    }

    return listBuilder.Append(Span(GeneralDiagnostics::Attributes::kMandatoryMetadata), Span(optionalAttributeEntries),
                              mOptionalAttributeSet);
}

CHIP_ERROR GeneralDiagnosticsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Commands::TestEventTrigger::kMetadataEntry,
        Commands::TimeSnapshot::kMetadataEntry,
#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1
        Commands::PayloadTestRequest::kMetadataEntry,
#endif
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR GeneralDiagnosticsCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kAcceptedCommands[] = {
        GeneralDiagnostics::Commands::TimeSnapshotResponse::Id,
#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1
        GeneralDiagnostics::Commands::PayloadTestResponse::Id,
#endif
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

void GeneralDiagnosticsCluster::OnDeviceReboot(BootReasonEnum bootReason)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(GeneralDiagnostics::Attributes::BootReason::Id);

    GeneralDiagnostics::Events::BootReason::Type event{ bootReason };

    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnHardwareFaultsDetect(const GeneralFaults<kMaxHardwareFaults> & previous,
                                                       const GeneralFaults<kMaxHardwareFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id);

    // Record HardwareFault event
    DataModel::List<const GeneralDiagnostics::HardwareFaultEnum> currentList(
        reinterpret_cast<const GeneralDiagnostics::HardwareFaultEnum *>(current.data()), current.size());
    DataModel::List<const GeneralDiagnostics::HardwareFaultEnum> previousList(
        reinterpret_cast<const GeneralDiagnostics::HardwareFaultEnum *>(previous.data()), previous.size());
    GeneralDiagnostics::Events::HardwareFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnRadioFaultsDetect(const GeneralFaults<kMaxRadioFaults> & previous,
                                                    const GeneralFaults<kMaxRadioFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveRadioFaults::Id);

    // Record RadioFault event
    DataModel::List<const GeneralDiagnostics::RadioFaultEnum> currentList(
        reinterpret_cast<const GeneralDiagnostics::RadioFaultEnum *>(current.data()), current.size());
    DataModel::List<const GeneralDiagnostics::RadioFaultEnum> previousList(
        reinterpret_cast<const GeneralDiagnostics::RadioFaultEnum *>(previous.data()), previous.size());
    GeneralDiagnostics::Events::RadioFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnNetworkFaultsDetect(const GeneralFaults<kMaxNetworkFaults> & previous,
                                                      const GeneralFaults<kMaxNetworkFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id);

    // Record NetworkFault event
    DataModel::List<const GeneralDiagnostics::NetworkFaultEnum> currentList(
        reinterpret_cast<const GeneralDiagnostics::NetworkFaultEnum *>(current.data()), current.size());
    DataModel::List<const GeneralDiagnostics::NetworkFaultEnum> previousList(
        reinterpret_cast<const GeneralDiagnostics::NetworkFaultEnum *>(previous.data()), previous.size());
    GeneralDiagnostics::Events::NetworkFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

CHIP_ERROR GeneralDiagnosticsCluster::ReadNetworkInterfaces(AttributeValueEncoder & aEncoder)
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

std::optional<DataModel::ActionReturnStatus>
GeneralDiagnosticsClusterFullConfigurable::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                         CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case GeneralDiagnostics::Commands::TestEventTrigger::Id: {
        GeneralDiagnostics::Commands::TestEventTrigger::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return HandleTestEventTrigger(request_data);
    }
    case GeneralDiagnostics::Commands::TimeSnapshot::Id: {
        GeneralDiagnostics::Commands::TimeSnapshot::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        if (mFunctionConfig.enablePosixTime)
        {
            return HandleTimeSnapshotWithPosixTime(*handler, request.path, request_data);
        }
        return HandleTimeSnapshot(*handler, request.path, request_data);
    }
    case GeneralDiagnostics::Commands::PayloadTestRequest::Id: {
        if (mFunctionConfig.enablePayloadSnapshot)
        {
            GeneralDiagnostics::Commands::PayloadTestRequest::DecodableType request_data;
            ReturnErrorOnFailure(request_data.Decode(input_arguments));
            return HandlePayloadTestRequest(*handler, request.path, request_data);
        }
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
