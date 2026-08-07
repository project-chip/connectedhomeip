/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <app/CommandHandler.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>
#include <app/data-model-provider/OperationTypes.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

// Wi-Fi PAF transport driver for the Linux commissioning-proxy example. Only
// compiled when CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF is on (the .cpp is gated at GN
// level by chip_device_config_enable_wifipaf). Implements
// CommissioningProxyTransport; the cluster owns all transport-agnostic
// session/scan/message bookkeeping and this driver reports results back through
// its host cluster's subsystem accessors.
//
// A single instance exists per device; the underlying WiFiPAFLayer callbacks are
// C-style singletons, so the driver's state is process-wide.
class CommissioningProxyPafTransport : public CommissioningProxyTransport
{
public:
    CapabilitiesBitmap GetTransportType() const override { return CapabilitiesBitmap::kWiFiPAF; }
    void SetHost(CommissioningProxyCluster * host) override;

    Protocols::InteractionModel::Status Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                                uint16_t discriminator, uint16_t timeout) override;
    Protocols::InteractionModel::Status CancelPendingConnect(FabricIndex fabricIndex) override;
    Protocols::InteractionModel::Status Disconnect(uint16_t sessionId) override;
    CHIP_ERROR SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf) override;
    Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime) override;
    Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands, FabricIndex fabricIndex,
                                                    NodeId nodeId) override;
    Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId) override;
    void OnAllSessionsClosed() override;
    bool IsConnectPending() const override;
    void Shutdown() override;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
