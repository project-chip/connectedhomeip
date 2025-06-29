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
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {

class GeneralDiagnosticsCluster : public DefaultServerCluster
{
public:
    GeneralDiagnosticsCluster(GeneralDiagnosticsEnabledAttributes attributes) :
        DefaultServerCluster({ kRootEndpointId, GeneralDiagnostics::Id }), mLogic(attributes)
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    void OnDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason);

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    void OnHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    void OnRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                             const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    void OnNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current);

private:
    template <typename T>
    CHIP_ERROR EncodeValue(T value, CHIP_ERROR readError, AttributeValueEncoder & encoder);

    template <typename T>
    CHIP_ERROR EncodeListOfValues(T valueList, CHIP_ERROR readError, AttributeValueEncoder & aEncoder);

    bool IsTestEventTriggerEnabled();

protected:
    GeneralDiagnosticsLogic mLogic;
};
} // namespace Clusters
} // namespace app
} // namespace chip
