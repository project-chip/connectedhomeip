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

#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/GeneralFaults.h>
#include <app/TestEventTriggerDelegate.h>

using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace Clusters {

struct GeneralDiagnosticsEnabledAttributes
{
    bool enableNetworkInterfaces : 1;
    bool enableRebootCount : 1;
    bool enableUpTime : 1;
    bool enableTotalOperationalHours : 1;
    bool enableBootReason : 1;
    bool enableActiveHardwareFaults : 1;
    bool enableActiveRadioFaults : 1;
    bool enableActiveNetworkFaults : 1;
    bool enableTestEventTriggersEnabled : 1;
};

class GeneralDiagnosticsLogic {
public:
    GeneralDiagnosticsLogic(const GeneralDiagnosticsEnabledAttributes enabledAttributes) : mEnabledAttributes(enabledAttributes){}
    ~GeneralDiagnosticsLogic() = default;

    CHIP_ERROR GetRebootCount(uint16_t & rebotCount) const { return GetDiagnosticDataProvider().GetRebootCount(rebotCount); }
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) const { return GetDiagnosticDataProvider().GetTotalOperationalHours(totalOperationalHours); }
    CHIP_ERROR GetBootReason(BootReasonEnum & bootReason) const { return GetDiagnosticDataProvider().GetBootReason(bootReason); }
    CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<DeviceLayer::kMaxHardwareFaults> & hardwareFaults) const { return GetDiagnosticDataProvider().GetActiveHardwareFaults(hardwareFaults); }
    CHIP_ERROR GetActiveRadioFaults(GeneralFaults<DeviceLayer::kMaxRadioFaults> & radioFaults) const { return GetDiagnosticDataProvider().GetActiveRadioFaults(radioFaults); }
    CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<DeviceLayer::kMaxNetworkFaults> & networkFaults) const { return GetDiagnosticDataProvider().GetActiveNetworkFaults(networkFaults); }
    CHIP_ERROR ReadNetworkInterfaces(AttributeValueEncoder & aEncoder);

    DataModel::ActionReturnStatus HandleTestEventTrigger(const GeneralDiagnostics::Commands::TestEventTrigger::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus> HandleTimeSnapshot(CommandHandler & handler, const ConcreteCommandPath & commandPath, const GeneralDiagnostics::Commands::TimeSnapshot::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus> HandlePayloadTestRequest(CommandHandler & handler, const ConcreteCommandPath & commandPath, const GeneralDiagnostics::Commands::PayloadTestRequest::DecodableType & commandData);

private:
    TestEventTriggerDelegate * GetTriggerDelegateOnMatchingKey(ByteSpan enableKey);
    bool IsByteSpanAllZeros(const ByteSpan & byteSpan);
    const GeneralDiagnosticsEnabledAttributes mEnabledAttributes;

};

}
}
}
