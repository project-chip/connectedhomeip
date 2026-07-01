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

// BLE central transport for the Linux commissioning-proxy example.  Only
// compiled when CONFIG_NETWORK_LAYER_BLE is on (the .cpp is gated at GN level
// by chip_config_network_layer_ble).  Mirrors the Paf:: API shape so the
// dispatcher in CommissioningProxyCommandDelegate.cpp can call the right
// transport without knowing transport-specific internals.
namespace Ble {

// ProxyConnectRequest path for BLE.  Pauses the proxy app's own BLE peripheral
// advertising while acting as a central; advertising is resumed when the last
// proxy session is closed.
chip::Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                  const chip::app::DataModel::InvokeRequest & request, uint16_t discriminator,
                                                  uint16_t timeout, CommissioningProxyCluster * cluster);

// Cancel an in-flight BLE connect started by Ble::Connect.  Sends Failure on
// the original IM exchange.  Returns InvalidInState if nothing is pending.
chip::Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex);

// Disconnect a BLE proxy session and close its BLEEndPoint.
chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId);

// Forward a Matter packet over the proxy's BLE endpoint to the commissionee.
// The reply arrives asynchronously via OnEndPointMessageReceived and is routed
// back through the dispatcher.
CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf);

// Foreground ProxyScanRequest path for BLE.  Starts an async BLE scan and, when
// the scan window elapses, reports its results to the dispatcher's aggregator
// via ProxyDispatcher::ContributeScanResults (the dispatcher owns the command
// handle and sends the combined ProxyScanResponse).  Returns Success once the
// scan is under way, or an error status if it could not be started.
chip::Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime);

// ProxyBackgroundScanStartRequest / Stop paths for BLE.  Phase 2B-2.
// Note: transport and wiFiBands are accepted for interface symmetry with the
// PAF transport but are unused — BLE scanning has no band concept.
chip::Protocols::InteractionModel::Status BgScanStart(CapabilitiesBitmap transport, uint16_t timeout, WiFiBandBitmap wiFiBands,
                                                      chip::FabricIndex fabricIndex, chip::NodeId nodeId,
                                                      CommissioningProxyCluster * cluster);
chip::Protocols::InteractionModel::Status BgScanStop(CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands,
                                                     chip::FabricIndex fabricIndex, chip::NodeId nodeId);

// Notify the BLE transport that the *generic* sProxySessions emptied.  No-op
// for BLE — kept for symmetry with the PAF transport (see the implementation
// comment for details).
void OnAllSessionsClosed();

// True while a BLE connect is in flight.  Used by the dispatcher's
// GetActiveSessionCount to combine pending connects across transports against
// MaxSessions.
bool IsConnectPending();

// Tear down all BLE background-scan state before the cluster is destroyed
// (endpoint unregistration / test teardown).  Cancels outstanding per-fabric
// lifetime timers, stops the hardware scan if it owns the scanner, and drops the
// transport's reference to the cluster so no later timer dereferences it.
void Shutdown();

} // namespace Ble
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
