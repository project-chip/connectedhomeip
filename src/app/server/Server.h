/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/server/AppDelegate.h>
#include <inet/InetConfig.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/PASESession.h>
#include <transport/FabricTable.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/BLE.h>
#include <transport/raw/UDP.h>

struct ServerConfigParams
{
    uint16_t securedServicePort   = CHIP_PORT;
    uint16_t unsecuredServicePort = CHIP_UDC_PORT;
};

constexpr size_t kMaxBlePendingPackets = 1;

using DemoTransportMgr = chip::TransportMgr<chip::Transport::UDP
#if INET_CONFIG_ENABLE_IPV4
                                            ,
                                            chip::Transport::UDP
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                            ,
                                            chip::Transport::BLE<kMaxBlePendingPackets>
#endif
                                            >;
/**
 * Currently, this method must be called BEFORE InitServer.
 * In the future, it would be nice to be able to call it
 * at any time but that requires handling for changes to every
 * field on ServerConfigParams (restarting port listener, etc).
 *
 */
void SetServerConfig(ServerConfigParams params);

/**
 * Initialize DataModelHandler and start CHIP datamodel server, the server
 * assumes the platform's networking has been setup already.
 *
 * @param [in] delegate   An optional AppDelegate
 */
void InitServer(AppDelegate * delegate = nullptr);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

CHIP_ERROR AddTestCommissioning();

chip::Transport::FabricTable & GetGlobalFabricTable();

namespace chip {

enum class ResetFabrics
{
    kYes,
    kNo,
};

enum class PairingWindowAdvertisement
{
    kBle,
    kMdns,
};

} // namespace chip

constexpr uint16_t kNoCommissioningTimeout = UINT16_MAX;

/**
 * Open the pairing window using default configured parameters.
 */
CHIP_ERROR
OpenBasicCommissioningWindow(chip::ResetFabrics resetFabrics, uint16_t commissioningTimeoutSeconds = kNoCommissioningTimeout,
                             chip::PairingWindowAdvertisement advertisementMode = chip::PairingWindowAdvertisement::kBle);

CHIP_ERROR OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                           chip::PASEVerifier & verifier, uint32_t iterations, chip::ByteSpan salt,
                                           uint16_t passcodeID);

void ClosePairingWindow();

bool IsPairingWindowOpen();
