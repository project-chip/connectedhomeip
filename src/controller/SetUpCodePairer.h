/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      Declaration of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/DLLUtil.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_BLE

#include <controller/DeviceDiscoveryDelegate.h>

namespace chip {
namespace Controller {

class DeviceCommissioner;

enum class SetupCodePairerBehaviour : uint8_t
{
    kCommission,
    kPaseOnly,
};
class DLL_EXPORT SetUpCodePairer
{
public:
    SetUpCodePairer(DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}
    virtual ~SetUpCodePairer() {}

    CHIP_ERROR PairDevice(chip::NodeId remoteId, const char * setUpCode,
                          SetupCodePairerBehaviour connectionType = SetupCodePairerBehaviour::kCommission);

    // Called by the DeviceCommissioner to notify that we have discovered a new device.
    void NotifyCommissionableDeviceDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData);

#if CONFIG_NETWORK_LAYER_BLE
    void SetBleLayer(Ble::BleLayer * bleLayer) { mBleLayer = bleLayer; };
#endif // CONFIG_NETWORK_LAYER_BLE

    // Called by the commissioner when PASE establishment fails.
    //
    // May start a new PASE establishment.
    //
    // Will return whether we might in fact have more rendezvous parameters to
    // try (e.g. because we started a new PASE establishment or are waiting on
    // more device discovery).
    //
    // The commissioner can use the return value to decide whether pairing has
    // actually failed or not.
    bool TryNextRendezvousParameters();

private:
    CHIP_ERROR Connect(SetupPayload & paload);
    CHIP_ERROR StartDiscoverOverBle(SetupPayload & payload);
    CHIP_ERROR StopConnectOverBle();
    CHIP_ERROR StartDiscoverOverIP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverIP();
    CHIP_ERROR StartDiscoverOverSoftAP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverSoftAP();

    // Returns whether we have kicked off a new connection attempt.
    bool ConnectToDiscoveredDevice();

    // Not an enum class because we use this for indexing into arrays.
    enum TransportTypes
    {
        kBLETransport = 0,
        kIPTransport,
        kSoftAPTransport,
        kTransportTypeCount,
    };

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
    void OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj);
    void OnBLEDiscoveryError(CHIP_ERROR err);
    /////////// BLEConnectionDelegate Callbacks /////////
    static void OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
#endif // CONFIG_NETWORK_LAYER_BLE

    bool NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData) const;
    Dnssd::DiscoveryFilter currentFilter;

    DeviceCommissioner * mCommissioner = nullptr;
    chip::NodeId mRemoteId;
    uint32_t mSetUpPINCode                   = 0;
    SetupCodePairerBehaviour mConnectionType = SetupCodePairerBehaviour::kCommission;

    // Boolean will be set to true if we currently have an async discovery
    // process happening via the relevant transport.
    bool mWaitingForDiscovery[kTransportTypeCount];

    // HasPeerAddress() for a given transport type will test true if we have
    // discovered an address for that transport and not tried connecting to it
    // yet.  The general discovery/pairing process will terminate once all
    // parameters test false for HasPeerAddress() and all the booleans in
    // mWaitingForDiscovery are false.
    RendezvousParameters mDiscoveredParameters[kTransportTypeCount];

    // mWaitingForConnection is true if we have called either
    // EstablishPASEConnection or PairDevice on mCommissioner and are now just
    // waiting to see whether that works.
    bool mWaitingForConnection = false;
};

} // namespace Controller
} // namespace chip
