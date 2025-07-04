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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-logic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <clusters/GeneralDiagnostics/Metadata.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {

template <typename LOGIC>
class GeneralDiagnosticsCluster : public DefaultServerCluster, private LOGIC
{
public:
    template <typename... Args>
    GeneralDiagnosticsCluster(Args &&... args) :
        DefaultServerCluster({ kRootEndpointId, GeneralDiagnostics::Id }), LOGIC(std::forward<Args>(args)...)
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case GeneralDiagnostics::Attributes::NetworkInterfaces::Id: {
            return LOGIC::ReadNetworkInterfaces(encoder);
        }
        case GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id: {
            DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> valueList;
            CHIP_ERROR err = LOGIC::GetActiveHardwareFaults(valueList);
            return EncodeListOfValues(valueList, err, encoder);
        }
        case GeneralDiagnostics::Attributes::ActiveRadioFaults::Id: {
            DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> valueList;
            CHIP_ERROR err = LOGIC::GetActiveRadioFaults(valueList);
            return EncodeListOfValues(valueList, err, encoder);
        }
        case GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id: {
            DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> valueList;
            CHIP_ERROR err = LOGIC::GetActiveNetworkFaults(valueList);
            return EncodeListOfValues(valueList, err, encoder);
        }
        case GeneralDiagnostics::Attributes::RebootCount::Id: {
            uint16_t value;
            CHIP_ERROR err = LOGIC::GetRebootCount(value);
            return EncodeValue(value, err, encoder);
        }
        case GeneralDiagnostics::Attributes::UpTime::Id: {
            System::Clock::Seconds64 system_time_seconds =
                std::chrono::duration_cast<System::Clock::Seconds64>(chip::Server::GetInstance().TimeSinceInit());
            return encoder.Encode(static_cast<uint64_t>(system_time_seconds.count()));
        }
        case GeneralDiagnostics::Attributes::TotalOperationalHours::Id: {
            uint32_t value;
            CHIP_ERROR err = LOGIC::GetTotalOperationalHours(value);
            return EncodeValue(value, err, encoder);
        }
        case GeneralDiagnostics::Attributes::BootReason::Id: {
            GeneralDiagnostics::BootReasonEnum value;
            CHIP_ERROR err = LOGIC::GetBootReason(value);
            return EncodeValue(value, err, encoder);
        }
        case GeneralDiagnostics::Attributes::TestEventTriggersEnabled::Id: {
            bool isTestEventTriggersEnabled = IsTestEventTriggerEnabled();
            return encoder.Encode(isTestEventTriggersEnabled);
        }
            // Note: Attribute ID 0x0009 was removed (#30002).

        case GeneralDiagnostics::Attributes::FeatureMap::Id: {
            uint32_t features = 0;

#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1
            features |= to_underlying(Clusters::GeneralDiagnostics::Feature::kDataModelTest);
#endif // CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1

            return encoder.Encode(features);
        }

        case GeneralDiagnostics::Attributes::ClusterRevision::Id: {
            return encoder.Encode(GeneralDiagnostics::kRevision);
        }
        default:
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override
    {
        switch (request.path.mCommandId)
        {
        case GeneralDiagnostics::Commands::TestEventTrigger::Id: {
            GeneralDiagnostics::Commands::TestEventTrigger::DecodableType request_data;
            ReturnErrorOnFailure(request_data.Decode(input_arguments));
            return LOGIC::HandleTestEventTrigger(request_data);
        }
        case GeneralDiagnostics::Commands::TimeSnapshot::Id: {
            GeneralDiagnostics::Commands::TimeSnapshot::DecodableType request_data;
            ReturnErrorOnFailure(request_data.Decode(input_arguments));
            return LOGIC::HandleTimeSnapshot(*handler, request.path, request_data);
        }
        case GeneralDiagnostics::Commands::PayloadTestRequest::Id: {
            GeneralDiagnostics::Commands::PayloadTestRequest::DecodableType request_data;
            ReturnErrorOnFailure(request_data.Decode(input_arguments));
            return LOGIC::HandlePayloadTestRequest(*handler, request.path, request_data);
        }
        default:
            return Protocols::InteractionModel::Status::UnsupportedCommand;
        }
    }

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    void OnDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason) {
        VerifyOrReturn(mContext != nullptr);
        NotifyAttributeChanged(GeneralDiagnostics::Attributes::BootReason::Id);

        GeneralDiagnostics::Events::BootReason::Type event{ bootReason };

        (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
    }

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    void OnHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current) {
        VerifyOrReturn(mContext != nullptr);
        NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id);

        // Record HardwareFault event
        DataModel::List<const GeneralDiagnostics::HardwareFaultEnum> currentList(reinterpret_cast<const GeneralDiagnostics::HardwareFaultEnum *>(current.data()),
                                                            current.size());
        DataModel::List<const GeneralDiagnostics::HardwareFaultEnum> previousList(reinterpret_cast<const GeneralDiagnostics::HardwareFaultEnum *>(previous.data()),
                                                            previous.size());
        GeneralDiagnostics::Events::HardwareFaultChange::Type event{ currentList, previousList };

        (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
    }

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    void OnRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                             const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current) {
        VerifyOrReturn(mContext != nullptr);
        NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveRadioFaults::Id);

        // Record RadioFault event
        DataModel::List<const GeneralDiagnostics::RadioFaultEnum> currentList(reinterpret_cast<const GeneralDiagnostics::RadioFaultEnum *>(current.data()), current.size());
        DataModel::List<const GeneralDiagnostics::RadioFaultEnum> previousList(reinterpret_cast<const GeneralDiagnostics::RadioFaultEnum *>(previous.data()), previous.size());
        GeneralDiagnostics::Events::RadioFaultChange::Type event{ currentList, previousList };

        (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
    }

    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    void OnNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current) {
        VerifyOrReturn(mContext != nullptr);
        NotifyAttributeChanged(GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id);

        // Record NetworkFault event
        DataModel::List<const GeneralDiagnostics::NetworkFaultEnum> currentList(reinterpret_cast<const GeneralDiagnostics::NetworkFaultEnum *>(current.data()), current.size());
        DataModel::List<const GeneralDiagnostics::NetworkFaultEnum> previousList(reinterpret_cast<const GeneralDiagnostics::NetworkFaultEnum *>(previous.data()),
                                                            previous.size());
        GeneralDiagnostics::Events::NetworkFaultChange::Type event{ currentList, previousList };

        (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
    }

private:
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
    CHIP_ERROR EncodeListOfValues(T valueList, CHIP_ERROR readError, AttributeValueEncoder & aEncoder)
    {
        if (readError == CHIP_NO_ERROR)
        {
            readError = aEncoder.EncodeList([&valueList](const auto & encoder) -> CHIP_ERROR {
                for (auto value : valueList)
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

    bool IsTestEventTriggerEnabled()
    {
        auto * triggerDelegate = chip::Server::GetInstance().GetTestEventTriggerDelegate();
        if (triggerDelegate == nullptr)
        {
            return false;
        }
        uint8_t zeroByteSpanData[TestEventTriggerDelegate::kEnableKeyLength] = { 0 };
        if (triggerDelegate->DoesEnableKeyMatch(ByteSpan(zeroByteSpanData)))
        {
            return false;
        }
        return true;
    }
};
} // namespace Clusters
} // namespace app
} // namespace chip
