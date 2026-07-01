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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/data-model-provider/OperationTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>

#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyCluster;

// WiFi-PAF transport for the Linux commissioning-proxy example.  Only compiled
// when CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF is on (the .cpp is gated at GN level by
// chip_device_config_enable_wifipaf).  Each function mirrors a method on the
// commissioning-proxy delegate; the dispatcher (CommissioningProxyCommandDelegate.cpp)
// calls these after applying transport-agnostic checks.
namespace Paf {

// ProxyConnectRequest path for WiFi-PAF.
chip::Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                  const chip::app::DataModel::InvokeRequest & request, uint16_t discriminator,
                                                  uint16_t timeout, CommissioningProxyCluster * cluster);

// Cancel an in-flight PAF connect started by Paf::Connect.  Sends Failure on
// the original IM exchange.  Returns InvalidInState if nothing is pending.
chip::Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex);

// Disconnect a PAF proxy session and tear down the PAF endpoint.
chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId);

// Forward a Matter packet over PAF.  Returns CHIP_NO_ERROR on success; the
// commissionee reply arrives asynchronously via the receive callback and is
// routed back through the dispatcher.
CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf);

// Foreground ProxyScanRequest path for WiFi-PAF.  Starts an async NAN scan and,
// when it completes, reports its results to the dispatcher's aggregator via
// ProxyDispatcher::ContributeScanResults (the dispatcher owns the command
// handle and sends the combined ProxyScanResponse).  Returns Success once the
// scan is under way, or an error status if it could not be started.
chip::Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime);

// ProxyBackgroundScanStartRequest path for WiFi-PAF.
chip::Protocols::InteractionModel::Status BgScanStart(CapabilitiesBitmap transport, uint16_t timeout, WiFiBandBitmap wiFiBands,
                                                      chip::FabricIndex fabricIndex, chip::NodeId nodeId,
                                                      CommissioningProxyCluster * cluster);

// ProxyBackgroundScanStopRequest path for WiFi-PAF.
chip::Protocols::InteractionModel::Status BgScanStop(CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands,
                                                     chip::FabricIndex fabricIndex, chip::NodeId nodeId);

// Notify the PAF transport that the *generic* sProxySessions emptied so it
// can resume any bg-scan it paused for the connect.
void OnAllSessionsClosed();

// True while a PAF connect is in flight (sent ProxyConnectRequest, waiting on
// success / error / timeout).  Used by the dispatcher's GetActiveSessionCount
// to combine pending connects across transports against MaxSessions.
bool IsConnectPending();

} // namespace Paf
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
