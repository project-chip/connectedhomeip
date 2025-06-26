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
#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-logic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <clusters/GeneralDiagnostics/Metadata.h>

using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace Clusters {

DataModel::ActionReturnStatus GeneralDiagnosticsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    using namespace Attributes;
    switch (request.path.mAttributeId)
    {
    case NetworkInterfaces::Id: {
        return mLogic.ReadNetworkInterfaces(encoder);
    }
    case ActiveHardwareFaults::Id: {
        GeneralFaults<kMaxHardwareFaults> valueList;
        CHIP_ERROR err = mLogic.GetActiveHardwareFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case ActiveRadioFaults::Id: {
        GeneralFaults<kMaxRadioFaults> valueList;
        CHIP_ERROR err = mLogic.GetActiveRadioFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case ActiveNetworkFaults::Id: {
        GeneralFaults<kMaxNetworkFaults> valueList;
        CHIP_ERROR err = mLogic.GetActiveNetworkFaults(valueList);
        return EncodeListOfValues(valueList, err, encoder);
    }
    case RebootCount::Id: {
        uint16_t value;
        CHIP_ERROR err = mLogic.GetRebootCount(value);
        return EncodeValue(value, err, encoder);
    }
    case UpTime::Id: {
        System::Clock::Seconds64 system_time_seconds =
            std::chrono::duration_cast<System::Clock::Seconds64>(chip::Server::GetInstance().TimeSinceInit());
        return encoder.Encode(static_cast<uint64_t>(system_time_seconds.count()));
    }
    case TotalOperationalHours::Id: {
        uint32_t value;
        CHIP_ERROR err = mLogic.GetTotalOperationalHours(value);
        return EncodeValue(value, err, encoder);
    }
    case BootReason::Id: {
        BootReasonEnum value;
        CHIP_ERROR err = mLogic.GetBootReason(value);
        return EncodeValue(value, err, encoder);
    }
    case TestEventTriggersEnabled::Id: {
        bool isTestEventTriggersEnabled = IsTestEventTriggerEnabled();
        return encoder.Encode(isTestEventTriggersEnabled);
    }
        // Note: Attribute ID 0x0009 was removed (#30002).

    case FeatureMap::Id: {
        uint32_t features = 0;

#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1
        features |= to_underlying(Clusters::Feature::kDataModelTest);
#endif // CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 1

        return encoder.Encode(features);
    }

    case ClusterRevision::Id: {
        return encoder.Encode(GeneralDiagnostics::kRevision);
    }
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> GeneralDiagnosticsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      chip::TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case TestEventTrigger::Id: {
        TestEventTrigger::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleTestEventTrigger(request_data);
    }
    case TimeSnapshot::Id: {
        TimeSnapshot::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleTimeSnapshot(*handler, request.path, request_data);
    }
    case PayloadTestRequest::Id: {
        PayloadTestRequest::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandlePayloadTestRequest(*handler, request.path, request_data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

void GeneralDiagnosticsCluster::OnDeviceReboot(BootReasonEnum bootReason)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(BootReason::Id);

    Events::BootReason::Type event{ bootReason };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnHardwareFaultsDetect(const GeneralFaults<kMaxHardwareFaults> & previous,
                                                       const GeneralFaults<kMaxHardwareFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveHardwareFaults::Id);

    // Record HardwareFault event
    DataModel::List<const HardwareFaultEnum> currentList(reinterpret_cast<const HardwareFaultEnum *>(current.data()),
                                                         current.size());
    DataModel::List<const HardwareFaultEnum> previousList(reinterpret_cast<const HardwareFaultEnum *>(previous.data()),
                                                          previous.size());
    Events::HardwareFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnRadioFaultsDetect(const GeneralFaults<kMaxRadioFaults> & previous,
                                                    const GeneralFaults<kMaxRadioFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveRadioFaults::Id);

    // Record RadioFault event
    DataModel::List<const RadioFaultEnum> currentList(reinterpret_cast<const RadioFaultEnum *>(current.data()), current.size());
    DataModel::List<const RadioFaultEnum> previousList(reinterpret_cast<const RadioFaultEnum *>(previous.data()), previous.size());
    Events::RadioFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void GeneralDiagnosticsCluster::OnNetworkFaultsDetect(const GeneralFaults<kMaxNetworkFaults> & previous,
                                                      const GeneralFaults<kMaxNetworkFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveNetworkFaults::Id);

    // Record NetworkFault event
    DataModel::List<const NetworkFaultEnum> currentList(reinterpret_cast<const NetworkFaultEnum *>(current.data()), current.size());
    DataModel::List<const NetworkFaultEnum> previousList(reinterpret_cast<const NetworkFaultEnum *>(previous.data()),
                                                         previous.size());
    Events::NetworkFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

template <typename T>
CHIP_ERROR GeneralDiagnosticsCluster::EncodeValue(T value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
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
CHIP_ERROR GeneralDiagnosticsCluster::EncodeListOfValues(T valueList, CHIP_ERROR readError, AttributeValueEncoder & aEncoder)
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

bool GeneralDiagnosticsCluster::IsTestEventTriggerEnabled()
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

} // namespace Clusters
} // namespace app
} // namespace chip
